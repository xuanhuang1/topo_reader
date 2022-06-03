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


#ifndef SMBPARSER_H
#define SMBPARSER_H

#include <iostream>
#include <stack>

#include "Parser.h"
#include "GenericData.h"

//! Parser interface for streaming meshes in ascii format
/*! This class implements the parser interface for simplicial meshes
 *  of any dimension stored in streaming format. The accepted format
 *  is as follows.
 * 
 *  v <float> <float> ... <float>  : a vertex with with n coordinates
 *  f <int> <int> .... <int>       : an m-dimensional simplex
 *  x <int>                        : explicit finalization of a vertex
 *
 *  All indices are 1-based following standard obj
 *  conventions. Vertices can be finalized in two ways. Either by
 *  explicit finalization using "x <index>" to finalize the vertex
 *  with the given index; Or by negating the index in a
 *  simplex. Meaning a triangle "f -2 3 -4" indicates that this
 *  triangle is the last triangle that uses vertices 2 and 4. 
 *
 *  The class uses a function called constructData to convert the n
 *  coordinates read for each vertex into an instance of
 *  DataClass. This is a virtual function that can be overwritten by
 *  dervied classes that want to create more sophisticated data
 *  classes.
 */
template <class DataClass = GenericData<float> >
class SMBParser : public Parser<DataClass>
{
public:

  typedef typename DataClass::FunctionType FunctionType;

  //! Default constructor
  /*! The default constructor which set the necessary dimensions.
   *  @param input: Pointer to the input stream
   *  @param emb_dim: Dimension of the embedding space / number of 
   *                  vertex coordinates
   *  @param sim_dim: Dimension+1 of the mesh / number of vertices
   *                  per simplex
   *  @param func_dim: Dimension that should be used as function
   */
  SMBParser(FILE* input, uint32_t edim, uint32_t sdim, uint32_t fdim=0,
            const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~SMBParser();

  //! Read the next token
  FileToken getToken();

  //! Return a pointer to the last read vertex coordinates
  const FunctionType* coord() const {return mCoord;}

protected:

  //! Mesh dimension+1 / number of vertices per simplex
  const uint32_t mSDim;

  //! Buffer of vertices that must be finalized
  std::stack<GlobalIndexType> mFinalize;

  //! Minimal number of bytes left in the buffer before reading more
  const int32_t mMinBufferSize;

  //! Maximal number of bytes read as one chunk
  int32_t mBufferSize;

  //! Buffer to hold each line
  char* mBuffer; 

  //! The current position in the buffer
  int32_t mPos;

  //! The coordinates of the last read vertex
  FunctionType* mCoord;
};
  

template <class DataClass>
SMBParser<DataClass>::SMBParser(FILE* input, uint32_t edim,uint32_t sdim, uint32_t fdim,
                                const std::vector<uint32_t>& adims) :
                                Parser<DataClass>(input,edim,fdim,adims), mSDim(sdim),
                                mMinBufferSize(std::max(edim*sizeof(FunctionType)+1,sdim*sizeof(GlobalIndexType)+1)),
                                mBufferSize(1000*mMinBufferSize),
                                mPos(0)
{
  this->mPath.resize(mSDim);

  // The first index will be 0 but it will use a ++ before it is read
  // So we need to start with a -1. Since the global index type is often
  // unsigned we need the cast
  this->mId = (GlobalIndexType)-1;

  mBuffer = new char[mBufferSize];

  mBufferSize = fread(mBuffer,1,mBufferSize,this->mInput);
}

template <class DataClass>
SMBParser<DataClass>::~SMBParser()
{
  delete mBuffer;
}

template <class DataClass>
FileToken SMBParser<DataClass>::getToken()
{
  // If we have stored finalization information that has not been read
  // output this first
  if (!mFinalize.empty()) {
    this->mFinal = mFinalize.top();
    mFinalize.pop();
    
    return FINALIZE;
  }
  

  // Now we need to make sure that we have enough data in the buffer
  // to parse at least one more vertex
  if (mPos + mMinBufferSize >= mBufferSize) {

    memcpy(mBuffer,mBuffer+mPos,std::max(mBufferSize-mPos,(int32_t)0));

    mBufferSize = fread(mBuffer+std::max(mBufferSize-mPos,(int32_t)0),1,mPos,this->mInput) + std::max(mBufferSize-mPos,(int32_t)0);
    mPos = 0;
  }

  // If there is nothing left to read we return empty
  if (mBufferSize - mPos == 0)
    return EMPTY;

  switch (mBuffer[mPos++]) {
  case 'v': {

    // Make sure that we have enough data left to fill a vertex
    sterror((mBufferSize-mPos) < this->mEDim*sizeof(FunctionType),"Vertex token found without enough coordinates left");

    // Create a convenience pointer to avoid constant casting
    mCoord = (FunctionType*)(mBuffer + mPos);

    //fprintf(stderr,"v %f %f %f %f %f %f \n",coord[0],coord[1],coord[2],coord[3],coord[4],coord[5]);
     // For portability this assignment has been replaced with a copy constructor
    // call to allow the given DataClass to store more than just the function
    // value if necessary. (ptb Jan 2010)
    // this->mData.f(mVertex[this->mFDim]);
    this->mData = DataClass(mCoord,this->mFDim);

    this->mId++;

    if (this->mId % 10000 == 0)
      std::cerr << "Parsing vertex "<<  this->mId << "\n";

    if (this->mPersistentAttributes.size() > 0) {
      
      for (uint32_t k=0;k<this->mPersistentAttributes.size();k++) {
        this->mAttributeCache[k]->add(this->mId,mCoord[this->mPersistentAttributes[k]]);
      }
    }

    mPos += this->mEDim * sizeof(FunctionType);
    return VERTEX;
  }
  case 'f': // For either a Face or an Edge we assume the SDim
  case 'e': // tells us what to do
  {
    // Make sure that we have enough data left to fill a vertex
    sterror((mBufferSize-mPos) < mSDim*sizeof(LocalIndexType),"Path token found without enough indices left");

    // Create a convenience pointer to avoid constant casting
    int32_t* indices = (int32_t*)(mBuffer + mPos);

    for (uint32_t i=0;i<mSDim;i++) {
      if (indices[i] < 0) {
        // This is the original SMB format which we don' t use anymore
        //indices[i] += this->mId + 1;

        indices[i] = -indices[i] - 1;
        mFinalize.push(static_cast<GlobalIndexType>(indices[i]));
      }
      else
        indices[i]--;
      
      this->mPath[i] = indices[i];
    }
    
    mPos += mSDim * sizeof(LocalIndexType);

    if (this->mSDim == 2)
      return EDGE;
    else
      return PATH;
  }
  case 'x': {
    this->mFinal = static_cast<GlobalIndexType>(*((int32_t*)(mBuffer+mPos)));

    return FINALIZE;
  }
  default:
    sterror(true,"Unknown token \"%c\"",mBuffer[mPos-1]);
    return UNKNOWN;
  }

  return UNKNOWN;
}
  

 



#endif
