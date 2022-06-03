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


#ifndef MULTIRESGRAPH_H
#define MULTIRESGRAPH_H

#include <vector>
#include <queue>
#include <stack>

#include "TopoGraph.h"
#include "BlockedArray.h"
#include "FeatureElement.h"
#include "Node.h"
#include "FileHandle.h"

//using namespace TopologyFileFormat;

enum HierarchyType {
  MINIMA_HIERARCHY = 0,
  MAXIMA_HIERARCHY = 1,
  MIXED_HIERARCHY  = 2,
};
 

template <class NodeData> class MultiResGraph;

//! The interface for a simplification metric
/*! An arc metric implements two important functions. The operator() evaluates
 *  the metric for the given branch. The greater function compares to
 *  cancellations to determine which should be performed first.
 */
template <class NodeData = DefaultNodeData>
class ArcMetric {
public:

  //! Default constructor
  ArcMetric(const TopoGraphInterface* graph) : mGraph(graph) {}

  //! Copy constructor
  ArcMetric(const ArcMetric& metric) : mGraph(metric.mGraph) {}
  
  virtual ArcMetric* clone() const = 0;
  
  //! Destructor
  virtual ~ArcMetric() {}
  
  //! Evaluate the metric for the branch u,v
  virtual float operator()(Node* u, Node* v) const = 0;

  //! Return an string identifying the metric
  virtual const char* name() const = 0;

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {
    life[0] = -0.001;
    life[1] = child.persistence();
    return 1;
  }   


  /*! For the given metric compare two cancellations. The one with
   *  lesser "persistence" will be cancelled first. This basic
   *  implementation breaks ties in persistence by cancelling
   *  lower/higher maxima/minima first.
   */
  virtual bool greater(const typename MultiResGraph<NodeData>::Cancellation& c0,
                       const typename MultiResGraph<NodeData>::Cancellation& c1) const {
    if (c0.p > c1.p) // If the persistences are unambiguous 
      return true;
    else if (c0.p == c1.p) {
      // If c0 is a max-saddle branch the lower maximum should be
      // cancelled (true == true), If c0 is a min-saddle branch the
      // higher minimum should be cancelled (false == false)
      if ((*c0.a.u > *c0.a.v) == (*c0.a.u > *c1.a.u)) 
        return true;

      return false;
    }    
    return false;
  }

protected:

  const TopoGraphInterface* mGraph;
};

//! The hierarchy mode describes how much information is stored
/*! When constructing a hierarchy the user has the choice of how much
 *  information is stored. 
 * 
 *  DESTRUCTIVE: No information is stored and the memory for all nodes
 *  that are simplified is freed. The simplified version of the graph
 *  becomes the new graph without any memory of the original graph
 *  
 *  RECORDED: All nodes that are simplified are permanently
 *  deactivated but they remain (inaccessible) parts of the
 *  graph. This allows to all original indices to be mapped to
 *  currently active nodes (via the parent pointers). However, the
 *  graph cannot be re-refined to include these nodes
 *
 *  RECOVERABLE: All cancellations are fully recorded and are
 *  reversible
 * 
 *  In practice the two primary use cases are 1) destructive
 *  cancellation where we do not have to adapt a segmentation and are
 *  not interested in any fine resolution information. 2) A recorded
 *  simplification followed by a recoverable hierarchy creation. This
 *  allows a noise removal with one metric (e.g. persistence) followed
 *  by a hierarchy construction using another metric. 
 */
enum HierarchyMode {

  DESTRUCTIVE = 0,
  RECORDED    = 1,
  RECOVERABLE = 2,
};

//! A TopoGraph that allows simplification according to an ArcMetric
/*! This class implements a multi-resolution TopoGraph. Given an
 *  initial graph the user can either destructively or
 *  non-destructively simplify the graph based as any given
 *  ArcMetric. The current algorithm will only cancel leaf branches
 *  but should be flexibile enough to handle non-monoton metrics (even
 *  though the resulting simplification sequence might not be what one
 *  would expect). If the hierarchy has been created non-destructively
 *  the graph can be adapted to any given persistence. Simplified
 *  MultiResGraphs can be used to complete (and simplify) a partial
 *  segmentation. However, due to the limited amount of information
 *  stored in a contour/merge/split/ tree  only special metrics will
 *  allow such a simplification. 
 */
template <class NodeData = DefaultNodeData>
class MultiResGraph : public TopoGraph<NodeData>
{
public:

  //! Typedef to define the map from global to local index space
  typedef std::map<GlobalIndexType,LocalIndexType> IndexMapType;

  typedef typename TopoGraph<NodeData>::InternalNode NodeType;
  
  class Arc {
  public:
    Arc(NodeType* n=NULL, NodeType* m=NULL) : u(n),v(m) {}
    ~Arc() {}
    
    NodeType* u;
    NodeType* v;
  };
      
  struct Cancellation {
    Arc a;
    float p;
  };

  // The string token used to indicate a simplification sequence
  static const char sXMLToken[30];

  //! Default constructor
  MultiResGraph();

  //! Destructor
  virtual ~MultiResGraph();
  
  //! Return the current persistence
  float persistence() {return mPersistence;}

  //! Return the last hierarchy metric used
  const ArcMetric<NodeData>* hierarchyMetric() const {return mHierarchyMetric;}

  //! Initialize the representative pointers assuming we have
  //! either a split or a merge tree
  int initializeRepresentatives(HierarchyType hierarchy_type);

  //! The recursive call to initialize representatives
  const Node* initializeRepresentative(HierarchyType hierarchy_type, Node* n);

  //! Construct a multi-resolution hierarchy corresponding to the given metric
  int constructHierarchy(const ArcMetric<NodeData>& metric,
                         HierarchyType hierarchy_type = MIXED_HIERARCHY,
                         float persistence = gMaxValue/2, 
                         HierarchyMode mode = RECOVERABLE);
  
  //! Clear a previously computed hierarchy
  void clearHierarchy();

  //! Adapt the hierarchy to the given persistence level
  void updatePersistenceLevel(int level);

  //! Adapt the hierarchy to the given persistence
  /*! This function will adapt the hierachy to the given persistence *
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

  //! Find the active leaf this index will eventually cancel to
  /*! Find the active leaf corresponding to the given index. If
   *  the index refers to a saddle or an inactive leaf this
   *  function will return the active node into which this node will
   *  ultimately cancel.
   */
  virtual const NodeType* findActiveLeaf(GlobalIndexType index);

  //! Find the active leaf this index will eventually cancel to
  /*! Find the active leaf corresponding to the given index. If
   *  the index refers to a saddle or an inactive leaf this
   *  function will return the active node into which this node will
   *  ultimately cancel.
   */
  virtual const NodeType* findActiveLeaf(const NodeType* node);


  //! Save the graph as ascii file 
  /*! This call will dump the current graph in ascii format. The
   *  format is as follows (all comments behind '%' are not written
   * 
   *  <TopoGraph ASCII Format>
   *  
   *  #################### 
   *  
   *  <int>  % The number of substitution
   *
   *  % For each substitution
   *  <Substitution in ASCII format> % See Substitution documentation
   */
  virtual void saveASCII(FILE* output);

  //! Write the simplification sequence xmlNode and information
  /*! This function will add its simplification xmlNode to the given family node
   *  and will write the simplification sequence in the format described by the
   *  wiki page to the file.
   *  @param output: The file pointer to which the data should be written
   *  @param family: The parent xml node to which we need to attach the new node
   *  @param ascii: A flag indicating whether to store the hierarchy in ascii or binary 
   *  @param hierarchy_type: Do we work with a maxima or minima hierarchy. Mixed
   *                         hierarchies are not supported
   *  @return The number of "features" / elements in the list that has been written
   */
  virtual LocalIndexType writeSimplificationSequence(std::ofstream& output, XMLNode& family, const bool ascii);

  //! Create the simplification sequence of active features 
  void createSimplificationSequence(TopologyFileFormat::Data<TopologyFileFormat::FeatureElement>& features,
                                    FunctionType& low, FunctionType& high, HierarchyType hierarchy_type);

  //! Create the list of global indices for the active features
  void createGlobalIndices(std::vector<GlobalIndexType>& indices);

private:

  
  /*! A CancellationCmp implements the "less" operator for the
   *  priority queue used to sort cancellations. The cancellation with
   *  *highest* priority is canceled first. Thus if
   *  operator()(c0,c1)==true then c1 will be cancelled before c0
   */
  class CancellationCmp {
  public:
    static const ArcMetric<NodeData>* mMetric;
    
    bool operator()(const Cancellation& c0, const Cancellation& c1) const {
      return mMetric->greater(c0,c1);
    }
  };
    
  //! Internal class storing the differences due to a cancellation
  class Substitution {
  public:
    Substitution(): p(gMaxValue), extremum(NULL), saddle(NULL) {}
    Substitution(const Substitution& sub) {*this = sub;}
    ~Substitution() {}

    Substitution& operator=(const Substitution& sub) {
      p = sub.p;
      extremum = sub.extremum;
      saddle = sub.saddle;
      incoming = sub.incoming;
      outgoing[0] = sub.outgoing[0];
      outgoing[1] = sub.outgoing[1];
      outgoing[2] = sub.outgoing[2];
      return *this;
    }
    
    //! Save the substitution in ascii file 
    /*! This call will dump the substitution in ascii format. The
     *  format is as follows (all comments behind '%' are not written
     *  <float> <int> <int> % persistence, extremum index, saddle index
     *          <int> <int> % node indices of the incoming arc
     *          <int> <int> % node indices of the first outgoing arc
     *          <int> <int> % node indices of the second outgoing arc
     *          <int> <int> % node indices of the third outgoing arc
     */
    void saveASCII(FILE* output,const MultiResGraph<NodeData>& graph,
                   const IndexMapType& index_map) const;

    float p;
    NodeType* extremum;
    NodeType* saddle;
    Arc incoming;
    Arc outgoing[3];
  };
  
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
  
  //! Pointer to the metric used to create the hierarchy
  const ArcMetric<NodeData>* mHierarchyMetric;
  
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
};

template <class NodeData>
void MultiResGraph<NodeData>::Substitution::saveASCII(FILE* output,
                                                      const MultiResGraph<NodeData>& graph,
                                                      const IndexMapType& index_map) const
{
  sterror(extremum==NULL,"Found substitution without extremum.");
  sterror(saddle==NULL,"Found substitution without extremum.");
  
  LocalIndexType e;
  LocalIndexType s;
  IndexMapType::const_iterator mIt;
  
  e = extremum->id();
  s = saddle->id();
  
  mIt = index_map.find(e);
  sterror(mIt==index_map.end(),"Could not find extremum %d in index map.",e);
  e = mIt->second;

  mIt = index_map.find(s);
  sterror(mIt==index_map.end(),"Could not find saddle %d in index map.",s);
  s = mIt->second;

  fprintf(output,"%f %d %d\n",p,e,s);
  

  if (incoming.u != NULL) {
    e = incoming.u->id();
    s = incoming.v->id();
    
    mIt = index_map.find(e);
    sterror(mIt==index_map.end(),"Could not find incoming u in index map.");
    e = mIt->second;
    
    mIt = index_map.find(s);
    sterror(mIt==index_map.end(),"Could not find incoming v index map.");
    s = mIt->second;

    fprintf(output,"\t %d %d\n",e,s);
  }
  else {
    fprintf(output,"\t -1 -1\n");
  }



  for (int i=0;i<3;i++) {

    if (outgoing[i].u != NULL) {
      e = outgoing[i].u->id();
      s = outgoing[i].v->id();
      
      mIt = index_map.find(e);
      sterror(mIt==index_map.end(),"Could not find outgoing[%d] u in index map.",i);
      e = mIt->second;
      
      mIt = index_map.find(s);
      sterror(mIt==index_map.end(),"Could not find outgoing[%d] v in index map.",i);
      s = mIt->second;
      
      fprintf(output,"\t %d %d\n",e,s);
    }
    else {
      fprintf(output,"\t -1 -1\n");
    }
      
  }
  
}

template<class NodeData>
const char MultiResGraph<NodeData>::sXMLToken[30] = "Simplification";


template <class NodeData>
const ArcMetric<NodeData>* MultiResGraph<NodeData>::CancellationCmp::mMetric = NULL;


template <class NodeData>
MultiResGraph<NodeData>::MultiResGraph() : TopoGraph<NodeData>(), mHierarchyMetric(NULL)
{
}

template <class NodeData>
MultiResGraph<NodeData>::~MultiResGraph()
{
  if (mHierarchyMetric != NULL)
    delete mHierarchyMetric;
}


template <class NodeData>
int MultiResGraph<NodeData>::initializeRepresentatives(HierarchyType hierarchy_type)
{
  typename STMappedArray<NodeType>::iterator nIt;
  std::stack<NodeType*> stack;
  typename std::vector<NodeType*>::const_iterator it;

  NodeType* top,rep;
  bool update;

  if ((hierarchy_type != MAXIMA_HIERARCHY) && (hierarchy_type != MINIMA_HIERARCHY)) {
    fprintf(stderr,"WARNING: Representatives are only define in pure hierarchies\n");
    return 0;
  }

  for (nIt=this->mNodes.begin();nIt!=this->mNodes.end();nIt++) {
    initializeRepresentative
    (hierarchy_type,nIt);
  }
  return 1;
}

template <class NodeData>
const Node* MultiResGraph<NodeData>::initializeRepresentative(HierarchyType hierarchy_type, Node* n)
{
  if (n->representative() != NULL)
    return n->representative();


  if (((hierarchy_type == MAXIMA_HIERARCHY) && (n->upSize() == 0)) ||
      ((hierarchy_type == MINIMA_HIERARCHY) && (n->downSize() == 0))) {
    n->representative(n);
    return n;
  }

  const Node* rep = n;
  std::vector<Node*>::const_iterator it;
  if (hierarchy_type == MAXIMA_HIERARCHY) {
    for (it=n->up().begin();it!=n->up().end();it++) {
      if (*rep < *initializeRepresentative(hierarchy_type,*it))
        rep = (*it)->representative();
    }
  }
  else {
    for (it=n->down().begin();it!=n->down().end();it++) {
      if (*rep > *initializeRepresentative(hierarchy_type,*it))
        rep = (*it)->representative();
    }
  }

  n->representative(rep);

  return rep;
}



template <class NodeData>
int MultiResGraph<NodeData>::constructHierarchy(const ArcMetric<NodeData>& metric,
                                                 HierarchyType hierarchy_type,
                                                 float persistence, HierarchyMode mode) 
{
  CancellationCmp::mMetric = &metric;
  std::priority_queue<Cancellation,std::vector<Cancellation>,CancellationCmp> queue;
  //std::priority_queue<Cancellation,std::vector<Cancellation>,metric.*greater> queue;
  typename STMappedArray<NodeType>::iterator nIt;
  Cancellation top;
  Substitution sub;
  NodeType* tmp;

  // If an old metric exists we want to free its memory first
  if (mHierarchyMetric != NULL) 
    delete mHierarchyMetric;

  mHierarchyMetric = metric.clone();
  
  initializeRepresentatives(hierarchy_type);

  // First we iterate through all nodes and collect the leafs branches
  // as potential cancellations
  for (nIt=this->mNodes.begin();nIt!=this->mNodes.end();nIt++) {

    //if (nIt->id() == 32887)
    //  fprintf(stderr,"break\n");

    if (nIt->type() == LEAF) {
      top.a.u = nIt;
      
      // Collect the correct branch end-points. Note, that for mixed hierarchies
      // a root branch will be entered into the queue twice. However, the active
      // flags should ensure that it is only canceled once
      if ((nIt->upSize() == 1) && (hierarchy_type != MAXIMA_HIERARCHY)){
        top.a.v = nIt->up(0);
        top.p = metric(top.a.u,top.a.v);
        //fprintf(stderr,"Adding %d %d   %f\n",top.a.u->id(),top.a.v->id(),top.p);
        queue.push(top);
      }
      else if ((nIt->downSize() == 1) && (hierarchy_type != MINIMA_HIERARCHY)) {
        top.a.v = nIt->down(0);
        top.p = metric(top.a.u,top.a.v);
        //fprintf(stderr,"Adding %d %d   %f\n",top.a.u->id(),top.a.v->id(),top.p);
        queue.push(top);
      }
    }
  }

  fprintf(stderr,"Collected leaf branches. Found %d branches\n",queue.size());
  while (!queue.empty()) {

    top = queue.top();
    queue.pop();

    // Note that top can potentially contain pointers to nodes that
    // have been removed. However, potentially the location has been
    // removed so it is not straight forward to determine whether or
    // not a pointer is still valid. Here we do it by checking whether
    // the global id currently stored matches the one the index map
    // has stored. 
    tmp = this->mNodes.findElement(top.a.u->id());
    if (tmp != top.a.u)
      continue;

    tmp = this->mNodes.findElement(top.a.v->id());
    if (tmp != top.a.v)
      continue;

    //If the saddle has already been
    // cancelled
    if (top.a.v->persistence() < gMaxValue) 
      continue;

    // If the persistence of the next smallest cancellation is larger
    // than the given threshold
    if (top.p > persistence)
      break;
    
    //if ((top.a.u->id() == 2146313) || (top.a.v->id() == 2146313))
    //fprintf(stderr,"Cancelling %d %d   %f\n",top.a.u->id(),top.a.v->id(),top.p);
    
    // Otherwise top represents the cancellation with smallest
    // "persistence"
    sub = cancelBranch(top,mode);
    
    // Now the last substitution tells us whether a new arc was
    // created
    if (sub.incoming.u != NULL) {
      //If so we must test whether this implies a new cancellation
      top.a.u = sub.incoming.u;
      top.a.v = sub.incoming.v;
      
      // We need u to be the extremu
      if (((hierarchy_type != MAXIMA_HIERARCHY) && (*top.a.u > *top.a.v)) ||
          ((hierarchy_type != MINIMA_HIERARCHY) && (*top.a.u < *top.a.v))) {
          std::swap(top.a.u,top.a.v);
      }

      if (top.a.u->type() == LEAF)  {

        if (((*top.a.u > *top.a.v) && (hierarchy_type != MINIMA_HIERARCHY)) || 
            ((*top.a.u < *top.a.v) && (hierarchy_type != MAXIMA_HIERARCHY))) {
          top.p = metric(top.a.u,top.a.v);
          //fprintf(stderr,"New arc %d %d  %.20f\n",top.a.u->id(),top.a.v->id(),top.p);
          
          /*
          if ((top.a.u->id() == 242728) || (top.a.v->id() == 242728)) {
            fprintf(stderr,"Added cancellation %d %d %f\n",top.a.u->id(),top.a.v->id(),top.p);
            top.p = metric(top.a.u,top.a.v);
          }
          */
          queue.push(top);
        }
      }
    }
  }

  fprintf(stderr,"Done with processing the queue.\n");

  if (mode != RECOVERABLE) {
    mLevel = 0;
    mPersistence = -1;
  }
  else {
    mLevel = mHierarchy.size();
    mPersistence = gMaxValue;
  }

  updatePersistenceLevel(0);

  fprintf(stderr,"Done with initializing hierarchy to 0\n");
  return 1;
}


template <class NodeData>
void MultiResGraph<NodeData>::clearHierarchy()
{
  mHierarchy.resize(0); // Remove all the old substitutions

  if (mHierarchyMetric != NULL) {
    delete mHierarchyMetric;
    mHierarchyMetric = NULL;
  }

  typename STMappedArray<NodeType>::iterator nIt;

  // Finally, iterate through all nodes and reset the persistences
  for (nIt=this->mNodes.begin();nIt!=this->mNodes.end();nIt++)
    nIt->persistence(gMaxValue);
}



template <class NodeData>
typename MultiResGraph<NodeData>::Substitution MultiResGraph<NodeData>::cancelBranch(const Cancellation& can,
                                                                                       HierarchyMode mode)
{
  Substitution sub;
  TreeType incoming_type;

  sub.extremum = can.a.u;
  sub.saddle = can.a.v;

  //if (sub.extremum->id() == 212)
  //  fprintf(stderr,"Cancelling %d %d\n",sub.extremum->id(),sub.saddle->id());
  incoming_type = sub.saddle->type();

  sub.outgoing[0] = Arc(sub.extremum,sub.saddle);

  this->removeArc(sub.extremum,sub.saddle);

  if (incoming_type == INTERIOR) {
    

    
    if (*sub.extremum > *sub.saddle) {
      sub.outgoing[1] = Arc(sub.saddle,sub.saddle->down(0));
    
      sub.incoming = Arc(sub.extremum,sub.saddle->down(0));
      this->removeArc(sub.saddle,sub.saddle->down(0));
    }
    else {
      sub.outgoing[1] = Arc(sub.saddle,sub.saddle->up(0));
    
      sub.incoming = Arc(sub.extremum,sub.saddle->up(0));
      this->removeArc(sub.saddle,sub.saddle->up(0));
    }
      

    this->addArc(sub.incoming.u,sub.incoming.v);
    
    if (mode == DESTRUCTIVE) {
      this->removeNode(sub.saddle);
    }
    else {
      sub.saddle->persistence(can.p);
      sub.p = can.p;

      if (*sub.extremum > *sub.saddle) {
        sub.saddle->parent(sub.incoming.u);
      }
      else {
        sub.saddle->parent(sub.incoming.v);
      }

      sub.saddle->active(false);

      if (mode == RECOVERABLE)
        mHierarchy.push_back(sub);
    }
  }
  else if (sub.saddle->type() == INTERIOR) {
    
    sub.outgoing[1] = Arc(sub.saddle,sub.saddle->up(0));
    sub.outgoing[2] = Arc(sub.saddle,sub.saddle->down(0));
    
    sub.incoming = Arc(sub.saddle->up(0),sub.saddle->down(0));
    
    this->removeArc(sub.saddle,sub.saddle->up(0));
    this->removeArc(sub.saddle,sub.saddle->down(0));
    
    this->addArc(sub.incoming.u,sub.incoming.v);
    
    if (mode == DESTRUCTIVE) {
      this->removeNode(sub.extremum);
      this->removeNode(sub.saddle);
    }
    else {
      sub.extremum->persistence(can.p);
      sub.saddle->persistence(can.p);
      sub.p = can.p;

      if (*sub.extremum > *sub.saddle) {
        sub.extremum->parent(sub.incoming.u);
        sub.saddle->parent(sub.incoming.u);
      }
      else {
        sub.extremum->parent(sub.incoming.v);
        sub.saddle->parent(sub.incoming.v);
      }

      sub.extremum->active(false);
      sub.saddle->active(false);

      if (mode == RECOVERABLE)
        mHierarchy.push_back(sub);
    }
  }
  else if (sub.saddle->type() == BRANCH) {
 

    if (mode == DESTRUCTIVE)
      this->removeNode(sub.extremum);
    else {
      sub.extremum->persistence(can.p);
      sub.p = can.p;

      if (*sub.extremum > *sub.saddle) 
        sub.extremum->parent(sub.saddle->up(0));
      else
        sub.extremum->parent(sub.saddle->down(0));
        
      sub.extremum->active(false);
    
      if (mode == RECOVERABLE)
        mHierarchy.push_back(sub);
    }
  }
  else if (sub.saddle->type() == ROOT) {
    
    //fprintf(stderr,"Cancelling root branch %d %d\n",sub.extremum->id(),sub.saddle->id());
    
    if (mode == DESTRUCTIVE) {
      this->removeNode(sub.extremum);
      this->removeNode(sub.saddle);
    }
    else {
      sub.extremum->persistence(can.p);
      sub.saddle->persistence(can.p);
      sub.p = can.p;

      sub.extremum->parent(NULL);
      sub.saddle->parent(NULL);
        
      sub.extremum->active(false);
      sub.saddle->active(false);
    
      if (mode == RECOVERABLE)
        mHierarchy.push_back(sub);
    }    
  }  
  else if (sub.saddle->type() == LEAF) {

    if (mode == DESTRUCTIVE)
      this->removeNode(sub.extremum);
    else {
      sub.extremum->persistence(can.p);
      sub.p = can.p;

      if (*sub.extremum > *sub.saddle) 
        sub.extremum->parent(sub.saddle->up(0));
      else
        sub.extremum->parent(sub.saddle->down(0));
        
      sub.extremum->active(false);
    
      if (mode == RECOVERABLE)
        mHierarchy.push_back(sub);
    }
    
  }
  else {
    sterror(true,"Case table not complete.");
  }

  return sub;
}

template <class NodeData>
void MultiResGraph<NodeData>::updatePersistenceLevel(int level)
{
  uint32_t effective;

  effective = MAX(MIN(level,(int)mHierarchy.size()),0);

  while ((mLevel > 0) && (mLevel > effective)) {
    mLevel--;
    refineGraph(mLevel);

    if (mLevel == 0)
      mPersistence = MIN(mHierarchy[mLevel].p-1,0);
    else
      mPersistence = mHierarchy[mLevel-1].p;
  }

  while ((mLevel < mHierarchy.size()) && (mLevel < effective)) {
    coarsenGraph(mLevel);
    mPersistence = mHierarchy[mLevel].p;

    mLevel++;
  } 
} 
 
template <class NodeData>
void MultiResGraph<NodeData>::updatePersistence(float p)
{
  uint32_t level = mLevel;


  while ((level > 0) && (mHierarchy[level].p > p))
    level--;

  while ((level < mHierarchy.size()) && (mHierarchy[level].p <= p))
    level++;

  //fprintf(stderr,"MultiResGraph<NodeData>::updatePersistence   %f  to level %d\n",p,level);
  
  updatePersistenceLevel(level);
} 
 
template <class NodeData>
typename MultiResGraph<NodeData>::NodeType* MultiResGraph<NodeData>::findActiveNode(GlobalIndexType index)
{
  NodeType* n;

  n = this->mNodes.findElement(index);
  
  sterror(n==NULL,"Node index %d not found MultiResGraph inconsistent.",index);
  
  while ((n != NULL) && (!n->isActive()))
    n = n->parent();
  
  return n;
}

template <class NodeData>
const typename MultiResGraph<NodeData>::NodeType* MultiResGraph<NodeData>::findActiveLeaf(GlobalIndexType index)
{
  NodeType* n;

  n = this->mNodes.findElement(index);
  
  sterror(n==NULL,"Node index %d not found MultiResGraph inconsistent.",index);
  
  return findActiveLeaf(n);
}

template <class NodeData>
const typename MultiResGraph<NodeData>::NodeType* MultiResGraph<NodeData>::findActiveLeaf(const NodeType* node)
{  
  sterror(node==NULL,"Cannot find NULL node.");
  
  while ((node != NULL) && (!node->isActive() || (node->type() != LEAF)))
    node = node->parent();
  
  return node;
}

template <class NodeData>
void MultiResGraph<NodeData>::refineGraph(int level)
{
  Substitution& sub = mHierarchy[level];
  
  sterror(this->mNodes.findElement(sub.saddle->id())!=sub.saddle,"Id mismatch multi-resolution graph inconsistent.");
  sterror(this->mNodes.findElement(sub.extremum->id())!=sub.extremum,"Id mismatch multi-resolution graph inconsistent.");
  // sterror(sub.extremum->type()!=ROOT,"Higher resolution nodes should be trivial.");

  sub.saddle->active(true);
  sub.extremum->active(true);
  
  // Unless this cancellation was the last one 
  if (sub.incoming.u != NULL)
    this->removeArc(sub.incoming.u,sub.incoming.v);
  
  this->addArc(sub.outgoing[0].u,sub.outgoing[0].v);
  
  if (sub.outgoing[1].u != NULL) 
    this->addArc(sub.outgoing[1].u,sub.outgoing[1].v);
  
  if (sub.outgoing[2].u != NULL) 
    this->addArc(sub.outgoing[2].u,sub.outgoing[2].v);
}


template <class NodeData>
void MultiResGraph<NodeData>::coarsenGraph(int level)
{
  Substitution& sub = mHierarchy[level];
  
  sterror(this->mNodes.findElement(sub.saddle->id())!=sub.saddle,"Id mismatch multi-resolution graph inconsistent.");
  sterror(this->mNodes.findElement(sub.extremum->id())!=sub.extremum,"Id mismatch multi-resolution graph inconsistent.");

  sub.saddle->active(false);

  if (sub.saddle->type() != INTERIOR)
    sub.extremum->active(false);

  //fprintf(stderr,"MultiResGraph<NodeData>::coarsenGraph deactivating %d %d\n",sub.saddle->id(),sub.extremum->id());

  this->removeArc(sub.outgoing[0].u,sub.outgoing[0].v);

  if (sub.outgoing[1].u != NULL) 
    this->removeArc(sub.outgoing[1].u,sub.outgoing[1].v);
  
  if (sub.outgoing[2].u != NULL) 
    this->removeArc(sub.outgoing[2].u,sub.outgoing[2].v);

  // Unless this cancellation was the last one 
  if (sub.incoming.u != NULL)
    this->addArc(sub.incoming.u,sub.incoming.v);
}


template <class NodeData>
void MultiResGraph<NodeData>::saveASCII(FILE* output)
{
  IndexMapType index_map;

  TopoGraph<NodeData>::writeASCII(output,&index_map);

  fprintf(output,"\n####################\n\n");
  
  fprintf(output,"%d\n",(int)mHierarchy.size()-mLevel);

  for (unsigned int i=mLevel;i<mHierarchy.size();i++) 
    mHierarchy[i].saveASCII(output,*this,index_map);

}


template <class NodeData>
LocalIndexType MultiResGraph<NodeData>::writeSimplificationSequence(std::ofstream& output,
                                                                     XMLNode& family, 
                                                                     const bool ascii)
{
  XMLNode simp;
  typename STMappedArray<NodeType>::iterator it;
  LocalIndexType count = 0;
  FunctionType life[2];
  uint8_t flag;
  LocalIndexType parent_index;
  GlobalIndexType index;
  map<GlobalIndexType,LocalIndexType> index_map;
  map<GlobalIndexType,LocalIndexType>::iterator mIt;
  TopologyFileFormat::FileOffsetType offset;

  sterror(mHierarchyMetric==NULL,"No graph hierarchy exists cannot write simplification sequence.");

  // First we the list of global indices that for the id of each features
  for (it=this->mNodes.begin();it!=this->mNodes.end();it++) {
    
    // Output only the nodes that are currently active 
    if (it->isActive()) {
      
      if (ascii) 
        output << " " << it->id() << "\n";
      else {
        index = it->id();
        output.write((const char*)&index,sizeof(GlobalIndexType));
      }

      index_map[it->id()] = count++;
    }
  }
  
  // Second we create the xml node to store our hierarchy information
  simp = family.addChild(sXMLToken);

  // Add the name of the hierarchy used to create this simplification sequence
  simp.addAttribute("name",mHierarchyMetric->name());
  
  // For the moment nodes can only store a single parent pointer thus the number
  // of dependence is always 1
  simp.addAttribute("numdependents",(uint32_t)1);
  
  // Add the address of the starting bytes of this simplification block
  offset = static_cast<TopologyFileFormat::FileOffsetType>(output.tellp());
  simp.addAttribute("addr",(uint64_t)offset);

  // Now output all vertices together with the simplification information
  for (it=this->mNodes.begin();it!=this->mNodes.end();it++) {

    if (it->isActive()) {

      flag = mHierarchyMetric->lifeTime(*it,life);
      
//       if (it->parent() != NULL) {
//         mIt = index_map.find(it->parent()->id());
//         sterror(mIt==index_map.end(),"Parent node not found in hierarchy.");
        
//         parent_index = mIt->second;
//         //fprintf(stderr,"Node %d with parent %d\n",it->id(),parent_index);
//       }
//       else
//         parent_index = LNULL;
      if (it->downSize() == 0)
        parent_index = LNULL;
      else {
        mIt = index_map.find(it->down(0)->id());
        sterror(mIt==index_map.end(),"Parent node not found in hierarchy.");
        
        parent_index = mIt->second;
      }
       

      
      
      // Write the simplification information which consists of the lifetime of
      // the feature a flag determining its "direction" and the single dependent
      // pointer
      if (ascii) {
        output << life[0] << " " << life[1] << " " << flag << " " << parent_index << "\n";
        //fprintf(output,"%e %e %u %llu\n",life[0],life[1],flag,(GlobalIndexType)parent_index);
      }
      else {
        output.write((const char*)life,sizeof(FunctionType)*2);
        output.write((const char*)&flag,sizeof(uint8_t));
        output.write((const char*)&parent_index,sizeof(GlobalIndexType));
      }
    }
  }
  
  return count;
}

template <class NodeData>
void MultiResGraph<NodeData>::createSimplificationSequence(TopologyFileFormat::Data<TopologyFileFormat::FeatureElement>& features,
                                                           FunctionType& low, FunctionType& high, HierarchyType hierarchy_type)
{
  typename STMappedArray<NodeType>::iterator it;
  FunctionType life[2];
  LocalIndexType count = 0;
  uint8_t flag;
  map<GlobalIndexType,LocalIndexType> index_map;
  map<GlobalIndexType,LocalIndexType>::iterator mIt;

  sterror(mHierarchyMetric==NULL,"No graph hierarchy exists cannot write simplification sequence.");
  sterror(hierarchy_type==MIXED_HIERARCHY,"NO mixed hierarchies are supported at this point.");

  // Make sure that the features are empty 
  features.clear();

  // Initialize the bounds
  low = 10e20;
  high = -10e20;

  // First we create the global -> local index map for all active nodes
  for (it=this->mNodes.begin();it!=this->mNodes.end();it++) {
    
    // For the nodes that are currently active 
    if (it->isActive()) 
      index_map[it->id()] = count++;
  }
  
  // The namespace here is ugly but for some reason the compiler will not accept
  // TopologyFileFormat::SINGLE_REPRESENTATIVE
  using namespace TopologyFileFormat;
  
  // Create enough FeatureElements 
  features.resize(count,FeatureElement(SINGLE_REPRESENTATIVE));

  count = 0;

  // Now create FeatureElements for all active nodes
  for (it=this->mNodes.begin();it!=this->mNodes.end();it++) {

    if (it->isActive()) {

      //if ((count == 285) || (count == 2899))
      //  fprintf(stderr,"break\n");

      flag = mHierarchyMetric->lifeTime(*it,life);
      
      //fprintf(stderr,"%d %f %f \n",it->id(),life[0],life[1]);

      low = std::min(low,life[0]);
      high = std::max(high,life[1]);

      features[count].lifeTime(life[0],life[1]);
      features[count].direction(flag);
      if ((hierarchy_type == MAXIMA_HIERARCHY) && (it->downSize() != 0)) {
        mIt = index_map.find(it->down(0)->id());
        sterror(mIt==index_map.end(),"Parent node not found in hierarchy.");
       
        features[count].addLink(mIt->second);
      }
      else if ((hierarchy_type == MINIMA_HIERARCHY) && (it->upSize() != 0)) {
        mIt = index_map.find(it->up(0)->id());
        sterror(mIt==index_map.end(),"Parent node not found in hierarchy.");

        features[count].addLink(mIt->second);
      }
      count++;
    }
  }
}


template <class NodeData>
void MultiResGraph<NodeData>::createGlobalIndices(std::vector<GlobalIndexType>& indices)
{  
  typename STMappedArray<NodeType>::const_iterator it;

  // First we create the global -> local index map for all active nodes
  for (it=this->nodes().begin();it!=this->nodes().end();it++) {
    
    // For the nodes that are currently active 
    if (it->isActive()) 
      indices.push_back(it->id());
  }

}


#endif
