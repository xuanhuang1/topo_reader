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

#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include "Vertex.h"
#include "Node.h"
#include "MorseNode.h"


template <class NodeDataClass = MorseNode>
class GraphNode : public Vertex
{
public:

  //! Bitmask used for storing the active flag 
  static const uint8_t sActiveMask = 128;

  GraphNode(GlobalIndexType id=GNULL, FunctionType f=0, MorseType t=ISOLATED) :
    Vertex(id,f), mData(id), mPersistence(gMaxValue), mParent(NULL) {morseType(t);active(true);}

  //! Return the stored Morse index
  MorseType morseType() const {return (MorseType)Vertex::type();}

  NodeDataClass data() {return mData;}

  //! Return the list of neighbors
  const vector<GraphNode*>& neighbors() const {return mNeighbors;}
  
  //! Return the list of neighbors
  vector<GraphNode*>& neighbors() {return mNeighbors;}

  //! Return the number of neighbors
  int valence() const {return mNeighbors.size();}

  //! Return the persistence of this node
  FunctionType persistence() const {return mPersistence;}
 
  //! Return the parent of this node
  const GraphNode* parent() const {return mParent;}

  //! Return the parent of this node
  GraphNode* parent() {return mParent;}

  //! Determine whether this node is currently active
  bool isActive() const {return this->getBitFlag(sActiveMask);}

  //! Set the Morse index
  void morseType(MorseType t) {Vertex::type((TreeType)t);}

  //! Add another neighbor without checking for duplicates
  void addNeighbor(GraphNode<NodeDataClass>* u) {mNeighbors.push_back(u);}

  //! Set the persistence
  void persistence(FunctionType p) {mPersistence = p;}

  //! Set the parent pointer
  void parent(GraphNode<NodeDataClass>* n) {mParent = n;}

  //! Set the active flag
  void active(bool a) {this->setBitFlag(sActiveMask,a);}

  //! Remove the given neighbor from the list
  int removeNeighbor(GraphNode<NodeDataClass>* n);

  //! Remove all neighbors
  int removeNeighbors() {mNeighbors.clear();return 1;}

private:

  //! Extra data
  NodeDataClass mData;

  //! Collection of all neighbors
  std::vector<GraphNode*> mNeighbors;

    //! Persistence of this node
  float mPersistence;

  //! Pointer to the parent node
  GraphNode* mParent;

  //! Private overload to avoid conflicts with morseType
  TreeType type() const {return LEAF;}
  
  void type(TreeType i) {}
};

template <class NodeDataClass>
int GraphNode<NodeDataClass>::removeNeighbor(GraphNode<NodeDataClass>* n)
{  
  typename vector<GraphNode<NodeDataClass>* >::iterator it;

  for (it=mNeighbors.begin();it!=mNeighbors.end();it++) {
    if (*it == n) {
      iter_swap(it,mNeighbors.end()-1);
      mNeighbors.pop_back();
      return 1;
    }
  }
  
  stwarning("Could not find neighbor pointer to remove.");
  return 0;
}


#endif

