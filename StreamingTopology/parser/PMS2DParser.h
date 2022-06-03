/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.
* Produced at the Lawrence Livermore National Laboratory
* Written by bremer5@llnl.gov
* OCEC-08-107
* All rights reserved.
*
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/

#ifndef PMSPARSER2D_H
#define PMSPARSER2D_H

#include <vector>
#include <stack>
#include "Parser.h"
#include "GenericData.h"

template<class DataClass = GenericData<float> >
class PMS2DParser : public Parser<DataClass>
{
public:

  //! Typedef for easy access o the function type
  typedef typename DataClass::FunctionType FunctionType;

  //! Typedef for an index pair
  typedef std::pair<GlobalIndexType,GlobalIndexType> Pair;

  //! The number of edges to lower vertices
  static const int8_t sEdgeNr = 6;

  //! The table of all backwards pointing edges
  static int8_t sEdgeTable[sEdgeNr][4];

  //! The actual offsets in index space for each edge
  int32_t mEdgeOffset[sEdgeNr][2];

  //! Default constructor
  /*! Default constructor which saves the FILE* and dimensions for later
   * acces
   * @param attributes: File pointers to blocks of floats
   * @param dim_x: Number of vertices in the x dimension
   * @param dim_y: Number of vertices in the y dimension
   * @param adims: indices of the coordinates that should be preserved
   */
  PMS2DParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y,
              uint32_t f_dim, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  virtual ~PMS2DParser();

  virtual FileToken getToken();

protected:

  //! The stack of edges that must still be added always including mI,mJ
  std::stack<Pair> mEdges;

  //! The index of the mid-point that must be written out
  GlobalIndexType mMidPoint;

  //! The vector of persistent attributes storing the blocks of floats
  std::vector<FunctionType*> mAttributeBuffer;

  //! An extra pointer to the buffer storing the function values
  FunctionType* mBuffer;

  //! The number of vertices in the x dimension
  uint32_t mDimX;

  //! The number of vertices in the y dimension
  uint32_t mDimY;

  //! The x-index of the current vertex
  uint32_t mI;

  //! The y-index of the current vertex
  uint32_t mJ;

  //! A vector containing file pointers to files holding the attributes
  std::vector<FILE*> mAttributeFiles;

 //! A vector containing a single plane of values for each attribute
  std::vector<FunctionType*> mAttributeBuffers;

  //! Set the data for the given
  virtual void setData(FunctionType f) {this->mData.f(f);}

  //! Set the data for the midpoint
  virtual void setMidPoint(FunctionType f) {this->mData.f(f);}
};


template <class DataClass>
int8_t PMS2DParser<DataClass>::sEdgeTable[PMS2DParser<DataClass>::sEdgeNr][4] = {

    {0,0,0,-2}, {0,0,-2,0}, // The edges out of the top vertex

    {-2,-2,-1,-1}, {-2,0,-1,-1}, {0,-2,-1,-1}, {0,0,-1,-1} // The edges out of the middle vertex
};



template<class DataClass>
PMS2DParser<DataClass>::PMS2DParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y,
                                    uint32_t f_dim, const std::vector<uint32_t>& adims) :
                                    Parser<DataClass>(attributes[f_dim],1,0,adims), mMidPoint(GNULL),
                                    mDimX(dim_x), mDimY(dim_y),mI(0),mJ(0), mAttributeFiles(attributes)
{
  this->mPath.resize(2);

  // Setup the edge offset
  for (int i=0;i<sEdgeNr;i++) {

    mEdgeOffset[i][0] = sEdgeTable[i][0] + sEdgeTable[i][1]*2*mDimX;
    mEdgeOffset[i][1] = sEdgeTable[i][2] + sEdgeTable[i][3]*2*mDimX;
  }


  // If we are supposed to store some attributes the user is supposed to use one of these as function
  if (!this->mPersistentAttributes.empty())
    mAttributeBuffers.resize(this->mPersistentAttributes.size(),NULL);
  else { // Otherwise we assume there is only one function given which we store as attribute
    mAttributeBuffers.resize(1,NULL);
  }

  for (uint16_t i=0;i<mAttributeBuffers.size();i++)
    mAttributeBuffers[i] = new FunctionType[mDimX*mDimY];

  // To make things easier later on we use a separate buffer for the function
  // used to compute things
  mBuffer = mAttributeBuffers[this->mFDim];

  // Finally read in all the data
  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<mAttributeBuffers.size();i++)
      fread(mAttributeBuffers[i],sizeof(FunctionType),mDimX*mDimY,mAttributeFiles[this->mPersistentAttributes[i]]);
  }
  else {
    fread(mAttributeBuffers[0],sizeof(FunctionType),mDimX*mDimY,mAttributeFiles[0]);
  }
}

template <class DataClass>
PMS2DParser<DataClass>::~PMS2DParser()
{
  for (uint16_t i=0;i<mAttributeBuffers.size();i++)
    delete[]  mAttributeBuffers[i];
}

template <class DataClass>
FileToken PMS2DParser<DataClass>::getToken()
{

  // If we have to write out the mid point it was always written by the
  // vertex above and to the right of him
  if (mMidPoint != GNULL) {

    setMidPoint(0.25*(mBuffer[mI-1 + (mJ-1)*mDimX] + mBuffer[mI + (mJ-1)*mDimX] +
        mBuffer[mI-1 + mJ*mDimX] + mBuffer[mI + mJ*mDimX]));

    this->mId = mMidPoint;
    mMidPoint = GNULL;

    return VERTEX;
  }
  else if (!mEdges.empty()) { // If there are edges left write these

    this->mPath[0] = mEdges.top().first;
    this->mPath[1] = mEdges.top().second;
    mEdges.pop();

    return EDGE;
  }
  else if (mJ >= mDimY) { // If we have processed the last vertex
    return EMPTY;
  }
  else {

    this->mId = 2*mI + 4*mJ*mDimX;

    setData(mBuffer[mJ*mDimX + mI]);

    // store all the necessary attributes
    if (!this->mAttributeCache.empty()) {
      for (uint16_t i=0;i<mAttributeBuffers.size();i++)
        this->mAttributeCache[i]->add(this->mId,mAttributeBuffers[i][mJ*mDimX + mI]);
    }

    // If necessary "create" a mid point
    if ((mI > 0) && (mJ > 0))
      mMidPoint = 2*mI-1 + (2*mJ-1)*2*mDimX;


    // Create all edges
    for (int i=0;i<sEdgeNr;i++) {
      if ((2*mI < (uint8_t)-sEdgeTable[i][0]) || (2*mI < (uint8_t)-sEdgeTable[i][2])
          || (2*mJ < (uint8_t)-sEdgeTable[i][1]) || (2*mJ < (uint8_t)-sEdgeTable[i][3])) {
        continue;
      }

      mEdges.push(Pair(this->mId+mEdgeOffset[i][0],this->mId+mEdgeOffset[i][1]));
    }


    mI++;
    mJ += mI / mDimX;
    mI = mI % mDimX;

    return VERTEX;
  }

  return UNKNOWN;
}


#endif /* PMSPARSER2D_H */
