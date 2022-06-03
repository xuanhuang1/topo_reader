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

#include "SortedUnionTree.h"


SortedUnionTree::SortedUnionTree(TopoGraphInterface* graph, UnionSegmentation* segmentation)
  : TopoTree<VertexClass>(graph), mGreater(NULL), mSegmentation(segmentation)
{
}

SortedUnionTree::~SortedUnionTree()
{
}

int SortedUnionTree::addVertexInternal(VertexClass* v)
{
  mSegmentation->insert(v->id(),GNULL);

  v->type(ROOT);

  return 1;
}

int SortedUnionTree::addEdgeInternal(VertexClass* v0, VertexClass* v1)
{
  if ((*mGreater)(v0,v1))
    std::swap(v0,v1);

  if (v1->type() == ROOT)
    v1->type(INTERIOR);

  // Now v0 < v1

  // In the following the child link is used to point to the reprensentative
  // node of each branch and implement the standard union find structure.

  // If v1 has no edge attached to it yet, it has no label and thus is a local
  // maximum
  if (v1->representative() == NULL) {

    if (mSegmentation != NULL)
      mSegmentation->insert(v1->id(),v1->id()); // Make v1 into its own contour

    v1->representative(v1); // v1 is its own representative
    v1->lowest(v1); // and its own lowest node

    // Add the corresonding node to the graph
    this->mGraph->addNode(v1->id(),v1->f());

    // Mark v1 as being a critical point
    v1->type(BRANCH);
  }

  // Change v1 into its own representative. Note that this call also shortcuts
  // the union find structure
  v1 = getRepresentative(v1);

  // Remember the lowest vertex for this representative
  v1->lowest(v0);

  sterror(v1!=v1->representative(),"v1 should be its own representative.");

  if (v0->representative() == NULL) {// If v0 has no representative

    // Find the current representative of v1 and asign it as the representative
    // of v0. Note that the getRep call also shortcuts the union-find pointers
    v0->representative(v1);

    // v0 is part of v1's contour
    if (mSegmentation != NULL)
      mSegmentation->insert(v0->id(),v1->id());

    return 1;
  }

  // Shortcut v0's representative
  v0->representative(getRepresentative(v0));

  //fprintf(stderr,"Vertex %d   has rep %d  \n",v0->id(),v0->representative()->id());

  // Setting the segmentation index
  if (mSegmentation != NULL)
    mSegmentation->insert(v0->id(),v0->representative()->id());

  if (v0->representative() != v1) { // If two contours meet

    if (v0->representative() != v0) { // If v0 is not a saddle yet

      if (mSegmentation != NULL)
        mSegmentation->insert(v0->id(),v0->id()); // Make it a saddle and starts its own contour

      // Shortcut the union-find pointers of v0's representative
      v0->representative()->representative(v0);

      // Create the saddle as node in the graph
      this->mGraph->addNode(v0->id(),v0->f());

      // Create the arc between v0's orignal rep and v0
      this->mGraph->addArc(v0->id(),v0->f(),
                           v0->representative()->id(),v0->representative()->f());

      // v0 is now the representative of everybody
      v0->representative(v0);

      // Mark v0 as being a critical point
      v0->type(BRANCH);
    }

    // Create the arc between v0 and v1
    this->mGraph->addArc(v0->id(),v0->f(),
                         v1->representative()->id(),v1->representative()->f());

    // Finally, shortcut the union-find pointers of v1's representative
    v1->representative(v0);
  }

  return 1;
}

int SortedUnionTree::finalizeVertexInternal(VertexClass* v)
{
  sterror(v == NULL,"Cannot finalize NULL vertex.");


  // If this vertex is the root of its branch
  if (v->type() == ROOT) {

    // Find v's representative
    v->representative(getRepresentative(v));

    // If the vertex has no representative it is a lonely node
    if (v->representative() == NULL) {
      // Add the corresponding node
      this->mGraph->addNode(v->id(),v->f());

      v->representative(v);
      v->type(BRANCH);
    }
    // If v is not the lowest vertex of its representative
    else if (v->representative()->lowest() != v) {
       v->type(INTERIOR);
    }
    else { // If v is the lowest vertex we make it into a min

      // Create the saddle as node in the graph
      this->mGraph->addNode(v->id(),v->f());


      // Create the arc between v0's orignal rep and v0
      this->mGraph->addArc(v->id(),v->f(),
                           v->representative()->id(),v->representative()->f());


      v->representative(v);
      v->type(BRANCH);
    }
  }



  if (v->type() != BRANCH) // If this vertex is not a critical point
    this->mVertices.deleteElement(v);

  return 1;
}


UnionVertex* SortedUnionTree::getRepresentative(VertexClass* v)
{
  VertexClass* rep;

  if (v->representative() == NULL)
    return NULL;
  else if (v->representative() == v)
    return v;

  rep = getRepresentative(v->representative());

  v->representative(rep);
  return rep;
}


