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

#include "GridGather.h"
#include "Multiplicity.h"

GridGather::GridGather(TopoTreeInterface* consumer,
                       uint32_t start_x, uint32_t start_y, uint32_t start_z,
                       uint32_t global_x, uint32_t global_y, uint32_t global_z,
                       uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                       uint32_t sub_x, uint32_t sub_y, uint32_t sub_z) : TreeGather(consumer),
                       mStartX(start_x), mStartY(start_y), mStartZ(start_z),
                       mGlobalX(global_x), mGlobalY(global_y), mGlobalZ(global_z),
                       mDimX(dim_x), mDimY(dim_y), mDimZ(dim_z),
                       mSubX(dim_x / sub_x), mSubY(dim_y / sub_y), mSubZ(dim_z / sub_z),
                       mLastGather((mDimX==mGlobalX) && (mDimY==mGlobalY) && (mDimZ==mGlobalZ))
{
}


int GridGather::addNode(GlobalIndexType i, FunctionType f)
{
  if (!mConsumer->containsVertex(i))
    return mConsumer->addVertex(i,f);
  else
    return 1;
}

int GridGather::finalizeNode(GlobalIndexType index, bool restricted)
{
  // If this vertex wasn't restricted on the lower levels it is not
  // restricted now
  if (!restricted)
    return this->mConsumer->finalizeVertex(index,false);

  // The x,y,z indices for the entire grid we are responsible for
  uint32_t x,y,z;
  uint8_t multiplicity = 1; // How often will this vertex appear ?

  if (index == 63126)
    fprintf(stderr,"break\n");

  x = index % mGlobalX - mStartX;
  y = (index % (mGlobalX*mGlobalY)) / mGlobalX  - mStartY;
  z = index / (mGlobalX*mGlobalY) - mStartZ;

  // First we determine the multiplicity. Only vertices on interior boundaries
  // are repeated
  if ((x > 0) && (x < mDimX-1) && (x%mSubX == 0))
    multiplicity = multiplicity << 1;

  if ((y > 0) && (y < mDimY-1) && (y % mSubY == 0))
    multiplicity = multiplicity << 1;

  if ((z > 0) && (z < mDimZ-1) && (z % mSubZ == 0))
    multiplicity = multiplicity << 1;

  if (multiplicity > 1) {
    std::map<GlobalIndexType,uint8_t>::iterator mIt;

    // Try to find the vertex in the multiplicity map
    mIt = mMultiplicityMap.find(index);

    if (mIt == mMultiplicityMap.end()) {// If it does not yet exist
      mMultiplicityMap[index] = multiplicity-1; // There need to be mult-1 other copies

      // and we do nothing here since we first have to wait for the other vertices
      // to come in
      return 1;
    }
    else {
      // We have found one more copy
      mIt->second--;

      // If there are still more copies to come
      if (mIt->second > 0)
        return 1; // We must wait
      else // Otherwise
        mMultiplicityMap.erase(mIt); // We remove it from the map
    }
  }

  // If we get here then we are either a vertex that only needs one copy
  // or we have seen all copies. The remaining decision is whether we
  // need to be restricted or not. For the moment we simply declare all
  // boundary vertices to be restricted. Theoretically, you can do better
  // than this since some vertices won't be needed but it is not clear
  // how to determine this


  if (mLastGather) // If this is the final collection of the entire grid
    return this->mConsumer->finalizeVertex(index,false); // There is no need for restrictions
  else if ((x == 0) || (y == 0) || (z == 0) ||  // Otherwise, if this index lies on the boundary
           (x == mDimX-1) || (y == mDimY-1) || (z == mDimZ-1))
    return this->mConsumer->finalizeVertex(index,true);

  return 1;
}

