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

#ifndef UNSTABLEMANIFOLDASSEMBLY_H
#define UNSTABLEMANIFOLDASSEMBLY_H

#include "ComplexAssembly.h"
#include "MSCVertex.h"

template <class VertexClass= MSCVertex<3,float> >
class UnstableComplexAssembly : public ComplexAssembly<VertexClass>
{
public:

  using ComplexAssembly<VertexClass>::mComplex;
  
  typedef typename ComplexAssembly<VertexClass>::SaddleStruct SaddleStruct;
  typedef typename map<typename ComplexAssembly<VertexClass>::IndexPair,SaddleStruct>::iterator MapIterator;

  UnstableComplexAssembly(const GradientComplexInterface<VertexClass>& complex) : ComplexAssembly<VertexClass>(complex) {}

  //! Destructor
  virtual ~UnstableComplexAssembly() {}

  //! Add an edge
  virtual double add_edge(LocalIndexType u, LocalIndexType v);
};

template <class VertexClass>
double UnstableComplexAssembly<VertexClass>::add_edge(LocalIndexType u, LocalIndexType v)
{
  LocalIndexType i,j;
  MapIterator mIt;

  //fprintf(stdout," UnstableComplexAssembly<VertexClass>::add_edge %d -> %d\n",u,v);

  i = mComplex.vertex(u).unstableInfo().extremum();
  j = mComplex.vertex(v).unstableInfo().extremum();
 
  if (i != j) {

    if (i > j) {
      swap(i,j);
      swap(u,v);
    }
    
    typename ComplexAssembly<VertexClass>::IndexPair pair(i,j);

    //if ((i == 365) || (j == 365))
    //  fprintf(stderr,"Found edge %d %d connecting %d %d\n",u,v,i,j);

    mIt = this->mNeighborMap.find(pair);

    if (mIt != this->mNeighborMap.end()) {

      if (mComplex.smaller(v,u)) {
        if (mComplex.smaller(u,mIt->second.saddle))
            mIt->second = SaddleStruct(u,u,v);
      }
      else {
        if (mComplex.smaller(v,mIt->second.saddle))
            mIt->second = SaddleStruct(v,u,v);
      }
    }
    else {

      if (mComplex.smaller(u,v))
        this->mNeighborMap[pair] = SaddleStruct(v,u,v);
      else
        this->mNeighborMap[pair] = SaddleStruct(u,u,v);
    }
  }
  

  return 0;
}



#endif
