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


#ifndef TOPOTREEINTERFACE_H
#define TOPOTREEINTERFACE_H

#include <vector>
#include "Definitions.h"
#include "Multiplicity.h"
#include "BoundaryMarker.h"

//! Interface definition for contour/merge/split trees
class TopoTreeInterface 
{
public:

  virtual ~TopoTreeInterface() {}

  //! Return the minimal function value of an accepted vertex
  virtual FunctionType minF() const = 0;

  //! Return the maximal function value of an accepted vertex
  virtual FunctionType maxF() const = 0;

  //! Set the highest used index
  virtual void maxIndex(GlobalIndexType id) = 0;
  
  //! Add the given vertex to the tree
  /*! If the function value of the given data is in the valid range
   *  (between mLowerBound and mUpperBound) this function constructs a
   *  new vertex with the given id and data and adds it to the
   *  internal value. In this case the function returns the local id
   *  of the new vertex. If the data is outside the valid range no
   *  vertex is constructed and the function returns LNULL
   *  @param id: global id of the new vertex
   *  @param f: the function associated with the new vertex
   *  @return local index of the new vertex or LNULL if the vertex was 
   *          filtered
   */
  virtual int addVertex(GlobalIndexType id, FunctionType f) = 0;
  
  //! Add the given path to the tree
  /*! This function add the given path to the tree using the virtual
   *  addPath interface function. First, all input id's are mapped to
   *  local pointers. If one or multiple of the input ids are not
   *  present in the tree (for example, because their values have been
   *  filtered) the path will be ignored and a 0 will be returned. For
   *  applications where all vertices *must* be present the compiler
   *  flag ST_COMPLETE_DOMAIN can be set to trigger an exception if an
   *  id is not found. If all ids can be mapped the virtual addPath
   *  function is called to incorporate the path into the current
   *  tree. All vertices of the path are assumed to be part of a
   *  *single* path in the tree.
   *  @return 1 if the path has been succesfully integrated; 0 otherwise
   */
  virtual int addPath(const std::vector<GlobalIndexType>& path) = 0;

  //! Add the given edge to the tree and return the order of the input vertices
  /*! This function adds the given edge to the tree. First, the two
   *  input id's are mapped to local pointers. If one or both of the
   *  input ids are not present in the tree (for example, because
   *  their values have been filtered) the edge will be ignored and a
   *  0 will be returned. For applications where all vertices *must*
   *  be present the compiler flag ST_COMPLETE_DOMAIN can be set to
   *  trigger an exception if an id is not found. In the process of
   *  adding the edge the function will also sort the input indices
   *  return 1 if the vertex corresponding to i0 is "higher" (closer
   *  to the leafs) than i1 and 2 otherwise.
   *  @return 1 if v(i0)>v(i1); 2 if v(0)<v(1); 0 if the edge could not
   *          be added.
   */
  virtual int addEdge(GlobalIndexType i0, GlobalIndexType i1) = 0;

  //! Finalize the vertex with the given index
  /*! This function will map the given global index to the appropriate
   *  vertex and call the virtual finalize function to process the
   *  actual event. In case the id is not found the finalization call
   *  will be ignored and 0 will be returned.  For applications where
   *  all vertices *must* be present the compiler flag
   *  ST_COMPLETE_DOMAIN can be set to trigger an exception if an id
   *  is not found. Finalizing a vertex indicates that no future paths
   *  will contain this vertex.
   *  @param index: global index of the vertex that should be finalized
   *  @param restricted: whether this vertex must be maintained in the
   *                     tree independent of whether it is critical
   */
  virtual int finalizeVertex(GlobalIndexType index, bool restricted=false) = 0;

  //! Determine whether the tree contains this vertex
  /*! Return whether the tree *currently* contains the vertex
   *  corresponding to the given index. 
   */
  virtual bool containsVertex(GlobalIndexType index) = 0;

  //! Indicate that no more vertices or paths are coming
  virtual int cleanup() = 0;

  //! Set upper bound of accepted function values
  /*! In many applications it is useful, for example, for performance
   *  reasons to ignore function values above/below a certain
   *  value. Vertices above the limit will not be added to the array
   *  and all path or finalization calls directed to them will be
   *  ignored. If vertices are filtered the ST_COMPLETE_DOMAIN should
   *  *not* be set as it will trigger an exception for missing
   *  ids. This function sets the upper bound which will ignore all
   *  vertices with function values strictly above the given bound
   *  @param bound: the new upper bound of acceptable function values
   */
  virtual void setUpperBound(double bound) = 0;

  //! Set lower bound of accepted function values
  /*! In many applications it is useful, for example, for performance
   *  reasons to ignore function values above/below a certain
   *  value. Vertices above the limit will not be added to the array
   *  and all path or finalization calls directed to them will be
   *  ignored. If vertices are filtered the ST_COMPLETE_DOMAIN should
   *  *not* be set as it will trigger an exception for missing
   *  ids. This function sets the lower bound which will ignore all
   *  vertices with function values strictly below the given bound
   *  @param bound: the new lower bound of acceptable function values
   */
  virtual void setLowerBound(double bound) = 0;

  //! Print the tree to the console
  virtual void printTree() {}
};


#endif
