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

#ifndef ACCELERATEDUNIONALGORITHM_H
#define ACCELERATEDUNIONALGORITHM_H

#include "Definitions.h"
#include "UnionAlgorithm.h"
#include "EnhancedUnionVertex.h"

//! The accelerated union algorithm using stl::sets to accellerate branch traversal
/*! The default union algorithm relies on a very small set of active vertices o
 *  be efficient. In particular, in the findIntegrationVertex function it uses a
 *  linear traversal and an extremely light weight data structure. However, in
 *  3D (and presumably beyond) this becomes highly inefficient as the size of a
 *  level set grows wrt. the size of the data set. The AcceleratedUnionAlgorithm
 *  improves on this by maintaining a search structure for each branch which
 *  allows finding the integration vertex in log(n) time. Furthermore, unlike
 *  the EnhancedUnionAlgorithm (which is depercated but kept for comparison
 *  purposes) the "Accelerated" version takes special care to create a single
 *  long main branch. While merging branches may become more expensive, this
 *  reduces the number of branches we need to travers when searching for
 *  vertices. This will further speed up the search and should prevent behavior
 *  linear in the size of the branches to occur (too often).
 */
template <class VertexClass = EnhancedUnionVertex >
class AcceleratedUnionAlgorithm : public virtual UnionAlgorithm<VertexClass>
{
public:

  typedef typename VertexClass::BranchType BranchType;
  
  static uint64_t sCallingCount;
  static uint64_t sBranchSkipSum;


  //! Default constructor
  AcceleratedUnionAlgorithm(VertexCompare* cmp,uint16_t threshold=0);

  //! Destructor
  virtual ~AcceleratedUnionAlgorithm() {}

  //! Add the given vertex to the tree
  virtual int add_vertex(VertexClass* v) const;

  //! Add a single edge to a merge- or split-tree
  virtual VertexClass* add_edge(VertexClass* head, VertexClass* tail) const;

  //! Remove a vertex from the tree
  virtual int remove_vertex(VertexClass* v) const;

protected:

   /********************************************************************
   ********************* Subrountines          *************************
   *******************************************************************/

  //! Find the lowest child of high that is >= low
  virtual VertexClass* findIntegrationVertex(VertexClass* high, VertexClass* low) const;
  
  //! Attach branch starting at head to the branch ending at tail
  /*! This call assumes that getChild(tail) == NULL and attaches the branch
   *  starting at head to the branch ending at tail. Potentially, this function
   *  will re-distribute points from head into tail to preserve a long trunk. If
   *  tail was finalized the function will return tail to re-evaluate its type
   *  otherwise it returns NULL.
   *  @param tail: Root of the branch to which we attach
   *  @param head: Node that must be attached to tail
   *  @return tail if it was finalized and changed type, NULL otherwise
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

  //! If appropriate pass the seg index of v to its child
  virtual void propagateSegmentationIndex(VertexClass* v) const {}

private:

  const uint16_t mBalancingThreshold;

  //! Function that actually performs the merge
  virtual VertexClass* mergeBranchesInternal(VertexClass* left, VertexClass* right) const;


  bool correctIntegrationOrder(VertexClass* left, VertexClass* right) const;
};

template <class VertexClass>
uint64_t AcceleratedUnionAlgorithm<VertexClass>::sCallingCount = 0;
template <class VertexClass>
uint64_t AcceleratedUnionAlgorithm<VertexClass>::sBranchSkipSum = 0;

template <class VertexClass>
AcceleratedUnionAlgorithm<VertexClass>::AcceleratedUnionAlgorithm(VertexCompare* cmp,
                                                                  uint16_t threshold) :
  UnionAlgorithm<VertexClass>(cmp), mBalancingThreshold(threshold)
{
}


template <class VertexClass>
int AcceleratedUnionAlgorithm<VertexClass>::add_vertex(VertexClass* v) const
{
  sterror(v->branch()!=NULL,"Vertex initialized twice aborting.");

  v->branch(new BranchType(this->cmp()));
  v->branch()->insert(v);

  return 1;
}

template <class VertexClass>
VertexClass* AcceleratedUnionAlgorithm<VertexClass>::add_edge(VertexClass* head, VertexClass* tail) const
{
  sterror(head->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(tail->branch()==NULL,"Cannot integrate uninitialized vertex.");
  
      
  stmessage(*(head->branch()->find(head)) != head,"Branch of vertex %d inconsistent.",head->id());
  stmessage(*(tail->branch()->find(tail)) != tail,"Branch of vertex %d inconsistent.",tail->id());

  // If both vertices are part of the same branch already there is
  // nothing to do
  if (head->branch() == tail->branch())
    return NULL;

  return UnionAlgorithm<VertexClass>::add_edge(head,tail);
}

template <class VertexClass>
int AcceleratedUnionAlgorithm<VertexClass>::remove_vertex(VertexClass* v) const
{
  uint8_t existed;

  existed = v->branch()->erase(v);
  v->branch(NULL);
  
  stmessage(existed != 1,"Could not remove vertex %d. Branch structure inconsisten.",v->id());

  return existed;
}

  


template <class VertexClass>
VertexClass* AcceleratedUnionAlgorithm<VertexClass>::findIntegrationVertex(VertexClass* high, 
                                                                           VertexClass* low) const
{
  sterror(high==NULL,"Cannot integrate NULL vertex.");
  sterror(low==NULL,"Cannot integrate NULL vertex.");
  sterror(high->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(low->branch()==NULL,"Cannot integrate uninitialized vertex.");

  // First we find the branch that contains low.
  VertexClass* back = high->branch()->back();
  uint64_t skip_count = 0;

  // While the head of the next branch is equal to or greater than
  // low
  while ((this->getChild(back) != NULL) &&  !this->greater(low,this->getChild(back))) {
    
    high = this->getChild(back);
    back = high->branch()->back();

    skip_count++;    
  }
  
#if 1
  
  sCallingCount = 1;
  sBranchSkipSum = std::max(skip_count,sBranchSkipSum);

#endif


  // Once we get here low must be integrated into the branch
  // [high,back]. Either this branch is the root branch
  // (getChild(back) == NULL) or the next branch starts at a lower
  // vertex (greater(low,getChild(back)))

  if (this->greater(back,low)) // Low must be attached at the end of this branch
    return back;
  else {
    
    if (high->branch() == low->branch())
      return low;
    else {
      // Otherwise, find the exact integration point

      typename  BranchType::iterator prev;
      prev = high->branch()->find(low);
      prev--;

      return *prev;
    }
  }
}


template <class VertexClass>
VertexClass* AcceleratedUnionAlgorithm<VertexClass>::attachBranch(VertexClass* tail, VertexClass* head) const
{
  sterror(tail->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(head->branch()==NULL,"Cannot integrate uninitialized vertex.");

  // If head really *is* the highest vertex of its branch the two branches should be merged
  if (this->getParent(head) == NULL) {

    //! Just to be sure we check that this means head is the highest
    //vertex of its branch
    sterror(head->branch()->front()!=head,"Branches inconsistent root %d  is not first in its branch %d is.",head->id(),head->branch()->front()->id());

    //! Insert all the vertices 
    tail->branch()->insert(head->branch()->begin(),head->branch()->end());

    typename  BranchType::iterator it;
    BranchType* old_branch = head->branch();
    for (it=old_branch->begin();it!=old_branch->end();it++) {
      propagateSegmentationIndex(*it);
      (*it)->branch(tail->branch());
    }

    // Now nobody should point to old_branch anymore
    delete old_branch;
  }

  // Attach the pointer structure
  this->setChild(tail,head);
  
  if (this->isFinalized(tail)) // If head was finalized earlier
    return tail;// We must re-evaluate its type
  else
    return NULL;
}

template <class VertexClass>
VertexClass* AcceleratedUnionAlgorithm<VertexClass>::mergeBranches(VertexClass* left, VertexClass* right) const
{
  sterror(left->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(right->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(this->getChild(left)==NULL,"You should have been using the attachBranch function.");
  sterror(!this->greater(left,right),"Assumption violated. Left should be > right.");

  VertexClass* return_value;
  BranchType* old_branch;
  typename  BranchType::iterator it,pos;
  
  return_value = mergeBranchesInternal(left,right);
  
  // The mergeBranchInternal call will primarily do the exact same merging as
  // the UnionAlgorithm and only modify the branch structure of those vertices
  // it traverses. However, there exists a special case when getParent(right)
  // used to be NULL. In this case the two branches left and right actually
  // merged and we need to reflect this in the branch structure. Note that we
  // can choose either branch to remove
  
  // If we are *not* in the special case we simply return
  if ((right->currentType() == BRANCH) || (left->branch()==right->branch()))
    return return_value;
  
  sterror(left->branch()==right->branch(),"Branch structure inconsistent these branches should have been kept distinct.");

  // Otherwise we pick the smaller branch to disappear
  if (left->branch()->size() <= right->branch()->size()) {

    right->branch()->insert(left->branch()->begin(),left->branch()->end());

    old_branch = left->branch();
    for (it=old_branch->begin();it!=old_branch->end();it++) 
      (*it)->branch(right->branch());
    
    delete old_branch;
  }
  else {
    left->branch()->insert(right->branch()->begin(),right->branch()->end());

    old_branch = right->branch();
    for (it=old_branch->begin();it!=old_branch->end();it++) 
      (*it)->branch(left->branch());
    
    delete old_branch;
  }
  
  return return_value;
}

template <class VertexClass>
VertexClass* AcceleratedUnionAlgorithm<VertexClass>::mergeBranchesInternal(VertexClass* left, VertexClass* right) const
{
  sterror(left->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(right->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(this->getChild(left)==NULL,"You should have been using the attachBranch function.");
  sterror(!this->greater(left,right),"Assumption violated. Left should be > right.");

  VertexClass* next_right;
  VertexClass* next_left;
  BranchType* old_branch;
  bool swap_sides;

  
  if ((left->id() == 203) && (right->id() == 201))
    fprintf(stderr,"break\n");
    
  // While the two paths have not merged
  while ((left != right) && (this->getChild(left) != NULL)){
    
    // We need to splice right between left and child(left) 
    
    // Store the next vertex on the right path
    next_right = this->getChild(right);
    
    // To make things slightly faster we use a tmp variable
    next_left = this->getChild(left);
    
    // Now we fix the branch structure. In particular, next_left and right
    // should be part of the same branch unless next_left is a saddle
    if (correctIntegrationOrder(left,right)) {

      old_branch = right->branch();
      
      left->branch()->insert(right);
      right->branch(left->branch());
      old_branch->erase(right);

      if (old_branch->empty())
        delete old_branch;
 
      this->setChild(left,right);
      this->setChild(right,next_left);

      swap_sides = false;
    }
    else {
      this->setChild(left,right);

      swap_sides = true;
    }

    // Now we determine whether we may have changed a finalized type

    // If we have reached the end of the right path
    if (next_right == NULL) {
      if (this->isFinalized(right)) // And it previous root was finalized
        return right; // It just changed to become a regular vertex
      else
        return NULL; // Otherwise we simply stop here 
    }

    // If left used to be the root of its tree 
    if (next_left == NULL) {
      if (this->isFinalized(left)) // and it was finalized
        return left; // It just changed to become a regular vertex
      else
        return NULL; // We are done anyway
    }

    // If both paths continue we reset the pointers

    left = right;

    if (swap_sides)
      right = next_left;
    else
      right = next_right;

    // Advance the left path until you find the next integration
    // point. In case a derived class wants to use this version of
    // mergeBranches we make sure we stay within our class hierarchy
    left = findIntegrationVertex(left,right);    
  }

  // If we get here this means that left == right != NULL
  sterror(left==NULL,"Two paths should not merge at NULL.");
  
  // If we broke the traversal because there only remains an attaching
  // to do
  if (left != right) {

    this->setChild(left,right); // tail becomes the new root
    
    if (this->isFinalized(left)) // If head was finalized earlier
      return left;// We must re-evaluate its type
    else
      return NULL;
  }

  // If the saddle at which we merge has been finalized it may have
  // changed type
  if ((left == right) && this->isFinalized(left))
    return left; // The saddle must be fixed
  
  return NULL;
}

template <class VertexClass>
bool AcceleratedUnionAlgorithm<VertexClass>::correctIntegrationOrder(VertexClass* left, VertexClass* right) const
{
  VertexClass* left_back;
  VertexClass* right_back;
  VertexClass* left_child;
  VertexClass* right_child;

  left_back = left->branch()->back();
  right_back = right->branch()->back();
  
  left_child = this->getChild(left_back);
  right_child = this->getChild(right_back);

  // These comparisons are very finiky you should know exactly what you are
  // doing before messing with them

  if ((left_child != NULL) && (right_child != NULL)) 
    return !this->greater(left_child,right_child);
  else if (left_child != NULL)
    return this->greater(right_back,left_child);
  else if (right_child != NULL)
    return !this->greater(left_back,right_child);
  else
    return !this->greater(left_back,right_back);
}
   
    
  


#endif
