/***********************************************************************
*
* copyright (c) 2008, Lawrence Livermore National Security, LLC.  
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


#ifndef DISTRIBUTEDBINARYPARSER_H
#define DISTRIBUTEDBINARYPARSER_H

#include <map>
#include <stack>
#include "Parser.h"
#include "GenericData.h"

//! Class to parse a binary stream of tokens
template <class DataClass = GenericData<FunctionType> >
class DistributedBinaryParser : public Parser<DataClass>
{
public:

  typedef typename DataClass::FunctionType FunctionType;

  typedef std::map<GlobalIndexType,int8_t> OpenMap;
  typedef std::map<GlobalIndexType,int8_t>::iterator OpenMapIterator;

  static const uint16_t sReadBufferSize = 1024;

  //! Default constructor
  DistributedBinaryParser(FILE* input, uint32_t edim, uint32_t fdim=0, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~DistributedBinaryParser();

  //! Read the next token
  virtual FileToken getToken();

protected:

  //! The number of bytes we read as a chunck from the input buffer
  uint16_t mBufferSize;

  //! The input buffer
  unsigned char* mBuffer;

  //! The current position in the buffer
  uint16_t mPos; 

  //! The count of the current vertex
  uint8_t mCount;

  //! The map of the currently unfinalized vertices with their
  //! corresponding count of number of outstanding edges
  OpenMap mOpenMap;

  //! The stack of vertices waiting to be finalized
  std::stack<GlobalIndexType> mStack;

  //! Advance the buffer to have at least step many bytes
  void advance(uint32_t step);

  //! Indicate that the given vertex has seen another edge
  void processEdge(GlobalIndexType id);

  //! Return the token type of the next token and advance the buffer
  virtual char tokenType() {return mBuffer[mPos++];}

  //! Insert a vertex
  virtual GlobalIndexType insertID(GlobalIndexType id,FunctionType f, uint8_t count);

  //! Map a vertex
  virtual GlobalIndexType mapID(GlobalIndexType id) {return id;}

  //! Map and erase a vertex
  virtual GlobalIndexType mapErase(GlobalIndexType id) {return id;}

  //! Indicate that we have read the last input
  virtual void finish() {}
};

template <class DataClass>
DistributedBinaryParser<DataClass>::DistributedBinaryParser(FILE* input, uint32_t edim, uint32_t fdim,
                                      const std::vector<uint32_t>& adims) : 
  Parser<DataClass>(input,edim,fdim,adims), mBufferSize(sReadBufferSize), mPos(0)
                                                                            
{
  this->mPath.resize(2);

  mBuffer = new unsigned char[mBufferSize];
  mBufferSize = fread(mBuffer,1,mBufferSize,this->mInput);
}


template <class DataClass>
DistributedBinaryParser<DataClass>::~DistributedBinaryParser()
{
  delete[] mBuffer;
}

template <class DataClass>
FileToken DistributedBinaryParser<DataClass>::getToken()
{
  // This while statement will take care of reading tokens until it finds a
  // valid one. Tokens can be invalid if vertices are not being processed or
  // edges contain such vertices
  while (true) {
    
    // If there are vertices that should be finalized
    if (!mStack.empty()) {
      this->mFinal = mStack.top();
      mStack.pop();

      this->mFinal = mapErase(this->mFinal);

      if (this->mFinal != GNULL)
        return FINALIZE;

      continue;
    }

    // Make sure that the buffer contains enough entries to at least parse one
    // more token
    advance(std::max(2*sizeof(GlobalIndexType),sizeof(GlobalIndexType)+this->mEDim*sizeof(FunctionType))+2);
    
    // If there is nothing left to read we will return the EMPTY token. Note
    // that this guarantees that the while (true) returns since once the file is
    // empty the loop will end
    if (mBufferSize == 0) {

      // We make sure to handle whatever information we have in various buffers
      finish();

      // End return the final token
      return EMPTY;
    }

    switch (tokenType()) {
    case 'v': {

      // Read the id of the vertex
      this->mId = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);

      // Read the count of the vertex
      mCount = *((uint8_t*)(mBuffer + mPos));
      mPos += sizeof(uint8_t);

      // Set the attribute array
      FunctionType* attributes = (FunctionType*)(mBuffer + mPos);
      mPos += this->mEDim*sizeof(FunctionType);

      this->mId = insertID(this->mId,attributes[this->mFDim],mCount);

      // If this vertex should be processed at all 
      if (this->mId != GNULL) {
      
        // For portability this assignment has been replaced with a copy constructor
        //call to allow the given DataClass to store more than just the function
        //value if necessary.
        //this->mData.f(attributes[this->mFDim]);
      
        // Read all the necessary attributes
        this->mData = DataClass(attributes,this->mFDim);
      
      
      
        if (this->mPersistentAttributes.size() > 0) {
          for (uint32_t i=0;i<this->mPersistentAttributes.size();i++)
            this->mAttributeCache[i]->add(this->mId,attributes[this->mPersistentAttributes[i]]);
        }
    

        return VERTEX;
      }
      break;
    }
    case 'e': {

      this->mPath[0] = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);
      this->mPath[1] = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);
 
      // Record the fact that these vertices have seen another edge
      processEdge(this->mPath[0]);
      processEdge(this->mPath[1]);

      // Get into the new index space if necessary
      this->mPath[0] = mapID(this->mPath[0]);
      this->mPath[1] = mapID(this->mPath[1]);

      if ((this->mPath[0] != GNULL) && (this->mPath[1] != GNULL))
        return EDGE;
      break;
    }
    default: {
      sterror(true,"Unkown token \"%c\": Could not parse file.",mBuffer[mPos-1]);
      exit(1);
      return UNKNOWN;
      break;
    }
    }
  }
    
  return UNKNOWN;
}

template <class DataClass>
GlobalIndexType DistributedBinaryParser<DataClass>::insertID(GlobalIndexType id,FunctionType f, uint8_t count)
{
  if ((f < this->mFMin) || (f > this->mFMax))
    return GNULL;

  //if (id == 0)
  //  fprintf(stderr,"insertId  %d %f %d\n",id,f,count);

  // See whether we have found this vertex before
  OpenMapIterator mIt;
  mIt = mOpenMap.find(id);

  // If so
  if (mIt != mOpenMap.end()) {
    if (count != (uint8_t)-1) {// If we now know how many edges to expect
      mIt->second += count; // Add this number to the count
      if (mIt->second == 0) {// If this brought the count to 0
        mStack.push(id); // Mark this vertex as to be finalized
        mOpenMap.erase(mIt); // And erase it from the open set
      }
    }

    return GNULL; // In any case we don't need to process it again
  }
  else if (count == (uint8_t)-1) // Otherwise, if we don't know yet how many edges are coming
    mOpenMap[id] = 0;
  else
    mOpenMap[id] = count;

  return id;
}


template <class DataClass>
void DistributedBinaryParser<DataClass>::advance(uint32_t step)
{
  if (mPos + step >= mBufferSize) {
    
    memcpy(mBuffer,mBuffer+mPos,std::max(mBufferSize-mPos,0));
    
    mBufferSize = fread(mBuffer+std::max(mBufferSize-mPos,0),1,mPos,this->mInput) + std::max(mBufferSize-mPos,0);
    mPos = 0;
  }
}

template <class DataClass>
void DistributedBinaryParser<DataClass>::processEdge(GlobalIndexType id)
{
  OpenMapIterator mIt;
  mIt = mOpenMap.find(id);

  //if (id == 0)
  //  fprintf(stderr,"insertId  %d \n",id);

  if (mIt!=mOpenMap.end()) { // If this vertex is unfinalized (i.e. it has not been filtered)
    mIt->second = mIt->second-1; // Reduce the number of edges we still expect
    if (mIt->second == 0) {// If this was the last edge
      mStack.push(id);
      mOpenMap.erase(mIt);
    }
  }
  else {
    stwarning("Vertex %u not found in set of unfinalized vertices",id);
    exit(0);
  }
}

#endif
