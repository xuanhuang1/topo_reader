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

#ifndef CONTOURTREEBASE_H
#define CONTOURTREEBASE_H

#include <algorithm>
#include "TopoTree.h"
#include "ContourVertex.h"
#include "UnionAlgorithm.h"

//! Contour tree implementation based on concurrent merge/split tree computation
template <class VertexClass = ContourVertex>
class ContourTreeBase : public TopoTree<VertexClass> 
{
public:
  
  //! Default constructor
  ContourTreeBase(TopoGraphInterface* graph);
  
  //! Destructor
  virtual ~ContourTreeBase();

  //! Return the algorithm used for the merge tree computation
  UnionAlgorithm<typename VertexClass::MergeType>* mergeAlgorithm() {return mMergeAlgorithm;}

  //! Return the algorithm used for the merge tree computation
  UnionAlgorithm<typename VertexClass::SplitType>* splitAlgorithm() {return mSplitAlgorithm;}

protected:

  //! Pointer to the merge tree algorithm
  UnionAlgorithm<typename VertexClass::MergeType>* mMergeAlgorithm;

  //! Pointer to the split tree algorithm
  UnionAlgorithm<typename VertexClass::SplitType>* mSplitAlgorithm;

    //! Add the given vertex to the tree
  virtual int addVertexInternal(VertexClass* v);

  //! Add the path path[0]->path[1]-> ... ->path[length-1] to the tree
  virtual int addPathInternal(vector<VertexClass*>& path, uint32_t length);
  
  //! Implementation to add an edge to the tree  
  virtual int addEdgeInternal(VertexClass* v0, VertexClass* v1);

  //! Finalize a vertex
  /*! Mark the given vertex as finalized and process it
   *  accordingly. If edges between finalized critical vertices are
   *  found they are output as arcs.
   */
  virtual int finalizeVertexInternal(VertexClass* v);

  //! Remove an element from the tree and from memory
  virtual void removeElement(VertexClass* v);

  //! Cleanup the roots of the trees
  virtual int cleanupInternal();
};

template <class VertexClass>
ContourTreeBase<VertexClass>::ContourTreeBase(TopoGraphInterface* graph)
  : TopoTree<VertexClass>(graph), mMergeAlgorithm(NULL), mSplitAlgorithm(NULL)
{
}

template <class VertexClass>
ContourTreeBase<VertexClass>::~ContourTreeBase()
{
  if (mMergeAlgorithm != NULL) 
    delete mMergeAlgorithm;

  if (mSplitAlgorithm != NULL) 
    delete mSplitAlgorithm;
};


template <class VertexClass>
int ContourTreeBase<VertexClass>::addVertexInternal(VertexClass* v)
{
  mMergeAlgorithm->add_vertex(v);
  mSplitAlgorithm->add_vertex(v);

  return 1; // For now ignore the return values
}

template <class VertexClass>
int ContourTreeBase<VertexClass>::addPathInternal(vector<VertexClass*>& path, uint32_t length)
{
  typename std::vector<VertexClass*>::const_iterator it;
  VertexClass* v_merge;
  VertexClass* v_split;
  
  // Sort the vertices according to the algorithm we need to use
  std::sort(path.begin(),path.begin()+length,mMergeAlgorithm->cmp());
  
  for (it=path.begin();it!=path.begin()+length-1;it++) {

    // Add each edge in the path to the two interleaved trees
    v_merge = dynamic_cast<VertexClass*>(mMergeAlgorithm->add_edge(*it,*(it+1)));
    v_split = dynamic_cast<VertexClass*>(mSplitAlgorithm->add_edge(*(it+1),*it));

    
    // If necessary re-finalize the given vertices
    if (v_merge != NULL) 
      finalizeVertexInternal(v_merge);
    if (v_split != NULL) 
      finalizeVertexInternal(v_split);
    
  }

  return 1;
}

template <class VertexClass>
int ContourTreeBase<VertexClass>::addEdgeInternal(VertexClass* v0, VertexClass* v1)
{
  VertexClass* v_merge;
  VertexClass* v_split;

  //if ((v0->id() == 9) && (v1->id() == 2))
  //   fprintf(stderr,"break\n");

  if (mMergeAlgorithm->greater(v0,v1)) {

    // Add the edge to the trees 
    v_merge = dynamic_cast<VertexClass*>(mMergeAlgorithm->add_edge(v0,v1));
    v_split = dynamic_cast<VertexClass*>(mSplitAlgorithm->add_edge(v1,v0));
    
    // If necessary re-finalize the given vertices
    if (v_merge != NULL) 
      finalizeVertexInternal(v_merge);
    if (v_split != NULL) 
      finalizeVertexInternal(v_split);

    return 1;
  }
  else {

    // Add the edge to the trees 
    v_merge = dynamic_cast<VertexClass*>(mMergeAlgorithm->add_edge(v1,v0));
    v_split = dynamic_cast<VertexClass*>(mSplitAlgorithm->add_edge(v0,v1));
    
    // If necessary re-finalize the given vertices
    if (v_merge != NULL) 
      finalizeVertexInternal(v_merge);
    if (v_split != NULL) 
      finalizeVertexInternal(v_split);

    return 2;
  }
}
  
template <class VertexClass>
int ContourTreeBase<VertexClass>::finalizeVertexInternal(VertexClass* v)
{
  if (!v->isProcessed()) 
    v->type(v->currentType());

  switch (v->type()) {

  case INTERIOR: {
    break;
  }
  case LEAF: {
    break;
  }
  case RESTRICTED: {
    break;
  }
  case BRANCH: {
    break;
  }
  case ROOT:
    break;
  }

  return 1;
}

template <class VertexClass>
void ContourTreeBase<VertexClass>::removeElement(VertexClass* v)
{
  mMergeAlgorithm->remove_vertex(v);
  mSplitAlgorithm->remove_vertex(v);

  this->mVertices.deleteElement(v);
}


template <class VertexClass>
int ContourTreeBase<VertexClass>::cleanupInternal()
{
  /*
  typename UnionTree<VertexClass>::iterator it;
  GlobalIndexType tmp;
 
  // Mark all remaining nodes as being processed since clearly their
  // types can't change anymore
  for (it=this->begin();it!=this->end();it++) {
    if (!it->isProcessed()) {      
      
      if (it->type() != ROOT) {
        // We expect root nodes to be unfinalized since for a merge
        // tree the only way to be sure we have seen the global root
        // is to wait for all vertices to come in. However, on rare
        // occasions when using decomposed domains interior nodes can
        // be unprocessed as well. This occurs when shared vertices
        // are saddles for some but not all of their incident
        // subdomains. The subdomain in which thie vertex in question
        // will ultimately become regular will not pass on the
        // coresponding node which in turn might be left unprocessed. 
        stwarning("Forced processing of vertex %d   type %d\n",it->id(),it->type());
      }

      this->mGraph->addNode(it->id(),it->data());
      it->process();
    }
  }
 
  it=this->begin();
  while (it!=this->end()) {

    if ((it->child() == NULL) && (it->parent() == NULL)) {
      tmp = it->id();
      this->removeElement(it);      
      this->mGraph->finalizeNode(tmp);
      it = this->begin();
    }
    else if (processArcs(it) > 0) 
      it = this->begin();
    else
      it++;
  }
  */
  return 1;
}


#endif 


