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


#ifndef UNIONINFO_H
#define UNIONINFO_H

#include <vector>

#include "Definitions.h"
#include "IteratorBase.h"
#include "Vertex.h"

//! The UnionInfo encapsulates all information for a merge-vertex
/*! The UnionInfo encapsulates the pointers and flags necessary to
 *  extend a default vertex to become a merge vertex. The UnionInfo
 *  stores a single child pointer as well as an arbitrary number of
 *  parent pointers. The parent pointers are stored as a single linked
 *  circular list in mParent. The list is considered to be unsorted
 *  and can of course be empty. While this might be slightly slower
 *  than storing a dynamic array it results in a constant memory
 *  foot-print.
 *
 *  The primary functional aspect of the UnionInfo is its interface
 *  consisting of the functions
 *
 *  child()
 *  isRegular()
 *  isLeaf()
 */ 

class UnionInfo {

public:

  friend class IteratorBase<UnionInfo>;

  //! Declaration of the merge tree parent iterator
  typedef IteratorBase<UnionInfo> iterator;

  //! Default constructor
  UnionInfo();
  
  //! Copy constructor
  UnionInfo(const UnionInfo& info) {*this = info;}
  
  //! Destructor
  ~UnionInfo() {}

  //! Assignment operator
  /*! This assignment operator actually does *not* copy the containter
   *  info as this should nto change in an assignment
   */
  UnionInfo& operator=(const UnionInfo& info);

  //! Start of the parent list
  iterator begin() const {return iterator(this,parent());}

  //! End of the parent list
  iterator end() const {return iterator(this,NULL);}

  //! Return whether this vertex is regular wrt. the current pointer
  bool isRegular() const;

  //! Return whether this vertex is currently a leaf in its tree
  bool isLeaf() const {return (mUnionParent == NULL);}
  
  //! Set the child pointer
  /*! Set the child pointer of the vertex to child and automatically
   *  clean-up all other references and the parent lists.  
   *  @param child: pointer to the new child
   */
  inline void child(UnionInfo* down);

  //! Relink all parents to point directly to the child
  void bypass();
  
  //! Initialize the next pointer with this
  void initializeNext() {mUnionNext = this;}
  
  //! Determine the vertex type according to the current pointers
  inline TreeType currentType();

  //! Hack function to allow setting the child pointer directly
  UnionInfo* representative() const {return mUnionChild;}

  //! Hack function to allow setting the child pointer directly
  void representative(UnionInfo* down) {mUnionChild = down;}

  //! Hack function to allow setting the mParent pointer directly
  UnionInfo* lowest() const {return mUnionParent;}

  //! Hack function to allow setting the mParent pointer directly
  void lowest(UnionInfo* down) {mUnionParent = down;}

protected:

  //! Pointer to one of the parents
  UnionInfo* mUnionParent;

  //! Pointer to the next parent in the list
  UnionInfo* mUnionNext;

  //! Pointer to the single union child
  UnionInfo* mUnionChild;

  //! Return the current child pointer
  UnionInfo* child() const {return mUnionChild;}

  //! Return the head of my parent list 
  UnionInfo* parent() const {return mUnionParent;}

  //! Return the next sibling in the parent list of my child
  UnionInfo* next() const {return mUnionNext;}

  //! Return a vector of all parents
  template <class DerivedClass, class MidLevelClass>
  void parents(std::vector<DerivedClass*>& p) const;

  void parent(UnionInfo* p) {mUnionParent = p;}

  //! Set the next pointer
  void next(UnionInfo* n) {mUnionNext = n;}

  //! Add v to the list of parents
  void addUp(UnionInfo* v);

  //! Remove v from the list of parents
  void removeUp(UnionInfo* v);

};  
   
template <class DerivedClass, class MidLevelClass>
void UnionInfo::parents(std::vector<DerivedClass*>& p) const
{
  iterator it;
  
  p.clear();
  for (it=begin();it!=end();it++)
    p.push_back(static_cast<DerivedClass*>(static_cast<MidLevelClass*>(*it)));
}

inline TreeType UnionInfo::currentType()
{
  if (mUnionParent == NULL)
    return LEAF;

  if (mUnionParent != mUnionParent->next())
    return BRANCH;

  if (mUnionChild == NULL)
    return ROOT;

  return INTERIOR;
}

inline void UnionInfo::child(UnionInfo* down)
{
  if (mUnionChild != NULL)
    mUnionChild->removeUp(this);

  if (down != NULL)
    down->addUp(this);

  mUnionChild = down;
}


inline void UnionInfo::removeUp(UnionInfo* v)
{
  iterator it;

  for (it=begin();it!=end();it++) {

    if ((*it)->next() == v)
      break;
  }

  sterror(it==end(),"Parent pointer to erase not found.");

  // If this is the only remaining parent
  if (*it == (*it)->next()) {

    mUnionParent = NULL;
  }
  else {
    // Shortcut the list
    (*it)->next((*it)->next()->next());

    // Relink v to itself
    v->next(v);

    // If we pointed to v as the head of the list
    if (parent() == v)
      parent(*it);
  }
}




#endif

