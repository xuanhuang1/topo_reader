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

#ifndef ENHANCEDUNIONALGORITHM_H
#define ENHANCEDUNIONALGORITHM_H

#include "UnionAlgorithm.h"
#include "EnhancedUnionVertex.h"

//#define PROFILE_INFO 1
#define USE_ENHANCED_ALGORITHM 1

//! The enhanced union algorithm using stl::sets to accellerate branch traversal
/*! The default union algorithm relies on a very small set of active vertices o
 *  be efficient. In particular, in the findIntegrationVertex function it uses a
 *  linear traversal and an extremely light weight data structure. However, in
 *  3D (and presumably beyond) this becomes highly inefficient as the size of a
 *  level set grows wrt. the size of the data set. The EnhancedUnionAlgorithm
 *  improves on this by maintaining a search structure for each branch which
 *  allows finding the integration vertex in log(n) time. 
 */
template <class VertexClass = EnhancedUnionVertex>
class EnhancedUnionAlgorithm : public virtual UnionAlgorithm<VertexClass>
{
public:

  //! Typedef to allow easy acces to the branch class
  typedef typename VertexClass::BranchType BranchType;

  //! Typedef to allow easy access to the branch iterator
  typedef typename VertexClass::BranchType::iterator BranchIterator;

  //! Static variables used for profiling
  static uint32_t sMergeBranchCount;
  static uint32_t sCombineCount;
  static uint32_t sCombineCount_small;
  static uint32_t sCombineCount_large;
  static uint32_t sJoinCount;

  EnhancedUnionAlgorithm(VertexCompare* cmp);

  virtual ~EnhancedUnionAlgorithm();

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
  /*! This call assumes that getChild(tail) == NULL and attaches the
   *  branch starting at head to the branch ending at tail. If tail
   *  was finalized the function will return tail to re-evaluate its
   *  type otherwise it returns NULL.
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

  //! Fix the branch information for the merge of longer and shorter
  /*! This function will correct the branch information involved in the merge of
   *  the two branches starting at longer and shorter respectively. Note that
   *  the function itself and subsequent functionality should not use the branch
   *  structure before it has been synchronized with the tree structure again.
   *  @param longer: pointer to the merge point in the longer (lower root/saddle) branch
   *  @param shorter: pointer to the merge point in the shorter (higher root/saddle) branch
   *  @return a vertex pointer if is used to be finalized; and NULL otherwise
   */
  virtual VertexClass* mergeBranchInfo(VertexClass* longer, VertexClass* shorter) const;

  //! If appropriate pass the seg index of v to its child
  virtual inline void propagateSegmentationIndex(VertexClass* v) const {}

  //! Make one branch out of two branches
  /*! Given two branched u and v this function will join their branch structures
   *  into one and deleting the other. For obvious reasons the smaller branch
   *  (very vertices) will disappear.
   *  @param u: pointer to one branch
   *  @param v: pointer to the other branch
   */
  virtual VertexClass* combineBranches(BranchType* u, BranchType* v) const;

  //! Make one branch out of two partial branches
  /*! Starting at longer and shorter this function will move all appropriate
   *  vertices below shorter into the longer branch.
   *  @param longer: pointer to the merge point in the longer (lower root/saddle) branch
   *  @param shorter: pointer to the merge point in the shorter (higher root/saddle) branch
   */
  virtual VertexClass* joinBranches(VertexClass* longer, VertexClass* shorter) const;

  //! Migrate all vertices from [start,source->end()] into destination
  /*! Move all vertices from the intervale [start,source->end()] into the destination branch.
   *  This includes handling the branch structure as well as the tree pointers.
   * @param start: start of the range to move
   * @param source: source branch
   * @param destination: destinatio nbranch
   * @return pointer to an unfinalized vertex; NULL otherwise
   */
  VertexClass* migrateVertices(BranchIterator start, BranchType* source, BranchType* destination) const;

  VertexClass* attachVertices(BranchIterator start, BranchType* source, BranchType* destination) const;
};

template <class VertexClass>
uint32_t EnhancedUnionAlgorithm<VertexClass>::sMergeBranchCount = 0;
template <class VertexClass>
uint32_t EnhancedUnionAlgorithm<VertexClass>::sCombineCount = 0;
template <class VertexClass>
uint32_t EnhancedUnionAlgorithm<VertexClass>::sCombineCount_small = 0;
template <class VertexClass>
uint32_t EnhancedUnionAlgorithm<VertexClass>::sCombineCount_large = 0;
template <class VertexClass>
uint32_t EnhancedUnionAlgorithm<VertexClass>::sJoinCount = 0;

template <class VertexClass>
EnhancedUnionAlgorithm<VertexClass>::EnhancedUnionAlgorithm(VertexCompare* cmp) :
  UnionAlgorithm<VertexClass>(cmp)
{
}

template <class VertexClass>
EnhancedUnionAlgorithm<VertexClass>::~EnhancedUnionAlgorithm()
{
#ifdef PROFILE_INFO
  fprintf(stderr,"\n\nThere have been %u mergeBranch calls resulting in:\n",sMergeBranchCount);
  fprintf(stderr,"\t %u combineBranch calls with on average \n",sCombineCount);
  fprintf(stderr,"\t\t %f vertices in the short and %f vertices in the long branch,\n",
          sCombineCount_small / (float)sCombineCount,sCombineCount_large / (float)sCombineCount);
  fprintf(stderr,"\t %u joinBranch calls\n",sJoinCount);
#endif
}


template <class VertexClass>
int EnhancedUnionAlgorithm<VertexClass>::add_vertex(VertexClass* v) const
{
  sterror(v->branch()!=NULL,"Vertex initialized twice aborting.");

  v->branch(new BranchType(this->cmp()));
  v->branch()->insert(v);

  return 1;
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::add_edge(VertexClass* head, VertexClass* tail) const
{
  sterror(head->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(tail->branch()==NULL,"Cannot integrate uninitialized vertex.");
  
  stmessage(*(head->branch()->find(head)) != head,"Branch of vertex %d inconsistent.",head->id());
  stmessage(*(tail->branch()->find(tail)) != tail,"Branch of vertex %d inconsistent.",tail->id());

  if ((head->id() == 73) && (tail->id() == 78))
    fprintf(stderr,"break\n");

  // If both vertices are part of the same branch already there is
  // nothing to do
  if (head->branch() == tail->branch())
    return NULL;

  return UnionAlgorithm<VertexClass>::add_edge(head,tail);
}

template <class VertexClass>
int EnhancedUnionAlgorithm<VertexClass>::remove_vertex(VertexClass* v) const
{
  uint8_t existed;

  existed = v->branch()->erase(v);
  
  if (v->branch()->empty()) {
    stmessage(v->type()==INTERIOR,"Deleting branch of interior vertex %d.",v->id());
    delete v->branch();
  }

    
  v->branch(NULL);
  
  if (existed != 1) 
    stwarning("Could not remove vertex %d. Branch structure inconsistent.",v->id());

  return existed;
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::findIntegrationVertex(VertexClass* high, 
                                                                        VertexClass* low) const
{
  sterror(high->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(low->branch()==NULL,"Cannot integrate uninitialized vertex.");

  // First we find the branch that contains low.
  VertexClass* back = high->branch()->back();

  // While the head of the next branch is equal to or greater than
  // low
  while ((this->getChild(back) != NULL) &&  !this->greater(low,this->getChild(back))) {
    
    high = this->getChild(back);
    back = high->branch()->back();
  }
  
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

      BranchIterator prev;
      prev = high->branch()->find(low);
      prev--;
      // Otherwise, find the exact integration point
      return *prev;
    }
  }
}


template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::attachBranch(VertexClass* tail, VertexClass* head) const
{
  sterror(tail->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(head->branch()==NULL,"Cannot integrate uninitialized vertex.");

  // If head really *is* the highest vertex of its branch the two branches should be merged
  if (this->getParent(head) == NULL) {

    // Attach the pointer structure
    this->setChild(tail,head);

    // Just to be sure we check that this means head is the highest
    // vertex of its branch
    sterror(head->branch()->front()!=head,"Branches inconsistent: %d should be top most vertex but found %d instead",
            head->id(),head->branch()->front()->id());

    // Insert all the vertices 
    tail->branch()->insert(head->branch()->begin(),head->branch()->end());

    /*
    count++;
    height_sum += head->branch()->size();
    if (count % 1000 == 0)
      fprintf(stderr,"Average attach size %d %d  %f\n",count,height_sum,height_sum / (float)count);
    */

    typename  BranchType::iterator it;
    BranchType* old_branch = head->branch();
    for (it=old_branch->begin();it!=old_branch->end();it++) {
      propagateSegmentationIndex(*it);
      (*it)->branch(tail->branch());
    }

    // Now nobody should point to old_branch anymore
    delete old_branch;
  }
  else { // If head has a parent 
    // Attach the pointer structure
    this->setChild(tail,head);
  }    

  // If head was an internal vertex than head is now a saddle and the two
  // branches should stay separate. In terms of the branch decomposition this is
  // equivalent to letting the branch with the lowest minimum be the "main"
  // branch
  
  if (this->isFinalized(tail)) // If head was finalized earlier
    return tail;// We must re-evaluate its type
  else
    return NULL;
}

#ifndef USE_ENHANCED_ALGORITHM

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::mergeBranches(VertexClass* longer, VertexClass* shorter) const
{
  sterror(longer->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(shorter->branch()==NULL,"Cannot integrate uninitialized vertex.");

#ifdef PROFILE_INFO
  sMergeBranchCount++;
#endif

  // First, we fix the branch information
  mergeBranchInfo(longer,shorter);
  
  // The we do the standard merge-sort
  if (this->greater(longer,shorter))
    return UnionAlgorithm<VertexClass>::mergeBranches(longer,shorter);
  else
    return UnionAlgorithm<VertexClass>::mergeBranches(shorter,longer);

}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::mergeBranchInfo(VertexClass* longer, VertexClass* shorter) const
{
  VertexClass* next_short;
  VertexClass* long_back;

  //if ((longer->id() == 1061) && (shorter->id() == 857))
  //  fprintf(stderr,"break\n");


  // As long as we have not found the actual integration point
  while (longer != shorter) {


    // Make sure that we have the order correct.
    if (this->greater(longer->branch()->back(),shorter->branch()->back())) {
      std::swap(longer,shorter);
    }

    // Make sure to find the integration vertex before we mess with the branch
    // structure. Note that, even if this is not the first iteration of the loop
    // we have not messed with shorter->branch()->back(). Either shorter is an
    // entirely new branch we have not touched or it used to be the longer
    // branch before. In the later case, the last vertex of the longer branch
    // could not have been changed by anything we did previously and thus will
    // return the correct result even though the branch structure is
    // inconsistent with the current tree structure
    next_short =  getChild(shorter->branch()->back());

    // This pointer should not change but we use it to test for exceptions
    long_back = longer->branch()->back();

    // Now we merge the two (partial) branches. First, we check whether the two
    // branches can be combined entirely
    if ((this->greater(shorter,longer) && (getParent(longer) == NULL)) ||
        (this->greater(longer,shorter) && (getParent(shorter) == NULL))) {

      combineBranches(longer->branch(),shorter->branch());
    }
    else { // If the branches can't be combined they must be joined

      joinBranches(longer,shorter);
    }

    sterror(long_back!=longer->branch()->back(),"The last vertex of the longer branch should not have changed.");

    // If the short branch ended in a root there is nothing left to do.
    if (next_short == NULL)
      return NULL;

    sterror(this->greater(next_short,longer),"Branch structure inconsistent.");

    // At this point we assume that we have dealt with all vertices from shorter
    // down to next_short (which may or may not have changed shorter's branch).
    // Thus adding the edge (shorter,longer) is equivalent to adding the edge
    // (longer,next_short)

    shorter = next_short;

    // Walk down the current links of the tree (not using the branch structure)
    // to find the new integration point of longer.
    // The next line is deprecated since we now use the search structures to
    // find the integration point
    // longer = UnionAlgorithm<VertexClass>::findIntegrationVertex(longer,shorter);

    // We use the search structure to find the integration point which is much
    // faster than the linear traversal. However, the disadvantage is that the
    // function may return a vertex that was originally not part of this branch
    // but only got added to the branch during this iteration of the while loop.
    // This poses no immediate problem except that the other functions (most
    // notabely cannot relie on the tree pointers of any vertex since those
    // will be incorrect for these recently migrated vertices
    longer = findIntegrationVertex(longer,shorter);

    sterror(this->greater(shorter,longer),"Integration vertex inconsistent.");
  }

  return NULL;
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::combineBranches(BranchType* u, BranchType* v) const
{
  typename BranchType::iterator it;

#ifdef PROFILE_INFO
  sCombineCount++;
#endif

  // Choose the smaller branch to disappear
  if (u->size() < v->size())
    std::swap(u,v);

  #ifdef PROFILE_INFO
  if (this->greater(u->back(),v->back())) {
    sCombineCount_small += 1;
    //fprintf(stderr,"Merging size %u into size %u   %p\n",v->size(),u->size(),getChild(u->back()));
  }
  else
    sCombineCount_large += 1;
#endif

  // For all vertices in the smaller branch
  for (it=v->begin();it!=v->end();it++) {
    (*it)->branch(u);
    propagateSegmentationIndex(*it);
  }

  // Now insert all vertices of v's branch into u's branch
  u->insert(v->begin(),v->end());

  // Finally, delete the shorter branch
  delete v;

  return NULL;
}


template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::joinBranches(VertexClass* longer, VertexClass* shorter) const
{
  typename BranchType::iterator middle,it;
  BranchType* branch;

#ifdef PROFILE_INFO
  sJoinCount++;
#endif

  sterror(shorter->branch()==longer->branch(),"We should not try to merge a branch with itself.");

  // We need to add all vertices from shorter into longer. However, if shorter >
  // longer than shorter itself will not move.

  branch = shorter->branch();

  // If shorter > longer
  if (this->greater(shorter,longer)) {

    // This code is deprecated in order to use the search structure to find the
    // integration vertex.
    // 10/09/10 ptb
    // We do not need to merge shorter
    //shorter = getChild(shorter);



    // We do not need to merge shorter. If it is already last vertex of its
    // branch we are done. Note that obviously the branch can't be empty so we
    // can just return
    if (shorter == branch->back())
      return NULL;

    // Otherwise we need to start combining at the child of shorter. However,
    // in the improved implementation shorter could be a vertex that has been
    // recently added to the branch but its tree pointers do not yet reflect
    // this change. Thus we cannot use getChild to find its child. Instead we
    // search for its next element in the branch.
    BranchIterator it;

    it = branch->find(shorter);

    // Clearly an element should be part of its own branch
    sterror(*it != shorter,"Branch structure inconsistent. Element not found in its own branch.");

    // Go to the next element in the branch. Note, that there must be such an
    // element since shorter wasn't the last vertex
    it++;

    // And assign shorter as the next vertex.
    shorter = *it;

    // While not immediately obvious, shorter now points to a vertex whose
    // child pointers are correct. The only vertices with incorrect pointers
    // are those that have switched branch. However, all of these must have
    // been above shorter. Thus after finding the old shorter vertex and
    // advancing to its "child" the remaining pointers are untouched.
  }

  while ((shorter != NULL) && (shorter->branch() == branch)) {
    shorter->branch(longer->branch());
    longer->branch()->insert(shorter);
    branch->erase(shorter);

    shorter = getChild(shorter);
  }

  if (branch->empty())
    delete branch;

  return NULL;
}

#else

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::mergeBranches(VertexClass* longer, VertexClass* shorter) const
{
  sterror(longer->branch()==NULL,"Cannot integrate uninitialized vertex.");
  sterror(shorter->branch()==NULL,"Cannot integrate uninitialized vertex.");

#ifdef PROFILE_INFO
  sMergeBranchCount++;
#endif

  // First, we fix the branch information
  return mergeBranchInfo(longer,shorter);
}


template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::mergeBranchInfo(VertexClass* left, VertexClass* right) const
{
  VertexClass* next_left;
  VertexClass* next_right;
  VertexClass* finalized = NULL;
  
  //if ((longer->id() == 1061) && (shorter->id() == 857))
  //  fprintf(stderr,"break\n");


  // As long as we have not found the actual integration point
  while (left != right) {

    next_left = this->getChild(left->branch()->back());
    next_right = this->getChild(right->branch()->back());

    // Now we merge the two (partial) branches. First, we check whether the two
    // branches can be combined entirely
    if ((this->greater(right,left) && (this->getParent(left) == NULL)) ||
        (this->greater(left,right) && (this->getParent(right) == NULL))) {
      
      finalized = combineBranches(left->branch(),right->branch());
    }
    else { // If the branches can't be combined they must be joined

      if (this->greater(left->branch()->back(),right->branch()->back()))
        finalized = joinBranches(right,left);
      else
        finalized = joinBranches(left,right);

    }
    
    // If a vertex has changed finalization we must be done with the merging
    if (finalized != NULL)
      return finalized;

    if (this->getChild(left->branch()->back()) != next_left) {
      sterror(this->getChild(right->branch()->back()) != next_right,"Only one branch should have changed at a time");

      left = next_left;
    }
    else {
      sterror(this->getChild(left->branch()->back()) != next_left,"Only one branch should have changed at a time");

      right = next_right;
    }

    // If the short branch ended in a root there is nothing left to do. 
    if ((right == NULL) || (left == NULL))
      return NULL;
    
    if (this->greater(right,left))
      std::swap(right,left);

    left = findIntegrationVertex(left,right);

    sterror(this->greater(right,left),"Integration vertex inconsistent.");
  }

  // If the merge point has been finalized it may have switched type and we need
  // to return it
  if (this->isFinalized(left))
    return left;
  else
    return NULL;
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::combineBranches(BranchType* u, BranchType* v) const
{
#ifdef PROFILE_INFO
  sCombineCount++;
#endif

  // Choose the smaller branch to disappear
  if (u->size() < v->size())
    std::swap(u,v);

  #ifdef PROFILE_INFO
  if (this->greater(u->back(),v->back()))
    sCombineCount_small += 1;
  else
    sCombineCount_large += 1;
#endif

  return migrateVertices(v->begin(),v,u);
}


template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::joinBranches(VertexClass* longer, VertexClass* shorter) const
{
  BranchIterator start;
  BranchType* branch;

#ifdef PROFILE_INFO
  sJoinCount++;
#endif

  sterror(shorter->branch()==longer->branch(),"We should not try to merge a branch with itself.");

  // We need to add all vertices from shorter into longer. However, if shorter >
  // longer than shorter itself will not move.

  branch = shorter->branch();

  // If shorter > longer
  if (this->greater(shorter,longer)) {

    // The shorter needs to redirect its child pointer but does not actually change its branch
    VertexClass* tmp = this->getChild(shorter);

    this->setChild(shorter,longer);

    // If shorter used to be a finalized root
    if ((tmp == NULL) && this->isFinalized(shorter))
      return shorter;

    shorter = tmp;
  }

  // If the branch is done
  if ((shorter == NULL) || (shorter->branch() != branch))
    return NULL;

  // Otherwise, we find shorter in its own branch
  start = branch->find(shorter);

  sterror(start==branch->end(),"The vertex %u must exist in its own branch.",shorter->id());

  return migrateVertices(start,branch,longer->branch());
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::migrateVertices(BranchIterator start, BranchType* source,
                                                                  BranchType* destination) const
{
  BranchIterator it,it2;
  bool is_root = (this->getChild(source->back()) == NULL);
  VertexClass* last;

  // For all vertices in the source branch
  for (it=start;it!=source->end();it++) {

    last = destination->back();

    // While *it belongs to the next branch below destination
    while (this->greater(last,*it) && (this->getChild(last) != NULL)
        && this->greater(this->getChild(last),*it)) {

      // We switch destination
      destination = this->getChild(destination->back())->branch();

      // If now the source is the same as the destination, we have reached
      // a merge point. All the tree pointers should be correct but we need
      // to fix the branch structure
      if (source == destination) {
        sterror(true,"Inconsistent tree this should not happen");
        return attachVertices(it,source,destination);
      }
      else
        last = destination->back();
    } // end-while

    // Now, find the position in the destination branch before which *it must
    // be inserted
    it2 = destination->find(*it);

    // If *it would be inserted as last vertex below a former root
    if ((it2 == destination->end()) && (this->getChild(destination->back()) == NULL)) {

      // Attach the remaining branch in the tree
      this->setChild(last,*it);

      // and in the search structure
      attachVertices(it,source,destination);

      if (this->isFinalized(last))
        return last;
      else
        return NULL;
    }
    else { // If *it must be insert before *it2

      (*it)->branch(destination);

      if (it2 == destination->begin()) { // If *it must be inserted on top of u

        // The it2 should point to a leaf
        sterror(this->getParent(*it2)!=NULL,"The top of a branch should not have a parent.");

        // Attach u to *it
        this->setChild(*it,*it2);

       // Insert it into the larger branch
        destination->insert(it2,*it);
      }
      else { // Otherwise, it must be insert before it2
        it2--;

        VertexClass* tmp = this->getChild(*it2);

        if (tmp != *it) {
          // First, integrate it into the larger branch to propagate the segmentation info
          this->setChild(*it2,*it);

          // Then fix it's child pointer
          this->setChild(*it,tmp);
        }
        else {
          // If the next vertex in the source branch is also the next child below
          // the destination branch we have found a merge point in tmp

          // The tree pointers are already correct but we must also attach the
          // remaining vertices to the search structure
          return attachVertices(it,source,destination);
         }

        // Finally, fix the branch structure
        destination->insert(it2,*it);
      }
    }
  }

  // We have inserted all vertices from source into destination, but potentially source's last
  // vertex was finalized and may have switched type. The only way this can happen is
  // if the last vertex was a root before but no longer is a root in this case we need
  // to return it.
  last = source->back();

  // Now we can clean the source branch
  source->erase(start,source->end());
  if (source->empty())
    delete source;

  if (is_root && this->isFinalized(last))
    return last;
  else
    return NULL;
}

template <class VertexClass>
VertexClass* EnhancedUnionAlgorithm<VertexClass>::attachVertices(BranchIterator start, BranchType* source,
                                                                 BranchType* destination) const
{
  VertexClass* last = *start;

  for (BranchIterator it=start;it!=source->end();it++) {
    propagateSegmentationIndex(*it);
    (*it)->branch(destination);
  }

  // And insert all remaining vertices
  destination->insert(start,source->end());

  // Finally clean the source branch
  source->erase(start,source->end());
  if (source->empty())
    delete source;

  if (this->isFinalized(last))
    return last;
  else
    return NULL;

}


#endif


#endif
