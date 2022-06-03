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


#ifndef TREEGATHER_H
#define TREEGATHER_H

#include "TopoTreeInterface.h"
#include "TopoGraphInterface.h"

class TreeGather : public TopoGraphInterface
{
public:

  //! Default constructor
  TreeGather(TopoTreeInterface* consumer);

  //! Destructor
  ~TreeGather();

  //! Return the minimal function value
  FunctionType minF() const {return mConsumer->minF();}

  //! Return the maximal function value
  FunctionType maxF() const {return mConsumer->maxF();}

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mConsumer->maxIndex(id);}
 
  //! Add the node with the given index and data to the graph
  int addNode(GlobalIndexType i, FunctionType f);
  
  //! Add the arc between i0 and i1 to the graph
  int addArc(GlobalIndexType i0, FunctionType f0,
             GlobalIndexType i1, FunctionType f1);

  //! Mark the vertex of the given index as final 
  int finalizeNode(GlobalIndexType index,  bool restricted);
  
  //! Indicate that no more vertices or paths are coming
  int cleanup() {return mConsumer->cleanup();}

  void printTree() {mConsumer->printTree();}

  //! Split the arc that corresponds to this node
  Node* splitArc(Node* node, GlobalIndexType index, FunctionType function) {return NULL;}

  //! Find the active node corresponding to this index
  Node* findActiveNode(GlobalIndexType index) {sterror(true,"Function not implemented");return NULL;}

  //! Create a compact map of the index space
  int createActiveMap(std::map<GlobalIndexType,GlobalIndexType>& index_map) {sterror(true,"Function not implemented");return 0;}


protected:

  //! Pointer to the consumer gathering all info
  TopoTreeInterface* mConsumer;
};


#endif
