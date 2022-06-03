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

#ifndef ENHANCEDSEGUNIONALGORITHM_H
#define ENHANCEDSEGUNIONALGORITHM_H

#include "EnhancedUnionAlgorithm.h"
#include "SegmentedUnionAlgorithm.h"
#include "EnhancedSegUnionVertex.h"

//! The enhanced union algorithm including the segementation
template <class VertexClass = EnhancedSegUnionVertex<> >
class EnhancedSegUnionAlgorithm : public EnhancedUnionAlgorithm<VertexClass>, public SegmentedUnionAlgorithm<VertexClass>
{
public:
  
  EnhancedSegUnionAlgorithm(VertexCompare* cmp) :
    UnionAlgorithm<VertexClass>(cmp), EnhancedUnionAlgorithm<VertexClass>(cmp), SegmentedUnionAlgorithm<VertexClass>(cmp) {}

  ~EnhancedSegUnionAlgorithm() {}

protected:

  //! If appropriate pass the seg index of v to its child
  virtual inline void propagateSegmentationIndex(VertexClass* v) const;


};
  

template <class VertexClass>
inline void EnhancedSegUnionAlgorithm<VertexClass>::propagateSegmentationIndex(VertexClass* v) const
{
  if (v->isProcessed()) {
    sterror(v->segIndex().index() != v->id(),"Found processed vertex %u that has segmentation index %u.",
            v->segIndex().index(),v->id());
    return;
  }

  if ((v->currentType() != ROOT) && (v->currentType() != INTERIOR))
    v->segIndex().index(v->id());
  else if (this->getParent(v) != NULL)
    v->segIndex().index(this->getParent(v)->segIndex().index());
}


#endif
