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


#ifndef SUBGRIDPARSER_H_
#define SUBGRIDPARSER_H_

#include <cstdio>
#include "GridParser.h"
#include "GenericData.h"

uint32_t subgrid_size(uint32_t global, uint32_t split, uint32_t i);

//! Helper class for SoS comparison
class SampleCompare
{
public:

  SampleCompare(uint8_t flag) : mFlag(flag) {}

  SampleCompare(const SampleCompare& cmp) : mFlag(cmp.mFlag) {}

  bool operator()(FunctionType f0, GlobalIndexType i0,FunctionType f1, GlobalIndexType i1) {
    if (mFlag) {
      if ((f0 > f1) || ((f0 == f1) && (i0 > i1)))
        return true;
      else
        return false;
    }
    else {
      if ((f1 > f0) || ((f1 == f0) && (i1 > i0)))
        return true;
      else
        return false;
    }
  }

private:

  uint8_t mFlag;
};

//! Class to parse subgrids of non-interleaved grids
template <class DataClass = GenericData<FunctionType> >
class SubGridParser : public GridParser<DataClass>
{
public:

  //! To avoid using the this-> pointer for too many things
  using GridParser<DataClass>::mDimX;
  using GridParser<DataClass>::mDimY;
  using GridParser<DataClass>::mDimZ;
  using GridParser<DataClass>::mI;
  using GridParser<DataClass>::mJ;
  using GridParser<DataClass>::mK;


  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param function: file stream of the raw function to be read
   *  @param dim: index of samples in each dimension
   *  @param sub: number of subgrids in each dimension
   *  @param sub_id: the index of this subgrid in row major order
   *  @param edim: number of coordinates per vertex (N/A)
   *  @param fdim: index of the coordinate that should be the function (N/A)
   *  @param adims: indices of the coordinates that should be preserved (N/A)
   *  @param attributes: file pointers to any additional attributes of interest
   *  @param map_file: optional file pointer to store the index map if compactifying
   */
  SubGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                uint32_t sub_x, uint32_t sub_y, uint32_t sub_z,
                uint32_t ix, uint32_t iy, uint32_t iz,
                uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                bool compact=true, FILE* map_file=NULL);

  //! Destructor
  virtual ~SubGridParser();

  //! Set the comparison function
  void setCompare(uint8_t flag) {mSampleCmp = SampleCompare(flag);}

protected:

  //! Global x-dimension of the super-grid
  const int32_t mGlobalX;

  //! Global y-dimension of the super-grid
  const int32_t mGlobalY;

  //! Global z-dimension of the super-grid
  const int32_t mGlobalZ;

  //! The number of subgrids in x dimension
  const uint32_t mSubX;

  //! The number of subgrids in x dimension
  const uint32_t mSubY;

  //! The number of subgrids in x dimension
  const uint32_t mSubZ;

  //! The x index of this subgrid
  const uint32_t mIX;

  //! The y index of this subgrid
  const uint32_t mIY;

  //! The z index of this subgrid
  const uint32_t mIZ;

  //! Global x-index of the lower left corner of the subgrid
  uint32_t mStartX;

  //! Global y-index of the lower left corner of the subgrid
  uint32_t mStartY;

  //! Global z-index of the lower left corner of the subgrid
  uint32_t mStartZ;

  //! Pointers to two planes of function values
  /*! Two planes of function values. However, mFunctionBuffer[1] is
   * equal to mBuffer and as such will be allocated and deallocated
   * by the baseclass.
   */
  FunctionType* mFunctionBuffer[2];

  //! Two planes of flags indicating whether a vertex is restricted
  bool* mRestricted[3];

  //! Pointers to two planes of boundary markers
  BoundaryMarker* mBoundary[2];

  //! Three planes of boundary markers for the first, middle, and last plane
  BoundaryMarker* mBoundaryTemplate[3];

  //! The comparison metric to determine whether a vertex is a boundary extremum
  SampleCompare mSampleCmp;

  //! Added the edges of the mesh to the stack
  virtual void addEdges();

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

  //! Advance all file pointers to skip potential header information
  virtual void skipHeader();

  //! Read the next plane of data
  virtual int readDataPlane();

  //! Read a subplane of data from the given file to the given buffer
  void readDataPlane(FILE** input, FunctionType* buffer);
};

template <class DataClass>
SubGridParser<DataClass>::SubGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                                        uint32_t sub_x, uint32_t sub_y, uint32_t sub_z, uint32_t ix, uint32_t iy, uint32_t iz,
                                        uint32_t fdim, const std::vector<uint32_t>& adims, bool compact, FILE* map_file) :
  GridParser<DataClass>(attributes,
                        subgrid_size(dim_x,sub_x,ix),
                        subgrid_size(dim_y,sub_y,iy),
                        subgrid_size(dim_z,sub_z,iz),
                        fdim,adims,compact,map_file),
  mGlobalX(dim_x), mGlobalY(dim_y),mGlobalZ(dim_z), mSubX(sub_x), mSubY(sub_y), mSubZ(sub_z), mIX(ix), mIY(iy), mIZ(iz),
  mStartX(ix*(dim_x / sub_x)), mStartY(iy*(dim_y / sub_y)), mStartZ(iz*(dim_z / sub_z)), mSampleCmp(1)
{
  sterror(ix*iy*iz >= sub_x*sub_y*sub_z,"Subgrid index out of range.");

  skipHeader();

  mFunctionBuffer[0] = new FunctionType[mDimX*mDimY];
  mFunctionBuffer[1] = this->mBuffer;

  // Allocated the restricted flags
  mRestricted[0] = new bool[this->mDimX*this->mDimY];
  mRestricted[1] = new bool[this->mDimX*this->mDimY];
  mRestricted[2] = new bool[this->mDimX*this->mDimY];

  // Allocate the boundary marker planes
  mBoundaryTemplate[0] = new BoundaryMarker[this->mDimX*this->mDimY];
  mBoundaryTemplate[1] = new BoundaryMarker[this->mDimX*this->mDimY];
  mBoundaryTemplate[2] = new BoundaryMarker[this->mDimX*this->mDimY];

  // Initialize the boundary templates
  int32_t i;

  // The first plane starts with all vertices being low z
  memset(mBoundaryTemplate[0],1<<5,this->mDimX*this->mDimY);

  // The middle planes start with all vertices being unflagged
  memset(mBoundaryTemplate[1],0,this->mDimX*this->mDimY);

  // The last plane starts with all vertices being high z
  memset(mBoundaryTemplate[2],1<<6,this->mDimX*this->mDimY);


  // The first and last row are y boundaries
  for (i=0;i<this->mDimX;i++) {// First row is a low y-boundary
    mBoundaryTemplate[0][i].set(2);
    mBoundaryTemplate[1][i].set(2);
    mBoundaryTemplate[2][i].set(2);
  }

  // Last row is a high y boundary
  for (i=this->mDimX*(this->mDimY-1);i<this->mDimX*this->mDimY;i++) {
    mBoundaryTemplate[0][i].set(3);
    mBoundaryTemplate[1][i].set(3);
    mBoundaryTemplate[2][i].set(3);
  }

  // The first and last column are x boundaries
  for (i=0;i<this->mDimX*this->mDimY;i+=this->mDimX) {
    mBoundaryTemplate[0][i].set(0);
    mBoundaryTemplate[1][i].set(0);
    mBoundaryTemplate[2][i].set(0);
  }

  // The first and last column are x boundaries
  for (i=this->mDimX-1;i<this->mDimX*this->mDimY;i+=this->mDimX) {
    mBoundaryTemplate[0][i].set(1);
    mBoundaryTemplate[1][i].set(1);
    mBoundaryTemplate[2][i].set(1);
  }


  this->mIndex = (mStartZ*mGlobalY + mStartY)*mGlobalX + mStartX;
}

template <class DataClass>
SubGridParser<DataClass>::~SubGridParser()
{
  // Only delete one of the function buffers since the other
  // one is part of the baseclass and will be deleted there

  delete mFunctionBuffer[0];

  delete mRestricted[0];
  delete mRestricted[1];
  delete mRestricted[2];

  // Currently, this produce a double free at construction for
  // no apparant reason. Until we figure out what happens I have
  // commented it out.
  /*
  delete mBoundaryTemplate[0];
  delete mBoundaryTemplate[1];
  delete mBoundaryTemplate[2];
  */
}

template <class DataClass>
void SubGridParser<DataClass>::addEdges()
{
  LocalIndexType plane_index = mJ*mDimX + mI;
  LocalIndexType z_index, p_index; // The z and plane index of the "other" vertex

  //if (this->mId == 25)
  //  fprintf(stderr,"break %p\n",tmp);

  // An interior vertex is never restricted
  if (mBoundary[1][plane_index] == BoundaryMarker())
    mRestricted[1][plane_index] = false;

  for (int i=0;i<this->sEdgeNr;i++) {
    if ((this->sEdgeTable[i][0]+mI < 0) || (this->sEdgeTable[i][0]+mI >= mDimX)
        || (this->sEdgeTable[i][1]+mJ < 0) || (this->sEdgeTable[i][1]+mJ >= mDimY)
        || (this->sEdgeTable[i][2]+mK < 0)) {
      continue;
    }

    // The z-coordinates of the edge table are either -1 or 0. However,
    // wrt. the mFunctionBuffer planes the 0'th plane corresponds to the
    // -1 edge table. Thus we add +1 here
    z_index = this->sEdgeTable[i][2] + 1;

    // Compute the idnex of the "other" vertex wrt. its plane
    p_index = (mJ+this->sEdgeTable[i][1])*mDimX + mI+this->sEdgeTable[i][0];

    // If the other vertex has bee determined valid
    if (this->mIndexMap[z_index][p_index] != GNULL) {

      // If the current vertex is part of the boundary, "the other" is in the closure
      // of its boundary component, and "the other" is greater than the current vertex
      // the current vertex is not an extremum restricted to the boundary.
      if ((mBoundary[1][plane_index] != BoundaryMarker())
          && (mBoundary[1][plane_index] < mBoundary[z_index][p_index])
          && mSampleCmp(mFunctionBuffer[z_index][p_index],this->mIndexMap[z_index][p_index],
                        mFunctionBuffer[1][plane_index],this->mIndexMap[1][plane_index]))
      {
        mRestricted[1][plane_index] = false;
      }
      else if ((mBoundary[z_index][p_index] != BoundaryMarker()) // reverse condition
          && (mBoundary[z_index][p_index] < mBoundary[1][plane_index])
          && mSampleCmp(mFunctionBuffer[1][plane_index],this->mIndexMap[1][plane_index],
                        mFunctionBuffer[z_index][p_index],this->mIndexMap[z_index][p_index]))
      {
        mRestricted[z_index][p_index] = false;
      }


      this->mEdges.push(this->mIndexMap[z_index][p_index]);
    }
  }


  // For the current vertex with global index mIndex and local index mLocal
  // figure out which is the last global vertex that may have an edge using
  // the current vertex
  this->mProcessed.push(typename GridParser<DataClass>::FinalizationInfo(lastUsed(),this->mId,&mRestricted[1][plane_index]));

}


template <class DataClass>
GlobalIndexType SubGridParser<DataClass>::lastUsed() const
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
void SubGridParser<DataClass>::skipHeader()
{
  // Reset all file pointers to the correct starting positions. Note that we need an fseek
  // since the GridParser may have advanced our file pointers
  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<this->mAttributeBuffers.size();i++)
      fseek(this->mAttributeFiles[this->mPersistentAttributes[i]],
            sizeof(FunctionType)*((mStartZ*mGlobalY + mStartY)*mGlobalX + mStartX),
            SEEK_SET);
  }
  else {
    //fprintf(stderr,"Skipping %d elements \n",((mStartZ*mGlobalY + mStartY)*mGlobalX + mStartX));
    fseek(this->mAttributeFiles[0],sizeof(FunctionType)*((mStartZ*mGlobalY + mStartY)*mGlobalX + mStartX),
          SEEK_SET);
  }

}


template <class DataClass>
int SubGridParser<DataClass>::readDataPlane()
{
  // First we need to swap the function buffers in a way that is
  // transparent to the baseclass which doesn't know that we are
  // secretly keeping a second plane around
  std::swap(mFunctionBuffer[0],mFunctionBuffer[1]); // We swap our buffers

  // And insert the new "active" one into all the baseclass structures
  this->mAttributeBuffers[this->mFDim] = mFunctionBuffer[1];
  this->mBuffer = mFunctionBuffer[1];

  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<this->mAttributeBuffers.size();i++)
      readDataPlane(&this->mAttributeFiles[this->mPersistentAttributes[i]],this->mAttributeBuffers[i]);
  }
  else {
    readDataPlane(&this->mAttributeFiles[0],this->mAttributeBuffers[0]);
  }

  // Swap the restricted planes to rotate them around
  // After the swap the new planes are 1,2,0.
  std::swap(mRestricted[0],mRestricted[2]);
  std::swap(mRestricted[1],mRestricted[0]);


  std::swap(mBoundary[0],mBoundary[1]);

  // Reinitialize the restricted plane to be all restricted
  memset(mRestricted[1],1,this->mDimX*this->mDimY);

  if (this->mK == 0)
    mBoundary[1] = mBoundaryTemplate[0];
  else if (this->mK == this->mDimZ-1)
    mBoundary[1] = mBoundaryTemplate[2];
  else
    mBoundary[1] = mBoundaryTemplate[1];


  return 1;
}


template <class DataClass>
void SubGridParser<DataClass>::readDataPlane(FILE** input, FunctionType* buffer)
{
  // If the data is not split in x dimension we can read the data in one piece
  if (this->mDimX == mGlobalX)
    fread(buffer,sizeof(FunctionType),this->mDimX*this->mDimY,*input);
  else { // Otherwise we need read things in pieces
    for (int32_t i=0;i<this->mDimY;i++) {

      // First we read one x line
      fread(buffer,sizeof(FunctionType),this->mDimX,*input);

      // Now advance the buffer
      buffer += this->mDimX;

      // Finally, we need to seek to the next valid sample in the file
      fseek(*input, (mGlobalX - this->mDimX)*sizeof(FunctionType),SEEK_CUR);
    }
  }
  // Now we need to get to the start of the next plane
  fseek(*input,mGlobalX*(mGlobalY - this->mDimY)*sizeof(FunctionType),SEEK_CUR);
}



#endif /* SUBGRIDPARSER_H_ */
