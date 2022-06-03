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

#ifndef IMPLICITGRIDEDGEPARSER_H
#define IMPLICITGRIDEDGEPARSER_H

#include <vector>
#include "GridParser.h"

template<class DataClass>
class ImplicitGridEdgeParser: public GridParser<DataClass>
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
  ImplicitGridEdgeParser(const std::vector<FILE*>& attributes, uint32_t dim_x,
                         uint32_t dim_y, uint32_t dim_z, uint32_t fdim,
                         const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                         FILE* map_file = NULL);

  virtual ~ImplicitGridEdgeParser()
  {
  }

  virtual FileToken getToken();

private:

  virtual DataClass constructData(FunctionType* buffer, uint16_t fdim);

};


template<class DataClass>
ImplicitGridEdgeParser<DataClass>::ImplicitGridEdgeParser(const std::vector<FILE*>& attributes, uint32_t dim_x,
    uint32_t dim_y, uint32_t dim_z, uint32_t fdim, const std::vector<uint32_t>& adims, FILE* map_file) :
    GridParser<DataClass>(attributes, dim_x, dim_y, dim_z, fdim, adims, map_file)
{
}


template<class DataClass>
FileToken ImplicitGridEdgeParser<DataClass>::getToken()
{
  FileToken token;

  token = GridParser<DataClass>::getToken();

  if (token == VERTEX) {
    if (this->mDimX > 1)
      this->mData[0] = this->mId % this->mDimX;

    if (this->mDimY > 1)
      this->mData[1] = (this->mId % (this->mDimX * this->mDimY)) / this->mDimX;

    if (this->mDimZ > 1)
      this->mData[2] = this->mId / (this->mDimX * this->mDimY);
  }

  return token;
}

template<class DataClass>
DataClass ImplicitGridEdgeParser<DataClass>::constructData(FunctionType* buffer, uint16_t fdim)
{
  DataClass v;

  v.f(buffer[fdim]);

  return v;
}

#endif
