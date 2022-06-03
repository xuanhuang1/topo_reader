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

#ifndef SEGMENTEDCONTOURTREE_TREEMERGE_H
#define SEGMENTEDCONTOURTREE_TREEMERGE_H

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
class SegmentedContourTree_TreeMerge : public ContourTree_TreeMerge<VertexClass>
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
  SegmentedContourTree_TreeMerge(TopoGraphInterface* graph,
                                 FlexArray::BlockedArray<GlobalIndexType>* segmentation=NULL,
                                 SegmentedUnionTree<  >* mergeTree=NULL ,
                                 SegmentedUnionTree<  >* splitTree=NULL,
                                 TopoGraphInterface* mergeGraph=NULL,
                                 TopoGraphInterface* splitGraph=NULL,
                                 bool createSegIndexToArcMap = true);

  //! Destructor
  virtual ~SegmentedContourTree_TreeMerge();

  //Get the contour tree segmentation
  FlexArray::BlockedArray<GlobalIndexType>* segmentation() const {return mSegmentation;}

  /*! \brief Mapping of the contour tree segmentation.
   *
   *  The map is indexed with the key-pair consisting of the segmention index
   *  in the merge-tree and the segmentation index in split-tree. For each
   *  key the index of the corresponding arc in the contour tree is stored
   */
  //const map<pair<GlobalIndexType , GlobalIndexType> , ContourTreeArc>& segmentationMap() const{
  //const map<pair<GlobalIndexType , GlobalIndexType> , GlobalIndexType>& indexpairToSegmentationMap() const{
  //  return mIndexpairToSegMap;
  //}

  /* \brief Get the map from the arcs to the compact segmentation index
   */
  //const std::map<pair<GlobalIndexType , GlobalIndexType> , int>& arcToSegmentationIndexMap() const{
  //  return mArcToSegIndexMap;
  //}

  /* \brief Get the mapping from the segmentation index to the arc in the contour tree
   *
   *  The map is indexed with the segmenation index and stores the index in the contour
   *  tree of the two points that define the corresponding arc in the contour tree.
   *  NOTE: The map is only created if mCreateSegIndexToArcMap is set to true when
   *        instantiating and object of the class
   */
  const map<GlobalIndexType , pair<GlobalIndexType , GlobalIndexType> >& segmentationIndexToArcMap() const{
    return mSegIndexToArcMap;
  }

  ///Ask whether this object has/will create a map from the segmenation index to the arcs.
  bool getCreateSegmentationIndexToArcMap() const{ return mCreateSegIndexToArcMap; }

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

  /*! \brief Compute the arc mapping needed to compute the segmentation when an
   *         arc is added to the contour tree
   *  \param node1 : Frist incident vertex of the arc
   *  \param node2 : Second incident vertex of the arc
   *  \param mergeArc : Did the arc origninate from the mergeTree (==1) or the splitTree(==0)
   */
  virtual void addArcInternal( Node* node1,
                               Node* node2,
                               bool mergeArc );

  /* \brief Test function to compute the segmentation
   */
  //void computeSegmentation1(std::map<GlobalIndexType, GlobalIndexType>& mergeMap,
  //                          std::map<GlobalIndexType, GlobalIndexType>& splitMap);
  /* \brief Test function to compute the segmentation
   */
  //void computeSegmentation2();

  //! Array with the contour tree segmentation
  FlexArray::BlockedArray<GlobalIndexType>* mSegmentation;

  /*! \brief Mapping of a merge/split segmentation index pair to a contour tree segmentation index.
   *
   *  The map is indexed with the key-pair consisting of the segmention index
   *  in the merge-tree and the segmentation index in split-tree. For each
   *  key the respective arc in the contour tree consisting of the index
   *  of the adjacent vertices and the segmenation index of the arc is stored.
   */
  //map<pair<GlobalIndexType , GlobalIndexType> , ContourTreeArc> mIndexpairToSegMap;
  //map<pair<GlobalIndexType , GlobalIndexType> , int> mIndexpairToSegMap;

  /*! \brief Store for each segmentation index the index of the two points in the contour tree
   *         that define the corrresponding arc.
   *
   * This map is stored for convenience but not actually used. This means, if
   * memory or performance are an issue then the map can be omitted.
   */
  map<GlobalIndexType , pair<GlobalIndexType , GlobalIndexType> > mSegIndexToArcMap;

  /** \brief Define for each arc of the contour tree the corresponding compact segmenation index
   */
  //std::map<pair<GlobalIndexType , GlobalIndexType> , int> mArcToSegIndexMap;

  //! Do we own mSegmentation
  bool mPrivateSegmentation;

  //!Should we create a map defining for each segmentation index the arc in the contour tree
  bool mCreateSegIndexToArcMap;

  TopoGraph<typename VertexClass::DataType>* mG_Copy;
  TopoGraph<typename VertexClass::DataType>* sG_Copy;
  vector<GlobalIndexType> mUnsegmentedIds;
  unsigned int mCurrentArcIndex;

  void printSegmentation(int option);


  //void writeSubPart(std::string filename, GlobalIndexType id,
  //                          TopoGraph<typename VertexClass::DataType> *g, int resolution=10 );

  //void fixMultiSaddles();
};



template <class VertexClass>
SegmentedContourTree_TreeMerge<VertexClass>::SegmentedContourTree_TreeMerge(TopoGraphInterface* graph ,
                                                                            FlexArray::BlockedArray<GlobalIndexType>* segmentation,
                                                                            SegmentedUnionTree<>* mergeTree ,
                                                                            SegmentedUnionTree<>* splitTree ,
                                                                            TopoGraphInterface* mergeGraph,
                                                                            TopoGraphInterface* splitGraph,
                                                                            bool createSegIndexToArcMap)
  : ContourTree_TreeMerge<VertexClass>( graph, mergeTree , splitTree , mergeGraph, splitGraph ), mSegmentation(segmentation), mPrivateSegmentation(false), mCreateSegIndexToArcMap(createSegIndexToArcMap)
{
  //Create the segmenation if not given
  if (mSegmentation==NULL) {
    mPrivateSegmentation = true;
    mSegmentation = new FlexArray::OOCArray<GlobalIndexType>(0,SEGMENTATION_BLOCK_BITS);
  }
}

template <class VertexClass>
SegmentedContourTree_TreeMerge<VertexClass>::~SegmentedContourTree_TreeMerge()
{
  //Clear all maps
  //mIndexpairToSegMap.clear();
  //mArcToSegIndexMap.clear();
  if( mCreateSegIndexToArcMap )
     mSegIndexToArcMap.clear();
 delete mG_Copy;
 delete sG_Copy;
 mUnsegmentedIds.clear();

}

template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::finishTree(const FlexArray::BlockedArray< FunctionType >& attribute)
{
  //Finalize the merge and split tree
  cerr<<"Finalizing merge/split-tree segmentation"<<endl;
  ContourTree_TreeMerge<VertexClass>::finalizeMergeAndSplitTreeSegmentation(attribute);

  //Merge the merge tree and the split tree to compute the contour tree.
  computeContourTree();

  //Compute the augmented segmentation
  cerr<<"Computing the augmented segmentation"<<endl;
  ContourTree_TreeMerge<VertexClass>::finalizeMergeAndSplitTreeSegmentation(attribute);

  mSegmentation->resize( ContourTree_TreeMerge<VertexClass>::mMaxIndex+1 );
  mCurrentArcIndex = 0;
  mUnsegmentedIds.resize( ContourTree_TreeMerge<VertexClass>::mMaxIndex+1 );
  for( GlobalIndexType i =0 ; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex; ++i){
    mUnsegmentedIds[i]=i;
  }

  cerr<<"Compute the contour tree"<<endl;
  //3. Compute the contour tree by peeling of branches from the merge tree and split tree.
  unsigned int numArcs= ContourTree_TreeMerge<VertexClass>::mergeTrees();
  cerr<<"Number of arcs in contour tree: "<<numArcs<<endl;
  cerr<<"Unsegmented ids "<<mUnsegmentedIds.size()<<endl;
  
  typedef typename VertexClass::DataType DataType;
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  for(unsigned int i=0; i<mUnsegmentedIds.size();++i){
    cerr<<mUnsegmentedIds[i]<<" Critical Point: ";
    if( cG->findElement(mUnsegmentedIds[i]) != NULL){
      cerr<<"TRUE ";
      cerr<<" "<< cG->findElement(mUnsegmentedIds[i])->type();
    }
    else
      cerr<<"FALSE";
    cerr<<endl;
  }

  //printSegmentation(2);
}

template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::computeContourTree()
{
  //Some typedef to make the code more readable
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;

  //Declaration and initalization of basic varibales
  TopoGraph<DataType>* mG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mMergeGraph) ;
  TopoGraph<DataType>* sG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mSplitGraph) ;
  //TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  typename TopoGraph<DataType>::iterator it;

  //Test output of the two graphs
  writeTree( "/home/ruebel1/devel/mergeTree_1.dot" , *mG);
  writeTree( "/home/ruebel1/devel/splitTree_1.dot" , *sG);

  //1. Compute the mapping for the merge-tree and the split tree needed to
  //   compute the segmentation. We store for each vertex of the merge-tree
  //   its lower child and for each split vertex its upper child.
  /*std::map<GlobalIndexType, GlobalIndexType> mergeMap;
  std::map<GlobalIndexType, GlobalIndexType> splitMap;
  for( it=mG->begin() ; it!=mG->end() ; it++){
    mergeMap[ it->id() ] = it->downSize()>0 ? it->down()[0]->id() : it->id();
  }
  for( it=sG->begin() ; it!=sG->end() ; it++){
    splitMap[ it->id() ] = it->upSize()>0 ? it->up()[0]->id() : it->id();
  }*/

  cerr<<"Merge Tree: "<<mG->size()<<endl;
  cerr<<"Split Tree: "<<sG->size()<<endl;


  //1. Insert the split-points into the merge-tree and vice versa.
  //cerr<<"Augmenting the merge and split tree"<<endl;
  ContourTree_TreeMerge<VertexClass>::augmentMergeTreeAndSplitTree();

  //Create a copy of the merge-tree and split-tree
  cerr<<"Copy merge and split tree"<<endl;
  //2. Copy the merge and split tree
  mG_Copy =new TopoGraph<DataType>();
  sG_Copy =new TopoGraph<DataType>();
  nodeP temp1 , temp2;
  int index=0;
  for( it=mG->begin() ; it!=mG->end() ; it++){
    mG_Copy ->addNode( it->id() , it->data() );
  }
  for( it=mG->begin() ; it!=mG->end() ; it++){
    for(int i=0 ; i<it->upSize() ; ++i){
      temp1 = &mG_Copy->at(index);
      temp2 = mG_Copy->findElement(it->up()[i]->id());
      mG_Copy->addArc( temp1->id() , temp1->data() , temp2->id() , temp2->data()  );
    }
    ++index;
  }
  index =0;
  for( it=sG->begin() ; it!=sG->end() ; it++){
    sG_Copy ->addNode( it->id() , it->data() );
  }
  for( it=sG->begin() ; it!=sG->end() ; it++){
    for(int i=0 ; i<it->upSize() ; ++i){
      temp1 = &sG_Copy->at(index);
      temp2 = sG_Copy->findElement(it->up()[i]->id());
      sG_Copy->addArc( temp1->id() , temp1->data() , temp2->id() , temp2->data()  );
    }
    ++index;
  }

  /*writeTree( "/home/ruebel1/devel/mergeTree_Copy.dot" , *mG_Copy);
  writeTree( "/home/ruebel1/devel/splitTree_Copy.dot" , *sG_Copy);

  cerr<<"Compute the contour tree"<<endl;
  //3. Compute the contour tree by peeling of branches from the merge tree and split tree.
  unsigned int numArcs= ContourTree_TreeMerge<VertexClass>::mergeTrees();
  cerr<<"Number of arcs in contour tree: "<<numArcs<<endl;
  
  //Test output of the two graphs
  writeTree( "/home/ruebel1/devel/contourTree_3.dot" , *cG);
  writeTree( "/home/ruebel1/devel/mergeTree_3.dot" , *mG);
  writeTree( "/home/ruebel1/devel/splitTree_3.dot" , *sG);

  //4. compute the segmentation
  //computeSegmentation1(mergeMap, splitMap);
  computeSegmentation2();*/
}

template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::addArcInternal( Node<typename VertexClass::DataType>* node1 ,
                                                                  Node<typename VertexClass::DataType>* node2 ,
                                                                  bool mergeArc)
{
  /*if( mergeArc )
    mArcOriginMap[node1->id()]=true;
  else
    mArcOriginMap[node2->id()]=false;*/
  cerr<<"Adding arc "<<mCurrentArcIndex<<" remaining for segmentation "<<mUnsegmentedIds.size()<<" ("<<double(mUnsegmentedIds.size())/double(ContourTree_TreeMerge<VertexClass>::mMaxIndex+1)<<"%)"<<endl;

  mSegIndexToArcMap[mCurrentArcIndex] = pair<GlobalIndexType, GlobalIndexType>(node1->id() , node2->id() );

  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;
  FlexArray::BlockedArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  FlexArray::BlockedArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  
  if (mergeArc) {
    nodeP upperNode = mG_Copy->findElement( node1->id()  );
    nodeP tempNode = upperNode;
    vector<GlobalIndexType> downTrace;
    while (tempNode->down()[0]->id() != node2->id() ) {
      downTrace.push_back(tempNode->id());
      tempNode = tempNode->down()[0];
    }
    downTrace.push_back(tempNode->id());
    
    for(int i=0; i<int(mUnsegmentedIds.size()) ; ++i ){
      if( find( downTrace.begin() , downTrace.end() , (*mGSeg)[mUnsegmentedIds[i]] ) != downTrace.end()  ||
          mUnsegmentedIds[i] == node2->id() //make sure that all critical points get an assignement
        ){
        (*mSegmentation)[mUnsegmentedIds[i]] = mCurrentArcIndex;
        mUnsegmentedIds.erase( mUnsegmentedIds.begin() +i );
        --i;
      }
    }
    (*mSegmentation)[node1->id()] = mCurrentArcIndex; //make sure that the head of the arc gets this index
  }else{
    nodeP lowerNode = sG_Copy->findElement( node2->id()  );
    nodeP tempNode = lowerNode;
    vector<GlobalIndexType> upTrace;
    while (tempNode->up()[0]->id() != node1->id() ) {
      upTrace.push_back(tempNode->id());
      tempNode = tempNode->up()[0];
    }
    upTrace.push_back(tempNode->id());
  
    for(int i=0; i<int(mUnsegmentedIds.size()) ; ++i ){
      if( find( upTrace.begin() , upTrace.end() , (*sGSeg)[mUnsegmentedIds[i]] ) != upTrace.end() ||
          mUnsegmentedIds[i] == node1->id() //make sure that all critical points get an assignement
        ){
        (*mSegmentation)[mUnsegmentedIds[i]] = mCurrentArcIndex;
        mUnsegmentedIds.erase( mUnsegmentedIds.begin() + i);
        --i;
      }
    }
    (*mSegmentation)[node2->id()] = mCurrentArcIndex; //make sure that the head of the arc gets this index
  }
  mCurrentArcIndex++;
}




template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::printSegmentation(int option)
{
  pair<GlobalIndexType, GlobalIndexType> temp;
  typedef typename VertexClass::DataType DataType;
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>*> (ContourTree_TreeMerge<VertexClass>::mContourGraph);
  std::map<GlobalIndexType , pair<GlobalIndexType , GlobalIndexType> >::iterator it;

  switch (option) {
    case 0:
    {
      for (unsigned int i = 0; i < mSegmentation->size(); ++i) {
        cout << "ID: " << i << " Index: " << (*mSegmentation)[i];
        temp = mSegIndexToArcMap[(*mSegmentation)[i]];
        cout << " Arc:( " << temp.first << " " << temp.second << " ) ";
        if (cG->findElement(i) != NULL)
          cout << " (critical point)";
        cout << endl;
      }
    }
    break;
    case 1:
    {
      for( it=mSegIndexToArcMap.begin() ; it != mSegIndexToArcMap.end() ; ++it ){
        cout<<"Arc: ("<<(*it).second.first<<","<<(*it).second.second<<") :";
        for(GlobalIndexType i=0 ; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex ; ++i){
          if((*mSegmentation)[i] == (*it).first ){
            cout<<i<<" ";
          }
        }
        cout<<endl;
      }
    }
    case 2:
      for(unsigned int i=0; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex ; i++){
        cerr<<(*mSegmentation)[i]<<endl;
      }
    break;
    default:
    break;
  }

}













/*
 * Idea:
 * Each arc contains all points with the following properties:
 * 1) mergeIndex == mergeIndex of lower node &&
 *    splitIndex == splitIndex of upper node
 * 2) In additiona an arc will contain all points with
 *    splitIndex == splitIndex of upper node &&
 *    mergeIndex == mergeIndex of upper node if upper node is a split point ||
 *    mergeIndex == any possible merge index along the direct downward path
 *                  between the upper and lower point in the merge-tree
 * 3) mergeIndex == mergeIndex of lower node &&
 *    splitIndex == splitIndex of lower nodw if lower node is a merge point ||
 *    splitIndex == any possible split index along the direct upward path
 *                  betwenn the lower and the upper point in the split tree.
 */
/*template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::addArcInternal( Node<typename VertexClass::DataType>* node1 ,
                                                                  Node<typename VertexClass::DataType>* node2)
{
  typedef typename VertexClass::DataType DataType;
  typedef pair<GlobalIndexType, GlobalIndexType> indexPair;
  typedef Node<DataType>* nodeP;
  VertexCompare<DataType> mGComp = VertexCompare<DataType>(1); //GREATER
  sterror( mGComp.smaller(node1 , node2) , "node1 expected to be larger than node2." )

  GlobalIndexType id1 = node1->id();
  GlobalIndexType id2 = node2->id();
  int arcIndex = mArcToSegIndexMap.size();

  //Add the arc to our index map if needed
  if( mCreateSegIndexToArcMap ){
     mSegIndexToArcMap[ arcIndex ] =pair<GlobalIndexType, GlobalIndexType>( id1 , id2 );
  }
  //Add the arc to the arc map
  mArcToSegIndexMap[ pair<GlobalIndexType, GlobalIndexType>( id1 , id2 ) ] = arcIndex;

  //Define for each pair of merge and split tree segmentation indices which arc the
  //corresponding points will belong to
  FlexArray::OOCArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  FlexArray::OOCArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  typename map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator tempIt;
  pair<GlobalIndexType, GlobalIndexType> insertPair;
  //1) mergeIndex == mergeIndex of upper node &&
  //   splitIndex == splitIndex of lower node
  insertPair=indexPair( (*mGSeg)[id1] , (*sGSeg)[id2] );
  tempIt = mIndexpairToSegMap.find( insertPair );
  if (tempIt != mIndexpairToSegMap.end()) {
    cerr << "Pair: " << insertPair.first << " " << insertPair.second << " " << mIndexpairToSegMap[insertPair] << endl;
    cerr << "Arc: " << id1 << " " << id2 << endl;
    cerr << "Previous Arc: " << mSegIndexToArcMap[mIndexpairToSegMap[insertPair]].first << " ";
    cerr << mSegIndexToArcMap[mIndexpairToSegMap[insertPair]].second << " ";
    sterror((*tempIt).second != arcIndex, "Indexpair already in map.");
  }
  mIndexpairToSegMap[insertPair] = arcIndex;
  
  //2.1) splitIndex == splitIndex of lower node &&
  //     mergeIndex == any possible merge index along the direct downward path
  //                   between the upper and lower point in the merge-tree
  nodeP upperNode = mG_Copy->findElement( id1 );
  nodeP lowerNode = mG_Copy->findElement( id2 );
  nodeP tempNode;
  sterror( upperNode==NULL , "Upper node not found in merge tree.");
  sterror( lowerNode==NULL , "Lower node not found in merge tree.");
  vector< vector<GlobalIndexType> > traces;
  traces.resize(1);
  traces[0].push_back( upperNode->id() );
  vector<GlobalIndexType> foundDownTrace;
  while(foundDownTrace.size()==0 && traces.size()>0)
  {
    for(unsigned int i=0; i<traces.size(); ++i){
      tempNode = mG_Copy->findElement( traces[i].back() );
      sterror( tempNode==NULL , "Node not found in merge tree.");
      if( mGComp.smaller(tempNode , lowerNode) || //discontiue trace if we surpassed our point
          tempNode->downSize() == 0                //erase trace if we can't continue
      ){
        traces.erase( traces.begin()+i );
        --i;
      }
      else{ //coninue the trace
        for(int j=0; j<tempNode->downSize(); ++j){
          if( tempNode->down()[j]->id() == id2 ){
            foundDownTrace = traces[i];
            traces.clear();
            break;
          }else{
            traces.push_back( traces[i] );
            traces.back().push_back( tempNode->down()[j]->id() );
          }
        }
      }
    }
  }
  for(unsigned int i=1 ; i<foundDownTrace.size() ; ++i){
    insertPair=indexPair( (*mGSeg)[foundDownTrace[i]] ,   (*sGSeg)[id2] );
    tempIt = mIndexpairToSegMap.find(insertPair);
    if( tempIt != mIndexpairToSegMap.end() )
    {
      sterror( (*tempIt).second != arcIndex ,
             "Mapped points to multiple arcs" );
      (*tempIt).second = arcIndex;
    }else{
      mIndexpairToSegMap[insertPair]=arcIndex;
    }
  }
  
  //2.2) mergeIndex == mergeIndex of upper node &&
  //     splitIndex == any possible split index along the direct upward path
  //                   between the lower and upper point in the split-tree
  upperNode = sG_Copy->findElement( id1 );
  lowerNode = sG_Copy->findElement( id2 );
  sterror( upperNode==NULL , "Upper node not found in split tree.");
  sterror( lowerNode==NULL , "Lower node not found in split tree.");
  traces.resize(1);
  traces[0].push_back( lowerNode->id() );
  vector<GlobalIndexType> foundUpTrace;
  tempNode=NULL;
  while(foundUpTrace.size()==0 && traces.size()>0)
  {
    for(unsigned int i=0; i<traces.size(); ++i){
      tempNode = sG_Copy->findElement( traces[i].back() );
      sterror( tempNode==NULL , "Node not found in split tree.");
      if( mGComp.greater(tempNode , upperNode) ||  //discontinue trace if we surpassed our point
          tempNode->upSize() == 0                //erase trace if we can't continue
      ){
        traces.erase( traces.begin()+i );
        --i;
      }
      else{ //continue the trace
        for(int j=0; j<tempNode->upSize(); ++j){
          if( tempNode->up()[j]->id() == id1 ){
            foundUpTrace = traces[i];
            traces.clear();
            break;
          }else{
            traces.push_back( traces[i] );
            traces.back().push_back( tempNode->up()[j]->id() );
          }
        }
      }
    }
  }
  for(unsigned int i=1 ; i<foundUpTrace.size() ; ++i){
    insertPair=indexPair( (*mGSeg)[id1] ,   (*sGSeg)[foundUpTrace[i]] );
    tempIt = mIndexpairToSegMap.find(insertPair);
    if( tempIt != mIndexpairToSegMap.end() )
    {
      sterror( (*tempIt).second != arcIndex ,
             "Mapped points to multiple arcs" );
      (*tempIt).second = arcIndex;
    }else{
      mIndexpairToSegMap[insertPair]=arcIndex;
    }
  }
}*/

/*template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::computeSegmentation2()
{
  cerr<<"Segmenation map size: "<<mIndexpairToSegMap.size()<<endl;
  typedef typename VertexClass::DataType DataType;
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  TopoGraph<DataType>* mG = static_cast<TopoGraph<DataType>* >(mG_Copy) ;
  TopoGraph<DataType>* sG = static_cast<TopoGraph<DataType>* >(sG_Copy) ;

  //Initalize some variables
  FlexArray::OOCArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  FlexArray::OOCArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  typename map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator tempIt;
  
  //Compute the segmentation for each point
  mSegmentation->resize( ContourTree_TreeMerge<VertexClass>::mMaxIndex );
  pair<GlobalIndexType, GlobalIndexType> indexPair;
  unsigned int notFound=0;
  unsigned int equalIndexPairs=0;
  for( GlobalIndexType i=0; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex ; ++i)
  {
    indexPair = pair<GlobalIndexType, GlobalIndexType>( (*mGSeg)[i] , (*sGSeg)[i] );
    if( indexPair.first == indexPair.second ){ //Count the multi saddles
      (*mSegmentation)[i] = indexPair.first;
      equalIndexPairs++;
    }
    else{
      tempIt = mIndexpairToSegMap.find(indexPair);
      if (tempIt == mIndexpairToSegMap.end()) {
        Node<DataType>* tempNode;
        cerr<<"Pair not found:"<<i<<" ("<<indexPair.first<<","<<indexPair.second<<")  ";
               
        
        tempNode = mG->findElement(indexPair.first);
        cerr<<"----"<<indexPair.first<<":";
        cerr<<" MT "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        tempNode = sG->findElement(indexPair.first);
        cerr<<" | ST "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        tempNode = cG->findElement(indexPair.first);
        cerr<<" | CT "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        cerr<<"----"<<indexPair.second<<":";
        tempNode = mG->findElement(indexPair.second);
        cerr<<" MT "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        tempNode = sG->findElement(indexPair.second);
        cerr<<" | ST "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        tempNode = cG->findElement(indexPair.second);
        cerr<<" | CT "<<tempNode->type()<<" ("<<tempNode->upSize()<<" "<<tempNode->downSize()<<") ";
        cerr<<endl;
        mIndexpairToSegMap[indexPair] = -1;
        notFound++;
      } else {
        (*mSegmentation)[i] = (*tempIt).second;
      }
    }
  }
  cerr<<"Contour-tree size: "<<static_cast<TopoGraph<typename VertexClass::DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph)->size()<<endl;
  cerr<<"Equal index pairs: "<<equalIndexPairs<<endl;
  cerr<<"Missing index pairs: "<<notFound<<endl;
  unsigned int numHighOrderSaddles=0;
  for(typename TopoGraph<DataType>::iterator it=cG->begin() ; it!=cG->end() ; it++ ){
    if( it->upSize()>2 || it->downSize()>2 )
      numHighOrderSaddles++;
  }
  cerr<<"High-order Saddles: "<<numHighOrderSaddles<<endl;
}*/

/*
template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::computeSegmentation1(std::map<GlobalIndexType, GlobalIndexType>& mergeMap,
                                                                       std::map<GlobalIndexType, GlobalIndexType>& splitMap)
{
   //Some typedef to make the code more readable
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;
  
  //Declaration and initalization of basic varibales
  TopoGraph<DataType>* cG = static_cast<TopoGraph<DataType>* >(ContourTree_TreeMerge<VertexClass>::mContourGraph) ;
  FlexArray::OOCArray<GlobalIndexType >* mGSeg = ContourTree_TreeMerge<VertexClass>::mMergeTree->segmentation();
  FlexArray::OOCArray<GlobalIndexType >* sGSeg = ContourTree_TreeMerge<VertexClass>::mSplitTree->segmentation();
  VertexCompare<DataType> mGComp = VertexCompare<DataType>(1); //GREATER
  VertexCompare<DataType> sGComp = VertexCompare<DataType>(0); //SMALLER
  typename TopoGraph<DataType>::iterator it;
  nodeP parentNode, childNode, tempNodeMG, tempNodeSG;

  cerr<<"Segmentation map "<<mArcToSegIndexMap.size()<<endl;
  //4. Compute the segmentation for the contour tree
  //4.1 Allocate memory for the segmenation and define some temporary variables
  mSegmentation->resize( ContourTree_TreeMerge<VertexClass>::mMaxIndex );
  pair<GlobalIndexType, GlobalIndexType> tempKey;
  //typename map<pair<GlobalIndexType , GlobalIndexType> , ContourTreeArc>::iterator tempIt;
  typename map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator tempIt;
  typename map<pair<GlobalIndexType , GlobalIndexType> , int>::iterator tempArc;
  GlobalIndexType splitMapIndex, mergeMapIndex, upperIndex, lowerIndex;
  GlobalIndexType s1, s2, m1,m2;
  unsigned int numExceptions=0;
  //4.2 Iterate through all points and define their segmenation index
  for( GlobalIndexType i=0; i<=ContourTree_TreeMerge<VertexClass>::mMaxIndex ; ++i)
  {
    tempKey = pair<GlobalIndexType, GlobalIndexType>( (*mGSeg)[i] , (*sGSeg)[i] );
    tempIt = mIndexpairToSegMap.find(tempKey);
    //4.2.1 If we have seen a point before with the same combination of merge tree
    //      segmenationa index and split tree segmentation index then use the
    //      information stored in the our map
    if( tempIt != mIndexpairToSegMap.end()  ){
     //(*mSegmentation)[i] = mIndexpairToSegMap[tempKey].arcIndex;
      (*mSegmentation)[i] = mIndexpairToSegMap[tempKey];
    }
    //4.2.2 If we have not seen a vertex before with this particular combination
    //      of merge/split segmenation index then compute to which arc in the
    //      contour tree the point belongs to and save the information in the
    //      segmenation map (mIndexpairToSegMap) for later reuse.
    else{
      //4.2.2.1 Find the upper node of the arc in the contour tree the vertex
      //        belongs to based on the information from the split tree
      //splitMapIndex = splitMap[ tempKey.second ];   //Get the parent of the splitIndex node in the split tree
      //tempNodeMG = cG->findElement(tempKey.first);  //Get the mergeIndex node in the contour tree
      //tempNodeSG = cG->findElement(splitMapIndex ); //Get the splitMapIndex node in the contour tree
      //parentNode = mGComp.smaller( tempNodeSG , tempNodeMG ) ? tempNodeSG : tempNodeMG; //The lower of the two is our parent

      //4.2.2.2 Find the lower node of the arc in the contour tree the vertex
      //        belongs to based on the information from the merge tree
      //mergeMapIndex = mergeMap[ tempKey.first ];    //Get the parent of the splitIndex node in the merge tree
      //tempNodeMG = cG->findElement(mergeMapIndex ); //Get the mergeMapIndex node in the contour tree
      //tempNodeSG = cG->findElement(tempKey.second); //Get the splitIndex node in the contour tree
      //childNode = mGComp.greater( tempNodeMG , tempNodeSG ) ? tempNodeMG : tempNodeSG; //The greater one is our child

      //4.2.2.3 Look up the arc in the mArcToSegIndexMap and update the segmentation map
      //upperIndex = parentNode->id();
      //lowerIndex = childNode->id();
      //tempArc = mArcToSegIndexMap.find( pair<GlobalIndexType, GlobalIndexType>(upperIndex , lowerIndex )  );
      //sterror( tempArc == mArcToSegIndexMap.end() , "Arc not found in contour tree.")
      //mIndexpairToSegMap[ tempKey ] = ContourTreeArc( upperIndex , lowerIndex , (*tempArc).second );

      splitMapIndex = splitMap[ tempKey.second ];   //Get the parent of the splitIndex node in the split tree
      tempNodeMG = cG->findElement(tempKey.first);  //Get the mergeIndex node in the contour tree
      tempNodeSG = cG->findElement(splitMapIndex ); //Get the splitMapIndex node in the contour tree
      parentNode = mGComp.smaller( tempNodeSG , tempNodeMG ) ? tempNodeSG : tempNodeMG;
      s1 = tempNodeMG->id();
      s2 = tempNodeSG->id();
      mergeMapIndex = mergeMap[ tempKey.first ];    //Get the parent of the splitIndex node in the merge tree
      tempNodeMG = cG->findElement(mergeMapIndex ); //Get the mergeMapIndex node in the contour tree
      tempNodeSG = cG->findElement(tempKey.second); //Get the splitIndex node in the contour tree
      childNode = mGComp.greater( tempNodeMG , tempNodeSG ) ? tempNodeMG : tempNodeSG;
      m1 = tempNodeMG->id();
      m2 = tempNodeSG->id();

      unsigned int found=0;
      tempArc = mArcToSegIndexMap.find( pair<GlobalIndexType, GlobalIndexType>(s1 , m1 )  );
      if( tempArc != mArcToSegIndexMap.end() ){ found++; }
      tempArc = mArcToSegIndexMap.find( pair<GlobalIndexType, GlobalIndexType>(s1 , m2 )  );
      if( tempArc != mArcToSegIndexMap.end() ){ found++; }
      tempArc = mArcToSegIndexMap.find( pair<GlobalIndexType, GlobalIndexType>(s2 , m1 )  );
      if( tempArc != mArcToSegIndexMap.end() ){ found++; }
      tempArc = mArcToSegIndexMap.find( pair<GlobalIndexType, GlobalIndexType>(s2 , m2 )  );
      if( tempArc != mArcToSegIndexMap.end() ){ found++; }

      if( found == 0 ){
        cerr<<"Did not find any of the possible acrs: "<<i;
        cerr<<"("<<s1<<" , "<<m1<<") ";
        cerr<<"("<<s1<<" , "<<m2<<") ";
        cerr<<"("<<s2<<" , "<<m1<<") ";
        cerr<<"("<<s2<<" , "<<m2<<") ";
        cerr<<endl;
        for(int k=0; k<parentNode->downSize() ; ++k){
          if( mGComp.greater( childNode , cG->findElement(parentNode->down()[k]->id()) ) ){
            cerr<<"----->("<<parentNode->id()<<" , "<<parentNode->down()[k]->id()<<")";
             cerr<<"["<<parentNode->down()[k]->type()<<"]";
            cerr<<"["<<parentNode->down()[k]->type()<<"]"<<endl;
          }
        }
        for(int k=0; k<childNode->upSize() ; ++k){
          if( mGComp.smaller( parentNode , cG->findElement(childNode->up()[k]->id() ) ) ){
              cerr<<"<-----("<<childNode->id()<<" , "<<childNode->up()[k]->id()<<")";
              cerr<<"["<<childNode->up()[k]->type()<<"]";
              cerr<<"["<<childNode->up()[k]->type()<<"]"<<endl;
          }
        }

        numExceptions++;
      }
      if( found >1 ){
        cerr<<"Found "<<found<<" possible arcs "<<i<<endl;
        numExceptions++;
      }
      mIndexpairToSegMap[ tempKey ] = 0;

      //mIndexpairToSegMap[ tempKey ] = (*tempArc).second;

      //4.2.2.4 Assign the segmenation index to the point
      //(*mSegmentation)[i] = (*tempArc).second;
    }
  }
  cerr<<"Number of exceptions: "<<numExceptions<<endl;
}
*/

/*template <class VertexClass>
void SegmentedContourTree_TreeMerge<VertexClass>::writeSubPart(std::string filename, GlobalIndexType id,
                            TopoGraph<typename VertexClass::DataType> *g, int resolution )
{
  typedef typename VertexClass::DataType DataType;
  typedef Node<DataType>* nodeP;


  TopoGraph<DataType>* tempGraph =  new  TopoGraph<DataType>();
  nodeP temp1 , temp2 ,temp0;
  int index=0;
  //for( typename TopoGraph<DataType>::iterator it=g->begin() ; it!=g->end() ; it++){
  //  tempGraph ->addNode( it->id() , it->data() );
  //}
  temp0 = g->findElement(id);
  tempGraph->addNode( temp0->id() , temp0->data() );
  temp2 = tempGraph->findElement(id);
  for( int i=0; i<temp0->upSize() ; ++i){
    tempGraph->addNode( temp0->up()[i]->id()  , temp0->up()[i]->data() );
    temp1 = tempGraph->findElement( temp0->up()[i]->id() );
    //mG_Copy->addArc( temp1->id() , temp1->data() , temp2->id() , temp2->data()  );
    tempGraph->addArc( temp1 , temp2);
    ++index;
  }

  temp1 = tempGraph->findElement(id);
  for( int i=0; i<temp0->downSize() ; ++i){
    tempGraph->addNode( temp0->down()[i]->id()  , temp0->down()[i]->data() );
    temp2 = tempGraph->findElement( temp0->down()[i]->id()  );
    //mG_Copy->addArc( temp1->id() , temp1->data() , temp2->id() , temp2->data()  );
    tempGraph->addArc( temp1 , temp2);
    ++index;
  }

  FILE* f = fopen((const char*)filename.c_str() ,"w");
   if(f){
      write_dot<typename VertexClass::DataType> (f, *tempGraph, resolution, tempGraph->minF(), tempGraph->maxF(), "shape=ellipse,fontsize=10");
      fclose(f);
   }
  delete tempGraph;
}*/

#endif 


