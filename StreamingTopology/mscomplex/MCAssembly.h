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

#ifndef MCASSEMBLY_H
#define MCASSEMBLY_H

#include "GradientComplexInterface.h"
#include "MCVertex.h"
#include "AssemblyInterface.h"

//! Class that adds edges to a gradient complex in a thread safe manner
/*! Instances of MCAssemblies are used to add edges to an existing gradient
 *  complex in a thread safe manner. This functionality is separated from the
 *  gradient complex itself to allow an arbitrary number of MCAssemblies to
 *  exist and work on the same gradient complex in parallel. 
 */
template <class VertexClass= MCVertex<3,float> >
class MCAssembly : public AssemblyInterface
{
public:

  //! Default constructor
  MCAssembly(GradientComplexInterface<VertexClass>& complex);

  //! Destructor
  virtual ~MCAssembly() {}

  //! Add an edge
  virtual double add_edge(LocalIndexType u, LocalIndexType v);

protected:

  //! Reference to the actual (shared) Morse complex
  GradientComplexInterface<VertexClass>& mComplex;
};


template<class VertexClass>
MCAssembly<VertexClass>::MCAssembly(GradientComplexInterface<VertexClass>& complex) :
  AssemblyInterface(), mComplex(complex)
{
}

template<class VertexClass>
double MCAssembly<VertexClass>::add_edge(LocalIndexType u, LocalIndexType v)
{
  static double slope;

  // Compute the slope from u to v
  slope = mComplex.slope(u,v);
  
  //if ((u == 999) || (v == 999))
  //fprintf(stderr,"Edge %d %d    slope: %f\n",u,v,slope);
    
  // If u is smaller than v than we check whether this edge is steeper than u's
  // previous neighbor
  if (mComplex.smaller(u,v)) {
    
    if (slope > mComplex.vertex(u).stableInfo().slope()) {
      mComplex.lock(u); // Request exclusive access to the slope values
      
      // We must test again since another thread may have updated the slope in
      // between
      if (slope > mComplex.vertex(u).stableInfo().slope()) 
        mComplex.vertex(u).stableInfo().update(slope,v);
      
      
      mComplex.unlock(u); // Unlock the value again and continue
    }
  }
  else { // Otherwise, the negative of the slope is tested against v's steepest
         // neighbor

    if (-slope > mComplex.vertex(v).stableInfo().slope()) {
      mComplex.lock(v);
      
      if (-slope > mComplex.vertex(v).stableInfo().slope()) 
        mComplex.vertex(v).stableInfo().update(-slope,u);
      
      mComplex.unlock(v);
    } 
  }     
    
  return slope;
}




#endif
