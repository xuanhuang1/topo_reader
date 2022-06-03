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


#ifndef UNIONALGORITHM_H
#define UNIONALGORITHM_H

#include "Vertex.h"
#include "UnionVertex.h"

#define ST_PROFILE_INFO 1



//! Interface for the merge/split tree construction algorithm 
/*! The UnionAlgorithm provides the basic interface and implementation
 *  for the integration of a new edge into a merge- or split-tree data
 *  structure. The algorithm is implemented using a common interface
 *  containing the functions
 *
 *  isFinalized(UnionVertex*)
 *  getChild(UnionVertex*)
 *  setChild(UnionVertex*, UnionVertex*)
 *
 *  smaller(UnionVertex*, UnionVertex*)
 *  greater(UnionVertex*, UnionVertex*)
 * 
 *  While this type of interface seems redundate (given that all
 *  function could be implemented by UnionVertex) and awkward to
 *  implement with, it has significant advantages:
 * 
 *  1) Differences between merge- and split-trees can be implemented
 *  by derived classes simply by overloading the comparison
 *  operators. Note that this requires the algorithm class to use
 *  virtual functions but *not* The vertex class which saves a pointer
 *  in the vertex class.
 * 
 *  2) To implement contour trees using this algorithm twice with
 *  different operators and pointer we do not need multiple
 *  inheritance. Instead we can re-implement this interface twice
 *  making the code structure cleaner and less prone to compiler
 *  problems.
 *
 *  3) The algorithm class / function no longer needs to be a template
 *  on the vetex type but only on the data type which seems to be
 *  cleaner as well
 */
template <class VertexClass = UnionVertex>
class UnionAlgorithm
{
public:

  //! Default constructor
  UnionAlgorithm(VertexCompare* cmp);
  
  //! Destructor
  virtual ~UnionAlgorithm();
  
  virtual void printStatistics();
  
  /********************************************************************
   ********************* Algorithm Interface **************************
   *******************************************************************/

  //! Add the given vertex to the tree
  virtual int add_vertex(VertexClass* v) const {return 1;}

  //! Add a single edge to a merge- or split-tree
  /*! This function adds the edge head-tail to the union tree both
   *  vertices are part of. The edge will be oriented according to the
   *  comparison operator and therefore the calling order does not
   *  matter. If the function encounters a finalized saddle or root
   *  which by adding this edge become regular it will return a
   *  pointer to this vertex. Otherwise, it will return NULL.
   *  @param head: Pointer to one vertex of a merge-/split-tree
   *  @param tail: Pointer to another vertex of a merge-/split-tree
   *  @return Pointer to an updated saddle or root or NULL
   */
  virtual VertexClass* add_edge(VertexClass* head, VertexClass* tail) const;

  //! Remove a vertex from the tree
  virtual int remove_vertex(VertexClass* v) const {return 1;}

  //! Return a reference to the comparison object
  const VertexCompare& cmp() const {return *mGreater;}
  
  //! Determine wether *u > *v 
  bool greater(const VertexClass* u, const VertexClass* v) const {return (*mGreater)(u,v);}

protected:
  
  VertexCompare* mGreater;

   /********************************************************************
   ********************* Computation Interface *************************
   *******************************************************************/

  //! Return the child of v
  virtual VertexClass* getChild(const VertexClass* v) const {return static_cast<VertexClass*>(v->child());}

  //! Return the parent of v 
  virtual VertexClass* getParent(const VertexClass* v) const {return static_cast<VertexClass*>(v->parent());}

  //! Set the child of v to down
  virtual void setChild(VertexClass* v, VertexClass* down) const {sterror(v==down,"Illegal child pointer, no loops allowed.");v->child(down);}

  //! Indicate whether v has been finalized
  virtual bool isFinalized(VertexClass* v) const {return v->isFinalized();}

   /********************************************************************
   ********************* Subrountines          *************************
   *******************************************************************/

  //! Find the lowest child of high that is >= low
  virtual VertexClass* findIntegrationVertex(VertexClass* high, VertexClass* low) const;
  
  //! Attach branch starting at head to the branch ending at tail
  /*! This call assumes that getChild(tail) == NULL and attaches the
   *  branch starting at head to the branch ending at tail. If tail
   *  was finalized the function will return tail to re-evaluate its
   *  type otherwise it return NULL.
   */
  virtual VertexClass* attachBranch(VertexClass* tail, VertexClass* head) const;

  //! Merge the two branches starting at left and right respectively
  /*! This call merge-sorts the two paths starting at left and right
   *  respectively into a single branch. The initial assumption is
   *  that left > right. If the merging causes a finalized critial
   *  point to potentially change its type a pointer to this vertex is
   *  returned. Otherwise, the function returns NULL.
   */
  virtual VertexClass* mergeBranches(VertexClass* left, VertexClass* right) const;

};


template <class VertexClass>
void UnionAlgorithm<VertexClass>::printStatistics()
{
};

template <class VertexClass>
UnionAlgorithm<VertexClass>::UnionAlgorithm(VertexCompare* cmp) : mGreater(cmp)
{
  sterror(mGreater==NULL,"Algorithm cannot be greated with empty comparison.");
}

template <class VertexClass>
UnionAlgorithm<VertexClass>::~UnionAlgorithm()
{
  delete mGreater;
}


template <class VertexClass>
VertexClass* UnionAlgorithm<VertexClass>::add_edge(VertexClass* head, 
                                                   VertexClass* tail) const
{

  //if ((head->id() == 192) && (tail->id() == 191))
  //  fprintf(stderr,"break\n");

  // We want to assume that head is above (towards the leafs) tail
  if (greater(tail,head))
    std::swap(head,tail);

  // Search for the lowest child of head that is still above tail
  head = findIntegrationVertex(head,tail);

  
  if (head == tail) { // If this edge already exists
    return NULL; // There is nothing to do
  }
  else if (getChild(head) == NULL) {// If even the root of the tree is higher than tail
    // Attach the tail branch to the head branch
    return attachBranch(head,tail);
  }
  else {
    // Otherwise, we need to merge two path 
    return mergeBranches(head,tail);
  }

}

template <class VertexClass>
VertexClass* UnionAlgorithm<VertexClass>::findIntegrationVertex(VertexClass* high, VertexClass* low) const
{
  // Search for the lowest child of high that is still above low
  while ((getChild(high) != NULL) && !greater(low,getChild(high))) {    
    high = getChild(high);
  }

  return high;
}

template <class VertexClass>
VertexClass* UnionAlgorithm<VertexClass>::attachBranch(VertexClass* tail, VertexClass* head) const
{
  sterror(getChild(tail)!=NULL,"You can only attach to disjoined branches.");

  setChild(tail,head); // tail becomes the new root
  
  if (isFinalized(tail)) // If head was finalized earlier
    return tail;// We must re-evaluate its type
  else
    return NULL;
}

template <class VertexClass>
VertexClass* UnionAlgorithm<VertexClass>::mergeBranches(VertexClass* left, VertexClass* right) const
{
  sterror(!greater(left,right),"Assumption violated. Left should be > right.");

  VertexClass* next_right;
  VertexClass* next_left;

  // While the two paths have not merged
  while ((left != right) && (getChild(left) != NULL)){
    
    // We need to splice right between left and child(left) 
    
    // Store the next vertex on the right path
    next_right = getChild(right);
    
    // To make things slightly faster we use a tmp variable
    next_left = getChild(left);
    setChild(left,right);
    setChild(right,next_left);

    // Now we determine whether we may have changed a finalized type

    // If we have reached the end of the right path
    if (next_right == NULL) {
      if (isFinalized(right)) // And it previous root was finalized
        return right; // It just changed to become a regular vertex
      else
        return NULL; // Otherwise we simply stop here 
    }

    // If left used to be the root of its tree 
    if (next_left == NULL) {
      if (isFinalized(left)) // and it was finalized 
        return left; // It just changed to become a regular vertex
      else
        return NULL; // We are done anyway
    }

    // If both paths continue we reset the pointers

    left = right;
    right = next_right;

    // Advance the left path until you find the next integration
    // point. In case a derived class wants to use this version of
    // mergeBranches we make sure we stay within our class hierarchy
    left = UnionAlgorithm<VertexClass>::findIntegrationVertex(left,right);    
  }

  // If we get here this means that left == right != NULL
  sterror(left==NULL,"Two paths should not merge at NULL.");
  
  // If we broke the traversal because there only remains an attaching
  // to do
  if (left != right) {

    setChild(left,right); // tail becomes the new root
    
    if (isFinalized(left)) // If head was finalized earlier it was a root
      return left;// We must re-evaluate its type (it became regular)
    else
      return NULL;
  }

  // If the saddle at which we merge has been finalized it may have
  // changed type
  if ((left == right) && isFinalized(left)) 
    return left; // The saddle must be fixed

  
  return NULL;
}



#if 0
template <class VertexClass>
VertexClass* UnionAlgorithm<VertexClass>::add_edge(VertexClass* head, 
                                                   VertexClass* tail)
{
#ifdef ST_PROFILE_INFO
  uint8_t edge_type = 0;
  
  if ((getChild(head) != NULL) && (getChild(tail) == NULL))
    edge_type = 0;
  else if ((getChild(head) == NULL))
    edge_type = 1;
  else if ((getChild(head) != NULL) && (getChild(tail) != NULL))
    edge_type = 2;

  sCallCount[edge_type]++;
#endif

  // We want to assume that head is above (towards the leafs) tail
  if (greater(tail,head))
    std::swap(head,tail);

  // Search for the lowest child of head that is still above tail
  while ((getChild(head) != NULL) && greater(getChild(head),tail)) {

#ifdef ST_PROFILE_INFO
    sStepToLevelCount[edge_type]++;
    if ((edge_type == 0) && (head->parent() != NULL) && (head->parent()->next() != head->parent()))
      sStepToLevelCount[3]++;
#endif

    head = getChild(head);
  }

  // If even the root of the tree is higher than tail
  if (getChild(head) == NULL) {
    setChild(head,tail); // tail becomes the new root

    if (isFinalized(head)) // If head was finalized earlier
      return head;// We must re-evaluate its type
    else
      return NULL;
  }
  else if (getChild(head) == tail) { // If this edge already exists
    return NULL; // There is nothing to do
  }
  else {
    // Otherwise, we need to merge two path which we call the "right"
    // and the "left" path. Head is thought to exists on the "left"
    // and tail on the "right". In the following these paths will
    // advance in a merge sort fashion such that right > left

    VertexClass* left;
    VertexClass* right;
    VertexClass* tmp;
    VertexClass* old_root = NULL;

    // If the right path ends at tail or the two paths merge at the
    // child of head and tail
    if ((getChild(tail) == NULL) || (getChild(tail) == getChild(head)))
      left = getChild(head); // we consider this child as the second vertex of the left path
    else
      left = tail; // otherwise tail is the second vertex of the left path

    // The second vertex from the top of the right path is always the
    // child of tail
    right = getChild(tail);

    // Here we need to splice in tail between head and
    // head->child. This should be slightly faster since tmp must
    // search through one less parent compared to assigning
    // tail->child() first
    tmp = getChild(head);
    setChild(head,tail);
    setChild(tail,tmp);

    // Tail could be the root of its path but it should not be
    // finalized so this will not result in a return tail
    sterror(tail->isFinalized(),"Neither vertex of an edge being added should be finalized.");

    // Now left->parent() == right->parent() meaning we have a split
    // in the tree were none should exist. 

    // While the two paths have not joined 
    while ((left != right) && (right != NULL)) {
      
#ifdef ST_PROFILE_INFO
      sStepToMergeCount[edge_type]++;
#endif

     sterror(left==NULL,"The left path should not end.");

      // While the next vertex in the left path is above right
      // advance the left path
     while ((getChild(left) != NULL) && greater(getChild(left),right)) {

#ifdef ST_PROFILE_INFO
       sStepToMergeCount[edge_type]++;
#endif

        left = getChild(left);
     }
      
      // If the left path is at an end we found a root that no longer
      // is a root
      if (getChild(left) == NULL) {
        setChild(left,right); // Attach the remaining right path to
                              // the root of the left on

        if (isFinalized(left)) // If the root was finalized
          return left; // We need to re-evaulate its type
        else 
          return NULL;
      }
      else if (getChild(left) == right) { // If we found a saddle 
        
        if (isFinalized(right)) // and it was finalized
          return right; // It might not be a saddle anymore 
        else 
          return NULL; 
      } 
      else { // Otherwise, merge sort the two paths
        if ((getChild(right) == NULL) || (getChild(right) == getChild(left)))
          head = getChild(left);
        else 
          head = right;

        old_root = right;
        tail = getChild(right);
       
        // Here we splice right in between left and left->child(). The
        // same discussion as before holds and we use a temporary
        // variable
        tmp = getChild(left);
        setChild(left,right);
        setChild(right,tmp);

        left = head;
        right = tail;
      }
    }
    
    // If we have closed a loop with a finalized saddle at the bottom
    if ((left == right) && isFinalized(left)) 
      return left; // The saddle must be fixed
    // If we have followed the right path to its end and its root is
    // finalized
    else if ((old_root != NULL) && (right == NULL) && isFinalized(old_root))
      return old_root;
    else // Otherwise we just closed a split
      return NULL;
  }
}
#endif




#endif
