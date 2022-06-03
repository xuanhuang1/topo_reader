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

#ifndef STABLEMANIFOLDASSEMBLY_H
#define STABLEMANIFOLDASSEMBLY_H

#include "ComplexAssembly.h"

template <class VertexClass= MCVertex<3,float> >
class StableComplexAssembly : public ComplexAssembly<VertexClass>
{
public:

  using ComplexAssembly<VertexClass>::mComplex;

  typedef typename ComplexAssembly<VertexClass>::SaddleStruct SaddleStruct;
  typedef typename map<typename ComplexAssembly<VertexClass>::IndexPair,SaddleStruct>::iterator MapIterator;
  
  StableComplexAssembly(const GradientComplexInterface<VertexClass>& complex) : ComplexAssembly<VertexClass>(complex) {}

  //! Destructor
  virtual ~StableComplexAssembly() {}

  //! Add an edge
  virtual double add_edge(LocalIndexType u, LocalIndexType v);
};

template <class VertexClass>
double StableComplexAssembly<VertexClass>::add_edge(LocalIndexType u, LocalIndexType v)
{
  LocalIndexType i,j;
  MapIterator mIt;

  //fprintf(stdout," StableComplexAssembly<VertexClass>::add_edge %d -> %d\n",u,v);

  // Get the stable manifold indices of the two vertices
  i = mComplex.vertex(u).stableInfo().extremum();
  j = mComplex.vertex(v).stableInfo().extremum();

  // If we found an edge that connects two stable manifolds
  if (i != j) {

    if (i > j) {
      swap(i,j);
      swap(u,v);
    }

    typename ComplexAssembly<VertexClass>::IndexPair pair(i,j);
    
    // We see whether we have seen this combination before
    mIt = this->mNeighborMap.find(pair);

    // If we have seen this index pair before we have stored the pair with the higher
    // lowest vertex
    if (mIt != this->mNeighborMap.end()) {

      // If u is the potential saddle
      if (mComplex.smaller(u,v)) {
        if (mComplex.smaller(mIt->second.saddle,u)) // And it is higher than the current saddle
            mIt->second = SaddleStruct(u,u,v); // It become the new saddle
      }
      else { // Other wise we do the same check with v
        if (mComplex.smaller(mIt->second.saddle,v))
            mIt->second = SaddleStruct(v,u,v);
      }
    }
    else { // Otherwise, we create a new "saddle"
      
      //if ((i==108) || (j==108) )
      //  fprintf(stderr,"Adding max pair %d %d at edge %d %d\n",i,j,u,v);

      // Using the smaller of the two vertices as saddle
      if (mComplex.smaller(u,v))
        this->mNeighborMap[pair] = SaddleStruct(u,u,v);
      else
        this->mNeighborMap[pair] = SaddleStruct(v,u,v);
    }
  }
  

  return 0;
}



#endif
