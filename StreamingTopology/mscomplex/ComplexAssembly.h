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

#ifndef COMPLEXASSEMBLY_H
#define COMPLEXASSEMBLY_H

#include <map>
#include "MCVertex.h"
#include "AssemblyInterface.h"
#include "GradientComplexInterface.h"

using namespace std;

template <class VertexClass = MCVertex<3,float> > 
class ComplexAssembly : public AssemblyInterface
{
public:

  class IndexPair {
  public:
    IndexPair(LocalIndexType ii, LocalIndexType jj) : i(ii), j(jj) {sterror(i > j,"The smaller index should come first");}

    bool operator<(const IndexPair& pair) const {return ((i == pair.i) ? (j < pair.j) : (i < pair.i));}

    LocalIndexType i;
    LocalIndexType j;
  };

  class SaddleStruct {
  public:
    SaddleStruct(LocalIndexType s=LNULL, LocalIndexType u=LNULL, LocalIndexType v=LNULL)
    : saddle(s),left(u),right(v) {}

    SaddleStruct& operator=(const SaddleStruct s) {
      saddle = s.saddle; left = s.left; right = s.right;
      return *this;
    }
    LocalIndexType saddle;
    LocalIndexType left;
    LocalIndexType right;
  };

  typedef typename map<IndexPair,SaddleStruct>::iterator MapIterator;

  ComplexAssembly(const GradientComplexInterface<VertexClass>& complex) : mComplex(complex) {}

    //! Destructor
  virtual ~ComplexAssembly() {}

  //! Add an edge
  virtual double add_edge(LocalIndexType u, LocalIndexType v) = 0;

  //! Reference to the array containing all vertices
  const GradientComplexInterface<VertexClass>& mComplex;

  //! The map storing all possible neighborhoods
  map<IndexPair,SaddleStruct> mNeighborMap;
};


#endif

