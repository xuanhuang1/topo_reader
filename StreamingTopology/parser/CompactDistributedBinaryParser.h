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


#ifndef COMPACTDISTRIBUTEDBINARYPARSER_H
#define COMPACTDISTRIBUTEDBINARYPARSER_H

#include "DistributedBinaryParser.h"

//! Class to parse a binary stream of tokens
template <class DataClass = GenericData<FunctionType> >
class CompactDistributedBinaryParser : public DistributedBinaryParser<DataClass>
{
public:

  static const uint16_t sWriteBufferSize = 256;

  //! Default constructor
  CompactDistributedBinaryParser(FILE* input, FILE* map_file = NULL,uint32_t edim=3, uint32_t fdim=0,
                                 const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~CompactDistributedBinaryParser();

protected:

  //! The index map into compactified index space
  std::map<GlobalIndexType,GlobalIndexType> mIndexMap;

  //! The current compact index
  GlobalIndexType mCompactIndex;

  //! Pointer to write the index map to file
  FILE* mMapFile;

  //! Number of indices written as a chunk
  uint16_t mIndexBufferSize;

  //! The write buffer
  GlobalIndexType* mIndexBuffer;

  //! Position in the index buffer
  uint16_t mIndexPos;

  //! Insert a vertex
  virtual GlobalIndexType insertID(GlobalIndexType id,FunctionType f, uint8_t count);

  //! Map a vertex
  virtual GlobalIndexType mapID(GlobalIndexType id);

  //! Map and erase a vertex
  virtual GlobalIndexType mapErase(GlobalIndexType id);

  //! Indicate that we have read the last input
  virtual void finish();
};

template <class DataClass>
CompactDistributedBinaryParser<DataClass>::CompactDistributedBinaryParser(FILE* input, FILE* map_file,
                                                                          uint32_t edim, uint32_t fdim,
                                                                          const std::vector<uint32_t>& adims) :
     DistributedBinaryParser<DataClass>(input,edim,fdim,adims), mCompactIndex(0), mMapFile(map_file),
     mIndexBufferSize(sWriteBufferSize), mIndexPos(0)
                                                                            
{
  mIndexBuffer = new GlobalIndexType[mIndexBufferSize];
}


template <class DataClass>
CompactDistributedBinaryParser<DataClass>::~CompactDistributedBinaryParser()
{
  delete[] mIndexBuffer;
}


template <class DataClass>
GlobalIndexType CompactDistributedBinaryParser<DataClass>::insertID(GlobalIndexType id,FunctionType f, uint8_t count)
{
  id = DistributedBinaryParser<DataClass>::insertID(id,f,count);

  if (id == GNULL)
    return GNULL;

  mIndexMap[id] = mCompactIndex++;

  if (mMapFile != NULL) {

    if (mIndexPos == mIndexBufferSize) {
      fwrite(mIndexBuffer,sizeof(GlobalIndexType),mIndexBufferSize,mMapFile);
      mIndexPos = 0;
    }

    mIndexBuffer[mIndexPos++] = id;
  }

  return mCompactIndex-1;
}

template <class DataClass>
GlobalIndexType CompactDistributedBinaryParser<DataClass>::mapID(GlobalIndexType id)
{
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return GNULL;
  else
    return mIt->second;
}

template <class DataClass>
GlobalIndexType CompactDistributedBinaryParser<DataClass>::mapErase(GlobalIndexType id)
{
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt != mIndexMap.end()) {
    GlobalIndexType tmp = mIt->second;

    mIndexMap.erase(mIt);

    return tmp;
  }
  else
    return GNULL;
}

template <class DataClass>
void CompactDistributedBinaryParser<DataClass>::finish()
{
  if (mMapFile != NULL)
    fwrite(mIndexBuffer,sizeof(GlobalIndexType),mIndexPos,mMapFile);
}


#endif
