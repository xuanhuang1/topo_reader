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


#ifndef SEGMENTEDUNIONALGORITHM_H
#define SEGMENTEDUNIONALGORITHM_H

#include "UnionAlgorithm.h"
#include "SegmentedUnionVertex.h"

//! The union algorithm that includes computing the segmentation
template <class VertexClass = SegmentedUnionVertex<> >
class SegmentedUnionAlgorithm : public virtual UnionAlgorithm<VertexClass>
{
public:
  SegmentedUnionAlgorithm(VertexCompare* cmp);

private:

  virtual inline void setChild(VertexClass* v, VertexClass *u) const;
};


template <class VertexClass>
SegmentedUnionAlgorithm<VertexClass>::SegmentedUnionAlgorithm(VertexCompare* cmp)
  : UnionAlgorithm<VertexClass>(cmp)
{
}


template <class VertexClass>
inline void SegmentedUnionAlgorithm<VertexClass>::setChild(VertexClass* v, VertexClass *u) const
{
  VertexClass* w;

  w = this->getChild(v);
  
  v->child(u);
  
  if (w != NULL) {

    if (w->isProcessed() || ((w->currentType() != ROOT) && (w->currentType() != INTERIOR))) 
      w->segIndex().index(w->id());
    else
      w->segIndex().index(this->getParent(w)->segIndex().index());
    
    //if (w->segIndex().index() == 0)
    //  fprintf(stderr,"Setting segmentation index 0 for vertex %d\n",w->id());
  }

  if (u != NULL) {
    if (u->isProcessed() || ((u->currentType() != ROOT) && (u->currentType() != INTERIOR)))
      u->segIndex().index(u->id());
    else
      u->segIndex().index(v->segIndex().index());

    //if (u->segIndex().index() == 0)
    //  fprintf(stderr,"Setting segmentation index 0 for vertex %d\n",u->id());
  }
}

#endif

