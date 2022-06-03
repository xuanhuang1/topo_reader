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

#ifndef IMPLICITGRIDPARSER_H
#define IMPLICITGRIDPARSER_H

#include <vector>
#include "GridParser.h"

/*! This class implements a grid parser that attaches the x
 *  y and
 *
 */
template<class DataClass = GenericData<FunctionType> >
class ImplicitGridParser: public virtual GridParser<DataClass>
{
public:

  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param input: file stream of the file to be read
   *  @param dim_x: number of samples on the x-axis
   *  @param dim_y: number of samples on the y-axis
   *  @param dim_z: number of samples on the z-axis
   *  @param edim: number of coordinates per vertex (N/A)
   *  @param fdim: index of the coordinate that should be the function (N/A)
   *  @param adims: indices of the coordinates that should be preserved
   */
  ImplicitGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x,
                         uint32_t dim_y, uint32_t dim_z, uint32_t fdim,
                         const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                         bool compact=true, FILE* map_file = NULL);

  virtual ~ImplicitGridParser()
  {
  }


protected:

  //! Read the next plane of data and if necessary add
  //! planes of x-,y-, and z-coordinates
  virtual int readDataPlane();

private:
  //! A convenient pointer to the array of x-coordinates
  FunctionType* mXCoords;

  //! A convenient pointer to the array of y-coordinates
  FunctionType* mYCoords;

  //! A convenient pointer to the array of z-coordinates
  FunctionType* mZCoords;

  //! A counter for the number of planes we have read
  uint32_t mPlaneCount;


};


template<class DataClass>
ImplicitGridParser<DataClass>::ImplicitGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x,
    uint32_t dim_y, uint32_t dim_z, uint32_t fdim, const std::vector<uint32_t>& adims, bool compact,FILE* map_file) :
    GridParser<DataClass>(attributes, dim_x, dim_y, dim_z, fdim, adims, compact,map_file),mXCoords(NULL),mYCoords(NULL),
    mZCoords(NULL),mPlaneCount(0)
{
  // There are attributes.size() many real attributes, plus three implicit ones.
  // By construction the implicit ones will be the last three. This means if we
  // Find persistent attributes with indices beyond attributes.size() we know
  // what they represent
  for (uint8_t i=0;i<adims.size();i++) {
    if (adims[i] == attributes.size())
      mXCoords = this->mAttributeBuffers[i];

    if (adims[i] == attributes.size() + 1)
      mYCoords = this->mAttributeBuffers[i] + 1;

    if (adims[i] == attributes.size() + 2)
      mZCoords = this->mAttributeBuffers[i] + 2;
  }

  if (mXCoords != NULL) {
    // Pre-fill the x and y coordinate buffers since these will not change
    for (uint32_t j=0;j<this->mDimY;j++) {
      for (uint32_t i=0;i<this->mDimX;i++) {
        mXCoords[j*this->mDimX + i] = i;
      }
    }
  }

  if (mYCoords != NULL) {
    // Pre-fill the x and y coordinate buffers since these will not change
    for (uint32_t j=0;j<this->mDimY;j++) {
      for (uint32_t i=0;i<this->mDimX;i++) {
        mYCoords[j*this->mDimX + i] = j;
      }
    }
  }

}


template <class DataClass>
int ImplicitGridParser<DataClass>::readDataPlane()
{
  if (!this->mPersistentAttributes.empty()) {
    for (uint16_t i=0;i<this->mPersistentAttributes.size();i++) {

      // If this persistent attribute was one that got passed in originally
      if (this->mPersistentAttributes[i] < this->mAttributeFiles.size()) {
        // We read the file
        fread(this->mAttributeBuffers[i],sizeof(FunctionType),this->mDimX*this->mDimY,this->mAttributeFiles[this->mPersistentAttributes[i]]);
      }
      // If we are supposed to read the z coordinate
      else if (this->mPersistentAttributes[i] == this->mAttributeFiles.size()+2) {
        for (uint32_t k=0;k<this->mDimX*this->mDimY;k++)
          mZCoords[k] = mPlaneCount;
      }
    }
  }
  else {
    fread(this->mAttributeBuffers[0],sizeof(FunctionType),this->mDimX*this->mDimY,this->mAttributeFiles[0]);
  }

  mPlaneCount++;

  std::swap(this->mIndexMap[0],this->mIndexMap[1]);

  return 1;
}




#endif
