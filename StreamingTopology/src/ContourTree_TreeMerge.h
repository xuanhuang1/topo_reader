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

#ifndef CONTOURTREE_TREEMERGE_H
#define CONTOURTREE_TREEMERGE_H

#include <iostream>
#include <algorithm>
#include <TopoTreeInterface.h>
#include "GenericData.h"
#include "SegmentedUnionTree.h"
#include "SegmentedSplitTree.h"
#include "SegmentedMergeTree.h"
#include "FlexArray/OOCArray.h"
#include "MTSegmentation.h"
#include "STSegmentation.h"
#include "UnionSegmentation.h"
#include "Parser.h" //Needed for completion of the segmentation
#include "EnhancedSegUnionVertex.h"
#include <TopoTreeInterface.h>
#include <TopoGraph.h>
#include "GraphIO.h"
#include "Vertex.h"
#include "MultiResGraph.h"
#include "Definitions.h"
#include <stdint.h> //usage of uint8_t
#include <math.h>
#include <queue>
#include <map>
#include <limits.h>


using namespace std;


/*! \brief Contour tree implementation based on concurrent merge/split tree computation
 *
 *  This implementation computes the merge and split-tree first and then merges
 *  the two trees (and their segementation) once computation of the merge tree
 *  and split tree are done. The algorithm is in this sencse close to the algorithm
 *  described by Carr et al.
 */
template < class VertexClass=EnhancedSegUnionVertex<>  >
class ContourTree_TreeMerge : public TopoTreeInterface
{
public:

  /*! \brief Default constructor
   *  \param graph : The contour tree graph
   *  \param segmentation : The contour tree segmentation (optional)
   *  \param mergeTree : The merge tree to be used (optional)
   *  \param splitTree : The split tree to be used (optional)
   *  \param mergeGraph : The graph of the merge tree (optional)
   *  \param splitGraph : The graph of the split tree (optional)
   */
  ContourTree_TreeMerge(TopoGraphInterface* graph,
                        SegmentedUnionTree< VertexClass >* mergeTree=NULL ,
                        SegmentedUnionTree< VertexClass >* splitTree=NULL,
                        TopoGraphInterface* mergeGraph=NULL,
                        TopoGraphInterface* splitGraph=NULL);

  //! Destructor
  virtual ~ContourTree_TreeMerge();

  //! Return the merge tree
  SegmentedUnionTree< VertexClass >* mergeTree() {return mMergeTree;}

  //! Return the split tree
  SegmentedUnionTree< VertexClass >* splitTree() {return mSplitTree;}

  /*! \brief Get the graph of the merge tree
   *
   * NOTE: The mergeTree will be empty after the computation is complete, i.e.,
   *       finalizeVertex(...)  has been executed. The complete mergeTree is
   *       available after all vertices and edges have been added and finalized
   *       but finishTree(...) has not been called yet.
   */
  TopoGraphInterface* mergeGraph() const { return mMergeGraph; }

  /*! \brief Get the graph of the split tree
   *
   * NOTE: The splitTree will be empty after the computation is complete, i.e.,
   *       finalizeVertex(...)  has been executed. The complete spltTree is
   *       available after all vertices and edges have been added and finalized
   *       but finishTree(...) has not been called yet.
   */
  TopoGraphInterface* splitGraph() const { return mSplitGraph; }

  /*! \brief Get the graph of the contour tree
   *
   * NOTE: The contour tree will be empty until the computation is actually
   *       complete, i.e., finishTree(...) is done.
   */
  TopoGraphInterface* contourGraph() const { return mContourGraph; }

  //Get the merge tree segmentation
  FlexArray::OOCArray<GlobalIndexType>* mergeTreeSegmentation() const {return mMergeTree->segmentation();}

   //Get the split tree segmentation
  FlexArray::OOCArray<GlobalIndexType>* splitTreeSegmentation() const {return mSplitTree->segmentation();}


  //! Return the minimal function value of an accepted vertex
  FunctionType minF() const {return mMinF;}

  //! Return the maximal function value of an accepted vertex
  FunctionType maxF() const {return mMaxF;}

  //! Set the highest used index
  void maxIndex(GlobalIndexType id) {mMaxIndex = MAX(mMaxIndex,id);}

  //! Add the given vertex to the tree
  virtual LocalIndexType addVertex(GlobalIndexType id, FunctionType data,
                                   bool shared = false);

  //! Add the given path to the the tree
  virtual int addPath(const vector<GlobalIndexType>& path);

  //! Add the given edge to the tree
  virtual int addEdge(GlobalIndexType i0, GlobalIndexType i1);

  //! Finalize the given vertex
  virtual int finalizeVertex(GlobalIndexType index);

  //! NOTE: This function is implemented only because it is required by the interface but is without function here.
  virtual bool containsVertex(GlobalIndexType index){ return false; }

  //! Mark the vertex with the given index as unshared
  virtual int markUnshared(GlobalIndexType index);

  //! Indicate that no more vertices or paths are coming
  virtual int cleanup();

  /*! \brief Complete the computation of the segmented merge and split tree
   *         and compute the contour tree
   *  \param attribute : The attribute data needed to complete the
   *                     segmentation of the merge tree and split tree.
   */
  virtual void finishTree(const FlexArray::BlockedArray< FunctionType >& attribute);

  //! Define the upper threshold value
  virtual void setUpperBound(double bound);

  //! Define the lower threshold value
  virtual void setLowerBound(double bound);


protected:
  /** \brief Finalize the segmentation computed for the merge-tree and the split-tree.
   */
  virtual void finalizeMergeAndSplitTreeSegmentation(const FlexArray::BlockedArray< FunctionType >& attribute);

  /** \brief This function inserts the merge-indices in the split-tree and vice versa.
   *
   *  NOTE: Call this function before calling merge Trees.
   */
  virtual void augmentMergeTreeAndSplitTree();


   /*! \brief Function to compute the contour tree from the augmented
   *          merge and split-tree.
   *
   * This function is called at the end by finishTree function to complete
   * the computation of the contour-tree. NOTE: Call augmentMergeTreeAndSplitTree()
   * first.
   * \return unsigned int: The number of arcs of the contour tree
   */
  virtual unsigned int mergeTrees();

  /*! \brief This function is called directly after an arc has been added to the contour tree.
   *
   *  Use this function to implement and additional functionality needed to, e.g,
   *  create a mapping for the arcs.
   *  \param node1 : Frist incident vertex of the arc
   *  \param node2 : Second incident vertex of the arc
   *  \param mergeArc : Did the arc origninate from the mergeTree (==1) or the splitTree(==0)
   */
  virtual void addArcInternal( Node* node1, Node* node2, bool mergeArc ) {}

   /* \brief Simple test function for writing a graph to a dot file
   * \param baseName : Name+path of the outputfile
   * \param graph    : The graph to be written.
   * \param resolution : Resolution of the output dot file
   */
  void writeTree( std::string filename, TopoGraph<>& graph , int resolution);

  //! Maximal function value seen so far
  FunctionType mMaxF;

  //! Minimal function value seen so far
  FunctionType mMinF;

  //! Upper bound of acceptable function values
  double mUpperBound;

  //! Lower bound of acceptable function values
  double mLowerBound;

  //! Highest incoming index seen so far
  GlobalIndexType mMaxIndex;

  //! The graph for the contour tree
  TopoGraphInterface* mContourGraph;

   //! Pointer to the merge tree algorithm
  SegmentedUnionTree<>* mMergeTree;

  //! Pointer to the split tree algorithm
  SegmentedUnionTree<>* mSplitTree;

  //! The graph of the merge tree
  TopoGraphInterface* mMergeGraph;

  //!The graph of the split tree
  TopoGraphInterface* mSplitGraph;

  //! Do we own mMergeTree
  bool mPrivateMergeTree;

  //! Do we own mMergeGraph
  bool mPrivateMergeGraph;

  //! Do we own mSplitTree
  bool mPrivateSplitTree;

  //! Do we own mSplitGraph
  bool mPrivateSplitGraph;
};



template <class VertexClass>
ContourTree_TreeMerge<VertexClass>::ContourTree_TreeMerge(TopoGraphInterface* graph ,
                                                          SegmentedUnionTree<VertexClass>* mergeTree ,
                                                          SegmentedUnionTree<VertexClass>* splitTree ,
                                                          TopoGraphInterface* mergeGraph,
                                                          TopoGraphInterface* splitGraph
                                                                         )
  : mMaxF(gMinValue), mMinF(gMaxValue), mUpperBound(gMaxValue), mLowerBound(gMinValue),
    mContourGraph(graph), mMergeTree(mergeTree), mSplitTree(splitTree),
    mMergeGraph(mergeGraph), mSplitGraph(splitGraph), mPrivateMergeTree(false),
    mPrivateMergeGraph(false), mPrivateSplitTree(false), mPrivateSplitGraph(false) 
{
  //Create the merge graph if not given
  if(!mMergeGraph){
    mPrivateMergeGraph=true;
    mMergeGraph = new MultiResGraph<>();
  }

  //Create the merge tree if not given
  if( !mMergeTree ){
    mPrivateMergeTree = true;
    mMergeTree = new EnhancedSegMergeTree<>( mMergeGraph , NULL);
  }

  //Create the split graph if not given
  if(!mSplitGraph){
    mPrivateSplitGraph=true;
    mSplitGraph = new MultiResGraph<>();
  }

  //Create the split tree if not given
  if( !mSplitTree ){
    mPrivateSplitTree = true;
    mSplitTree = new EnhancedSegSplitTree<>( mSplitGraph , NULL);
  }
}

template <class VertexClass>
ContourTree_TreeMerge<VertexClass>::~ContourTree_TreeMerge()
{
  //Free up all memory that belongs to us
  if ( mPrivateMergeTree )
    delete mMergeTree;

  if ( mPrivateSplitTree )
    delete mSplitTree;

  if( mPrivateMergeGraph )
    delete mMergeGraph;

  if( mPrivateSplitGraph )
    delete mSplitGraph;
}

template <class VertexClass>
LocalIndexType ContourTree_TreeMerge<VertexClass>::addVertex(GlobalIndexType id, FunctionType f, bool shared)
{
  //Compute the max id
  maxIndex(id);

  //Check if the vertex is within the threshold range
  if ((f < mLowerBound) || (f > mUpperBound))
    return LNULL;

  //Compute the max/min function value within the threshold range
  mMaxF = MAX(mMaxF,f);
  mMinF = MIN(mMinF,f);

  //Add the vertex to the merge tree and the split tree
  mMergeTree->addVertex( id , f, shared );
  mSplitTree->addVertex( id , f, shared );
  return LNULL; // For now ignore the return values
}

template <class VertexClass>
int ContourTree_TreeMerge<VertexClass>::addPath(const vector<GlobalIndexType>& path)
{
  //Add the path to the merge and split tree
  mMergeTree->addPath( path );
  mSplitTree->addPath( path );

  return 1; // For now ignore the return values
}

template <class VertexClass>
int ContourTree_TreeMerge<VertexClass>::addEdge(GlobalIndexType i0, GlobalIndexType i1)
{
  //Add the edge to the merge tree and the split tree
  mMergeTree->addEdge( i0 , i1 );
  mSplitTree->addEdge( i0 , i1 );
  return 1;// For now ignore the return values
}

template <class VertexClass>
int ContourTree_TreeMerge<VertexClass>::finalizeVertex(GlobalIndexType index)
{
  //Finalize the vertex in the merge tree and the split tree
  mMergeTree->finalizeVertex( index );
  mSplitTree->finalizeVertex( index );
  return 1;
}


template <class VertexClass>
int ContourTree_TreeMerge<VertexClass>::markUnshared(GlobalIndexType index)
{
  //mark the vertex as unshared in the merge tree and the split tree
  int mt = mMergeTree->markUnshared(index);
  int st = mSplitTree->markUnshared(index);
  return std::max(mt,st);
}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::setUpperBound(double bound)
{
  //Save the upper bound and inform the merge tree and the split tree
  mUpperBound = bound;
  mMergeTree->setUpperBound( bound );
  mSplitTree->setUpperBound( bound );
}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::setLowerBound(double bound)
{
  //Save the lower bound and inform the merge tree and the split tree
  mLowerBound = bound;
  mMergeTree->setLowerBound( bound );
  mSplitTree->setLowerBound( bound );
}

template <class VertexClass>
int ContourTree_TreeMerge<VertexClass>::cleanup()
{
  //Make sure everything is finished
  mMergeTree->cleanup();
  mSplitTree->cleanup();
  return 1;
}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::finishTree(const FlexArray::BlockedArray< FunctionType >& attribute)
{
  //Now compute the acutal contour tree
  //Finalize the merge and split tree
  finalizeMergeAndSplitTreeSegmentation(attribute);
  //Insert all split nodes in the contour tree and vice versa
  augmentMergeTreeAndSplitTree();
  //Merge the merge tree and the split tree to compute the contour tree.
  mergeTrees();

}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::finalizeMergeAndSplitTreeSegmentation(const FlexArray::BlockedArray< FunctionType >& attribute)
{
   //Finish the segmentation for the merge tree
  FlexArray::BlockedArray<GlobalIndexType >* partialMergeSegmentation = mMergeTree->segmentation();
  MTSegmentation<>* mtSegmentation = new MTSegmentation<>();
  mtSegmentation->complete_uint32t(static_cast<TopoGraph<>&>(*mMergeGraph), partialMergeSegmentation, attribute, false, NULL);

  //Finish the segmentation for the split tree
  FlexArray::BlockedArray<GlobalIndexType >* partialSplitSegmentation = mSplitTree->segmentation();
  STSegmentation<> *stSegmentation = new STSegmentation<>();
  stSegmentation->complete_uint32t(static_cast<TopoGraph<>&>(*mSplitGraph), partialSplitSegmentation, attribute, false, NULL);

  delete stSegmentation;
  delete mtSegmentation;
}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::augmentMergeTreeAndSplitTree()
{
  //Some typedef to make the code more readable
  typedef Node* nodeP;

  //1. Declaration and initalization of basic varibales
  TopoGraph<>* mG = static_cast<TopoGraph<>* >(mMergeGraph) ;
  TopoGraph<>* sG = static_cast<TopoGraph<>* >(mSplitGraph) ;
  FlexArray::BlockedArray<GlobalIndexType >* mGSeg = mMergeTree->segmentation();
  FlexArray::BlockedArray<GlobalIndexType >* sGSeg = mSplitTree->segmentation();
  VertexCompare mGComp = VertexCompare(1); //GREATER
  VertexCompare sGComp = VertexCompare(0); //SMALLER
  MappedArray<TopoGraph<>::InternalNode>::iterator it;
  nodeP parentNode, insertedNode, childNode;

  //2. Insert the split-points into the merge-tree and vice versa.
  //2.1 Iterate through all points of the merge tree and insert them in the
  //    split tree if needed.
  for(it = mG->nodes().begin(); it!=mG->nodes().end() ; it++){
    //If the vertex is not in the split tree then insert it
    if( !sG->findElement( it->id() )  ){
      //Find the insertion point
      parentNode = sG->findElement( (*sGSeg)[it->id()] );
      sterror( !parentNode , "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Unable to locate vertex in SplitTree");
      //Now we need to traverse the tree down to find the actual insertion point
      //This is necessary because we have already added other internal (non-critical)
      //points along the same arc, and we need to insert the point at the correct
      //location within the arc.
      while(  parentNode->upSize()>0 && sGComp.smaller( parentNode->up()[0] , it ) ){
        parentNode = parentNode->up()[0];
      }
      childNode = parentNode->up()[0];

      //Add the new node to the split tree
      sG->addNode( it->id() , it->f()  );
      insertedNode = sG->findElement( it->id() );
      //Now define the connections between the points to insert the node in the graph
      sG->removeArc( parentNode , childNode );
      sG->addArc( parentNode , insertedNode );
      sG->addArc( insertedNode , childNode);
    }
  }

  //2.2 Iterate through all points of the merge tree and insert them in the
  //    split tree if needed.
  for (it = sG->nodes().begin(); it != sG->nodes().end(); it++) {
    //INTERIOR nodes are from the merge tree and, must hence already be in the merge tree (avoid unnecessary search operation)
    if (it->type() != INTERIOR) 
    {
      //If the vertex is not in the split tree then insert it
      if (!mG->findElement(it->id())) {
        //Find the insertion point
        parentNode = mG->findElement((*mGSeg)[it->id()]);
        sterror(!parentNode, "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Unable to locate vertex in MergeTree");
        //Now we need to traverse the tree down to find the actual insertion point
        //This is necessary because we have already added other internal (non-critical)
        //points along the same arc, and we need to insert the point at the correct
        //location within the arc.
        while (parentNode->downSize() > 0 && mGComp.greater(parentNode->down()[0], it)) {
          parentNode = parentNode->down()[0];
        }
        childNode = parentNode->down()[0];

        //Add the new node to the split tree
        mG->addNode(it->id(), it->f());
        insertedNode = mG->findElement(it->id());
        //Now define the connections between the points to insert the node in the graph
        mG->removeArc(parentNode, childNode);
        mG->addArc(parentNode, insertedNode);
        mG->addArc(insertedNode, childNode);
      }
    }
  }
}

template <class VertexClass>
unsigned int ContourTree_TreeMerge<VertexClass>::mergeTrees()
{
  //Some typedef to make the code more readable
  typedef Node* nodeP;

  //1. Declaration and initalization of basic varibales
  TopoGraph<>* mG = static_cast<TopoGraph<>* >(mMergeGraph) ;
  TopoGraph<>* sG = static_cast<TopoGraph<>* >(mSplitGraph) ;
  TopoGraph<>* cG = static_cast<TopoGraph<>* >(mContourGraph) ;
  VertexCompare mGComp = VertexCompare(1); //GREATER
  VertexCompare sGComp = VertexCompare(0); //SMALLER
  typename MappedArray<TopoGraph<>::InternalNode>::iterator it;
  nodeP parentNode, childNode, tempNodeMG, tempNodeSG;
  unsigned int numArcs=0; //count the number of arcs

  //2. Compute the contour tree by peeling of branches from the merge tree and split tree.
  //2.1 Enqueue all leafs of the contour tree that are regular internal nodes in the
  //    split-tree and vice versa. NOTE: All vertices from the splitTree are now also
  //    contained in the mergeTree so that we only need to iterate through the nodes
  //    of one tree here.
  std::queue<GlobalIndexType> nodeQueue;
  //std::queue<bool> processMergeTree;
  for(it = mG->nodes().begin(); it!=mG->nodes().end() ; it++){
    tempNodeSG = sG->findElement( it->id() );
    sterror( tempNodeSG == NULL , "MergeTree node not found in the SplitTree %i ( %i ) ", it->id() , mMaxIndex );
    if( (it->upSize() == 0) && (tempNodeSG->downSize() == 1) ){
      nodeQueue.push( it->id() );
      //processMergeTree.push(true);
    }
    if( (it->upSize() == 1) && (tempNodeSG->downSize() == 0) ){
      nodeQueue.push( it->id() );
      //processMergeTree.push(false);
    }
  }

  //2.2 Iterate through the queue and build the contour tree
  //bool useMT;
  while( !nodeQueue.empty() ){
    //2.2.1 Dequeue the first entry of our queue
    tempNodeSG = sG->findElement( nodeQueue.front() );
    tempNodeMG = mG->findElement( nodeQueue.front() );
    //useMT = processMergeTree.front();
    nodeQueue.pop();
    //processMergeTree.pop();
    //2.2.2 Find which arc + nodes we need to add to our contour tree
    //2.2.3 Add the nodes to the contour tree if needed and add the arc to the contour tree
    //2.2.4 Remove the current node from both trees
    if( tempNodeMG )
    if( tempNodeMG->upSize() == 0 && tempNodeMG->type() != ROOT )//If the node is a leaf in the merge tree
    //if( useMT )
    {
      //Add the nodes and arc to the contour tree
      parentNode = tempNodeMG;
      childNode  = tempNodeMG->down()[0];
      if( !cG->findElement(parentNode->id() ) ){
        cG->addNode( parentNode->id() , parentNode->f() );
      }
      if( !cG->findElement(childNode->id() ) ){
        cG->addNode( childNode->id() , childNode->f() );
      }

      //Add the arc to the contour tree
      cG->addArc( parentNode->id() , parentNode->f() , childNode->id() , childNode->f() );
      addArcInternal( parentNode , childNode , true );
      numArcs++;

      //Remove tempNodeMG from the merge tree
      mG->removeArc( parentNode , childNode );
      mG->removeNode( parentNode );

      //Remove tempNodeSG from the split tree
      if( tempNodeSG->type() == INTERIOR ){
        tempNodeSG->bypass();
      }else{
        if( tempNodeSG->downSize() > 0){
          tempNodeSG->down()[0]->removeUp( tempNodeSG );
          tempNodeSG->removeDown( tempNodeSG->down()[0] );
        }
        if( tempNodeSG->upSize() > 0){
          tempNodeSG->up()[0]->removeDown( tempNodeSG );
          tempNodeSG->removeUp( tempNodeSG->up()[0] );
        }
      }

      //Remove tempNodeSG from the split tree
      sterror((tempNodeSG->upSize() != 0) || (tempNodeSG->downSize() != 0) ,
               "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Try to remove connected node. (1)");
      sG->removeNode(tempNodeSG);
    }
    if( tempNodeSG )
    if( tempNodeSG->downSize() == 0 && tempNodeSG->type() != ROOT )//If the node is a leaf in the split tree
    //else
    {
      //Add the nodes and arc to the contour tree
      parentNode = tempNodeSG;
      childNode  = tempNodeSG->up()[0];
      if( !cG->findElement(parentNode->id() ) ){
        cG->addNode( parentNode->id() , parentNode->f() );
      }
      if( !cG->findElement(childNode->id() ) ){
        cG->addNode( childNode->id() , childNode->f() );
      }

      //Add the arc to the contour tree
      //We switch parent and child here, so that all arcs are downward arcs in the contour tree
      cG->addArc( childNode->id() , childNode->f(), parentNode->id() , parentNode->f()  );
      addArcInternal( childNode , parentNode , false );
      numArcs++;

       //Remove tempNodeSG from the split tree
      sG->removeArc( parentNode , childNode );
      sG->removeNode( parentNode );

      //Remove tempNodeMG from the merge tree
      if( tempNodeMG->type() == INTERIOR ){
        tempNodeMG->bypass();
      }else{
        if( tempNodeMG->upSize() > 0){
          tempNodeMG->up()[0]->removeDown( tempNodeMG );
          tempNodeMG->removeUp( tempNodeMG->up()[0] );
        }
        if( tempNodeMG->downSize() > 0){
          tempNodeMG->down()[0]->removeUp( tempNodeMG );
          tempNodeMG->removeDown( tempNodeMG->down()[0] );
        }
      }

      //Remove tempNodeMG from the merge tree
      sterror((tempNodeMG->upSize() != 0) || (tempNodeMG->downSize() != 0) ,
               "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Try to remove connected node. (2)");
      mG->removeNode(tempNodeMG);
    }
    //2.2.5 If out childNode is now a leaf node then enqueue it for processing
    tempNodeSG = sG->findElement( childNode->id() );
    tempNodeMG = mG->findElement( childNode->id() );
    if( (tempNodeMG->upSize() == 0) && (tempNodeSG->downSize() == 1)  ) {
      nodeQueue.push(childNode->id());
      //processMergeTree.push(true);
    }
    if( (tempNodeMG->upSize() == 1) && (tempNodeSG->downSize() == 0) ){
      nodeQueue.push(childNode->id());
      //processMergeTree.push(false);
    }

    //cerr<<nodeQueue.size()<<" "<<mG->size()<<" "<<sG->size()<<" "<<cG->size()<<endl;
  }

  //Test output of the two graphs
 //writeTree( "/home/ruebel1/devel/contourTree_3.dot" , *cG);
 //writeTree( "/home/ruebel1/devel/topology/StreamingTopology/scripts/mergeTree_3.dot" , *mG);
 //writeTree( "/home/ruebel1/devel/topology/StreamingTopology/scripts/splitTree_3.dot" , *sG);
  return numArcs;
}

template <class VertexClass>
void ContourTree_TreeMerge<VertexClass>::writeTree(std::string filename, TopoGraph<>& graph, int resolution=50 )
{
   FILE* f = fopen((const char*)filename.c_str() ,"w");
   if(f){
      write_dot<DefaultNodeData> (f, graph, resolution, graph.minF(), graph.maxF(), "shape=ellipse,fontsize=10");
      fclose(f);
   }
}

#endif


