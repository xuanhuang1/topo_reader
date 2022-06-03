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


#ifndef SORTEDGRIDPARSER_H
#define SORTEDGRIDPARSER_H

#include <cstdio>
#include <algorithm>
#include <vector>

#include "BlockedArray.h"
#include "Parser.h"
#include "GenericData.h"

template <class DataClass = GenericData<float> >
class SortedGridParser : public Parser<DataClass>
{
public:

  SortedGridParser(FILE* input,  int dim_x=1, int dim_y=1, int dim_z=1, FunctionType low=-gMinValue,
                   FunctionType high=gMaxValue, uint32_t edim=1, uint32_t fdim=0, 
                   const std::vector<uint32_t>& adims = std::vector<uint32_t>(), bool descending = true,
                   FILE* map_file = NULL);

  //! Destructor
  virtual ~SortedGridParser();

  //! Read the next token
  virtual FileToken getToken();

protected:

  class IndexSort 
  {
  public:

    IndexSort(const SortedGridParser& parser) : mParser(parser) {}
    virtual ~IndexSort() {}

    virtual bool operator()(const LocalIndexType i, const LocalIndexType j) const = 0;
  protected:
    
    const SortedGridParser& mParser;
  };

  class DescendingSort : public IndexSort
  {
  public:
    DescendingSort(const SortedGridParser& parser) : IndexSort(parser) {}
    virtual ~DescendingSort() {}

    bool operator()(const LocalIndexType i, const LocalIndexType j) const {
      if (this->mParser.mAttributeCache[0]->at(i) > this->mParser.mAttributeCache[0]->at(j))
        return true;
      if ((this->mParser.mAttributeCache[0]->at(i) == this->mParser.mAttributeCache[0]->at(j))
          && (i > j))
        return true;

      return false;
    }
  };

  class AscendingSort : public IndexSort
  {
  public:
    AscendingSort(const SortedGridParser& parser) : IndexSort(parser) {}
    virtual ~AscendingSort() {}

    bool operator()(const LocalIndexType i, const LocalIndexType j) const {
      if (this->mParser.mAttributeCache[0]->at(i) < this->mParser.mAttributeCache[0]->at(j))
        return true;
      if ((this->mParser.mAttributeCache[0]->at(i) == this->mParser.mAttributeCache[0]->at(j))
          && (i < j))
        return true;

      return false;
    }
  };


  //! The number of edges connected to each vertex
  static const uint8_t sEdgeNr = 26;

  //! The table of all edges
  static int8_t sEdgeTable[sEdgeNr][3];

  //! The actual offsets in the index space for each edge
  int32_t mEdgeOffset[sEdgeNr];

  //! Number of samples on the x-axis 
  const uint32_t mDimX;

  //! Number of samples on the y-axis 
  const uint32_t mDimY;

  //! Number of samples on the z-axis 
  const uint32_t mDimZ;

  //! The low threshold below which we should ignore vertices
  const FunctionType mLowThreshold;

  //! The high threshold above which we should ignore vertices
  const FunctionType mHighThreshold;

  //! Flag indicating whether we sort ascending or descending
  const bool mDescending;

  //! Pointer to a sorting function
  IndexSort* mIndexSort;

  //! Array of resorted indices
  std::vector<LocalIndexType> mSorted;
  
  //! Array of re-mapped indices if compactifying
  FlexArray::BlockedArray<GlobalIndexType,LocalIndexType> mIndexMap;

  //! Array of re-mapped indices if compactifying
  FlexArray::BlockedArray<LocalIndexType,LocalIndexType> mLocalIndex;

  //! Pointer to the file containing the index maps if applicable
  FILE* mMapFile;

  //! Local buffer to avoid reading vertices from disk one by one
  FunctionType* mBuffer;

  //! Construct and sort the necessary arrays
  int prepareArrays();
};

template <class DataClass>
int8_t SortedGridParser<DataClass>::sEdgeTable[SortedGridParser<DataClass>::sEdgeNr][3] = {

  {-1,-1,-1}, {-1,-1,0}, {-1,-1,1}, 
  {-1, 0,-1}, {-1, 0,0}, {-1, 0,1}, 
  {-1, 1,-1}, {-1, 1,0}, {-1, 1,1}, 

  {0,-1,-1}, {0,-1,0}, {0,-1,1}, 
  {0, 0,-1},           {0, 0,1}, 
  {0, 1,-1}, {0, 1,0}, {0, 1,1}, 

  {1,-1,-1}, {1,-1,0}, {1,-1,1}, 
  {1, 0,-1}, {1, 0,0}, {1, 0,1}, 
  {1, 1,-1}, {1, 1,0}, {1, 1,1}, 
};


template <class DataClass>
SortedGridParser<DataClass>::SortedGridParser(FILE* input,  int dim_x, int dim_y, int dim_z, FunctionType low,
                                              FunctionType high, uint32_t edim, uint32_t fdim, 
                                              const std::vector<uint32_t>& adims, bool descending,
                                              FILE* map_file) 
  : Parser<DataClass>(input,edim,fdim,adims), mDimX(dim_x), mDimY(dim_y), mDimZ(dim_z),
    mLowThreshold(low), mHighThreshold(high), mDescending(descending), mMapFile(map_file)
{
  mBuffer = new FunctionType[this->mEDim*mDimX*mDimY];
  this->mPath.resize(2);
  
  for (int i=0;i<sEdgeNr;i++) 
    mEdgeOffset[i] = sEdgeTable[i][0] + sEdgeTable[i][1]*mDimX + sEdgeTable[i][2]*mDimX*mDimY;
 
  if (mDescending) 
    mIndexSort = new DescendingSort(*this);
  else
    mIndexSort = new AscendingSort(*this);

  prepareArrays();
}

template <class DataClass>
SortedGridParser<DataClass>::~SortedGridParser()
{
  delete[] mBuffer;
  delete mIndexSort;
}

template <class DataClass>
FileToken SortedGridParser<DataClass>::getToken()
{
  static uint32_t vertex_count = 0;
  static uint8_t edge_count = sEdgeNr;
  
  while (true) {

    // If there is no edge left for the current vertex
    if (edge_count == sEdgeNr) {
      edge_count = 0;
      
      // If we have processed all vertices
      if (vertex_count == mSorted.size())
        return EMPTY;
      else { // Otherwise get the next vertex

        this->mId = mSorted[vertex_count++];
        this->mData = DataClass(this->mAttributeCache[0]->at(this->mId));

        return VERTEX;
      }
    }
    else { // There remains an edge to try out

      GlobalIndexType neighbor;
      int32_t x,y,z;

      // Get the x,y,z indices of the neighbor
      x = sEdgeTable[edge_count][0] + mIndexMap[this->mId] % mDimX;
      y = sEdgeTable[edge_count][1] + (mIndexMap[this->mId] / mDimX) % mDimY;
      z = sEdgeTable[edge_count][2] + (mIndexMap[this->mId] / (mDimY*mDimX)) % mDimZ;

      //fprintf(stderr,"%d %d %d\n",x,y,z);

      // Only if these are within range do we have a valid edge
      if ((x >= 0) && (x < mDimX) &&
          (y >= 0) && (y < mDimY) &&
          (z >= 0) && (z < mDimZ)) {
        
        //fprintf(stderr,"%d %d %d\n",x,y,z);
        //fprintf(stderr,"local id %d   global id %d    global neighbor %d   local neighbor %d\n",this->mId,mIndexMap[this->mId],
        //        mIndexMap[this->mId] + mEdgeOffset[edge_count], mLocalIndex[mIndexMap[this->mId] + mEdgeOffset[edge_count]]);
        // Get the local index of the neighbor
        neighbor = mLocalIndex[mIndexMap[this->mId] + mEdgeOffset[edge_count]];

      // Only use the edge if the neighbor is valid and larger
        if ((neighbor != LNULL) && (*mIndexSort)(neighbor,this->mId)) {
          this->mPath[0] = this->mId;
          this->mPath[1] = neighbor;
        
          edge_count++;
          return EDGE;
        }
      }
      edge_count++;
    }
  }
}


template <class DataClass>
int SortedGridParser<DataClass>::prepareArrays()
{
  uint32_t i,k;
  LocalIndexType local = 0;
  GlobalIndexType global = 0;
  typename DataClass::FunctionType f;

  mLocalIndex.resize(mDimX*mDimY*mDimZ);

  for (k=0;k<mDimZ;k++) {
    
    // We read the data slice by slice
    fread(mBuffer,sizeof(FunctionType),this->mEDim*mDimX*mDimY,this->mInput);

    // Now process each vertex one by one
    for (i=0;i<mDimX*mDimY;i++) {
      
      f = mBuffer[i*this->mEDim + this->mFDim];

      // If this vertex should be considered
      if ((f >= mLowThreshold) && (f <= mHighThreshold)) {

        if (this->mPersistentAttributes.size() > 0) {
          
          for (uint32_t p=0;p<this->mPersistentAttributes.size();p++) 
            this->mAttributeCache[p]->add(local,mBuffer[this->mEDim*i + this->mPersistentAttributes[p]]);
        }

        mSorted.resize(local+1);
        mSorted[local] = local;
          
        mIndexMap.add(local,global);
        mLocalIndex[global] = local;
        
        local++;
      }
      else
        mLocalIndex[global] = LNULL;

      global++;
    }
  } // Processed all vertices

  fprintf(stderr,"Found %d valid vertices\n\n",local);

  if (mDescending) {
    DescendingSort cmp(*this);
    std::sort(mSorted.begin(),mSorted.end(),cmp);
  }
  else {
    AscendingSort cmp(*this);
    std::sort(mSorted.begin(),mSorted.end(),cmp);
  }

  if (mMapFile != NULL)
    mIndexMap.dumpBinary(mMapFile);

  return 1;
}

#endif
