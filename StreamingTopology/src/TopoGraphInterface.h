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


#ifndef TOPOGRAPHINTERFACE_H
#define TOPOGRAPHINTERFACE_H

#include <map>
#include "Definitions.h"
#include "Node.h"

/*! This class defines the public interface used to create and handle
 *  a TopoGraph which is a merge-, split-, contour-tree, or Reeb graph
 *  containing only critical points. This interface only includes the
 *  calls used to add nodes and arcs and to query the current
 *  function range.
 */
class TopoGraphInterface 
{
public:
  
  //! Virtual destructor to shut up the compiler
  virtual ~TopoGraphInterface() {}

  //! Return the minimal function value
  virtual FunctionType minF() const = 0;

  //! Return the maximal function value
  virtual FunctionType maxF() const = 0;
  
  //! Set the highest used index
  virtual void maxIndex(GlobalIndexType id) = 0;

  //! Add the node with the given index and data to the graph
  virtual int addNode(GlobalIndexType i, FunctionType f) = 0;
  
  //! Add the arc between i0 and i1 to the graph
  virtual int addArc(GlobalIndexType i0, FunctionType f0,
                     GlobalIndexType i1, FunctionType f1) = 0;

  //! Mark the vertex of the given index as final 
  virtual int finalizeNode(GlobalIndexType i, bool restricted) = 0;

  //! Indicate that no more nodes or arcs are coming
  virtual int cleanup() = 0;

  //! Split the arc that corresponds to this node
  /*! Each node in the graph corresponds to an arc either above or
   *  below. This function will determine which direction the current
   *  graph uses and split the corresponding arc. The new node will
   *  be inserted at the given function value with the given index.
   * @param node: pointer to the corresponding node
   * @param index: The index of the new node
   * @param function: The function of the new node
   * @return a pointer to the newly inserted node
   */
  virtual Node* splitArc(Node* node, GlobalIndexType index, FunctionType function) = 0;
  
  //! Find the active node corresponding to this index
  /*! Find the active node corresponding to the given index. This
   *  function is part of the interface for completing segmentation
   *  and is re-implemented in MultiResGraph.
   */
  virtual Node* findActiveNode(GlobalIndexType index) = 0;

  //! Create a compact map of the index space
  /*! Create a list of all active nodes in order of their appearance
   *  in the graph. The construct a map from the node id's to this
   *  compact index space. This function is used when storing
   *  graphs to avoid a secondary look-up structure
   */
  virtual int createActiveMap(std::map<GlobalIndexType,GlobalIndexType>& index_map) = 0;
};

#endif
