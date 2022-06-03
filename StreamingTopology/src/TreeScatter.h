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


#ifndef TREESCATTER_H
#define TREESCATTER_H

#include <vector>

#include "TopoTreeInterface.h"
#include "TopoGraphInterface.h"
#include "DomainDecomposition.h"
#include "GenericData.h"

class TreeScatter : public TopoTreeInterface
{
public:

  //! Default constructor
  TreeScatter(const std::vector<TopoTreeInterface*>& consumers,
              DomainDecomposition* decomposition);

  //! Destructor
  ~TreeScatter();

  //! Return the minimal function value of an accepted vertex
  FunctionType minF() const;

  //! Return the maximal function value of an accepted vertex
  FunctionType maxF() const;

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mMaxIndex = MAX(mMaxIndex,id);}
  
  //! Add the given vertex to the tree
  int addVertex(GlobalIndexType id, FunctionType f);
  
  //! Add the given path to the tree
  int addPath(const std::vector<GlobalIndexType>& path);

  //! Add the given edge to the tree and return the order of the input vertices
  int addEdge(GlobalIndexType i0, GlobalIndexType i1);

  //! Finalize the vertex with the given index
  int finalizeVertex(GlobalIndexType index, bool restricted=false);

  //! Determine whether any of the consumers contain this vertex
  bool containsVertex(GlobalIndexType index);
  
  //! Mark the vertex with the given index as unshared
  int markUnshared(GlobalIndexType index);

  //! Indicate that no more vertices or paths are coming
  int cleanup();
  
  //! Set upper bound of accepted function values
  void setUpperBound(double bound);

  //! Set lower bound of accepted function values
  void setLowerBound(double bound);

protected:

  //! Array of consumers
  const std::vector<TopoTreeInterface*>& mConsumers;

  //! The decomposition of the domain
  DomainDecomposition* mDecomposition;

  //! Highest index seen so far
  GlobalIndexType mMaxIndex;
};


#endif
