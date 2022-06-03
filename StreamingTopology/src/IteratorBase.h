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


#ifndef ITERATORBASE_H
#define ITERATORBASE_H

//! Iterator through the parent pointers
/*! The iterator class provides a convinient way of cycling through
 *  all parent pointers without having to worry about the circular
 *  nature of the list. However, the iterator should be treated as
 *  std:vector iterators, meaning the underlying list should not be
 *  modified since that might invalidate the iterator.
 */
template <class VertexClass>
class IteratorBase {
public:
  
  friend IteratorBase<VertexClass> VertexClass::begin() const;
  friend IteratorBase<VertexClass> VertexClass::end() const;
  
  //! Default constructor
  IteratorBase();
    
  //! Copy constructor
  IteratorBase(const IteratorBase& it) {*this = it;}

  //! Destructor
  ~IteratorBase() {}
    
  //! Assignment operator 
  IteratorBase& operator=(const IteratorBase& it);

  //! Comparison operator
  bool operator==(const IteratorBase& it) const;

  //! Comparison operator
  bool operator!=(const IteratorBase& it) const {return !(*this == it);}

  //! Dereference operator
  VertexClass* operator*() const;

  //! Dereference operator
  VertexClass* operator->() const {return this->operator*();}

  //! Increment
  IteratorBase& operator++(int i);

private:
    
  //! Pointer to the MergeVertex through whose up-list we cycle
  const VertexClass* mSource;

  //! Pointer to the current parent
  VertexClass* mCurrent;

  //! Alternative constructor 
  IteratorBase(const VertexClass* cource, VertexClass* current);

};

template <class VertexClass>
IteratorBase<VertexClass>::IteratorBase() : mSource(NULL), mCurrent(NULL)
{
}

template <class VertexClass>
IteratorBase<VertexClass>::IteratorBase(const VertexClass* source,
                                        VertexClass* current) 
  : mSource(source), mCurrent(current)
{
  sterror(source == NULL,"Cannot construct IteratorBase from NULL Pointer.");
}

template <class VertexClass>
IteratorBase<VertexClass>& IteratorBase<VertexClass>::operator=(const IteratorBase<VertexClass>& it)
{
  mSource = it.mSource;
  mCurrent = it.mCurrent;
  
  return *this;
}
    
template <class VertexClass>
bool IteratorBase<VertexClass>::operator==(const IteratorBase& it) const
{
  if ((mSource == it.mSource) && (mCurrent == it.mCurrent))
    return true;
  
  return false;
}


template <class VertexClass>
VertexClass* IteratorBase<VertexClass>::operator*() const
{
  sterror(mSource == NULL,"Cannot dereference invalid IteratorBase.");
  sterror(mCurrent == NULL,"Cannot dereference end() IteratorBase.");

  return mCurrent;
}


template <class VertexClass>
IteratorBase<VertexClass>& IteratorBase<VertexClass>::operator++(int i)
{
  sterror(mSource == NULL,"Cannot increment invalid IteratorBase.");
  sterror(mCurrent == NULL,"Cannot increment IteratorBase  beyond \"end()\".");
  
  // If we have traversed the list once
  if (mCurrent->next() == mSource->parent()) {
    mCurrent = NULL; // We are now at the end
  }
  else {
    mCurrent = mCurrent->next();
    sterror(mCurrent->child() != mSource,"mMergeNext pointers inconsistent.");
  }

  return *this;
}
  

#endif
