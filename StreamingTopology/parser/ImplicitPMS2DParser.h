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

#ifndef IMPLICITPMS2DPARSER_H
#define IMPLICITPMS2DPARSER_H

#include "PMS2DParser.h"

template<class DataClass>
class ImplicitPMS2DParser : public PMS2DParser<DataClass>
{
public:

  //! Default constructor
  /*! Default constructor which saves the FILE* and dimensions for later
   * acces
   * @param attributes: File pointers to blocks of floats
   * @param dim_x: Number of vertices in the x dimension
   * @param dim_y: Number of vertices in the y dimension
   * @param adims: indices of the coordinates that should be preserved
   */
  ImplicitPMS2DParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y,
                      uint32_t f_dim, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  virtual ~ImplicitPMS2DParser() {}

private:

  //! Set the data for the given
  virtual void setData(FunctionType f);

  //! Set the data for the midpoint
  virtual void setMidPoint(FunctionType f);
};

template<class DataClass>
ImplicitPMS2DParser<DataClass>::ImplicitPMS2DParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y,
                                                    uint32_t f_dim, const std::vector<uint32_t>& adims) :
                                                    PMS2DParser<DataClass>(attributes,dim_x,dim_y,f_dim,adims)
{
}

template<class DataClass>
void ImplicitPMS2DParser<DataClass>::setData(FunctionType f)
{

  this->mData[0] = 2*this->mI;
  this->mData[1] = 2*this->mJ;
  this->mData[2] = f;
}

template<class DataClass>
void ImplicitPMS2DParser<DataClass>::setMidPoint(FunctionType f)
{

  this->mData[0] = 2*this->mI-1;
  this->mData[1] = 2*this->mJ-1;
  this->mData[2] = f;
}

#endif /* IMPLICITPMS2DPARSER_H */
