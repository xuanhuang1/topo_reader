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


#ifndef BLOCKDECOMPOSITION_H
#define BLOCKDECOMPOSITION_H

#include "DomainDecomposition.h"

//! Domain decomposition of a regular grid into blocks
/*! A BlockDecomposition implements a the DomainDecomposition
 *  interface for the decomposition of aregular grid into uniform
 *  sized blocks.
 */
class BlockDecomposition : public DomainDecomposition
{
public:

  //! Default constructor to initialize the decomposition
  /*! Initialize the decomposition to split the entire given domain
   *  into sub_domain many pieces along each axis. 
   *  @param domain: 3-dimensional array containing the dimensions of 
   *                 the original grid
   *  @param sub_domains: 3-dimensional array containing the number of 
   *                      pieces each axis should be split into
   */
  BlockDecomposition(GlobalIndexType domain[], uint32_t sub_domains[]);

  //! Copy constructor
  BlockDecomposition(const BlockDecomposition& decomposition);

  //! Destructor
  virtual ~BlockDecomposition();

  //! Return the number of sub-domains
  virtual DomainID size() const {return mSubDomains[0]*mSubDomains[1]*mSubDomains[2];}
   
  //! Indicate whether a vertex is shared among diffferent sub-domains
  virtual bool isShared(GlobalIndexType index) const;

  //! Indicate whether the given edge is shared among sub-domains
  virtual bool isShared(GlobalIndexType index0, GlobalIndexType index1) const;

  //! Indicate whether index shares a boundary component with boundary
  virtual bool commonBoundary(GlobalIndexType index, GlobalIndexType boundary) const;

  //! Return a vector of domain ids incident to the given vertex
  virtual const std::vector<DomainID>& getDomains(GlobalIndexType index);

  //! Return a vector of domain ids incident to the given edge
  virtual const std::vector<DomainID>& getDomains(GlobalIndexType index0,
                                                  GlobalIndexType index1);

protected:
  
  //! Number of samples in each dimension of the global domain
  GlobalIndexType mGlobalDomain[3];

  //! Number of sub-domains in each axis
  DomainID mSubDomains[3];

  //! One less than the maximal size of the sub-domains along the x-axis
  /*! Vertices are repeated along boundaries. Thus 100 vertices cut in
   *  10 pieces must result in 11 vertices a piece. However, for the
   *  subdomain index computation you still want to compute relative
   *  to size 10. For example, all vertices that are multiples of 10
   *  are boundary vertices. Therefore, we store one less than the
   *  actial size
   */
  GlobalIndexType mSizeX;
  
  //! One less than the maximal size of the sub-domains along the y-axis
  /*! See mSizeX
   */
  GlobalIndexType mSizeY;
  
  //! One less than the maximal size of the sub-domains along the z-axis
  /*! See mSizeX
   */
  GlobalIndexType mSizeZ;
  
  //! Compute the result of cutting a set of cardinality size into at most n pieces
  GlobalIndexType computeSize(GlobalIndexType size, uint32_t n);

  //! Compute the index in each dimension for the given index
  void splitIndex(GlobalIndexType index, GlobalIndexType& i, GlobalIndexType& j, 
                  GlobalIndexType& k) const;
};
  
  
#endif
