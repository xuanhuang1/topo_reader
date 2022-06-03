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


#ifndef GRIDEDGEPARSER_H
#define GRIDEDGEPARSER_H

#include <stack>

#include "Parser.h"
#include "GenericData.h"

//! Class to parse interleaved grids
template <class DataClass = GenericData<FunctionType> >
class GridEdgeParser : public Parser<DataClass>
{

public:

  //! Repeat of the typedef since most compilers are not smart enough
  typedef typename DataClass::FunctionType FunctionType;

  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param input: file stream of the file to be read
   *  @param dim_x: number of samples on the x-axis
   *  @param dim_y: number of samples on the y-axis
   *  @param dim_z: number of samples on the z-axis
   *  @param edim: number of coordinates per vertex
   *  @param fdim: index of the coordinate that should be the function
   *  @param adims: indices of the coordinates that should be preserved
   */
  GridEdgeParser(FILE* input,  int dim_x=1, int dim_y=1, int dim_z=1, uint32_t edim=3,  
                 uint32_t fdim=0,const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~GridEdgeParser();
  
  //! Read the next token
  virtual FileToken getToken();

protected:

  //! The number of edges to lower vertices
  static const int8_t sEdgeNr = 13;

  //! The table of all backwards pointing edges
  static int8_t sEdgeTable[sEdgeNr][3];

  //! The actual offsets in index space for each edge 
  int32_t mEdgeOffset[sEdgeNr];

  //! Number of samples on the x-axis 
  const int32_t mDimX;

  //! Number of samples on the y-axis 
  const int32_t mDimY;

  //! Number of samples on the z-axis 
  const int32_t mDimZ;

  //! x index of the last vertex processed
  int32_t mI;

  //! y index of the last vertex processed
  int32_t mJ;

  //! z index of the last vertex processed
  int32_t mK;

  //! The current running index
  GlobalIndexType mIndex;
  
  //! The set of edges that need to be added
  std::stack<GlobalIndexType> mEdges;

  //! The set of vertices that have been finished
  std::stack<GlobalIndexType> mProcessed;

  //! The set of periodic edges that need to be added
  std::stack<GlobalIndexType> mPeriodicEdges;

  FunctionType* mBuffer;

  //! Add the necessary edges for the current vertex
  virtual void addEdges();

  //! Add the necessary processed vertices
  virtual void addProcessed();

  virtual bool isPeriodic() const {return false;}

};


template <class DataClass>
int8_t GridEdgeParser<DataClass>::sEdgeTable[GridEdgeParser<DataClass>::sEdgeNr][3] = {

  {-1,-1,-1}, {0,-1,-1}, {1,-1,-1}, 
  
  {-1,0,-1}, {0,0,-1}, {1,0,-1}, 
  
  {-1,1,-1}, {0,1,-1}, {1,1,-1},

  {-1,-1,0}, {0,-1,0}, {1,-1,0},
  
  {-1,0,0}
};


template <class DataClass>
GridEdgeParser<DataClass>::GridEdgeParser(FILE* input, int dim_x, int dim_y, int dim_z, uint32_t edim,
                                          uint32_t fdim, const std::vector<uint32_t>& adims) 
  : Parser<DataClass>(input,edim,fdim,adims), mDimX(dim_x), mDimY(dim_y), mDimZ(dim_z), mI(-1), mJ(0),
    mK(0), mIndex(-1)
{
  mBuffer = new FunctionType[this->mEDim*mDimX*mDimY];
  
  this->mPath.resize(2);

  for (int i=0;i<sEdgeNr;i++) 
    mEdgeOffset[i] = sEdgeTable[i][0] + sEdgeTable[i][1]*mDimX + sEdgeTable[i][2]*mDimX*mDimY;
}

template <class DataClass>
GridEdgeParser<DataClass>::~GridEdgeParser()
{
  delete[] mBuffer;
}

template <class DataClass>
FileToken GridEdgeParser<DataClass>::getToken()
{

  if (!mEdges.empty()) {
    
    this->mPath[1] = mEdges.top();
    mEdges.pop();
    
    return EDGE;
  }
  else if (isPeriodic() && !mPeriodicEdges.empty()) {

    this->mPath[0] = mPeriodicEdges.top();
    mPeriodicEdges.pop();

    this->mPath[1] = mPeriodicEdges.top();
    mPeriodicEdges.pop();

    return EDGE;
  }
  else if (!mProcessed.empty()) {
    this->mFinal = mProcessed.top();
    mProcessed.pop();

    return FINALIZE;
  }
  else {
    
    // If there is no more data left to read
    if ((mK == mDimZ-1) && (mJ == mDimY-1) && (mI == mDimX-1))
      return EMPTY;

    // Keep track of its running index
    mIndex++;
    this->mId = mIndex;

    // All edges will be incident to this vertex
    this->mPath[0] = this->mId;

    mI++;
    if (mI == mDimX) {
      mI = 0;
      mJ++;

      if (mJ == mDimY) {
        
        if (mK % 20 == 0)
          fprintf(stderr,"Last vertex of plane %d\n",mK);

        mJ = 0;
        mK++;
      }
    }

    if ((mI == 0) && (mJ == 0))
      fread(mBuffer,sizeof(FunctionType),this->mEDim*mDimX*mDimY,this->mInput);

    // For protability his assignment has been replaced with a copy constructor
    // call to allow the given DataClass to store more than just the function
    // value if necessary. (ptb Jan 2010)
    //this->mData.f(mBuffer[this->mEDim*(mJ*mDimX + mI) + this->mFDim]);
    
    // Read all the necessary attributes
    this->mData = DataClass(mBuffer + this->mEDim*(mJ*mDimX + mI),this->mFDim);
 
    if (this->mPersistentAttributes.size() > 0) {
      
      for (uint32_t i=0;i<this->mPersistentAttributes.size();i++) 
        this->mAttributeCache[i]->add(this->mId,mBuffer[this->mEDim*(mJ*mDimX + mI) + this->mPersistentAttributes[i]]);
    }
   

    addEdges();

    addProcessed();

    return VERTEX;
  }
}

template <class DataClass>
void GridEdgeParser<DataClass>::addEdges()
{
  for (int i=0;i<sEdgeNr;i++) {
    if ((sEdgeTable[i][0]+mI < 0) || (sEdgeTable[i][0]+mI >= mDimX)
        || (sEdgeTable[i][1]+mJ < 0) || (sEdgeTable[i][1]+mJ >= mDimY)
        || (sEdgeTable[i][2]+mK < 0)) {
      continue;
    }

    mEdges.push(mIndex + mEdgeOffset[i]);
  }
}

template <class DataClass>
void GridEdgeParser<DataClass>::addProcessed()
{
  if (mK > 0) {

    if ((mI > 0) && (mJ > 0))
      mProcessed.push(mIndex + mEdgeOffset[0]);

    if ((mI == mDimX-1) && (mJ > 0))
      mProcessed.push(mIndex + mEdgeOffset[1]);

    if ((mJ == mDimY-1) && (mI > 0))
      mProcessed.push(mIndex + mEdgeOffset[3]);

    if ((mJ == mDimY-1) && (mI == mDimX-1))
      mProcessed.push(mIndex + mEdgeOffset[4]);
  }

  if (mK == mDimZ-1) {

    if ((mI > 0) && (mJ > 0))
      mProcessed.push(mIndex + mEdgeOffset[9]);

    if ((mI == mDimX-1) && (mJ > 0))
      mProcessed.push(mIndex + mEdgeOffset[10]);

    if ((mJ == mDimY-1) && (mI > 0))
      mProcessed.push(mIndex + mEdgeOffset[12]);
      
    if ((mJ == mDimY-1) && (mI == mDimX-1))
      mProcessed.push(mIndex);
  }
}

#endif
