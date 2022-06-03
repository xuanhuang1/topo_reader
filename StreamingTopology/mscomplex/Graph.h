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

#ifndef GRAPH_H
#define GRAPH_H

#include "GraphNode.h"
#include "STMappedArray.h"
#include "GenericData.h"
#include "Definitions.h"
#include "Node.h"


class Graph : public STMappedArray<GraphNode<MorseNode> >
{
public:

  //! Typedef to define the map from global to local index space
  typedef std::map<GlobalIndexType,LocalIndexType> IndexMapType;

  //! Convinience typedef for the node class
  typedef GraphNode<MorseNode> NodeType;

  //! Default constructor
  Graph();

  //! Default destructor
  virtual ~Graph() {}

  //! Return the minimal function value
  FunctionType minF() const {return mMinF;}

  //! Return the maximal function value
  FunctionType maxF() const {return mMaxF;}

  //! Return the minimal function value
  void minF(FunctionType f) {mMinF = f;}

  //! Return the maximal function value
  void maxF(FunctionType f) {mMaxF = f;}

  GlobalIndexType maxIndex() const {return  mMaxIndex;}

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mMaxIndex = MAX(mMaxIndex,id);}
  
  //! Add the node with the given index and data to the graph
  int addNode(GlobalIndexType i, FunctionType f, MorseType t);

  //! Add the arc between i0 and i1 to the graph
  int addArc(GlobalIndexType i0, GlobalIndexType i1);

  //! Add the arc between n0 and n1 to the graph
  int addArc(NodeType* n0, NodeType* n1);

  //! Remove the given node from the graph
  /*! Remove the given node from the graph unless it is still
   *  connected to arcs. 
   *  @param node: Pointer to the node we want to remove
   *  @return 1 if node was succesfully removed; 0 otherwise
   */
  int removeNode(NodeType* node);

  //! Remove the given node from the graph
  /*! Remove the given node from the graph unless it is still
   *  connected to arcs. 
   *  @param i: Global index of the node we want to remove
   *  @return 1 if node was succesfully removed; 0 otherwise
   */
  int removeNode(GlobalIndexType i);

  //! Remove one arc between n0 and n1 if it exists
  /*! Remove one arc between n0 and n1 if such an arc
   *  exists. Otherwise, do nothing and return 0.
   *  @param n0: Start of the arc to remove
   *  @param n1: End of the arc to remove 
   *  @return: 1 if an arc has been sucessfully removed
   *           0 otherwise
   */
  int removeArc(NodeType* n0, NodeType *n1);
 
  //! Find the active node corresponding to this index
  /*! Find the active node corresponding to the given index. This
   *  function is part of the interface for completing segmentation
   *  and is re-implemented in MultiResGraph.
   */
  virtual NodeType* findActiveNode(GlobalIndexType index) {return this->findElement(index);}

private:


  //! Minimal function value seen so far
  FunctionType mMinF;

  //! Maxmima function value seen so far
  FunctionType mMaxF;

  //! The maximal index of all incoming vertices
  GlobalIndexType mMaxIndex;
};

#endif
