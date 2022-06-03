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

#ifndef IDXPARSER_H_
#define IDXPARSER_H_

#include <vector>
#include "visus.h"
#include "visusio/visus_io.h"
#include "GridParser.h"

//! Helper function to compute the GridParser dimensions in the constructor
const uint32_t* effective_dimension(const char* idx_file, uint32_t low[],
                                    uint32_t high[], uint32_t res);

//! Class to parser (pieces of) regular grids stored in IDX-format
template <class DataClass = GenericData<float> >
class IDXParser : public GridParser<DataClass>
{
public:

  //! Repeat of the typedef since most compilers are not smart enough
  typedef typename DataClass::FunctionType FunctionType;

  //! Default constructor
  IDXParser(const char* idx_file, uint32_t low[], uint32_t high[],uint32_t res,
            uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
            bool compact=true, FILE* map_file=NULL);

  //! Destructor
  ~IDXParser();

protected:

  //! A reference to the IDX file in question
  VisusFile* mIDX;

  //! The world box stored separate for convinience
  VisusBox mWorldBox;

  //! The dimension and location of the samples
  VisusBox mQueryBox;

  //! The last plane query
  VisusBox mQueryPlane;

  //! The resolution of this query
  uint32_t mResolution;

  //! The index of the field used as function
  uint32_t mField;

  //! Container classes for the data access
  std::vector<VisusData> mAttributeData;

  /*! Advance the global index of a vertex. This function should be called
  * *after* all local indices have been corrected since they might be used
  * to compute the next index.
  */
 virtual void advanceGlobalIndex() {this->mIndex = ((this->mK+mStartZ)*mGlobalY + this->mJ+mStartY)*mGlobalX + this->mI + mStartX;}

 /*! According to the current local indices determine the global index of the
  * last vertex that may have an edge with mIndex / mId / mLocal
  * @return global index of the last vertex that may have an edge with this one
  */
 virtual GlobalIndexType lastUsed() const;

 //! Read the next plane of data
 virtual int readDataPlane();
};

template <class DataClass>
IDXParser<DataClass>::IDXParser(const char* idx_file, uint32_t low[], uint32_t high[],uint32_t res,
                                uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                                bool compact=true, FILE* map_file=NULL)
  : GridParser(std::vector<FILE*>(1,NULL),
               effective_dimension(idx_file,low,high,res)[0],
               effective_dimension(idx_file,low,high,res)[1],
               effective_dimension(idx_file,low,high,res)[2],
               fdim,adims,compact,map_file), mIDX(VisusOpen((char*)idx_file)), mResolution(res), mField(fdim)
{
  // The first thing we do are some basic sanity checks
  assert (mIDX); // The file must be valid
  assert (mIDX->fields[0].ndtype==1); // We can only deal with a single number
  assert (strcmp(mIDX->fields[0].sdtype,"float32")==0); // For now that better be a float


  // Lets first get the world box
  mWorldBox = VisusGetWorldBox(mIDX,VisusGetMaxH(mIDX));

  // Now create the local box as the intersection of the world box and
  // the one the user has given us
  for (uint8_t i=0;i<3;i++) {
    PGET(mQueryBox.p1,i) = std::max(PGET(WorldBox.p1,i),low[i]);
    PGET(mQueryBox.p2,i) = std::max(PGET(WorldBox.p2,i),high[i]);
  }

  // The first query plane is the lowest plane of this box
  mQueryPlane = mQueryBox;
  PGET(mQueryPlane.p2,2) = PGET(mQueryPlane.p1,2);

  // Now we create the VisusData wrappers to point at the attribute buffers
  mAttributeData.resize(this->mAttributeBuffers.size());
  for (uint16_t i=0;i<this->mAttributeBuffers.size();i++)
    mAttributeData[i] = VisusCreateData(this->mDimX*this->mDimY,this->mAttributeBuffers[i],0,0);
}

template <class DataClass>
void IDXParser<DataClass>::advanceGlobalIndex()
{
  this->mIndex = ((this->mK+mStartZ)*mGlobalY + this->mJ+mStartY)*mGlobalX + this->mI + mStartX;
}

template <class DataClass>
GlobalIndexType IDXParser<DataClass>::lastUsed() const
{

  if (this->mK < this->mDimZ-1) {
    if (this->mJ < this->mDimY-1) {
      if (this->mI < this->mDimX-1)
        return ((mStartZ+this->mK+1)*mGlobalY + mStartY+this->mJ+1)*mGlobalX + mStartX + this->mI + 1;
      else
        return ((mStartZ+this->mK+1)*mGlobalY + mStartY+this->mJ+1)*mGlobalX + mStartX + this->mI;
    }
    else {
      if (this->mI < this->mDimX-1)
        return ((mStartZ+this->mK+1)*mGlobalY + mStartY+this->mJ)*mGlobalX + mStartX + this->mI + 1;
      else
        return ((mStartZ+this->mK+1)*mGlobalY + mStartY+this->mJ)*mGlobalX + mStartX + this->mI;
    }
  }
  else {
    if (this->mJ < this->mDimY-1) {
      if (this->mI < this->mDimX-1)
        return ((mStartZ+this->mK)*mGlobalY + mStartY+this->mJ+1)*mGlobalX + mStartX + this->mI + 1;
      else
        return ((mStartZ+this->mK)*mGlobalY + mStartY+this->mJ+1)*mGlobalX + mStartX + this->mI;
    }
    else {
      if (this->mI < this->mDimX-1)
        return ((mStartZ+this->mK)*mGlobalY + mStartY+this->mJ)*mGlobalX + mStartX + this->mI + 1;
      else
        return ((mStartZ+this->mK)*mGlobalY + mStartY+this->mJ)*mGlobalX + mStartX + this->mI;
    }
  }
}

template <class DataClass>
int IDXParser<DataClass>::readDataPlane()
{
  VisusQuery* query;

  PGET(mQueryPlane.p1,2) = PGET(mQueryBox.p1,2) + this->mK;
  PGET(mQueryPlane.p2,2) = PGET(mQueryBox.p1,2) + this->mK;

  query = VisusCreateQuery(mIDX,mQueryPlane,mResolution,0,mResolution);

  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<this->mAttributeBuffers.size();i++) {
      VisusRead(mIDX,mAccess,query,this->mPersistentAttributes[i],mIDX->time_from,
                this->mAttributeData[i]);
      query->status = VISUS_QUERY_CREATED;
    }
  }
  else {
    VisusRead(mIDX,mAccess,query,mField,mIDX->time_from,this->mAttributeData[0]);
  }

  VisusDestroyQuery(mIDX,query);
}



#endif /* IDXPARSER_H_ */
