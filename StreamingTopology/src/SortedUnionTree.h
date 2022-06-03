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


#ifndef SORTEDUNIONTREE_H
#define SORTEDUNIONTREE_H

#include <set>
#include "TopoTree.h"
#include "UnionVertex.h"
#include "UnionSegmentation.h"

class SortedUnionTree : public TopoTree<UnionVertex>
{
public:
 
  typedef UnionVertex VertexClass;

  //! Default constructor
  SortedUnionTree(TopoGraphInterface* graph, UnionSegmentation* segmentation = NULL);

  //! Destructor
  virtual ~SortedUnionTree();

protected:
  
  // The comparison function used
  VertexCompare* mGreater;

  //! The array of segmentation indices
  UnionSegmentation*  mSegmentation;

  //! The set of root nodes
  std::set<VertexClass*> mRoots;

  //! Add the given vertex to the tree
  virtual int addVertexInternal(VertexClass* v);

  //! Add the path path[0]->path[1]-> ... ->path[length-1] to the tree
  virtual int addPathInternal(vector<VertexClass*>& path, uint32_t length) {sterror(true,"Deprecated");return 0;}
  
  //! Implementation to add an edge to the tree  
  virtual int addEdgeInternal(VertexClass* v0, VertexClass* v1);

  //! Finalize a vertex
  /*! Mark the given vertex as finalized and process it
   *  accordingly. If edges between finalized critical vertices are
   *  found they are output as arcs.
   */
  virtual int finalizeVertexInternal(VertexClass* v);

  //! Find the union-find representative of v
  VertexClass* getRepresentative(VertexClass* v);

  //! Indicate that all previously undetermined vertices are now known
  virtual int cleanupInternal() {return 1;}
};


#endif
