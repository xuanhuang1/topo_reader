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


#ifndef TOPOTREE_H
#define TOPOTREE_H

#include <map>
#include <vector>
#include <iostream>

#include "Definitions.h"
#include "TopoTreeInterface.h"
#include "TopoGraphInterface.h"
#include "STMappedArray.h"
#include "Vertex.h"
#include "Node.h"
#include "UnionVertex.h"
#include "BlockedArray.h"

//#define ST_COMPLETE_DOMAIN 1

using namespace std;


//! Common base class for contour/merge/split trees
/*! A TopoTree is the common base class of all merge-, split-, and
 *  contour trees. Its main purpose is to define a common interface
 *  and to collect all memory managment in one place. In particular,
 *  the TopoTree implements the the mapping from global to local index
 *  space, manages the dynamic array, and keeps track of the minimal
 *  and maximal function value seen so far.
 */
template <class VertexClass = UnionVertex>
class TopoTree : public TopoTreeInterface
{
public:
  
  //! Maximal number of vertices allowed in a path
  static const int sMaxPathLength = 20;

  //! Default constructor
  /*! This is the default and only constructor. A TopoTree is only
   *  usefull as a means to compute a TopoGraph. Making only this
   *  constructor available enforces this constraint.
   *  @param graph: reference to the topological graph that should 
   *                be build
   *  @param compact: if true the incoming indices will be re-mapped
   *                  to their order and the reverse map will be 
   *                  stored
   */
  TopoTree(TopoGraphInterface* graph);

  //! Destructor
  virtual ~TopoTree() {}

  /********************************************************************
   ************        Public Interface   *****************************
   *******************************************************************/
  
  //! Return the minimal function value of an accepted vertex
  FunctionType minF() const {return mMinF;}

  //! Return the maximal function value of an accepted vertex
  FunctionType maxF() const {return mMaxF;}

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mMaxIndex = MAX(mMaxIndex,id);}

  //! return a pointer to the given vertex or NULL if no such vertex exists
  VertexClass* findVertex(GlobalIndexType id) {return mVertices.findElement(id);}
  
  //! Add the given vertex to the tree
  virtual int addVertex(GlobalIndexType id, FunctionType f);

  //! Add the given vertex to the tree
  virtual int addVertex(const VertexClass& v);

  //! Add the given path to the tree
  /*! This function add the given path to the tree using the virtual
   *  addPath interface function. First, all input id's are mapped to
   *  local pointers. If one or multiple of the input ids are not
   *  present in the tree (for example, because their values have been
   *  filtered) the path will be ignored and a 0 will be returned. For
   *  applications where all vertices *must* be present the compiler
   *  flag ST_COMPLETE_DOMAIN can be set to trigger an exception if an
   *  id is not found. If all ids can be mapped the virtual addPath
   *  function is called to incorporate the path into the current
   *  tree. All vertices of the path are assumed to be part of a
   *  *single* path in the tree.
   *  @return 1 if the path was successfully integrated; 0 otherwise.
   */
  int addPath(const vector<GlobalIndexType>& path);
  
  //! Add the given edge to the tree and sort the input indices
  virtual int addEdge(GlobalIndexType i0, GlobalIndexType i1);

  //! Finalize the vertex with the given index
  /*! This function will map the given global index to the appropriate
   *  vertex and call the virtual finalize function to process the
   *  actual event. In case the id is not found the finalization call
   *  will be ignored and 0 will be returned.  For applications where
   *  all vertices *must* be present the compiler flag
   *  ST_COMPLETE_DOMAIN can be set to trigger an exception if an id
   *  is not found. Finalizing a vertex indicates that no future paths
   *  will contain this vertex.
   *  @param index: global index of the vertex that should be finalized
   *  @param restricted: whether this vertex must be maintained in the
   *                     tree independent of whether it is critical
   */
  virtual int finalizeVertex(GlobalIndexType index, bool restricted=false);

  //! Determine whether the tree contains this vertex
  /*! Return whether the tree *currently* contains the vertex
   *  corresponding to the given index. 
   */
  virtual bool containsVertex(GlobalIndexType index);

  //! Indicate that no more vertices or paths are coming
  virtual int cleanup();

  //! Set upper bound of accepted function values
  /*! In many applications it is useful, for example, for performance
   *  reasons to ignore function values above/below a certain
   *  value. Vertices above the limit will not be added to the array
   *  and all path or finalization calls directed to them will be
   *  ignored. If vertices are filtered the ST_COMPLETE_DOMAIN should
   *  *not* be set as it will trigger an exception for missing
   *  ids. This function sets the upper bound which will ignore all
   *  vertices with function values strictly above the given bound
   *  @param bound: the new upper bound of acceptable function values
   */
  void setUpperBound(double bound) {mUpperBound = bound;}

  //! Set lower bound of accepted function values
  /*! In many applications it is useful, for example, for performance
   *  reasons to ignore function values above/below a certain
   *  value. Vertices above the limit will not be added to the array
   *  and all path or finalization calls directed to them will be
   *  ignored. If vertices are filtered the ST_COMPLETE_DOMAIN should
   *  *not* be set as it will trigger an exception for missing
   *  ids. This function sets the lower bound which will ignore all
   *  vertices with function values strictly below the given bound
   *  @param bound: the new lower bound of acceptable function values
   */
  void setLowerBound(double bound) {mLowerBound = bound;}

  //! Free the memory of an element
  virtual int deleteElement(VertexClass* v) {return mVertices.deleteElement(v);}

protected:

  /********************************************************************
   *******************  Functional Interface **************************
   *******************************************************************/
  
  //! Add the given vertex to the tree
  /*! This function is call *after* a vertex has been created and
   *  entered into the global array. It allows derived classes to
   *  further initialize a vertex if necessary.
   *  @param v: Pointer to the vertex that has been added
   *  @return 1 if successful; 0 otherwise;
   */
  virtual int addVertexInternal(VertexClass* v) = 0;

  //! Function stub to add a path to be overload by each specific tree  
  /*! This function should implement each specific algorithm to ass
   *  paths to a topological tree. The length of the path is passed
   *  explicitly to all the path vector to be of static size which
   *  avoids significant overheads in allocating and re-allocating
   *  memory.
   *  @param path: array of vertices that form a path in the tree
   *  @param length: number of vertices in the path
   *  @return 1 if successful; 0 otherwise
   */
  virtual int addPathInternal(std::vector<VertexClass*>& path, uint32_t length) = 0;
  
  //! Function stub to add an edge to be overload by each specific tree  
  /*! This function should implement each specific algorithm to add
   *  edges to a topological tree. The assumption is that v0!=v1 are
   *  pointers to two vertices forming an edge. The function will
   *  return 1 if v0 is "higher" (closer to the leafs) than v1; 2 if
   *  v0 is "lower" (closer to the root) than v1; and 0 if there was a
   *  problem adding the edge
   *  @param v0: pointer to the first vertex of the edge
   *  @param v1: pointer to the second vertex of the edge
   *  @return  1 if v0 > v1; 2 if v1 > v2; 0 if the edge could not be 
   *           added
   */
  virtual int addEdgeInternal(VertexClass* v0, VertexClass* v1) = 0;
  
  //! Function stub to finalize a vertex be overload by each specific tree
  virtual int finalizeVertexInternal(VertexClass* v) = 0;
  
  //! Indicate that all previously undetermined vertices are now known
  virtual int cleanupInternal() = 0;

protected:
  
  //! Array containing all unfinalized vertices
  STMappedArray<VertexClass> mVertices;

  //! Pointer to the resulting graph
  TopoGraphInterface* mGraph;

  //! Maximal function value seen so far
  FunctionType mMaxF;

  //! Minimal function value seen so far
  FunctionType mMinF;


  //! Upper bound of acceptable function values
  double mUpperBound;

  //! Lower bound of acceptable function values
  double mLowerBound;  

  //! Pointer to all vertices of a path. 
  /*! Member variable to avoid continuous allocation and deallocation
   *  of the path in the addPathInternal function. For the same reason
   *  this path has at most sMaxPathLength many elements and will not
   *  get resized. 
   */
  vector<VertexClass*> mPath;

  //! Highest incoming index seen so far
  GlobalIndexType mMaxIndex;
};

template<class VertexClass>
TopoTree<VertexClass>::TopoTree(TopoGraphInterface* graph) :
  TopoTreeInterface(), mVertices(TOPOTREE_BLOCK_BITS), mGraph(graph),
  mMaxF(gMinValue), mMinF(gMaxValue), mUpperBound(gMaxValue), mLowerBound(gMinValue), 
  mPath(sMaxPathLength), mMaxIndex(0)
{
}

template<class VertexClass>
int TopoTree<VertexClass>::addVertex(GlobalIndexType id, FunctionType f)
{
  VertexClass* v;

  mMaxIndex = std::max(mMaxIndex,id);

  //if (id % 10000 == 0)
  //  fprintf(stderr,"Current tree size %d\n",this->elementCount());

  if ((f < mLowerBound) || (f > mUpperBound))
    return 0;

  //if (id == 90) {
  //  fprintf(stderr,"break\n");
  //}

  mMaxF = MAX(mMaxF,f);
  mMinF = MIN(mMinF,f);

  v = mVertices.insertElement(VertexClass(id,f));

  // IMPORTANT !! It is important to remember that the next pointers
  // of the VertexClass must be reset now to their "initial" stage of
  // pointing to itself. Only the previous assignment actually copied
  // the data to its final position so we could not have taken care of
  // this before
  v->initializeNext();
  
  return addVertexInternal(v);
}

template<class VertexClass>
int TopoTree<VertexClass>::addVertex(const VertexClass& v)
{
  VertexClass* vv;

  mMaxIndex = std::max(mMaxIndex,v.id());

  //if (id % 10000 == 0)
  //  fprintf(stderr,"Current tree size %d\n",this->elementCount());

  if ((v.f() < mLowerBound) || (v.f() > mUpperBound))
    return 0;

  //if (id == 90) {
  //  fprintf(stderr,"break\n");
  //}

  mMaxF = MAX(mMaxF,v.f());
  mMinF = MIN(mMinF,v.f());

  vv = mVertices.insertElement(v);

  // IMPORTANT !! It is important to remember that the next pointers
  // of the VertexClass must be reset now to their "initial" stage of
  // pointing to itself. Only the previous assignment actually copied
  // the data to its final position so we could not have taken care of
  // this before
  vv->initializeNext();

  return addVertexInternal(vv);
}


template<class VertexClass>
int TopoTree<VertexClass>::addPath(const vector<GlobalIndexType>& path)
{
  vector<GlobalIndexType>::const_iterator it;
  typename vector<VertexClass*>::iterator it2;

  it2 = mPath.begin();
  for (it=path.begin();it!=path.end();it++) {
    *it2 = mVertices.findElement(*it);

#ifdef ST_COMPLETE_DOMAIN
    sterror(*it2 == NULL,"Cannot find vertex %lD when adding path.",(int64_t)(*it));
#endif

    if (*it2 == NULL)
      return 0;
    
    it2++;
  }

  return addPathInternal(mPath,path.size());
}
  
template<class VertexClass>
int TopoTree<VertexClass>::addEdge(GlobalIndexType i0, GlobalIndexType i1)
{
  static VertexClass* edge[2];

  edge[0] = mVertices.findElement(i0);

#ifdef ST_COMPLETE_DOMAIN
    sterror(edge[0] == NULL,"Cannot find vertex %lD when adding path.",(int64_t)i0);
#endif
  if (edge[0] == NULL)
    return 0;

  edge[1] = mVertices.findElement(i1);

#ifdef ST_COMPLETE_DOMAIN
    sterror(edge[1] == NULL,"Cannot find vertex %lD when adding path.",(int64_t)i1);
#endif
  if (edge[1] == NULL)
    return 0;

  return addEdgeInternal(edge[0],edge[1]);
}
  

template<class VertexClass>
int TopoTree<VertexClass>::finalizeVertex(GlobalIndexType id, bool restricted)
{
  VertexClass* v;

  v = mVertices.findElement(id);
  if (v != NULL) {
    //static_cast<Vertex<typename VertexClass::DataType>*>(v)->type(v->type());
    v->finalize();

    if (restricted)
      v->restrict();

    return finalizeVertexInternal(v);
  }
  
#ifdef ST_COMPLETE_DOMAIN
  sterror(true,"Index %ld to finalize not found. Ignoring finalization.",id);
#endif

  return 0;
}

template<class VertexClass>
bool TopoTree<VertexClass>::containsVertex(GlobalIndexType index)
{
  if (mVertices.findElementIndex(index) == LNULL)
    return false;
  else
    return true;
}

template<class VertexClass>
int TopoTree<VertexClass>::cleanup()
{
  typename STMappedArray<VertexClass>::iterator it;
  GlobalIndexType id;

  // First we tell the graph the highest index we have seen
  mGraph->maxIndex(mMaxIndex);

  // We must be careful here since the finalizeVertex call might
  // remove the vertex from the array which may invalidate the
  // iterator
  it = mVertices.begin();
  while (it!=mVertices.end()) {
    if (!it->isFinalized()) {
      // We want to finalize it
      
      std::cerr << "Forced finalization of vertex " << it->id() << " type " << it->type() << "\n";

      // First store the index we must finalize
      id = it->id();
     
      // Then look for the next element that is *not*
      // finalized. Elements that are not finalized will never be
      // removed which makes their iterator save
      it++;
      while ((it != mVertices.end()) && it->isFinalized())
        it++;
      
      // Finally, finalize the previously unfinalized vertex
      finalizeVertex(id);
    }
    else {
      it++;
    }
  }

  // Finally, cleanup any left overs from whatever algorithm was used
  return cleanupInternal();
}

#endif
