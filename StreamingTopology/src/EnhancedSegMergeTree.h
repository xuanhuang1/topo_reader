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


#ifndef ENHANCEDSEGMERGETREE_H
#define ENHANCEDSEGMERGETREE_H

#include "SegmentedUnionTree.h"
#include "EnhancedSegUnionVertex.h"
#include "EnhancedSegUnionAlgorithm.h"


template <typename SegIndexType = uint32_t>
class EnhancedSegMergeTree : public SegmentedUnionTree<EnhancedSegUnionVertex<SegIndexType> >
{
public:

  EnhancedSegMergeTree(TopoGraphInterface* graph, UnionSegmentation* segmentation);

  //! Write the current tree to stdout
  virtual void printTree();

};
  
template <typename SegIndexType>
EnhancedSegMergeTree<SegIndexType>::EnhancedSegMergeTree(TopoGraphInterface* graph, UnionSegmentation* segmentation)
  : SegmentedUnionTree<EnhancedSegUnionVertex<SegIndexType> >(graph,segmentation)
{
  this->mAlgorithm = new EnhancedSegUnionAlgorithm<EnhancedSegUnionVertex<SegIndexType> >(new VertexCompare(1));
}


template <typename SegIndexType>
void EnhancedSegMergeTree<SegIndexType>::printTree()
{
  typename STMappedArray<EnhancedSegUnionVertex<SegIndexType> >::iterator it;

  fprintf(stdout,"\n\nPrintTree:\n");
  for (it=this->mVertices.begin();it!=this->mVertices.end();it++) {
    if (it->child() != NULL)
      std::cout << "Node " << it->id() << "," << it->segIndex().index() << ","  << it->branch()->back()->id() << " -> "
          << it->child()->id() << "," << static_cast<EnhancedSegUnionVertex<SegIndexType>*>(it->child())->segIndex().index() << ","
          << static_cast<EnhancedSegUnionVertex<SegIndexType>*>(it->child())->branch()->back()->id() << "\n";


  }
}


#endif
