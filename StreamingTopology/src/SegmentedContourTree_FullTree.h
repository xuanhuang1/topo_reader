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

#ifndef SegmentedContourTree_FullTree_H
#define SegmentedContourTree_FullTree_H

#include <iostream>
#include <algorithm>
#include <ContourTree_TreeMerge.h>
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
template < class VertexClass=EnhancedSegUnionVertex<> >
class SegmentedContourTree_FullTree : public ContourTree_TreeMerge<VertexClass>
{
public:

  typedef typename VertexClass::DataType::FunctionType FunctionType;

#ifndef ST_INCORE_ARRARY
  //! Typedef to easily switch between incore and out of core arrays
  typedef FlexArray::OOCArray<GlobalIndexType> SegmentationArray;
#else
  typedef FlexArray::BlockedArray<GlobalIndexType> SegmentationArray;
#endif

  /*! \brief Default constructor
   *  \param graph : The contour tree graph
   *  \param segmentation : The contour tree segmentation (optional)
   *  \param mergeTree : The merge tree to be used (optional)
   *  \param splitTree : The split tree to be used (optional)
   *  \param mergeGraph : The graph of the merge tree (optional)
   *  \param splitGraph : The graph of the split tree (optional)
   */
  SegmentedContourTree_FullTree(TopoGraphInterface<typename VertexClass::DataType>* graph,
                                 FlexArray::BlockedArray<GlobalIndexType>* segmentation=NULL,
                                 SegmentedUnionTree< VertexClass >* mergeTree=NULL ,
                                 SegmentedUnionTree< VertexClass >* splitTree=NULL,
                                 TopoGraphInterface<typename VertexClass::DataType>* mergeGraph=NULL,
                                 TopoGraphInterface<typename VertexClass::DataType>* splitGraph=NULL,
                                 bool createSegIndexToArcMap = true,
                                 bool cleanContourTree = true
          );

  //! Destructor
  virtual ~SegmentedContourTree_FullTree();

  //Get the contour tree segmentation
  FlexArray::BlockedArray<GlobalIndexType>* segmentation() const {return mSegmentation;}

  ///Finish the contour tree
  virtual void finishTree(const FlexArray::BlockedArray< FunctionType >& attribute);

protected:

  /*! \brief Function to compute the contour tree and the according segmentation from
   * the merge and split-tree.
   *
   * This function is called at the end by the finishTree function to complete
   * the computation of the contour-tree. This function also calls the
   * augmentMergeTreeAndSplitTree() and mergeTrees() function.
   */
  void computeContourTree();

  /**! \brief Clean-up contour tree, i.e., remove all interior nodes from the tree
   *
   * This function is called at the very end if requested
   */
  void cleanUpContourTree();

  /*! \brief Compute the arc mapping needed to compute the segmentation when an
   *         arc is added to the contour tree
   *  \param node1 : Frist incident vertex of the arc
   *  \param node2 : Second incident vertex of the arc
   */
  virtual void addArcInternal( Node<typename VertexClass::DataType>* node1,
                               Node<typename VertexClass::DataType>* node2 );

  void augmentMergeTreeAndSplitTree(const FlexArray::BlockedArray< FunctionType >& attribute);

  /*! \brief Test function to compute the segmentation
   */
  void computeSegmentation();
  
  //! Array with the contour tree segmentation
  FlexArray::BlockedArray<GlobalIndexType>* mSegmentation;

  //! Do we own mSegmentation
  bool mPrivateSegmentation;

  /** \brief Define for each arc of the contour tree the corresponding compact segmenation index
   */
  std::map<pair<GlobalIndexType , GlobalIndexType> , int> mArcToSegIndexMap;

  //! Should all interior nodes be removed from the contour tree after completion
  bool mCleanUpContourTree;
};



template <class VertexClass>
SegmentedContourTree_FullTree<VertexClass>::SegmentedContourTree_FullTree(TopoGraphInterface<typename VertexClass::DataType>* graph ,
                                                                            FlexArray::BlockedArray<GlobalIndexType>* segmentation,
                                                                            SegmentedUnionTree<VertexClass>* mergeTree ,
                                                                            SegmentedUnionTree<VertexClass>* splitTree ,
                                                                            TopoGraphInterface<typename VertexClass::DataType>* mergeGraph,
                                                                            TopoGraphInterface<typename VertexClass::DataType>* splitGraph,
                                                                            bool createSegIndexToArcMap,
                                                                            bool cleanContourTree
                                                                          )
  : ContourTree_TreeMerge<VertexClass>( graph, mergeTree , splitTree , mergeGraph, splitGraph ),
    mSegmentation(segmentation), mPrivateSegmentation(false), mCleanUpContourTree( cleanContourTree )
{
  typedef typename VertexClass::DataType DataType;

  //Create the segmenation if not given
  if (mSegmentation==NULL) {
    mPrivateSegmentation = true;
    mSegmentation = new SegmentationArray(0,SEGMENTATION_BLOCK_BITS);
  }
}

template <class VertexClass>
SegmentedContourTree_FullTree<VertexClass>::~SegmentedContourTree_FullTree()
{
}

template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::finishTree(const FlexArray::BlockedArray< FunctionType >& attribute)
{
  //Now compute the acutal contour tree
  //Finalize the merge and split tree
  ContourTree_TreeMerge<VertexClass>::finalizeMergeAndSplitTreeSegmentation(attribute);
  //Augment the merge and split tree
  augmentMergeTreeAndSplitTree(attribute);
  //Merge the merge tree and the split tree to compute the contour tree.
  computeContourTree();
  //Clean-up the contour tree if necessary
  if(mCleanUpContourTree){
    cleanUpContourTree();
  }
}

template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::augmentMergeTreeAndSplitTree(const FlexArray::BlockedArray< FunctionType >& attribute)
{
  cerr<<"Augmenting merge/split tree"<<endl;
//Some typedef to make the code more readable
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;

  //1. Declaration and initalization of basic varibales
  TopoGraph<DataType>* mG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mMergeGraph) ;
  TopoGraph<DataType>* sG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mSplitGraph) ;
  FlexArray::BlockedArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  FlexArray::BlockedArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  VertexCompare<DataType> mGComp = VertexCompare<DataType>(1); //GREATER
  VertexCompare<DataType> sGComp = VertexCompare<DataType>(0); //SMALLER
  nodeP parentNode, insertedNode, childNode;
  typename TopoGraph<DataType>::iterator it;

  //Avoid searches in extremely large trees by remembering which points were originally in the trees
  vector<GlobalIndexType> splitIndices;
  splitIndices.reserve( sG->size() );
  vector<GlobalIndexType> mergeIndices;
  mergeIndices.reserve( mG->size() );
  for(it = mG->begin(); it!=mG->end() ; it++){
    mergeIndices.push_back(it->id());
  }
  for(it = sG->begin(); it!=sG->end() ; it++){
    splitIndices.push_back(it->id());
  }

   //2. Insert all points in the split-tree and merge-tree
  for( GlobalIndexType i=0; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex ; ++i)
  {
    //If the vertex is not in the split tree then insert it
    if( std::find(  splitIndices.begin() , splitIndices.end() , i ) == splitIndices.end() ){ //if(sG->findElement(i) ==NULL ){
      //Add the new node to the split tree
      sG->addNode( i , attribute[i]  );
      insertedNode = sG->findElement( i );
      //Find the insertion point
      parentNode = sG->findElement( sGSeg->at(i) );
      sterror( !parentNode , "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Unable to locate vertex in SplitTree");
      //Now we need to traverse the tree down to find the actual insertion point
      //This is necessary because we have already added other internal (non-critical)
      //points along the same arc, and we need to insert the point at the correct
      //location within the arc.
      while(  parentNode->upSize()>0 && sGComp.smaller( parentNode->up()[0] , insertedNode ) ){
        parentNode = parentNode->up()[0];
      }
      childNode = parentNode->up()[0];

      //Now define the connections between the points to insert the node in the graph
      sG->removeArc( parentNode , childNode );
      sG->addArc( parentNode , insertedNode );
      sG->addArc( insertedNode , childNode);
    }
    
    //If the vertex is not in the split tree then insert it
    if( std::find( mergeIndices.begin() , mergeIndices.end() , i ) == mergeIndices.end() ){ //if( mG->findElement( i ) == NULL  ){
      //Add the new node to the split tree
      mG->addNode( i , attribute[i]  );
      insertedNode = mG->findElement( i );
      //Find the insertion point
      parentNode = mG->findElement( mGSeg->at(i) );
      sterror( !parentNode , "ContourTree_TreeMerge<VertexClass>::mergeTreesAndSegmentation(): Unable to locate vertex in MergeTree");
      //Now we need to traverse the tree down to find the actual insertion point
      //This is necessary because we have already added other internal (non-critical)
      //points along the same arc, and we need to insert the point at the correct
      //location within the arc.
      while( parentNode->downSize()>0 && mGComp.greater( parentNode->down()[0] , insertedNode ) ){
        parentNode = parentNode->down()[0];
      }
      childNode = parentNode->down()[0];

      //Now define the connections between the points to insert the node in the graph
      mG->removeArc( parentNode , childNode );
      mG->addArc( parentNode , insertedNode );
      mG->addArc( insertedNode , childNode);
    }
  }

  cerr<<"   Merge tree size: "<<mG->size()<<" with "<<mergeIndices.size()<<" merge nodes."<<endl;
  cerr<<"   Split tree size: "<<sG->size()<<" with "<<splitIndices.size()<<" split nodes."<<endl;
}


template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::computeContourTree()
{
  //Some typedef to make the code more readable
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;

  cerr<<"Merging the trees"<<endl;
  //3. Compute the contour tree by peeling of branches from the merge tree and split tree.
  ContourTree_TreeMerge<VertexClass>::mergeTrees();  

  cerr<<"Computing the segmentation"<<endl;
  //4. compute the segmentation
  computeSegmentation();
}

template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::addArcInternal( Node<typename VertexClass::DataType>* node1 ,
                                                                  Node<typename VertexClass::DataType>* node2)
{
}

template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::computeSegmentation()
{
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  //FlexArray::OOCArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  //FlexArray::OOCArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  //pair<GlobalIndexType , GlobalIndexType> segPair;
  //std::map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator segPairMapVal;
  //std::map<pair<GlobalIndexType , GlobalIndexType> , int> segmentationPairMap;

  (*mSegmentation).resize( ContourTree_TreeMerge<VertexClass>::mMaxIndex +1 );
  nodeP tempUp , tempDown;
  bool found;
  pair<GlobalIndexType , GlobalIndexType> arcPair;
  std::map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator arc;
  GlobalIndexType i;
  int index;
  //cout<<"Segmentation:"<<endl;
  for( typename TopoGraph<DataType>::iterator it= cG->begin() ; it != cG->end() ; it++ )
  {
    i= it->id();
    tempDown = it;
    found = false;
    while( !found ){
      if(tempDown->downSize() == 1 ){
        tempDown = tempDown->down()[0];
      }else{
        found = true;
      }
    }
    found = false;
    tempUp = it;
     while( !found ){
      if(tempUp->upSize() == 1 ){
        tempUp = tempUp->up()[0];
      }else{
        found = true;
      }
    }
    arcPair = pair<GlobalIndexType , GlobalIndexType>( tempUp->id() , tempDown->id() );
    arc = mArcToSegIndexMap.find( arcPair );
    if( arc != mArcToSegIndexMap.end() ){
      index = (*arc).second;
    }else{
      index = mArcToSegIndexMap.size();
      mArcToSegIndexMap[arcPair] = index;
    }
    (*mSegmentation)[i] = index;

    /*segPair = pair<GlobalIndexType , GlobalIndexType>((*mGSeg)[i] , (*sGSeg)[i]);
    segPairMapVal = segmentationPairMap.find( segPair );
    if( segPairMapVal != segmentationPairMap.end() ){
      if( (*segPairMapVal).second != (*mSegmentation)[i] ){
        cerr<<"Points with same (MI,SI) are located in different arcs."<<endl;
      }
    }else{
      segmentationPairMap[segPair] = (*mSegmentation)[i];
    }*/

    //cout<<"ID: "<<i<<" Index: "<<index;
    //cout<<" Arc:( "<<arcPair.first<<" "<<arcPair.second<<" ) ";
    //cout<<"IndexPair:( "<<segPair.first<<" "<<segPair.second<<" ) ";
    //cout<<endl;
  }


  //Test print the segmentation
  /*std::map<int , pair<GlobalIndexType , GlobalIndexType> > segToArc;
  for(arc=mArcToSegIndexMap.begin(); arc!=mArcToSegIndexMap.end() ; arc++){
    segToArc[(*arc).second] = (*arc).first;
  }
  pair<GlobalIndexType , GlobalIndexType> temp;
  typedef typename VertexClass::DataType DataType;
  for( unsigned int i=0 ; i<mSegmentation->size() ; ++i){
    cout<<"ID: "<<i<<" Index: "<<(*mSegmentation)[i];
    temp = segToArc[(*mSegmentation)[i]];
    cout<<" Arc:( "<<temp.first<<" "<<temp.second<<" ) ";
    if( cG->findElement(i) != NULL)
      cout<<" (critical point)";
    cout<<endl;
  }*/
}

template <class VertexClass>
void SegmentedContourTree_FullTree<VertexClass>::cleanUpContourTree()
{
  cerr<<"Removing interior nodes from the contour tree."<<endl;
  typedef typename VertexClass::DataType DataType;
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  typename TopoGraph<DataType>::iterator previous = cG->begin();
  for( typename TopoGraph<DataType>::iterator it= cG->begin() ; it != cG->end() ; it++ )
  {
    if( it->type() == INTERIOR ){
      it->bypass();
      cG->removeNode(it);
      it = previous;
    }else{
      previous = it;
    }
  }
}

#endif 


