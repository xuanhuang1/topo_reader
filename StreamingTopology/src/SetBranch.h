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

#ifndef SETBRANCH_H
#define SETBRANCH_H

#include <set>
#include "Vertex.h"



//! A SetBranch implements the BranchInterface using an STL set
template <class VertexClass>
class SetBranch 
{
public:

  //! Typedef for the comparison operator usde in the set
  //typedef bool (*VertexCmp)(const VertexClass*,const VertexClass*);

  //! Typedef for the underlying std::set class
  typedef std::set<VertexClass*,VertexCompare> BranchType;

  //! Typedef for teh set iterator
  typedef typename std::set<VertexClass*,VertexCompare>::iterator SetIterator;

  class iterator : public SetIterator
  {
  public:
    
    iterator() : SetIterator() {}
    
    //iterator(const iterator& it) : SetIterator(it) {}
    
    iterator(SetIterator it) : SetIterator(it) {}
    
    ~iterator() {}

    //iterator& operator=(const iterator& it) {static_cast<SetIterator>(*this) = it; return *this;}
    
    //! Comparison operator
    bool operator==(const iterator& it) const {return (static_cast<SetIterator>(*this) == it);}

    //protected:

    template <class DerivedVertex>
    DerivedVertex* pointer() {return static_cast<DerivedVertex*>(*static_cast<SetIterator>(*this));}

    template <class DerivedVertex>
    DerivedVertex& reference() {return *static_cast<DerivedVertex*>(*static_cast<SetIterator>(*this));}
  };
  
   
  //! Default constructor
  SetBranch(const VertexCompare& cmp);

  //! Destructor
  ~SetBranch() {}

  //! Return an iterator to the top of the branch
  iterator begin() const {return mBranch.begin();}

  //! Return the first element
  VertexClass* front() const {return *mBranch.begin();}

  //! Return an iterator point behind the last element of the branch
  iterator end() const {return mBranch.end();}

  //! Return the last element
  VertexClass* back() const {return *mBranch.rbegin();}

  //! Return the current size
  int size() {return mBranch.size();}

  //! Return whether the set is empty
  bool empty() {return mBranch.empty();}
  
  /******************************************************************
   ******************  Search Interface  ****************************
   ******************************************************************/

  //! Find the first position at which v would be inserted
  iterator find(VertexClass* v) const {return mBranch.lower_bound(v);}

  //! Insert an element into the branch
  iterator insert(VertexClass* v);

  //! Insert v using pos as starting point for the insertion
  iterator insert(const iterator& pos, VertexClass* v);

  //! Insert all elements of a sorted range
  void insert(const iterator& start, const iterator& stop);
  
  //! Erase v from the branch and return the number of erased elements
  int erase(VertexClass* v) {return mBranch.erase(v);}

  //! Erase the element pos is refering to
  void erase(const iterator& pos) {mBranch.erase(pos);}

  //! Erase all elements between start and stop [start,stop)
  void erase(const iterator& start,const iterator& stop) {mBranch.erase(start,stop);}

  /******************************************************************
   ******************  Global Information  **************************
   ******************************************************************/

  const VertexClass* upperBound() const {return &mMax;}
  
private:

  //! The stl data type used to store the information
  BranchType mBranch;
  
  //! The "largest" of all vertices ever part of this branch
  VertexClass mMax;
  
  //! The comparison operator used for this branch
  const VertexCompare mCmp;
};
  

template <class VertexClass>
SetBranch<VertexClass>::SetBranch(const VertexCompare& cmp) :
  mBranch(cmp), mMax(GNULL,gMinValue), mCmp(cmp)
{
}

template <class DerivedVertex>
class SetBranchIterator : public SetBranch<typename DerivedVertex::DataType>::iterator
{
public:

  typedef typename SetBranch<typename DerivedVertex::DataType>::iterator BaseClass;

  SetBranchIterator() : BaseClass() {}
    
  SetBranchIterator(const BaseClass& it) : BaseClass(it) {}
      
  ~SetBranchIterator() {}

  DerivedVertex* operator->() {return static_cast<DerivedVertex*>(*static_cast<BaseClass>(*this));}
  
  DerivedVertex& operator*() {return *static_cast<DerivedVertex*>(*static_cast<BaseClass>(*this));}

  // I would love to do this 
  // DerivedVertex* operator->() {return this->pointer<DerivedVertex>();}
  // DerivedVertex& operator*()  {return this->reference<DerivedVertex>();}
  // instead but I can't get this to compile
  // ptb 02/09/09
};

template <class VertexClass>
typename SetBranch<VertexClass>::iterator SetBranch<VertexClass>::insert(VertexClass* v) 
{
  if (mCmp.greater(v,&mMax))
    mMax = *v;
  
  return mBranch.insert(v).first;
}

template <class VertexClass>
typename SetBranch<VertexClass>::iterator SetBranch<VertexClass>::insert(const iterator& pos, VertexClass* v) 
{
  if (mCmp.greater(v,&mMax))
    mMax = *v;

  return mBranch.insert(pos,v);  
}

template <class VertexClass>
void SetBranch<VertexClass>::insert(const iterator& start, const iterator& stop) 
{
  if (mCmp.greater(*start,&mMax))
    mMax = *(*start);
  
  mBranch.insert(start,stop);  
}

#endif

