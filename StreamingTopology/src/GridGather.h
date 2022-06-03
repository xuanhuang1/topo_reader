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


#ifndef GRIDGATHER_H
#define GRIDGATHER_H

#include <map>
#include "TreeGather.h"

//! A specialized gather operator to collect vertices from regular subgrids
class GridGather : public TreeGather
{
public:

  //! Default constructor
  /*! The constructor of a GridGather is passed in the coordinates
   * of the lower left corner of the gird from which it will collect
   * vertices. Furthermore, it gets passed the size of the grid and
   * into how many pieces the grid is subdivided in each dimension
   * @param consumer the pointer to the consuming topo tree
   * @param start coordinates of the lower left corner
   * @param global dimension of the global grid
   * @param dim the size of the box
   * @param sub the number of splits per dimension
   */
  GridGather(TopoTreeInterface* consumer,
             uint32_t start_x, uint32_t start_y, uint32_t start_z,
             uint32_t global_x, uint32_t global_y, uint32_t global_z,
             uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
             uint32_t sub_x, uint32_t sub_y, uint32_t sub_z);

  //! Destructor
  ~GridGather() {}

  //! Add the node with the given index and data to the graph
  int addNode(GlobalIndexType i, FunctionType f);

  //! Mark the vertex of the given index as final
  int finalizeNode(GlobalIndexType index, bool restricted);

protected:

  //! x-coordinates of the lower left corner
  const uint32_t mStartX;

  //! y-coordinates of the lower left corner
  const uint32_t mStartY;

  //! z-coordinates of the lower left corner
  const uint32_t mStartZ;

  //! Global x-dimension
  const uint32_t mGlobalX;

  //! Global y-dimension
  const uint32_t mGlobalY;

  //! Global z-dimension
  const uint32_t mGlobalZ;

  //! x-dimension
  const uint32_t mDimX;

  //! y-dimension
  const uint32_t mDimY;

  //! z-dimension
  const uint32_t mDimZ;

  //! Size of the regular subgrids in x direction
  const uint32_t mSubX;

  //! Size of the regular subgrids in y direction
  const uint32_t mSubY;

  //! Size of the regular subgrids in z direction
  const uint32_t mSubZ;

  //! A flag indicating whether this gather is responsible for
  //! the entire grid
  const bool mLastGather;

  //! The map which for each shared vertex stores its
  //! remaining multiplicty (the number of outstanding
  //! copies)
  std::map<GlobalIndexType,uint8_t> mMultiplicityMap;
};


#endif
