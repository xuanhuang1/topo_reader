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

#ifndef HIERARCHICALMORSECOMPLEX_H
#define HIERARCHICALMORSECOMPLEX_H

#include <vector>
#include <queue>
#include <map>
#include <set>

#include "Graph.h"
#include "MultiResGraph.h"
#include "MorseNode.h"
#include "BlockedArray.h"

class HierarchicalMorseComplex : public Graph
{
public:

  typedef Graph::NodeType NodeType;

  class Arc {
  public:
    Arc(NodeType* n=NULL, NodeType* m=NULL) : u(n),v(m) {}
    ~Arc() {}
    
    NodeType* u;
    NodeType* v;
  };

  class Substitution {
  public:
    Substitution() : p(gMaxValue), extremum(NULL), saddle(NULL) {}
    Substitution(const Substitution& sub) {*this = sub;}
    ~Substitution() {}

    Substitution& operator=(const Substitution& sub) {
      p = sub.p;
      extremum = sub.extremum;
      saddle = sub.saddle;
      incoming = sub.incoming;
      outgoing = sub.outgoing;
      return *this;
    }

    bool operator<(const Substitution& sub) const {
      return (p < sub.p);
    }
    
    float p;
    NodeType* extremum;
    NodeType* saddle;
    std::vector<Arc> incoming;
    std::vector<Arc> outgoing;
  };
    
  HierarchicalMorseComplex() : Graph() {}

  virtual ~HierarchicalMorseComplex() {}

  //! Return the current persistence
  float persistence() {return mPersistence;}

  const FlexArray::BlockedArray<Substitution,LocalIndexType>& hierarchy() const {return mHierarchy;}

  //! Construct a multi-resolution hierarchy corresponding to the given metric
  int constructHierarchy(float persistence = gMaxValue/2, 
                         HierarchyMode mode = RECOVERABLE,
                         int direction=1);
 
  //! Adapt the hierarchy to the given persistence level
  void updatePersistenceLevel(int level);

  //! Adapt the hierarchy to the given persistence
  /*! This function will adapt the hierachy to the given persistence
   *  level. Note that, for non-montone hierarchy the function will
   *  look for the first cancellation that satisfies the given
   *  persistence rating so the results might be un-expected
   */
  void updatePersistence(float p);

  //! Find the active node corresponding to this index
  /*! Find the active node corresponding to the given index. This
   *  function will take the current state of the MultiresGraph into
   *  account. If the node that originally corresponded to this index
   *  has been simplified its parent/ancestor will be returned.
   */
  virtual NodeType* findActiveNode(GlobalIndexType index);

  //! Add the geometry associated to a certain arc
  void addPath(const std::vector<LocalIndexType>& path) {mGeometry.push_back(path);}

  //! Add a node to the list of multi-saddles
  void addMultiSaddle(NodeType* node);

  //! Output an xmgrace-style data file with the # extrema vs persistence graph
  void outputExtremaVsPersistence(FILE* output, const char* label=NULL);

  //! Output the cancellation tree
  void outputCancellationTree(FILE* output,bool ascii=true);

  //! Output full complex
  void outputComplex(FILE* output);

  //! Output the label hierarchy
  void outputLabelHierarchy(FILE* output);

private:

  struct Cancellation {
    Arc a;
    float p;
  };

  /*! A CancellationCmp implements the "less" operator for the
   *  priority queue used to sort cancellations. The cancellation with
   *  *highest* priority is canceled first. Thus if
   *  operator()(c0,c1)==true then c1 will be cancelled before c0
   */
  struct CancellationCmp {
    bool operator()(const Cancellation& c0, const Cancellation& c1) const {
      return (c0.p > c1.p);
    }
  };

  //! A vector of paths containing the arc geometry ordered from saddle -> extremum
  std::vector<std::vector<LocalIndexType> > mGeometry;

  //! The hierarchy encoded as a sequence of substitutions
  FlexArray::BlockedArray<Substitution,LocalIndexType> mHierarchy;

  //! Current persistence value
  /*! Persistence value corresponding to the current resolution of the
   *  hierarchy. Note that this value should be interpreted carefully
   *  in particular for non-monotone hierarchies. 
   */
  float mPersistence;

  //! Currently active hierarchy level
  /*! The level serves as integer-style metric allowing us to break
   *  metric ties as well as supporting non-monotone simplification
   *  metrics. The levels refer to the initial order of
   *  cancellations. Level 0 refers to the finest scale hierarchy,
   *  level mHierarchy.size() to the coarsest possible
   *  resolution. Thus, roughly speaking level 0 corresponds to
   *  persistence mHierarchy[0].p - epsilon. and level n to
   *  mHierarchy.back().p. Note that in principle these persistences do
   *  not need to be monotone.
   */
  uint32_t mLevel;

  //! A map of multi-saddles indexed by their mesh index
  std::map<GlobalIndexType,std::set<NodeType*> > mMultiSaddles;

  //! Remove the indicated branch 
  /*! Remove the arc indicated by the given cancellation from the
   *  hierarchy. Depending on the hierarchy mode used the arc and
   *  corresponding nodes will be permanetly removed from the
   *  hierarchy. Otherwise, the corresponding substitution is appended
   *  at the end of the hierarchy and the parent flags of the nodes
   *  are set accordingly. Given the lack or information currently the
   *  parent pointers are set to the first up/down node of the saddle
   *  that is removed. Note that this assignment is correct only for
   *  very specific hierarchies (e.g. highestSaddleFirst)
   */
  Substitution cancelBranch(const Cancellation& can, HierarchyMode mode);

  //! Refine the graph by adding the substitution of the given level
  void refineGraph(int level);

  //! Coarsen the graph by removing the substitution of the given level
  void coarsenGraph(int level);

  //! Compute and return teh extended persistence of a cycle
  /*! The extended persistence of a cycle is defined as the persistence
   * of the last possible cancellation that would have removed the given
   * saddle.
   */
  float getExtendedPersistence(const NodeType* saddle);

};



#endif
