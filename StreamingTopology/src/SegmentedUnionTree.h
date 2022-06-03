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


#ifndef SEGMENTEDUNIONTREE_H
#define SEGMENTEDUNIONTREE_H

#include <cstdio>
#include <iostream>

#include <vector>
#include <algorithm>
#include "UnionAlgorithm.h"
#include "UnionTree.h"
#include "OOCArray.h"
#include "UnionSegmentation.h"

using namespace std;

//! Out-of-core implementation of a segmented merge- or split tree
/*! This class implements the a straight forward out-of-core type
 *  segmented merge tree. Each vertex stores a global index indicating
 *  the index of the higher critical point of its arc (higher being
 *  dependend on merge or split-tree of course). The corresponding
 *  array will be stored as OOCArray in the order given by the vertex
 *  id's. The implementation is almost identical to UnionTree except
 *  that a new version of the mergePath algorithm is needed to handle
 *  the indices and the finalization call must store the index
 */
template <class VertexClass = UnionVertex>
class SegmentedUnionTree : public UnionTree<VertexClass> 
{
public:

  typedef GlobalIndexType VertexInfoType;

  //! Default constructor
  SegmentedUnionTree(TopoGraphInterface* graph, UnionSegmentation* segmentation);
  
  //! Destructor
  virtual ~SegmentedUnionTree();

  //! Add the given vertex to the tree
  virtual int addVertex(GlobalIndexType id, FunctionType f);

  //! Add the given vertex to the tree
  virtual int addVertex(const VertexClass& v);

  //! Write the current tree to stdout
  virtual void printTree();

  //! Overloaded call to remove a vertex and save its current segmentation id
  virtual int deleteElement(VertexClass* v);

private:

  //! The array of segmentation indices
  UnionSegmentation* mSegmentation;

  //! Update and shortcut the segmentation index if possible
  void updateSegmentationIndex(VertexClass* v);
};

template <class VertexClass>
SegmentedUnionTree<VertexClass>::SegmentedUnionTree(TopoGraphInterface* graph,UnionSegmentation* segmentation)
  : UnionTree<VertexClass>(graph), mSegmentation(segmentation)
{ 
}

template <class VertexClass>
SegmentedUnionTree<VertexClass>::~SegmentedUnionTree()
{
}


template <class VertexClass>
int SegmentedUnionTree<VertexClass>::addVertex(const VertexClass& v)
{
  int success;

  //if (id == 63766)
  //  fprintf(stderr,"break\n");

  success = TopoTree<VertexClass>::addVertex(v);

  // IF this vertex was culled we indicate this in the segmentation by a GNULL
  if (!success)
    mSegmentation->insert(v.id(),GNULL);

  return success;
}

template <class VertexClass>
int SegmentedUnionTree<VertexClass>::addVertex(GlobalIndexType id, FunctionType f)
{
  int success;
  
  //if (id == 63766)
  //  fprintf(stderr,"break\n");

  success = TopoTree<VertexClass>::addVertex(id,f);

  // IF this vertex was culled we indicate this in the segmentation by a GNULL
  if (!success)
    mSegmentation->insert(id,GNULL);

  return success;
}


template <class VertexClass>
void SegmentedUnionTree<VertexClass>::updateSegmentationIndex(VertexClass* v)
{
  static vector<VertexClass*> path;
  typename vector<VertexClass*>::iterator it;
  VertexClass* up;

  sterror(v->segIndex().index()==v->segIndex().segNull(),"Vertex with unassigned segmentation index.");
  
  if (v->type() == ROOT) {
    v->segIndex().index(v->id());
    return;
  }

  path.clear();
  path.push_back(v);

  // While we have not reach a leaf
  while (path.back()->segIndex().index() != path.back()->id()) {

    // See whether the next index is still in memory
    up = this->mVertices.findElement(path.back()->segIndex().index());

    // If the next element has been flushed already
    if (up == NULL) 
      break; // We cannot go on
    else
      path.push_back(up);
  }

  // Now shortcut all vertices along the path
  for (it=path.begin();it!=path.end()-1;it++) 
    (*it)->segIndex().index(path.back()->segIndex().index());

}

template <class VertexClass>
void SegmentedUnionTree<VertexClass>::printTree()
{
  typename STMappedArray<VertexClass>::iterator it;
  
  fprintf(stdout,"\n\nPrintTree:\n");
  for (it=this->mVertices.begin();it!=this->mVertices.end();it++) {
    if (it->child() != NULL) {
      std::cout << "Node " << it->id() << "," << it->segIndex().index() << " -> "
          << it->child()->id() << "," << static_cast<VertexClass*>(it->child())->segIndex().index() << "\n";
    }
    //else
    //  fprintf(stdout,"Node %d -> NULL\n",it->id());
      
  }
}


template <class VertexClass>
int SegmentedUnionTree<VertexClass>::deleteElement(VertexClass* v)
{
  // Determine the latest segmentation index
  updateSegmentationIndex(v);
  
  // Store the segmentation index
  mSegmentation->insert(v->id(),v->segIndex().index());
  
  return this->mVertices.deleteElement(v);
};


#endif
