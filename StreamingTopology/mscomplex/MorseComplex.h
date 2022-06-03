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

#ifndef MORSECOMPLEX_H
#define MORSECOMPLEX_H

#include "FlexArray/BlockedArray.h"
#include "Slopes.h"


template <class VertexClass = MCVertex<3,float>, class ArrayClass=FlexArray::BlockedArray<VertexClass> >
class TopoComplex 
{
public:

  //! Default constructor
  MorseComplex(const Slope<VertexClass>& s) : mSlope(s) {}

  //! Destructor
  virtual ~MorseComplex() {}
  
  //! Add a vertex
  virtual void addVertex(LocalIndexType index, VertexClass& data)  {mVertices.add(index,data);}

  //! Compute the slope between two vertices
  virtual double slope(LocalIndexType u, LocalIndexType v) {return mSlope(mVertices[u],mVertices[v]);}

  //! No-op for locking to computed non-threaded 
  virtual void lock(LocalIndexType i) {}

  //! No-op for unlocking to computed non-threaded 
  virtual void unlock(LocalIndexType i) {}

protected:

  //! The operator to compute the slope between two vertices
  const Slope<VertexClass>& mSlope;

  //! The array storing the vertex data and gradient information
  ArrayClass mVertices;
};
  

#endif
