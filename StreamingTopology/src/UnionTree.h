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


#ifndef UNIONTREE_H
#define UNIONTREE_H

#include <vector>
#include <algorithm>
#include "UnionAlgorithm.h"
#include "UnionVertex.h"
#include "TopoTree.h"

using namespace std;

//! General implementation of a merge- or split tree
/*! This class implements a merge- or split tree using the common
 *  interface of MergeVertex and SplitVertex. It implements two
 *  functions: addPath sorts the input vertices from leafs to the root
 *  and calls mergePath; finalizeVertex deletes vertices and writes
 *  out arcs of the resulting tree
 */
template <class VertexClass = UnionVertex>
class UnionTree : public TopoTree<VertexClass>
{
public:

  //! Default constructor
  UnionTree(TopoGraphInterface* graph);

  //! Destructor
  virtual ~UnionTree();

  UnionAlgorithm<VertexClass>* algorithm() {return mAlgorithm;}

  virtual void printTree();

protected:
  
  //! Pointer to the appropriate union algorithm
  UnionAlgorithm<VertexClass>* mAlgorithm;

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

  //! Write and finalize the arc v -> v->child()
  virtual int processArcs(VertexClass* v);

  //! Cleanup the root of the tree
  virtual int cleanupInternal();

};


template <class VertexClass>
UnionTree<VertexClass>::UnionTree(TopoGraphInterface* graph)
  : TopoTree<VertexClass>(graph), mAlgorithm(NULL)
{
}

template <class VertexClass>
UnionTree<VertexClass>::~UnionTree()
{
  if (mAlgorithm != NULL) 
    delete mAlgorithm;
}


template <class VertexClass>
int UnionTree<VertexClass>::addVertexInternal(VertexClass* v)
{
  return mAlgorithm->add_vertex(v);
}

template <class VertexClass>
int UnionTree<VertexClass>::addPathInternal(vector<VertexClass*>& path, uint32_t length)
{
  typename std::vector<VertexClass*>::const_iterator it;
  VertexClass* v;
  
  // Sort the vertices according to the algorithm we need to use
  sort(path.begin(),path.begin()+length,mAlgorithm->cmp());
  
  for (it=path.begin();it!=path.begin()+length-1;it++) {

    // Add each edge in the path to the tree
    v = mAlgorithm->add_edge(*it,*(it+1));
    
    // If necessary re-finalize a vertex
    if (v != NULL) {
      finalizeVertexInternal(v);
    }
  }

  return 1;
}

template <class VertexClass>
int UnionTree<VertexClass>::addEdgeInternal(VertexClass* v0, VertexClass* v1)
{
  VertexClass* v;

  //if ((v0->id() == 86656) && (v1->id() == 86672))
  //  fprintf(stderr,"break\n");

  if (mAlgorithm->greater(v0,v1)) {

    // Add the edge to the tree according to the algorithm we use
    v = mAlgorithm->add_edge(v0,v1);
    
    // If necessary re-finalize a vertex
    if (v != NULL) {
      //fprintf(stderr,"Re-finalizing vertex %d\n",v->id());
      finalizeVertexInternal(v);
    }
    return 1;
  }
  else {

    // Add the edge to the tree according to the algorithm we use
    v = mAlgorithm->add_edge(v1,v0);
    
    // If necessary re-finalize a vertex
    if (v != NULL) {
      //fprintf(stderr,"Re-finalizing vertex %d\n",v->id());
      finalizeVertexInternal(v);
    }
    return 2;
  }
}
  
template <class VertexClass>
int UnionTree<VertexClass>::finalizeVertexInternal(VertexClass* v)
{
  typename VertexClass::iterator it;
  
  sterror(v == NULL,"Cannot finalize NULL vertex."); 

  //fprintf(stderr,"finalizeVertexInternal %d\n",v->id());
	//if (v->id() == 26)
  //fprintf(stderr,"break\n");

  // First we set the type of the vertex according to the current pointers
  if (!v->isProcessed()) {
    v->type(v->currentType());

    // If this is an interior vertex which must be preserved
    if ((v->type() == INTERIOR) && v->isRestricted())
      v->type(RESTRICTED); // We mark is as restricted
  }



  //if (v->id() == 167379) 
  //  fprintf(stderr,"break %d\n",v->type());

  switch (v->type()) {

  case INTERIOR: {
    // If the vertex is regular we remove it from the tree and retire it

    // The static cast is needed for derive vertex classes
    VertexClass* parent = static_cast<VertexClass*>(v->parent());
    
    // Unlink v from the tree
    v->bypass();

    // Release its memory
    removeElement(v);
    
    // If we might have created a leaf arc
    if (parent->isLeaf() && parent->isFinalized()) 
      processArcs(parent);
    
    break;
  }
  case LEAF: // If v is a leaf of the tree
    
    // A finalized leaf will remain a leaf
    v->process();

    // Make sure that even lonely nodes are part of the tree
    this->mGraph->addNode(v->id(),v->f());

    // v itself might be the head of an arc
    processArcs(v);

    break;
  case RESTRICTED:
  case BRANCH: {// If we are at a saddle critical point 
    // v might be the tail of an arc and we test all parents to see
    // whether they might be the head of a new arc. Note that even though in 
   
    // Get all parents in a vector to avoid complications when editing
    // the parent lists
    vector<UnionVertex*> parents;

    v->parents(parents);
      
    for (unsigned int i=0;i<parents.size();i++) {
      
      // If this parent is finalized and a leaf of the tree
      if (parents[i]->isProcessed()) 
        processArcs(static_cast<VertexClass*>(parents[i]));
    }

    break;
  }
  case ROOT:
    break;
  default:
    sterror(true,"Unkown vertex type %d or vertex %d.",v->type(),v->id());
    break;
  }
  
  return 1;
}

template <class VertexClass>
void  UnionTree<VertexClass>::removeElement(VertexClass* v)
{
  mAlgorithm->remove_vertex(v);

  this->deleteElement(v);
}




template <class VertexClass>
int UnionTree<VertexClass>::processArcs(VertexClass* v)
{
  sterror(!v->isFinalized(),"Only finalized vertices can be parts of arcs.");
  
  VertexClass* tmp_child;
  GlobalIndexType tmp;
  bool restricted;
  int arc_count = 0;

  // While v is a leaf that points to a finalized saddle we output the
  // arc. Note that by construction the child can never be a leaf of
  // the tree. This leaves it being regular, a saddle, a restricted
  // interior vertex, or a root. A regular vertex would have been removed
  // already. A root provides no information since it can become
  // regular whether it is finalized or not. Therefore, the only time
  // we found a guaranteed arc is if the child is a saddle or a
  // restricted interior vertex. However, this is less obvious than it
  // looks. We know that finalized saddles as well as restricted vertices
  // can become regular. However, this is
  // not the case if they are connected to a completely finalized leaf
  // branch of the tree. Since currently there exist at least two
  // branches and one of them is finalized the child must always
  // remain a saddle.
  while (v->isLeaf() && (v->child() != NULL) 
         && ((v->child()->isFinalized() && (v->child()->type() != ROOT)) 
             || v->child()->isProcessed())) {
    
     sterror(!v->isProcessed(),"We should not get here without having processed v.");

    // If v->child() has not been processed yet, it is so now. We also
    // use this opportunity to add the critical point to the graph
    if (!v->child()->isProcessed()) {
      this->mGraph->addNode(v->child()->id(),v->child()->f());
      v->child()->process();
    }
      
    arc_count++;

    // Add the corresponding arc to the graph
    this->mGraph->addArc(v->id(),v->f(),v->child()->id(),v->child()->f());
    
    // Store the information of the child
    tmp_child = static_cast<VertexClass*>(v->child());

    // Break the link 
    v->child(NULL);

    // Store the index 
    tmp = v->id();
    // and the restricted flag
    restricted = v->isRestricted();

    // Remove the child from memory
    removeElement(v);
    
    // It is important to remember that must *first* delete the
    // element from the local tree and *then* finalize the
    // corresponding node downstream. in case of decomposed domains
    // and when computing segmentation information the downstream tree
    // and this tree share the same segmentation array. In both cases
    // the finalize/deleteElement call causes the segmentation
    // informatin to be updated. Since the downstream tree has more
    // information it is important that it adds its information last

    // This leaf has no more arcs and thus will never be used again
    this->mGraph->finalizeNode(tmp,restricted);

    v = tmp_child;
  }
  
  return arc_count;
}

template <class VertexClass>
int UnionTree<VertexClass>::cleanupInternal()
{
  typename STMappedArray<VertexClass>::iterator it;
  GlobalIndexType tmp;
  bool restricted;
 
  // Mark all remaining nodes as being processed since clearly their
  // types can't change anymore
  for (it=this->mVertices.begin();it!=this->mVertices.end();it++) {
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
        stwarning("Forced processing of vertex %d   type %d  \n",it->id(),it->type());
      }

      this->mGraph->addNode(it->id(),it->f());
      it->process();
    }
  }
 
  it=this->mVertices.begin();
  while (it!=this->mVertices.end()) {

    if ((it->child() == NULL) && (it->parent() == NULL)) {
      tmp = it->id();
      restricted = it->isRestricted();
      removeElement(it);
      this->mGraph->finalizeNode(tmp,restricted);
      it = this->mVertices.begin();
    }
    else if (processArcs(it) > 0) 
      it = this->mVertices.begin();
    else
      it++;
  }
  
  return 1;
}


template <class VertexClass>
void UnionTree<VertexClass>::printTree()
{
  typename STMappedArray<VertexClass>::iterator it;
  
  fprintf(stdout,"\n\nPrintTree:\n");
  for (it=this->mVertices.begin();it!=this->mVertices.end();it++) {
    if (it->child() != NULL) {
      //fprintf(stdout,"Node %d -> %d\n",it->id(),it->child()->id());
      fprintf(stdout,"Node %s",it->toString());
      fprintf(stdout," -> %s\n",it->child()->toString());
    }
    //else
    //  fprintf(stdout,"Node %d -> NULL\n",it->id());
  }
}

#endif
