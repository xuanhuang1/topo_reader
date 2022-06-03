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


#ifndef UNIONSEGMENATION_H
#define UNIONSEGMENATION_H

#include "Definitions.h"
#include "TopoGraph.h"

//! A segmentation
class UnionSegmentation
{
public:

  //! Default constructor
  UnionSegmentation() {}

  //! Destructor
  virtual ~UnionSegmentation() {}

  //! Insert a vertex into the segmentation
  /*! Add the vertex wit the given id to the segment of the given id
   *  @param vertex_id:id of the vertex
   *  @param seg_id: index of the segment it (currently) belong to
   */
  virtual int insert(GlobalIndexType vertex_id, GlobalIndexType seg_id) = 0;

  //! Finalize a given partial segmentation
  /*! Given a partially complete segmentation this function uses the given
   *  TopoGraph to fully segment all vertices. In the initial segmentation each
   *  vertex contains the index of the vertex that used to be its next highest
   *  critical point. Following these "pointers" upward we can find a current
   *  critical point above the vertex. This critical point must be a node in the
   *  graph in which case we can follow the graph downward again to find the arc
   *  corresponding to the given vertex.
   *  @param graph: the current graph potentially simplified to some 
   *                persistence and split to whatever maximal branch
   *                desired.
   *  @param compact_indices: Flag to indicate whether the segmentation indices
   *                          should be compacted. If true, the function will
   *                          first determine all the active nodes in the graph
   *                          and number them by appearance (in the graph). The
   *                          segmentation will then store indices into this
   *                          compactified index space rather than mesh indices.
   */
  virtual int complete(TopoGraphInterface& graph) = 0;

protected:
  
  virtual Node* child(Node* node) = 0;

  virtual bool smaller(const Node& node, GlobalIndexType i, FunctionType f) = 0;
  virtual bool greater(const Node& node, GlobalIndexType i, FunctionType f) = 0;
};



#endif

