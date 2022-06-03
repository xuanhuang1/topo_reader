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


#ifndef UNIONVERTEX_H
#define UNIONVERTEX_H

#include <vector>

#include "Definitions.h"
#include "Vertex.h"
#include "IteratorBase.h"
#include "UnionInfo.h"
#include "BoundaryMarker.h"

//! A UnionVertex is the vertex of of a complete merge- or split-tree
/*! A UnionVertex is the vertex of a complete merge tree. It stores a
 *  single child pointer as well as an arbitrary number of parent
 *  pointers. The parent pointers are stored as a single linked
 *  circular list in mParent. The list is considered to be unsorted
 *  and can of course be empty. While this might be slightly slower
 *  than storing a dynamic array it results in a constant memory
 *  foot-print.
 *
 *  The primary functional aspect of a UnionVertex is its interface
 *  consisting of the functions
 *
 *  child()
 *  isRegular()
 *  isLeaf()
 * 
 *  Note that a SplitVertex will implement the same interface but with
 *  very different results. For example, the type() of a MergeVertex
 *  can be REGULAR even if its alter-ego SplitVertex is a SADDLE. 
 */
class UnionVertex : public virtual Vertex, public UnionInfo
{
public:
  
  //! Default constructor
  UnionVertex(GlobalIndexType id=GNULL,FunctionType f=0);
  
  //! Copy constructor
  UnionVertex(const UnionVertex& vertex) : Vertex(vertex), UnionInfo(vertex) {}

  //! Destructor
  virtual ~UnionVertex() {}

  UnionVertex* child() const {return static_cast<UnionVertex*>(UnionInfo::child());}

  UnionVertex* parent() const {return static_cast<UnionVertex*>(UnionInfo::parent());}

  UnionVertex* next() const {return static_cast<UnionVertex*>(UnionInfo::next());}

  void parents(std::vector<UnionVertex*>& p) const {UnionInfo::parents<UnionVertex,UnionInfo>(p);}
  
  void child(UnionVertex* down) {UnionInfo::child(down);}
 
  UnionVertex* representative() const {return static_cast<UnionVertex*>(UnionInfo::representative());}
  
  void representative(UnionVertex* down) {UnionInfo::representative(down);}

  UnionVertex* lowest() const {return static_cast<UnionVertex*>(UnionInfo::lowest());}

  void lowest(UnionVertex* down) {UnionInfo::lowest(down);}
 
  //! Determine the vertex type according to the current pointers
  TreeType currentType() {return UnionInfo::currentType();}

};  
  

#endif
