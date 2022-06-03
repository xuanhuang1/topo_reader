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

#include "Graph.h"

Graph::Graph() : STMappedArray<GraphNode<MorseNode> >(TOPOGRAPH_BLOCK_BITS),
mMinF(gMaxValue), mMaxF(gMinValue), mMaxIndex(0)
{
}


int Graph::addArc(GlobalIndexType i0, GlobalIndexType i1)
{
  NodeType* u;
  NodeType* v;

  //fprintf(stdout," Graph::Adding arc %d -> %d\n",i0,i1);;

  u = this->findElement(i0);
  sterror(u==NULL,"All nodes should be added before adding an arc.");

  v = this->findElement(i1);
  sterror(v==NULL,"All nodes should be added before adding an arc.");

  sterror(u == v,"Graph allows no loops.");

  return addArc(u,v);
}

int Graph::addArc(NodeType* n0, NodeType* n1)
{
  n0->addNeighbor(n1);
  n1->addNeighbor(n0);

  return 1;
}

int Graph::addNode(GlobalIndexType i, FunctionType f, MorseType t)
{
  NodeType* node;

  node = this->findElement(i);
  if (node == NULL) {
    node = this->insertElement(NodeType(i,f,t));
    node->active(true);
    mMinF = MIN(mMinF,node->f());
    mMaxF = MAX(mMaxF,node->f());
  }

  return 1;
}


int Graph::removeNode(NodeType* node)
{
  if (node == NULL)
    return 0;

  if (node->valence() != 0) {
    stwarning("Cannot remove node with active arcs.");
    return 0;
  }

  return this->deleteElement(node->id());
}

int Graph::removeNode(GlobalIndexType i)
{
  NodeType* node;

  node = this->findElement(i);

  if (node == NULL) {
    stwarning("Index not found in array. Cannot remove non-existing node.");
    return 0;
  }

  return removeNode(node);
}


int Graph::removeArc(NodeType* n0, NodeType* n1)
{
  sterror(n0==NULL,"Cannot remove arcs between NULL node pointer.");
  sterror(n1==NULL,"Cannot remove arcs between NULL node pointer.");
  sterror(n0==n1,"Cannot remove illegal loop.");

  if (n1->removeNeighbor(n0) == 0) {
    stwarning("Could not find arc %d -> %d to remove.",n1->id(),n0->id());
    return 0;
  }
  if (n0->removeNeighbor(n1) == 0) {
      stwarning("Could not find arc %d -> %d to remove.",n0->id(),n1->id());
      return 0;
  }


  return 1;
}



