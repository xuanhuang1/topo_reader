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


#ifndef GRIDPARSER_H
#define GRIDPARSER_H

#include <stack>
#include <queue>
#include <cstdio>

#include "Parser.h"
#include "GenericData.h"

//! Class to parse non-interleaved grids
template <class DataClass = GenericData<FunctionType> >
class GridParser : public Parser<DataClass>
{

public:

  //! Repeat of the typedef since most compilers are not smart enough
  typedef typename DataClass::FunctionType FunctionType;

  // Number of indices written as a chunk
  static const uint16_t sWriteBufferSize = 8192;
  
  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param function: file stream of the raw function to be read
   *  @param dim: index of samples in each dimension
   *  @param edim: number of coordinates per vertex (N/A)
   *  @param fdim: index of the coordinate that should be the function (N/A)
   *  @param adims: indices of the coordinates that should be preserved (N/A)
   *  @param attributes: file pointers to any additional attributes of interest
   *  @param map_file: optional file pointer to store the index map if compactifying
   */
  GridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z, 
             uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
             bool compact=true, FILE* map_file=NULL);

  //! Destructor
  virtual ~GridParser();
  
  //! Read the next token
  virtual FileToken getToken();

protected:
  
  //! This struct encodes which vertex will be finalized after which global
  //! index is passed
  class FinalizationInfo {
  public:
    FinalizationInfo(GlobalIndexType last, GlobalIndexType i, const bool* res) :
      last_used(last), index(i), restricted(res) {}

    GlobalIndexType last_used; // The last global index that uses the local vertex
    GlobalIndexType index; // The local index of the vertex to be finalized
    const bool* restricted;// Pointer to he falg indicating whether a vertex is restricted
  };

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

  //! The current global index
  GlobalIndexType mIndex;

  //! The current local index of all valid vertices
  GlobalIndexType mLocal;
  
  //! A flag indicating whether we should compactify the index space
  const bool mCompact;

  //! The set of edges that need to be added
  std::stack<GlobalIndexType> mEdges;

  //! The queue of vertices that are eligible for finalization
  std::queue<FinalizationInfo> mProcessed;

  //! A buffer containing a single plane of function values
  FunctionType* mBuffer;

  //! A vector containing file pointers to files holding the attributes
  std::vector<FILE*> mAttributeFiles;

  //! A vector containing a single plane of values for each attribute
  std::vector<FunctionType*> mAttributeBuffers;

  //! Two planes of indices that store the index map to global indices
  GlobalIndexType* mIndexMap[2];

  //! Pointer to write the index map to file
  FILE* mMapFile;

  //! Number of indices written as a chunk
  uint16_t mIndexBufferSize;

  //! The write buffer
  GlobalIndexType* mIndexBuffer;

  //! Position in the index buffer
  uint16_t mIndexPos;

  //! Flag to indicate whether we need to load the first plane
  bool mFirstPlane;

  virtual DataClass constructData(FunctionType* buffer, uint16_t fdim) {return DataClass(buffer,fdim);}

  //! Added the edges of the mesh to the stack
  virtual void addEdges();

  //! Make an edge from the stack
  virtual void makeEdge();

  /*! Advance the global index of a vertex. This function should be called
   * *after* all local indices have been corrected since they might be used
   * to compute the next index.
   */
  virtual void advanceGlobalIndex() {mIndex++;}

  /*! According to the current local indices determine the global index of the
   * last vertex that may have an edge with mIndex / mId / mLocal
   * @return global index of the last vertex that may have an edge with this one
   */
  virtual GlobalIndexType lastUsed() const;

  //! Advance all file pointers to skip potential header information
  virtual void skipHeader() {}

  //! Read the next plane of data
  virtual int readDataPlane();

  //! Set the co-dimension and multiplicity
  virtual void setBoundaryInfo() {}
};


template <class DataClass>
int8_t GridParser<DataClass>::sEdgeTable[GridParser<DataClass>::sEdgeNr][3] = {

  {-1,-1,-1}, {0,-1,-1}, {1,-1,-1}, 
  
  {-1, 0,-1}, {0, 0,-1}, {1, 0,-1},
  
  {-1, 1,-1}, {0, 1,-1}, {1, 1,-1},

  {-1,-1, 0}, {0,-1, 0}, {1,-1, 0},
  
  {-1, 0, 0}
};


template <class DataClass>
GridParser<DataClass>::GridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z, 
                                  uint32_t fdim, const std::vector<uint32_t>& adims, bool compact, FILE* map_file)
  : Parser<DataClass>(attributes[fdim],1,0,adims), mDimX(dim_x), mDimY(dim_y), mDimZ(dim_z), mI(0), mJ(0), mK(0),
    mIndex(0), mLocal(0), mCompact(compact), mAttributeFiles(attributes), mMapFile(map_file),
    mIndexBufferSize(sWriteBufferSize), mIndexPos(0), mFirstPlane(true)
{
  
  mIndexMap[0] = new GlobalIndexType[mDimX*mDimY];
  mIndexMap[1] = new GlobalIndexType[mDimX*mDimY];

  this->mPath.resize(2);

  for (int i=0;i<sEdgeNr;i++) 
    mEdgeOffset[i] = sEdgeTable[i][0] + sEdgeTable[i][1]*mDimX + sEdgeTable[i][2]*mDimX*mDimY;

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

  mIndexBuffer = new GlobalIndexType[mIndexBufferSize];
}

template <class DataClass>
GridParser<DataClass>::~GridParser()
{
  for (uint16_t i=0;i<mAttributeBuffers.size();i++) 
    delete[]  mAttributeBuffers[i];

  delete[] mIndexBuffer;

  delete[] mIndexMap[0];
  delete[] mIndexMap[1];

}

template <class DataClass>
FileToken GridParser<DataClass>::getToken()
{
  // Make sure that before the first token we read we read one
  // data plane.
  if (mFirstPlane) {
    readDataPlane();
    mFirstPlane = false;
  }

  if (!mEdges.empty()) {

    makeEdge();
    
    //fprintf(stderr,"e: %d %d\n",this->mPath[0],this->mPath[1]);
    return EDGE;
  }
  else if (!mProcessed.empty() && (mProcessed.front().last_used < mIndex)) {

    this->mFinal = mProcessed.front().index;
    this->mRestrictedFlag = *mProcessed.front().restricted;

    mProcessed.pop();

    return FINALIZE;
  }
  else {
    
    // If the current vertex is beyond the range 
    if (mK >= mDimZ) {
      if (!mProcessed.empty()) {
        sterror(mProcessed.front().last_used >= mIndex,"Finalization information inconsistent.");
        
        this->mFinal = mProcessed.front().index;
        this->mRestrictedFlag = *mProcessed.front().restricted;

        mProcessed.pop();

        return FINALIZE;
      }
      else {
        // Take care of the remaining indices
        if (mMapFile != NULL)
          fwrite(mIndexBuffer,sizeof(GlobalIndexType),mIndexPos,mMapFile);

        return EMPTY; // There is no more data left to read
      }
    }

    // Until we find a valid vertex
    while ((mBuffer[mJ*mDimX + mI] < this->mFMin) || (mBuffer[mJ*mDimX + mI] > this->mFMax)) {

      // If this vertex is not valid we remember that by storing a GNULL as
      // global index
      mIndexMap[1][mJ*mDimX+mI] = GNULL;

      // We advance the indices
      mI++;
      mJ += mI / mDimX;
      mK += mJ / mDimY;
      

      mI = mI % mDimX;
      mJ = mJ % mDimY;

      advanceGlobalIndex();


      // If we have finished the next slice, we might be done
      if (mK >= mDimZ) {

        if (!mProcessed.empty()) {
          sterror(mProcessed.front().last_used >= mIndex,"Finalization information inconsistent.");
          
          this->mFinal = mProcessed.front().index;
          this->mRestrictedFlag = mProcessed.front().restricted;
          mProcessed.pop();
          
          return FINALIZE;
        }
        else {
          // Take care of the remaining indices
          if (mMapFile != NULL)
            fwrite(mIndexBuffer,sizeof(GlobalIndexType),mIndexPos,mMapFile);

          return EMPTY; // There is no more data left to read
        }
      }
      
      // Otherwise, read the next slice
      if ((mI == 0) && (mJ == 0)) {
        if ((mK-1) % 50 == 0)
          fprintf(stderr,"Last vertex of plane %d\n",mK-1);

        readDataPlane();
      }
    } // End until we find a valid vertex

    //fprintf(stderr,"%d %d %d\n",mI,mJ,mK);
    
    // If we get out of the while loop without returning we found a valid vertex.
    // Depending on whether we should compact the index space we use either the
    // running local or the current global id
    if (mCompact)
      this->mId = mLocal;
    else
      this->mId = mIndex;

    setBoundaryInfo();

    // We also remember the index we used in the second index map
    mIndexMap[1][mJ*mDimX + mI] = this->mId;

    // Now we take care of the index map if necessary 
    if (mMapFile != NULL) {
      if (mIndexPos == mIndexBufferSize) {
        fwrite(mIndexBuffer,sizeof(GlobalIndexType),mIndexBufferSize,mMapFile);
        mIndexPos = 0;
      }

      mIndexBuffer[mIndexPos++] = mIndex;
    }
    
    // All edges will be incident to this vertex
    this->mPath[0] = this->mId;
    
    // Read the function value
    this->mData = constructData(mBuffer + mJ*mDimX + mI,0);
 
    // store all the necessary attributes
    if (!this->mAttributeCache.empty()) {
      for (uint16_t i=0;i<mAttributeBuffers.size();i++) 
        this->mAttributeCache[i]->add(mLocal,mAttributeBuffers[i][mJ*mDimX + mI]);
    }      

    addEdges();

    // We advance the indices
    mLocal++;
    mI++;
    mJ += mI / mDimX;
    mK += mJ / mDimY;
    
    mI = mI % mDimX;
    mJ = mJ % mDimY;

    advanceGlobalIndex();

    // If we finished a plane and there is a plane left 
    if ((mI == 0) && (mJ == 0) && (mK < mDimZ)) {
      if ((mK-1) % 16 == 0)
        fprintf(stderr,"Last vertex of plane %d\n",mK-1);

      readDataPlane();
    }



    return VERTEX;
  }
}          

template <class DataClass>
void GridParser<DataClass>::addEdges()
{
  GlobalIndexType e;
  for (int i=0;i<sEdgeNr;i++) {
    if ((sEdgeTable[i][0]+mI < 0) || (sEdgeTable[i][0]+mI >= mDimX)
        || (sEdgeTable[i][1]+mJ < 0) || (sEdgeTable[i][1]+mJ >= mDimY)
        || (sEdgeTable[i][2]+mK < 0)) {
      continue;
    }

    //mEdges.push(mIndex + mEdgeOffset[i]);
    e = mIndexMap[sEdgeTable[i][2] + 1][(mJ+sEdgeTable[i][1])*mDimX + mI+sEdgeTable[i][0]];
    if (e != GNULL)
      mEdges.push(e);

  }

  // For the current vertex with global index mIndex and local index mLocal
  // figure out which is the last global vertex that may have an edge using
  // the current vertex
  mProcessed.push(FinalizationInfo(lastUsed(),this->mId,&(this->mRestrictedFlag)));
}

template <class DataClass>
void GridParser<DataClass>::makeEdge()
{
  this->mPath[1] = mEdges.top();
  mEdges.pop();
}


template <class DataClass>
GlobalIndexType GridParser<DataClass>::lastUsed() const
{
  if (mK < mDimZ-1) {
    if (mJ < mDimY-1) {
      if (mI < mDimX-1)
        return (mK+1)*mDimX*mDimY + (mJ+1)*mDimX + mI + 1;
      else
        return (mK+1)*mDimX*mDimY + (mJ+1)*mDimX + mI;
    }
    else {
      if (mI < mDimX-1)
        return (mK+1)*mDimX*mDimY + mJ*mDimX + mI + 1;
      else
        return (mK+1)*mDimX*mDimY + mJ*mDimX + mI;
    }
  }
  else {
    if (mJ < mDimY-1) {
      if (mI < mDimX-1)
        return mK*mDimX*mDimY + (mJ+1)*mDimX + mI + 1;
      else
        return mK*mDimX*mDimY + (mJ+1)*mDimX + mI;
    }
    else {
      if (mI < mDimX-1)
        return mK*mDimX*mDimY + mJ*mDimX + mI + 1;
      else
        return mK*mDimX*mDimY + mJ*mDimX + mI;
    }
  }

}

template <class DataClass>
int GridParser<DataClass>::readDataPlane()
{
  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<this->mPersistentAttributes.size();i++)
      fread(mAttributeBuffers[i],sizeof(FunctionType),mDimX*mDimY,mAttributeFiles[this->mPersistentAttributes[i]]);
  }
  else {
    fread(mAttributeBuffers[0],sizeof(FunctionType),mDimX*mDimY,mAttributeFiles[0]);
  }

  std::swap(mIndexMap[0],mIndexMap[1]);

  return 1;
}


#endif
