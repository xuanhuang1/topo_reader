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


#include "TreeScatter.h"

TreeScatter::TreeScatter(const std::vector<TopoTreeInterface*>& consumers,
                                    DomainDecomposition* decomposition) : mConsumers(consumers),
                                                                          mDecomposition(decomposition)
{
  sterror(mDecomposition==NULL,"Must pass a valid domain decomposition.");
  sterror(mConsumers.size()!=mDecomposition->size(),"The number of consumers in a\
 TreeScatter must equal the number of sub-domains in the decomposition");
}

TreeScatter::~TreeScatter()
{
}

FunctionType TreeScatter::minF() const
{
  FunctionType tmp = mConsumers[0]->minF();

  std::vector<TopoTreeInterface*>::const_iterator it;

  for (it=mConsumers.begin()+1;it!=mConsumers.end();it++)
    tmp = MIN(tmp,(*it)->minF());

  return tmp;
}

FunctionType TreeScatter::maxF() const
{
  FunctionType tmp = mConsumers[0]->maxF();

  std::vector<TopoTreeInterface*>::const_iterator it;

  for (it=mConsumers.begin()+1;it!=mConsumers.end();it++)
    tmp = MAX(tmp,(*it)->maxF());

  return tmp;
}

int TreeScatter::addVertex(GlobalIndexType id, FunctionType f)
{
  //if (id ==  85916)
  //  fprintf(stderr,"TreeScatter::Adding Vertex %d\n",id);

  const std::vector<DomainID>& domains = mDecomposition->getDomains(id);

  mMaxIndex = MAX(mMaxIndex,id);

  // If this vertex is shared by more than one sub-domains
  if (domains.size() > 1) {
    std::vector<DomainID>::const_iterator it;

    // We need to add it to all its incident domains
    for (it=domains.begin();it!=domains.end();it++) {
      //fprintf(stderr,"Adding vertex %d to domain %d.\n",id,*it);
      mConsumers[*it]->addVertex(id,f); // This vertex is obviously shared
    }
  }
  else {
    //fprintf(stderr,"Adding vertex %d to domain %d.\n",id,domains[0]);
    mConsumers[domains[0]]->addVertex(id,f);
  }
  return 1;
}


int TreeScatter::addPath(const std::vector<GlobalIndexType>& path)
{
  sterror(true,"TreeScatter::addPath is deprecated use addEdge instead.");
  return 0;
}

int TreeScatter::addEdge(GlobalIndexType i0, GlobalIndexType i1)
{
  //if ((i0 == 85916) || (i1 == 85916))
  //  fprintf(stderr,"TreeScatter:: Adding edge %d %d\n",i0,i1);

  const std::vector<DomainID>& domains = mDecomposition->getDomains(i0,i1);

  if (domains.size() > 1) {

    std::vector<DomainID>::const_iterator it;
    uint8_t shares = 0;
    uint8_t order;

    // Add the edge to the first consumer and precompute the boundary
    // information once and for all
    order = mConsumers[domains[0]]->addEdge(i0,i1);

    // If the edge could not be added because one of its vertices
    // hasn't been added no consumer contains that vertex and thus we
    // are done
    if (order == 0)
      return 0;

    // If v0 > v1 (v0 closer to the leafs than v1) and v1 is contained
    // in v0's boundary component
    if ((order == 1) && mDecomposition->commonBoundary(i1,i0)) {
      shares = 1;
    }
    // Otherwise, if v1 > v0 and v0 is contained in v1's boundary component
    else if ((order == 2) && mDecomposition->commonBoundary(i0,i1)) {
      shares = 2;
    }

    // Add the edge to the other consumers. Since neither the order
    // nor the boundary relations should change we can use the
    // pre-computed information to take care of the sharing
    for (it=domains.begin()+1;it!=domains.end();it++) {

      mConsumers[*it]->addEdge(i0,i1);

     }
  }
  else {
    return mConsumers[domains[0]]->addEdge(i0,i1);
  }

  return 1;
}

int TreeScatter::finalizeVertex(GlobalIndexType index, bool restricted)
{
  const std::vector<DomainID>& domains = mDecomposition->getDomains(index);
  std::vector<DomainID>::const_iterator it;

  //if (index == 85916)
  //  fprintf(stderr,"TreeScatter::Finalizing vertex %d\n",index);

  for (it=domains.begin();it!=domains.end();it++) {
    //fprintf(stderr,"Finalizing vertex %d to domain %d.\n",index,*it);
    mConsumers[*it]->finalizeVertex(index,restricted);
  }

  return 1;
}

bool TreeScatter::containsVertex(GlobalIndexType index)
{
  const std::vector<DomainID>& domains = mDecomposition->getDomains(index);
  std::vector<DomainID>::const_iterator it;

  bool contains = false;

  for (it=domains.begin();it!=domains.end();it++)
    contains = contains || mConsumers[*it]->containsVertex(index);

  return contains;
}

int TreeScatter::markUnshared(GlobalIndexType index)
{
  const std::vector<DomainID> domains = mDecomposition->getDomains(index);
  std::vector<DomainID>::const_iterator it;

  uint8_t success = 1;

  return success;
}

void  TreeScatter::setLowerBound(double bound)
{
  std::vector<TopoTreeInterface*>::const_iterator it;

  for (it=mConsumers.begin();it!=mConsumers.end();it++)
    (*it)->setLowerBound(bound);
}

void  TreeScatter::setUpperBound(double bound)
{
  std::vector<TopoTreeInterface*>::const_iterator it;

  for (it=mConsumers.begin();it!=mConsumers.end();it++)
    (*it)->setUpperBound(bound);
}

int TreeScatter::cleanup()
{
  std::vector<TopoTreeInterface*>::const_iterator it;

  for (it=mConsumers.begin();it!=mConsumers.end();it++) {
    //fprintf(stderr,"Cleaning consumer %d\n",it - mConsumers.begin());
    (*it)->maxIndex(mMaxIndex);
    (*it)->cleanup();
  }

  fprintf(stderr,"Done cleaning consumers\n");
  return 1;
}

