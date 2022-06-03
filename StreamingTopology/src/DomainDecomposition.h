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


#ifndef DOMAINDECOMPOSITION_H
#define DOMAINDECOMPOSITION_H

#include <vector>
#include "Definitions.h"

//! Data type used to represent domain ids
typedef uint32_t DomainID;

//! Baseclass for all domain decompositions
/*! A DomainDecomposition defines the interface for all domain
 *  decompositions a domain decomposition is an abstract construct
 *  that for a given set of global indices (it's domain space) knows
 *  which indices refer to boundray versus interior
 *  vertices. Furthermore, for each vertex it can return all incident
 *  domains. Domains are represented as integers, but the specific
 *  naming scheme is up to each implementation.
 */
class DomainDecomposition
{
public:

  //! Maximal numbers of incident domains
  static const uint16_t sMaxIncidentDomains;

  //! Default consructor
  DomainDecomposition();

  //! Copy constructor
  DomainDecomposition(const DomainDecomposition& decomposition);

  //! Destructor
  virtual ~DomainDecomposition();

  //! Return the number of sub-domains
  virtual DomainID size() const = 0; 

  //! Indicate whether a vertex is shared among diffferent sub-domains
  /*! This function determines whether the vertex corresponding to the
   *  given index lies on a shared boundary between two different
   *  sub-domains. A called to isShared is at least as fast as a call
   *  to getDomain and potentially faster as it does not have to
   *  determine the list of shared domains.
   *  @param index: Index of the vertex in question
   *  @return true if the vertex is shared by at least two subdomains; 
   *          false otherwise
   */
  virtual bool isShared(GlobalIndexType index) const = 0;

  //! Indicate whether the given edge is shared among sub-domains
  /*! This function determines whether the edge between the given
   *  indices lies on a shared boundary between different
   *  dub-domains. It is important to realize that this is *not*
   *  simply a test whether both vertices are shared. At corners of 3D
   *  grids for example both vertices can be part of a different
   *  boundary making the edge an interior edge (cutting through the
   *  data set). Such an edge is thus not shared among sub-domains. A
   *  called to isShared is at least as fast as a call to getDomain
   *  and potentially faster as it does not have to determine the list
   *  of shared domains.
   *  @param i0: index of the start vertex of the edge
   *  @param i1: index of the stop vertex of the edge
   *  @return true if the edge lies entirely on the boundary between
   *          at least two domains; false otherwise.
   */
  virtual bool isShared(GlobalIndexType i0, GlobalIndexType i1) const = 0;

  //! Indicate whether index shares a boundary component with boundary
  /*! Assuming both indices correspond to vertices on the boundary of
   *  subdomains this function determines whether the vertex
   *  corresponding to index lies on the same boundary component than
   *  the vertex corresponding to boundary. Note, that this function
   *  is *not* commutative, since a corner is part of its edges but
   *  the edges are not part of the corner component.
   *  @param index: index of the vertex we are interested in
   *  @param boundary: index of vertex identifying the boundary component
   *  @return true if index is part of boundary's component; false otherwise
   */
  virtual bool commonBoundary(GlobalIndexType index, GlobalIndexType boundary) const = 0;

  //! Return a vector of domain ids incident to the given vertex
  /*! This function finds all the domains incident to the given vertex
   *  and returns a reference to a vector contain all their ids. The
   *  content of the vector is guaranteed to remain valid until the
   *  next call of either of the two getDomain functions.
   *  @param index: Index of the vertex in question
   *  @return Reference to a vector containing the ids of all domains
   *          incident to this vertex
   */
  virtual const std::vector<DomainID>& getDomains(GlobalIndexType index) = 0;


  //! Return a vector of domain ids incident to the given edge
  /*! This function finds all the domains that share the given edge
   *  and returns a reference to a vector contain all their ids. The
   *  content of the vector is guaranteed to remain valid until the
   *  next call of either of the two getDomain functions.
   *  @param i0: index of the start vertex of the edge
   *  @param i1: index of the stop vertex of the edge
   *  @return Reference to a vector containing the ids of all domains
   *          that share this edge
   */
  virtual const std::vector<DomainID>& getDomains(GlobalIndexType i0,
                                                  GlobalIndexType i1) = 0;
protected:

  //! Vector used to store domain ids
  std::vector<DomainID> mDomains;
};

#endif
