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

#include "UnionInfo.h"


UnionInfo::UnionInfo() :
  mUnionParent(NULL), mUnionNext(NULL), mUnionChild(NULL)
{
}

UnionInfo& UnionInfo::operator=(const UnionInfo& info)
{
  mUnionParent = info.mUnionParent;
  mUnionNext = info.mUnionNext;
  mUnionChild = info.mUnionChild;

  return *this;
}


bool UnionInfo::isRegular() const
{
  if (mUnionChild == NULL)
    return false;

  if (mUnionParent == NULL)
    return false;

  if (mUnionParent->next() != mUnionParent)
    return false;

  return true;
}
  
void UnionInfo::bypass()
{
  std::vector<UnionInfo*> p;

  parents<UnionInfo,UnionInfo>(p);
  

  mUnionChild->removeUp(this);

  for (unsigned int i=0;i<p.size();i++) {
    p[i]->mUnionChild = mUnionChild;
    mUnionChild->addUp(p[i]);
  }

  mUnionParent = NULL;
  mUnionNext = this;
  mUnionChild = NULL;
}


void UnionInfo::addUp(UnionInfo* v)
{
  sterror(v==NULL,"Cannot add NULL pointer to parent list.");
  sterror(v->next()!=v,"Incoming parent list should be trivial.");

  if (mUnionParent == NULL) 
    mUnionParent = v;
  else {
    v->next(mUnionParent->next());
    mUnionParent->next(v);    
  }
}

