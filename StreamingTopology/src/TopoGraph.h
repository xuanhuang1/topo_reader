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


#ifndef TOPOGRAPH_H
#define TOPOGRAPH_H

#include <math.h>
#include "Definitions.h"
#include "TopoGraphInterface.h"
#include "STMappedArray.h"
#include "Node.h"


using namespace std;


enum SplitType {
  ABSOLUTE_SPLIT    = 0,
  RELATIVE_SPLIT    = 1,
  LOG_SPLIT         = 2,
  RELLOG_SPLIT      = 3,
  VERTEXCOUNT_SPLIT = 4,
};

class DefaultNodeData
{
};

template <class NodeData = DefaultNodeData>
class TopoGraph : public TopoGraphInterface
{
public:

  //! Typedef to define the map from global to local index space
  typedef std::map<GlobalIndexType,LocalIndexType> IndexMapType;

  class InternalNode;

  //! Typedef to define the array containing the nodes
  typedef STMappedArray<InternalNode> NodeArrayType;

  /*! Internal class that combines the Node baseclass which contains
   *  all pointers and flags of a grah node with the user defined data
   **/
  class InternalNode : public Node, public NodeData
  {
  public:
    //! Constructor
    InternalNode() : Node(), NodeData() {}

    //! Constructor
    InternalNode(GlobalIndexType i, FunctionType f) : Node(i,f), NodeData() {}

    //! Copy Constructor
    InternalNode(const InternalNode& n) : Node(n), NodeData(n) {}

    //! Destructor
    ~InternalNode() {}

    //! Assignment operator
    InternalNode& operator=(const InternalNode& n) {
      Node::operator=(n);
      NodeData::operator=(n);
      return *this;
    }

    const vector<Node*>& up() const {return this->mUp;}

    InternalNode* up(uint32_t i) {return static_cast<InternalNode*>(this->mUp[i]);}

    const InternalNode* up(uint32_t i) const {return static_cast<InternalNode*>(this->mUp[i]);}

    const vector<Node*>& down() const {return this->mDown;}

    InternalNode* down(uint32_t i) {return static_cast<InternalNode*>(this->mDown[i]);}

    const InternalNode* down(uint32_t i) const {return static_cast<InternalNode*>(this->mDown[i]);}

    //! Return the parent of this node
    InternalNode* parent() {return static_cast<InternalNode*>(this->mParent);}

    //! Return the parent of this node
    const InternalNode* parent() const {return static_cast<InternalNode*>(this->mParent);}

    //! Set the parent pointer
    void parent(InternalNode *n) {this->mParent = n;}

  };

  //! Default constructor
  TopoGraph();

  //! Destructor
  virtual ~TopoGraph () {}

  //! Return the minimal function value
  FunctionType minF() const {return mMinF;}

  //! Return the maximal function value
  FunctionType maxF() const {return mMaxF;}

  //! Return the mapped array containing the nodes
  const STMappedArray<InternalNode>& nodes() const {return mNodes;}

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mMaxIndex = MAX(mMaxIndex,id);}
  
  //! Add the node with the given index and data to the graph
  int addNode(GlobalIndexType i, FunctionType f);

  //! Add the arc between i0 and i1 to the graph
  int addArc(GlobalIndexType i0, FunctionType f0,
             GlobalIndexType i1, FunctionType f1);
  
  //! Mark the vertex of the given index as final
  /*! Indicate that no more arcs will use the vertex with the given
   *  index. A basic TopoGraph has no notion of streaming and thus
   *  implements this function as no-op
   */
  virtual int finalizeNode(GlobalIndexType i, bool restriced) {return 1;}

  //! Indicate that no more nodes or arcs are coming
  virtual int cleanup() {return 1;}

  //! Create a compact map of the index space
  /*! Create a list of all active nodes in order of their appearance
   *  in the graph. The construct a map from the node id's to this
   *  compact index space. This function is used when storing
   *  graphs to avoid a secondary look-up structure
   */
  virtual int createActiveMap(std::map<GlobalIndexType,GlobalIndexType>& index_map);

  //! Add the arc between n0 and n1 to the graph
  int addArc(Node* n0, Node* n1);

  //! Remove the given node from the graph
  /*! Remove the given node from the graph unless it is still
   *  connected to arcs. 
   *  @param node: Pointer to the node we want to remove
   *  @return 1 if node was succesfully removed; 0 otherwise
   */
  int removeNode(Node* node);

  //! Remove the given node from the graph
  /*! Remove the given node from the graph unless it is still
   *  connected to arcs. 
   *  @param i: Global index of the node we want to remove
   *  @return 1 if node was succesfully removed; 0 otherwise
   */
  int removeNode(GlobalIndexType i);

  //! Remove one arc between n0 and n1 if it exists
  /*! Remove one arc between n0 and n1 if such an arc
   *  exists. Otherwise, do nothing and return 0.
   *  @param n0: Start of the arc to remove
   *  @param n1: End of the arc to remove 
   *  @return: 1 if an arc has been sucessfully removed
   *           0 otherwise
   */
  int removeArc(Node* n0, Node *n1);
  
  //! Remove the leaf branch that has node as leaf
  /*! This function assumes that the given node is a leaf of the tree
   *  (either only a single down or only a single up pointer) and it
   *  removes the leave along with its branch. Furthermore, if the
   *  branch point after the removal has become regular it will also
   *  be removed.
   */
  int removeLeafBranch(Node* node);
  
  //! Split the arc that corresponds to this node
  Node* splitArc(Node* node, GlobalIndexType index, FunctionType function);

  //! Find the active node corresponding to this index
  /*! Find the active node corresponding to the given index. This
   *  function is part of the interface for completing segmentation
   *  and is re-implemented in MultiResGraph.
   */
  virtual InternalNode* findActiveNode(GlobalIndexType index) {return mNodes.findElement(index);}

  //! Find the element with the given index
  virtual InternalNode* findElement(GlobalIndexType index) {return mNodes.findElement(index);}

  //! Find the element with the given index
  virtual const InternalNode* findElement(GlobalIndexType index) const {return mNodes.findElement(index);}

  //! Split a graph so that no arc is longer than the given delta
  virtual void splitGraph(FunctionType delta, SplitType type=ABSOLUTE_SPLIT);

  //! Determine whether a node with the given index exists and is virtual
  virtual bool isVirtualNode(GlobalIndexType index);
  
  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/
  
  //! Write the node in binary format to the given stream
  virtual void toFile(FILE* output) const;

  //! Save the current graph in ASCII format
  /*! This call will dump the current graph in ascii format. Only
   *  active vertices will be saved and the indices will be
   *  compactified accordingly. The format is as follows (all comments
   *  behind % are not written
   * 
   *  <float> <float> % minima-, maxima function value
   *  <int> % Number of nodes
   * 
   *  % For each node 
   *  <Node in ASCII format> % see Node<> 
   * 
   *  @param output: the stream to write the output to
   *  @param index_map: map that should be used to compactify the graph indices
   *                    If index_map == NULL a local map will be used
   */
  virtual void writeASCII(FILE* output,
                          IndexMapType* index_map=NULL);


  //! Read the node in binary format from the given stream
  virtual void fromFile(FILE* input);

protected:

  //! Mapped array of all node
  STMappedArray<InternalNode> mNodes;

  //! This inserts a node without id
  virtual InternalNode* insertVirtualNode(FunctionType f);

  //! Split all arcs 
  virtual void splitArc(Node* u,FunctionType delta,SplitType type);

  //! Return the length of an arc according to the given split type
  virtual FunctionType splitLength(FunctionType up, FunctionType down, SplitType type);

private:

  //! Minimal function value seen so far
  FunctionType mMinF;

  //! Maxmima function value seen so far
  FunctionType mMaxF;

  //! The maximal index of all incoming vertices
  GlobalIndexType mMaxIndex;
};


template <class NodeData>
TopoGraph<NodeData>::TopoGraph() : mNodes(TOPOGRAPH_BLOCK_BITS), mMinF(gMaxValue), mMaxF(gMinValue),
mMaxIndex(0)
{
}

template <class NodeData>
int TopoGraph<NodeData>::createActiveMap(std::map<GlobalIndexType,GlobalIndexType>& index_map)
{
  //typename MappedArray<typename TopoGraph<NodeData>::InternalNode>::const_iterator gIt;
  typename NodeArrayType::const_iterator gIt;
  GlobalIndexType count = 0;

  index_map.clear();
  for (gIt=nodes().begin();gIt!=nodes().end();gIt++) {
    if (gIt->isActive())
      index_map[gIt->id()] = count++;
  }

  return 1;
}


template <class NodeData>
int TopoGraph<NodeData>::addArc(GlobalIndexType i0, FunctionType f0,
                                GlobalIndexType i1, FunctionType f1)
{
  InternalNode* source;
  InternalNode* target;

  //fprintf(stdout," TopoGraph<NodeData>::Adding arc %d -> %d    %f  %f\n",i0,i1,f0,f1);;

  source = mNodes.findElement(i0);
  sterror(source==NULL,"All critical point should be added before adding an arc.");
  if (source == NULL) {
    source = mNodes.insertElement(InternalNode(i0,f0));
    mMinF = MIN(mMinF,f0);
    mMaxF = MAX(mMaxF,f0);
  }

  target = mNodes.findElement(i1);
  sterror(target==NULL,"All critical point should be added before adding an arc.");
  if (target == NULL) {
    target = mNodes.insertElement(InternalNode(i1,f1));
    mMinF = MIN(mMinF,f1);
    mMaxF = MAX(mMaxF,f1);
  }

  sterror(source == target,"TopoGraph allows no loops.");

  if (*target < *source)
    swap(source,target);


  source->addUp(target);
  target->addDown(source);

  return 1;
}

template <class NodeData>
int TopoGraph<NodeData>::addArc(Node* n0, Node* n1)
{
  if (*n0 < *n1) {
    n0->addUp(n1);
    n1->addDown(n0);
  }
  else {
    n1->addUp(n0);
    n0->addDown(n1);
  }

  return 1;
}

template <class NodeData>
int TopoGraph<NodeData>::addNode(GlobalIndexType i, FunctionType f)
{
  InternalNode* node;

  if (i == 991)
    fprintf(stderr,"break\n");

  node = mNodes.findElement(i);
  if (node == NULL) {
    mNodes.insertElement(InternalNode(i,f));
    mMinF = MIN(mMinF,f);
    mMaxF = MAX(mMaxF,f);
  }

  return 1;
}

template <class NodeData>
int TopoGraph<NodeData>::removeNode(Node* node)
{
  if (node == NULL)
    return 0;

  if ((node->upSize() != 0) || (node->downSize() != 0)) {
    stwarning("Cannot remove node with active arcs.");
    return 0;
  }

  return mNodes.deleteElement(node->id());
}

template <class NodeData>
int TopoGraph<NodeData>::removeNode(GlobalIndexType i)
{
  InternalNode* node;

  node = mNodes.findElement(i);

  if (node == NULL) {
    stwarning("Index not found in array. Cannot remove non-existing node.");
    return 0;
  }

  return removeNode(node);
}

template <class NodeData>
int TopoGraph<NodeData>::removeArc(Node* n0, Node* n1)
{
  sterror(n0==NULL,"Cannot remove arcs between NULL node pointer.");
  sterror(n1==NULL,"Cannot remove arcs between NULL node pointer.");
  sterror(n0==n1,"Cannot remove illegal loop.");

  if (*n0 < *n1) {
    if (n0->removeUp(n1) == 0) {
      stwarning("Could not find arc %d -> %d to remove.",n0->id(),n1->id());
      return 0;
    }
    if (n1->removeDown(n0) == 0) {
      stwarning("Could not find arc %d -> %d to remove.",n1->id(),n0->id());
      return 0;
    }
  }
  else {
    if (n1->removeUp(n0) == 0) {
      stwarning("Could not find arc %d -> %d to remove.",n1->id(),n0->id());
      return 0;
    }
    if (n0->removeDown(n1) == 0) {
      stwarning("Could not find arc %d -> %d to remove.",n0->id(),n1->id());
      return 0;
    }
  }

  return 1;
}


template <class NodeData>
int TopoGraph<NodeData>::removeLeafBranch(Node* node)
{
  Node* saddle;

  if (node->upSize() == 1) {
    if (node->downSize() != 0) {
      stwarning("Node %d is not a leaf and thus cannot be removed as leaf branch.",node->id());
      return 0;
    }

    saddle = node->up()[0];
  }
  else if (node->downSize() == 1) {
    if (node->upSize() != 0) {
      stwarning("Node %d is not a leaf and thus cannot be removed as leaf branch.",node->id());
      return 0;
    }

    saddle = node->down()[0];
  }
  else {
    stwarning("Node %d is not a leaf and thus cannot be removed as leaf branch.",node->id());
    return 0;
  }



  if (removeArc(node,saddle) == 0)
    return 0;

  if (saddle->type() == INTERIOR) {
    saddle->bypass();
    removeNode(saddle);
  }

  removeNode(node);

  return 1;
}

template <class NodeData>
Node* TopoGraph<NodeData>::splitArc(Node* node, GlobalIndexType index, FunctionType function)
{
  //LocalIndexType index;
  Node* split;
  Node* tmp;

  if (index == 13291)
    fprintf(stderr,"break\n");

  split = mNodes.insertElement(InternalNode(index,function));

  if (*node > *split) {
    sterror(node->downSize()>1,"Too many representatives invalid graph.");

    if (node->downSize() == 0)
      addArc(node,split);
    else {
      tmp = node->down(0);
      removeArc(node,tmp);
      addArc(node,split);
      addArc(split,tmp);
    }
  }
  else {
    sterror(node->upSize()>1,"Too many representatives invalid graph.");

    if (node->upSize() == 0)
      addArc(node,split);
    else {
      tmp = node->up(0);
      removeArc(node,tmp);
      addArc(node,split);
      addArc(split,tmp);
    }

  }

  return split;
}


template <class NodeData>
void TopoGraph<NodeData>::splitGraph(FunctionType delta, SplitType type)
{
  typename STMappedArray<InternalNode>::iterator it;

  sterror(type==VERTEXCOUNT_SPLIT,"A graph cannot split based on vertex count, only a segmentation can.");

  if (type == RELATIVE_SPLIT) {
    delta *= mMaxF - mMinF;
    type = ABSOLUTE_SPLIT;
  }
  else if (type == RELLOG_SPLIT) {
    delta *=  log(mMaxF-mMinF+1);
    type = LOG_SPLIT;
  }


  for (it=mNodes.begin();it!=mNodes.end();it++) {
    //if (it->id() == 5228)
    //  fprintf(stderr,"Splitting arc %d\n",it->id());
    splitArc(it,delta,type);
  }
}

template <class NodeData>
void TopoGraph<NodeData>::splitArc(Node* u,FunctionType delta, SplitType type)
{
  FunctionType middle;
  Node* v;
  Node* tmp;

  for (int i=0;i<u->downSize();i++) {
    if (splitLength(u->f(),u->down()[i]->f(),type) > delta) {
      middle = 0.5*(u->f() + u->down()[i]->f());

      tmp = u->down()[i];
      v = insertVirtualNode(middle);

      removeArc(u,u->down()[i]);
      addArc(u,v);
      addArc(v,tmp);

      splitArc(u,delta,type);
      splitArc(v,delta,type);
    }
  }
}

template <class NodeData>
bool TopoGraph<NodeData>::isVirtualNode(GlobalIndexType index)
{
  InternalNode* node;

  node = mNodes.findElement(index);

  if ((node != NULL) && node->isVirtual())
    return true;
  else
    return false;
}

template <class NodeData>
typename TopoGraph<NodeData>::InternalNode* TopoGraph<NodeData>::insertVirtualNode(FunctionType f)
{
  InternalNode* node;

  node = mNodes.insertElement(InternalNode(mMaxIndex+1,f));
  mMaxIndex++;
  node->setVirtual(true);

  mMinF = MIN(mMinF,node->f());
  mMaxF = MAX(mMaxF,node->f());

  return node;
}

template <class NodeData>
FunctionType TopoGraph<NodeData>::splitLength(FunctionType up, FunctionType down, SplitType type)
{
  switch (type) {

  case ABSOLUTE_SPLIT:
    return fabs(up-down);
    break;
  case RELATIVE_SPLIT:
    return fabs(up-down) / (mMaxF - mMinF);
    break;
  case LOG_SPLIT:
    if (fabs(up - down) < 10e-6)
      return 0;
    else
      return fabs(log(up-mMinF+1) - log(down-mMinF+1));
    break;
  case RELLOG_SPLIT:
    return fabs(log(up-mMinF+1) - log(down-mMinF+1)) / log(mMaxF-mMinF+1);
    break;
  case VERTEXCOUNT_SPLIT:
    sterror(true,"A TopoGraph cannot split based on vertex count. Only the segmentation can.");
    break;
  }

  return 0;
}


template <class NodeData>
void TopoGraph<NodeData>::toFile(FILE* output) const
{
  int8_t bytes;
  uint16_t data_size;

  mNodes.toFile(output);

  bytes = sizeof(GlobalIndexType);
  if ((GlobalIndexType)(-1) < 0)
    bytes = -bytes;

  fwrite(&bytes,sizeof(int8_t),1,output);
  fwrite(&GNULL,sizeof(GlobalIndexType),1,output);

  bytes = sizeof(LocalIndexType);
  if ((LocalIndexType)(-1) < 0)
    bytes = -bytes;

  fwrite(&bytes,sizeof(int8_t),1,output);
  fwrite(&LNULL,sizeof(LocalIndexType),1,output);

  fwrite(&gMaxValue,sizeof(double),1,output);
  fwrite(&gMinValue,sizeof(double),1,output);

  data_size = sizeof(FunctionType);
  fwrite(&data_size,sizeof(uint16_t),1,output);

  fwrite(&mMinF,sizeof(FunctionType),1,output);
  fwrite(&mMaxF,sizeof(FunctionType),1,output);

}

template <class NodeData>
void TopoGraph<NodeData>::writeASCII(FILE* output,IndexMapType* global_index_map)
{
  IndexMapType* index_map ;
  typename STMappedArray<InternalNode>::iterator it;
  int count;

  if (global_index_map == NULL)
    index_map = new IndexMapType();
  else
    index_map = global_index_map;

  count = 0;
  for (it=mNodes.begin();it!=mNodes.end();it++) {
    if (it->isActive()) {
      (*index_map)[it->id()] = count++;
    }
  }

  fprintf(output,"%f %f\n",mMinF,mMaxF);

  fprintf(output,"%d\n",count);

  for (it=mNodes.begin();it!=mNodes.end();it++) {
    if (it->isActive()) {
      it->saveASCII(output,*index_map);
    }
  }


  if (global_index_map == NULL)
    delete index_map;
}


template <class NodeData>
void TopoGraph<NodeData>::fromFile(FILE* output)
{
}

#endif

