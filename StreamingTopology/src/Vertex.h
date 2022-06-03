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


#ifndef VERTEX_H
#define VERTEX_H

#include "Definitions.h"
#include "Multiplicity.h"
#include "MappedElement.h"

/*
enum MorseIndex {
  MINIMUM      = 0,
  SADDLE       = 1,
  MERGE_SADDLE = 2,
  SPLIT_SADDLE = 3,
  MAXIMUM      = 4,
  REGULAR      = 5,
  UNDETERMINED = 6,
};
*/

enum TreeType {
  ROOT            = 0,  
  BRANCH          = 1,  
  LEAF            = 2,
  INTERIOR        = 3,
  RESTRICTED      = 4,
};



//! Baseclass for all contour/merge/split tree vertices
/*! The Vertex class is the baseclass for all
 *  merge-/split-/contour-tree vertices. It stores the information
 *  common to all of them which is the function value, the original
 *  mesh index, and a collection of various flags.
 *
 *  It is important to keep in mind that this baseclass is designed to
 *  be virtually inherited from both merge- and split- vertices which
 *  in turn form the multiple inheritance of a contour vertex. 
 */
class Vertex : public FlexArray::MappedElement<GlobalIndexType,LocalIndexType>
{
public:

  static char sStr[50];

  //! Bitmask used to store the tree type
  static const uint8_t sTypeMask = 7;

  //! Bitmask used for storing the finalized flag 
  static const uint8_t sFinalizeMask = 8;

  //! Bitmask used for storing the finalized flag 
  static const uint8_t sProcessMask = 16;

  //! Bitmask used for storing whether a vertex is restricted
  static const uint8_t sRestrictedMask = 32;

  //! Static comparison operator to sort vector<UnionVertex*>
  static inline bool smaller(const Vertex* v0, const Vertex* v1);

  //! Static comparison operator to sort vector<UnionVertex*>
  static inline bool greater(const Vertex* v0, const Vertex* v1);

  //! Default constructor
  Vertex(GlobalIndexType id, FunctionType f);

  //! Copy constructor
  //explicit Vertex(const Vertex& v) { *this = v;}

  //! Destructor
  virtual ~Vertex() {}
  
  //! Assignment operator
  Vertex& operator=(const Vertex& v);

  //! Return the function value
  FunctionType f() const {return mFunc;}

  //! Return whether this vertex has been finalized
  bool isFinalized() const {return getBitFlag(sFinalizeMask);}

  //! Return whether this vertex has been processed
  bool isProcessed() {return getBitFlag(sProcessMask);}

  //! Return whether this vertex is restricted
  bool isRestricted() {return getBitFlag(sRestrictedMask);}

  //! Return the stored Morse index
  TreeType type() const {return (TreeType)(mFlags & sTypeMask);}

  //! Set the finalized flag to true
  void finalize() {setBitFlag(sFinalizeMask,true);}

   //! Set the process flag to true
  void process() {setBitFlag(sProcessMask,true);}

  //! Set the restricted flag
  void restrict() {setBitFlag(sRestrictedMask,true);}

  //! Set the Morse index
  void type(TreeType i) {mFlags = (mFlags & ~sTypeMask) | i;}
  
  //! Setting a flag using an actual bit mask
  void setBitFlag(uint8_t mask, bool f) {if (f) mFlags |= mask; else mFlags &= ~mask;}

  //! Getting a flag using an actual bit mask
  bool getBitFlag(uint8_t mask) const {return mFlags & mask;}

  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/

  virtual const char* toString() {sprintf(sStr,"%d",this->id());return sStr;}
  
  //! Write the node in binary format to the given stream
  virtual void saveBinary(FILE* output) const;

  //! Save the vertex information in ascii format
  void saveASCII(FILE* output) const;

  //! Read the node in binary format from the given stream
  virtual void loadBinary(FILE* input);

  /*************************************************************************************
   *******************     Parallel Interface ******************************************
   ************************************************************************************/


private:

  //! Various BitFlags as well as the Morse index
  uint8_t mFlags;

  //! The data stored with this vertex
  FunctionType mFunc;
};

//! This class implements all different comparisons between vertices to be used
//! in stl algorithms
class VertexCompare
{
public:

  VertexCompare(uint8_t flag) : mFlag(flag) {}

  VertexCompare(const VertexCompare& cmp) : mFlag(cmp.mFlag) {}

  ~VertexCompare() {}

  bool operator()(const Vertex* u, const Vertex* v) const
  {
    if (mFlag)
      return greater(u,v);
    else
      return smaller(u,v);
  }

  bool greater(const Vertex* u, const Vertex* v) const
  {
    if (u->f() > v->f())
      return true;

    if ((u->f() == v->f()) && (u->id() > v->id()))
      return true;

    return false;
  }

  bool smaller(const Vertex* u, const Vertex* v) const
  {
    if (u->f() < v->f())
      return true;

    if ((u->f() == v->f()) && (u->id() < v->id()))
      return true;

    return false;
  }
private:

  const uint8_t mFlag;
};


#endif
