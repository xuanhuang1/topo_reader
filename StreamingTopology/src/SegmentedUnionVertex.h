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


#ifndef SEGMENTEDUNIONVERTEX_H
#define SEGMENTEDUNIONVERTEX_H

#include "UnionVertex.h"
#include "SegmentationInfo.h"

//! A SegmentedUnionVertex is a UnionVertex with a segmentation index
/*! A SegmentedUnionVertex inherits all functionality from a standard
 *  UnionVertex and adds an additional global index. The index will be
 *  used to store information about which branch of a merge tree this
 *  vertex belongs to by storing the index of the upper critical point
 *  of the branch. 
 *
 *  There exist multiple scenarios of what exactly the segmentation
 *  index stores. For example, it could store the global index of
 *  another SegmentedUnionVertex but it could also store the index of
 *  a critical point. In each case the number of bits necessary for
 *  the index can vary drastically (a global index can easily need
 *  64-bit while we would not expect more than 32-bit many critical
 *  points). Therefore, the segmentation index type is passed as
 *  another template parameter
 */
template <typename SegIndexType = uint32_t>
class SegmentedUnionVertex : public UnionVertex
{
public:
  
  SegmentedUnionVertex(GlobalIndexType id=GNULL, FunctionType f=0) :
    Vertex(id,f), UnionVertex(id,f), mSegIndex(id) {}
  
  SegmentedUnionVertex(const SegmentedUnionVertex& vertex) : 
    Vertex(vertex), UnionVertex(vertex), mSegIndex(vertex.mSegIndex) {}

  ~SegmentedUnionVertex() {}
  
  SegmentedUnionVertex& operator=(const SegmentedUnionVertex& vertex);

  const SegmentationInfo<SegIndexType>& segIndex() const {return mSegIndex;}

  SegmentationInfo<SegIndexType>& segIndex() {return mSegIndex;}

private:
  
  SegmentationInfo<SegIndexType> mSegIndex;

};
  
template <class SegIndexType>
SegmentedUnionVertex<SegIndexType>&
SegmentedUnionVertex<SegIndexType>::operator=(const SegmentedUnionVertex& vertex)
{
  *static_cast<UnionVertex*>(this) = vertex;

  mSegIndex = vertex.mSegIndex;

  return *this;
}



#endif
