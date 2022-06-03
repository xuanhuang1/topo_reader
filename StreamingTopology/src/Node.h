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


#ifndef NODE_H
#define NODE_H

#include <vector>

using namespace std;

#include "Definitions.h"
#include "Vertex.h"
#include "STMappedArray.h"

enum MorseType {
  MINIMUM      = 0,
  MERGE_SADDLE = 1,
  SPLIT_SADDLE = 2,
  MULTI_SADDLE = 3,
  MAXIMUM      = 4,
  REGULAR      = 5,
  ISOLATED     = 6,
};

class Node;

//! Comparison operator between node pointers
bool nodeCmp(const Node* u, const Node* v);

//! Node of a TopoGraph
class Node : public Vertex
{
public:

  //! Bitmask used for storing the finalized flag 
  static const uint8_t sActiveMask = 128;

  //! Bitmask used for storing the virtual flag
  static const uint8_t sVirtualMask = 64;

  //typedef vector<Node *>::iterator UpIterator;
  Node(GlobalIndexType id=GNULL, const FunctionType f=0) :
    Vertex(id,f), mPersistence(gMaxValue), mParent(NULL), mRepresentative(NULL)  {active(true);}

  virtual ~Node() {}

  bool operator<(const Node& n) const;

  bool operator>(const Node& n) const;

  const vector<Node*>& up() const {return mUp;}

  Node* up(uint32_t i) {return mUp[i];}

  const vector<Node*>& down() const {return mDown;}

  Node* down(uint32_t i) {return mDown[i];}

  //! Return the number of arcs pointing upwards
  int upSize() const {return mUp.size();}
  
  //! Return the number of arcs pointing downwards
  int downSize() const {return mDown.size();}

  //! Return the persistence of this node
  float persistence() const {return mPersistence;}

  //! Return the parent of this node
  Node* parent() {return mParent;}

  //! Return the parent of this node
  const Node* parent() const {return mParent;}

  //! Return the representative
  const Node* representative() const {return mRepresentative;}

  //! Determine the tree type of this node
  TreeType type() const;

  //! Determine the Morse type of this node
  MorseType morseType() const;

  //! Determine whether this node is currently active
  bool isActive() const {return this->getBitFlag(sActiveMask);}

  //! Determine whether this node is virtual
  bool isVirtual() const {return this->getBitFlag(sVirtualMask);}

  //! Add the up pointer to the mUp list
  void addUp(Node* u) {mUp.push_back(u);}

  //! Add the down pointer to the mDown list
  void addDown(Node* d) {mDown.push_back(d);}

  //! Set the persistence
  void persistence(float p) {mPersistence = p;}

  //! Set the parent pointer
  void parent(Node *n) {mParent = n;}

  //! Return the representative
  void representative(const Node* n) { mRepresentative = n;}

  //! Set the active flag
  void active(bool a) {this->setBitFlag(sActiveMask,a);}

  //! Set the virtual flag
  void setVirtual(bool v) {this->setBitFlag(sVirtualMask,v);}

  //! Remove the given up pointer
  int removeUp(Node* n);

  //! Remove the given down pointer
  int removeDown(Node* n);

  //! Bypass this node if it is regular
  int bypass();

  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/
  
  //! Write the node in binary format to the given stream
  /*! Write the node to the given stream in binary format. The format
   *  is the following
   *  <uint16_t> Number of up arcs
   *  <LocalIndexType> .... <LocalIndexType> index of the parents
   *  <uint16_t> Number of down arcs
   *  <LocalIndexType> .... <LocalIndexType> index of the children
   *  <float> persistence
   *  <LocalIndexType> Index of the parent pointer
   *  Binary vertex info
   */
  void toFile(FILE* output, const STMappedArray<Node>& graph) const;

  //! Save node in ASCII format
  /*! Dump the node in ASCII format. All local indices will be mapped
   *  using the given index map to allow compactification of a
   *  graph. The format is as follows
   *
   *  <float> % persistence
   *  <int> % parent index
   *  <int> % number of upward arcs
   *  <int> .. <int> % list of parent indices
   *  <int> % number of downward arcs
   *  <int> .. <int> % list of child indices
   */
  void saveASCII(FILE* output, const std::map<GlobalIndexType,LocalIndexType>& index_map) const;

  //! Read the node in binary format from the given stream
  virtual void fromFile(FILE* input, STMappedArray<Node>& graph);
  

protected:
  
  //! Collection of upwards pointers
  vector<Node*> mUp;

  //! Collection of downwards pointers
  vector<Node*> mDown;
  
  //! Persistence of this node
  float mPersistence;

  //! Pointer to the parent node
  Node* mParent;

  //! Pointer to the representative node
  const Node* mRepresentative;
};


#endif
