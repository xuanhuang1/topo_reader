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


#ifndef ENHANCEDSEGUNIONVERTEX_H
#define ENHANCEDSEGUNIONVERTEX_H

#include "SegmentedUnionVertex.h"
#include "SegmentationInfo.h"
#include "SetBranch.h"
#include "BranchInfo.h"

//! An EnhancedSegUnionVertex is an EnhancedUnionVertex with a segmentation index
template <typename SegIndexType = GlobalIndexType>
class EnhancedSegUnionVertex : public SegmentedUnionVertex<SegIndexType>
{
public:
  
  typedef SetBranch<EnhancedSegUnionVertex<SegIndexType> > BranchType;

  EnhancedSegUnionVertex(GlobalIndexType id=GNULL, FunctionType f=0) :
    Vertex(id,f), SegmentedUnionVertex<SegIndexType>(id,f) {}
  
  EnhancedSegUnionVertex(const EnhancedSegUnionVertex& vertex) :
    Vertex(vertex), SegmentedUnionVertex<SegIndexType>(vertex), mBranch(vertex.mBranch) {}

  ~EnhancedSegUnionVertex() {}
  
  EnhancedSegUnionVertex& operator=(const EnhancedSegUnionVertex<SegIndexType>& vertex);

  BranchType* branch() {return mBranch.branch();}

  void branch(BranchType* b) {mBranch.branch(b);}

private:
  
  BranchInfo<BranchType> mBranch;

};
  
template <class SegIndexType>
EnhancedSegUnionVertex<SegIndexType>&
EnhancedSegUnionVertex<SegIndexType>::operator=(const EnhancedSegUnionVertex<SegIndexType>& vertex)
{ 
  *static_cast<SegmentedUnionVertex<SegIndexType>*>(this) = vertex;
  
  mBranch = vertex.mBranch;
  
  return *this;
}
  
  

#endif
