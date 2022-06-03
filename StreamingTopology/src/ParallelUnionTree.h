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

#ifndef PARALLELUNIONTREE_H_
#define PARALLELUNIONTREE_H_


#include "UnionTree.h"

//! Extension of a UnionTree to enabled parallel computation
/*! This class implements the standard merge- or split tree
 * algorithm but in a parallel settings. Each vertex is expected
 * to provide its codimension and multiplicity which is used to
 * correctly handle finalization information for multiple copies
 * as well as ensuring that critical points restricted to the
 * boundary are preserved as valence two nodes in the graph.
 */
template <class VertexClass = UnionVertex>
class ParallelUnionTree : public UnionTree<VertexClass>
{
public:

  //! Default constructor
  ParallelUnionTree(TopoGraphInterface* graph);

  //! Destructor
  virtual ~ParallelUnionTree() {}

  //! Add the given vertex to the tree
  int addVertex(GlobalIndexType id, FunctionType f);

  //! Finalize the vertex with the given index
  virtual int finalizeVertex(GlobalIndexType index, bool restricted=false);

  //! Indicate that no more vertices or paths are coming
  virtual int cleanup();

protected:

  //! Finalize a vertex
  /*! Mark the given vertex as finalized and process it
   *  accordingly. If edges between finalized critical vertices are
   *  found they are output as arcs.
   */
  virtual int finalizeVertexInternal(VertexClass* v, bool restricted=false);

  int processArcs(VertexClass* v);
};


template <class VertexClass>
ParallelUnionTree<VertexClass>::ParallelUnionTree(TopoGraphInterface* graph)
  : UnionTree<VertexClass>(graph)
{
}

template <class VertexClass>
int ParallelUnionTree<VertexClass>::addVertex(GlobalIndexType id, FunctionType f)
{
  this->mMaxIndex = std::max(this->mMaxIndex,id);

  //if (id % 10000 == 0)
  //  fprintf(stderr,"Current tree size %d\n",this->elementCount());

  if ((f < this->mLowerBound) || (f > this->mUpperBound))
    return 0;

  //if (id == 90) {
  //  fprintf(stderr,"break\n");
  //}

  this->mMaxF = MAX(this->mMaxF,f);
  this->mMinF = MIN(this->mMinF,f);


  // In a parallel computation a vertex might encounter its clones
  VertexClass *v,*vv;

  v = this->mVertices.findElement(id);
  if (v == NULL) {// If if this is the first copy
    vv = this->mVertices.insertElement(VertexClass(id,f));

    // IMPORTANT !! It is important to remember that the next pointers
    // of the VertexClass must be reset now to their "initial" stage of
    // pointing to itself. Only the previous assignment actually copied
    // the data to its final position so we could not have taken care of
    // this before
    vv->initializeNext();

    return this->addVertexInternal(vv);
  }

  return 1;
}


template<class VertexClass>
int ParallelUnionTree<VertexClass>::finalizeVertex(GlobalIndexType id, bool restricted)
{
  VertexClass* v;

  v = this->mVertices.findElement(id);
  if (v != NULL) {
    return finalizeVertexInternal(v,restricted);
  }

#ifdef ST_COMPLETE_DOMAIN
  sterror(true,"Index to finalize not found. Ignoring finalization.");
#endif

  return 0;
}

template<class VertexClass>
int ParallelUnionTree<VertexClass>::cleanup()
{
  typename STMappedArray<VertexClass>::iterator it;
  GlobalIndexType id;

  // First we tell the graph the highest index we have seen
  this->mGraph->maxIndex(this->mMaxIndex);

  // We must be careful here since the finalizeVertex call might
  // remove the vertex from the array which may invalidate the
  // iterator
  it = this->mVertices.begin();
  while (it!=this->mVertices.end()) {
    if (!it->isFinalized()) {
      // We want to finalize it

      fprintf(stderr,"Forced finalization of vertex %d   type %d\n",it->id(),it->type());

      // First store the index we must finalize
      id = it->id();

      // Then look for the next element that is *not*
      // finalized. Elements that are not finalized will never be
      // removed which makes their iterator save
      it++;
      while ((it != this->mVertices.end()) && it->isFinalized())
        it++;

      // Finally, finalize the perviously unfinalized vertex
      finalizeVertex(id);
    }
    else {
      it++;
    }
  }

  // Finally, cleanup any left overs from whatever algorithm was used
  return this->cleanupInternal();
}


template <class VertexClass>
int ParallelUnionTree<VertexClass>::finalizeVertexInternal(VertexClass* v, bool restricted)
{
  typename VertexClass::iterator it;

  sterror(v == NULL,"Cannot finalize NULL vertex.");


  // First we set the type of the vertex according to the current pointers
  if (!v->isProcessed()) {
    v->type(v->currentType());

    // If v is an interior vertex but it's boundary markers are not
    // contained in its parents boundary markers. The v might be a
    // local maximum restricted to at least one of its boundary markers
    // and thus must be preserved
    //if ((v->type() == INTERIOR) && !(v->boundary() < v->parent()->boundary()))
   //   v->type(RESTRICTED);
  }

  //if (v->id() == 167379)
  //  fprintf(stderr,"break %d\n",v->type());

  switch (v->type()) {

  case INTERIOR: {
    // If the vertex is regular we remove it from the tree and retire it

    // The static cast is needed for derive vertex classes
    VertexClass* parent = static_cast<VertexClass*>(v->parent());

    if (parent->id() == 64125)
      fprintf(stderr,"break\n");

    // Unlink v from the tree
    v->bypass();

    // Release its memory
    this->removeElement(v);

    // If v's child used to be restricted it will be no longer if parent
    // has an equal or higher codimension. All such children can be
    // bypassed (they must be INTERIOR as well) and removed. Note that
    // parent->child() should be != NULL at all times since at some point
    // we must hit a BRANCH or a ROOT before running out of children.
    while (parent->child()->type() == RESTRICTED) {

      v = static_cast<VertexClass*>(parent->child());

      v->bypass();
      this->removeElement(v);
    }

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
  case BRANCH: {
    // If we are at a saddle critical point or a restricted interior point
    // v might be the tail of an arc and we test all parents to see
    // whether they might be the head of a new arc.

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
int ParallelUnionTree<VertexClass>::processArcs(VertexClass* v)
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


    if (v->child()->type() == RESTRICTED) {

      finalizeVertexInternal(static_cast<VertexClass*>(v->child()));
    }
    else {

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
      this->removeElement(v);

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
  }

  return arc_count;
}




#endif /* PARALLELUNIONTREE_H_ */
