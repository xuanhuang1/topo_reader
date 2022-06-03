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


#ifndef ARRAYSEGMENATION_H
#define ARRAYSEGMENATION_H

#include <vector>
#include <map>
#include <algorithm>
#include "BlockedArray.h"
#include "ClanHandle.h"
#include "UnionSegmentation.h"

//! An ArraySegmentation stores a segmentation as an array indexed by vertex id
template <class SegmentationArray, class FunctionArray>
class ArraySegmentation : public UnionSegmentation
{
public:

  //! Default constructor
  ArraySegmentation(const FunctionArray& function);

  //! Destructor
  virtual ~ArraySegmentation();

  //! Insert a vertex into the segmentation
  /*! Add the vertex wit the given id to the segment of the given id
   *  @param vertex_id:id of the vertex
   *  @param seg_id: index of the segment it (currently) belong to
   */
  int insert(GlobalIndexType vertex_id, GlobalIndexType seg_id);

  //! Finalize the segmentation according to the graph
  /*! Given a partially complete segmentation this function uses the given
   *  TopoGraph to fully segment all vertices. In the initial segmentation each
   *  vertex contains the index of the vertex that used to be its next highest
   *  critical point. Following these "pointers" upward we can find a current
   *  critical point above the vertex. This critical point must be a node in the
   *  graph in which case we can follow the graph downward again to find the arc
   *  corresponding to the given vertex.
   *  @param graph: the current graph potentially simplified to some 
   *                persistence and split to whatever maximal branch
   *                desired.
   */
  int complete(TopoGraphInterface& graph);

  //! Compact the index space according to the active nodes of the given graph.
  /*! Function to compact the index space to use a contiguous number of
   *  indices corresponding to the list of active nodes of the given graph
   *  The function will and number them by appearance (in the graph). The
   *  segmentation will then store indices into thisq compactified index space rather than mesh indices.
   * @param graph: The graph in question
   * @return
   */
  int compactify(TopoGraphInterface& graph);

  //! Write this segmentation to an xml-based segmentation file
  /*! Write the segmentation by attaching its data to the segmentation handle
   *  provided by the given clan handle. The function assumes the given clan
   *  contains exactly one family which contains exactly one segmentation.
   * @param clan: The clan to which this segmentation belongs
   * @param graph: The corresponding topo graph needed for its index map
   * @param map_file: An optional map file to transform the vertex indices
   * @param compactify: A flag indicating whether we need to compact the feature index space
   * @return
   */
  int writeSegmentationFile(TopologyFileFormat::ClanHandle clan,
                            TopoGraphInterface& graph,
                            const char* map_file = NULL, bool compactify=false);


  //! Split segments such that no segment is larger than count many vertices
  int splitByVertices(TopoGraphInterface& graph, uint32_t max_count, bool merge_tree);

  const SegmentationArray& segmentation() const {return mSegmentation;}

protected:

  class Compare {
  public:

    Compare(const FunctionArray& function, const bool flag) : mFunction(function), mFlag(flag) {}

    bool operator()(GlobalIndexType v0, GlobalIndexType v1) const {
      if (mFlag) {
        if (mFunction[v0] > mFunction[v1])
          return true;
        else if ((mFunction[v0] == mFunction[v1]) && (v0 > v1))
          return true;
        else
          return false;
      }
      else
        if (mFunction[v0] < mFunction[v1])
          return true;
        else if ((mFunction[v0] == mFunction[v1]) && (v0 < v1))
          return true;
        else
          return false;
    }

    const FunctionArray& mFunction;

    const bool mFlag;
  };


  //! Some form of array to store the segmentation id's of all vertices
  SegmentationArray mSegmentation;

  //! Reference to an array of function values for all vertices
  const FunctionArray& mFunction;

  //! Split the given segment into piece no larger than max_count vertices
  int splitSegment(TopoGraphInterface& graph, GlobalIndexType seg_id,
                   std::vector<GlobalIndexType>& vertices, uint32_t max_count,
                   const Compare& cmp);

};


template <class SegmentationArray, class FunctionArray>
ArraySegmentation<SegmentationArray,FunctionArray>::ArraySegmentation(const FunctionArray& function)
: mFunction(function)
{
}

template <class SegmentationArray, class FunctionArray>
ArraySegmentation<SegmentationArray,FunctionArray>::~ArraySegmentation()
{
}

template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::insert(GlobalIndexType vertex_id, GlobalIndexType seg_id)
{
  mSegmentation.insert(vertex_id,seg_id);

  return 1;
}


template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::complete(TopoGraphInterface& graph)
{
  GlobalIndexType top;
  Node* node;
  std::vector<GlobalIndexType> path;
  typename SegmentationArray::iterator it;

  uint32_t jump_count = 0;
  //for (i=0;i<mSegmentation.size();i++) {
  for (it=mSegmentation.begin();it!=mSegmentation.end();it++) {
       
    // If the i'th node is for some reason not part of the
    // segmentation we pass on its non-index and continue
    if (*it == GNULL) {

#ifdef ST_COMPLETE_DOMAIN
      sterror(true,"Unsegmented vertex found in segmentation.");
#endif
      continue;
    }


    //if (i % 50000 == 0)
    //  fprintf(stderr,"break  %f  %d\n",function.at(i),mSegmentation. at(i));

    // Here we assign top = i even though it would appear that one
    // must assign top = mSegmentation. at(i). However, if this id
    // must be fixed then we must fix the i'th id which will only
    // happen if we start at top = it
    top = it;
    path.clear();

    //fprintf(stdout,"Path:\n\t%u\n",top);
    // While this vertex is not virtual nor critical. Note, that virtual nodes
    // by construction have an index larger than the largest index present in
    // the input file. Thus, once we reach an index leading outside the given
    // segmentation we know it must be virtual. Furthermore, a hierarchy can
    // entirely remove subtrees (e.g. below a persistence threshold) in which
    // case we might point to a GNULL segmentation index. This will be passed
    // on.
    while (mSegmentation.contains(top) // not virtual
        && (top != mSegmentation.at(top))  // not critical
        && (mSegmentation.at(top) != GNULL)) { // has not been removed

      // Store this vertex and move upwards
      path.push_back(top);
      top = mSegmentation.at(top);

      jump_count++;
    }
    //fprintf(stdout,"\n");
    
    // If we found a valid index
    if (!mSegmentation.contains(top) // of a virtual vertex
        || (mSegmentation.at(top) != GNULL)) // or of something valid
      // Look for the corresponding node in the graph
      node = graph.findActiveNode(top);
    else
      node = NULL;

    // A hierarchy could return node==NULL if a branch was removed entirely
    // This is a valid use case and thus must be considered
    //sterror(node==NULL,"Segmentation inconsistent: Each critical point must exist in the graph. But for vertex %d node %d was not found.",i,top);

    // If a node was not found or the union find let to a GNULL index
    // then all children pointing to top as well as top
    // itself are no longer part of the segmentation and get assigned GNULL
    if (node == NULL) {
      if (mSegmentation.contains(top))
        mSegmentation.at(top) = GNULL;
      while (!path.empty()) {
        mSegmentation.at(path.back()) = GNULL;
        path.pop_back();
      }

      continue;
    }


    // Fix the segmentation id since top might be a simplified
    // critical point thus the change in id
    if (!node->isVirtual())
      mSegmentation.at(top) = node->id();

    
    // Going backward along the path we stored earlier we find the correct arcs
    // for all vertices along the path.

    // Until we have processed all vertices in the path (in reverse order)
    while (!path.empty()) {

      //sterror(smaller(*node,path.back(),mSegmentation.at(path.back()).f),"Function values inconsistent with segmentation %f >= %f.\n\tAre you certain that your metric allows completing the hierarchy ?",node->f(),mSegmentation.at(path.back()).f);

      // While the next node in the graph is still above the vertex we
      // are looking for
      while (greater(*this->child(node), path.back(), mFunction.at(path.back())))
        node = this->child(node);

      //sterror(smaller(*node,path.back(),function.at(path.back())),"Function values inconsistent with segmentation.");
      //sterror(greater(*child(node),path.back(),function.at(path.back())),"Function values inconsistent with segmentation.");

      //fprintf(stderr,"\t index %d  seg %d  remaining path vertices %d\n", path.back(),node->id(),path.size()-1);

      // We have found our arc and store the index
      if (mSegmentation.contains(path.back()))
        mSegmentation.at(path.back()) = node->id();

      path.pop_back();
    }

  }

  fprintf(stderr,"Completed segmentation with jump count %u.\n",jump_count);

  return 1;
}

template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::compactify(TopoGraphInterface& graph)
{
  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;
  typename SegmentationArray::iterator it;

  graph.createActiveMap(index_map);


  for (it=mSegmentation.begin();it!=mSegmentation.end();it++) {

    if (*it != GNULL) {

      mIt = index_map.find(*it);
      sterror(mIt==index_map.end(),"Mesh index %llu not found in index map.",*it);

      *it = mIt->second;

    }
  }

  return 1;
}

template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::writeSegmentationFile(TopologyFileFormat::ClanHandle clan,
                                                                              TopoGraphInterface& graph,
                                                                              const char* map_file, bool compactify)
{

  std::vector<LocalIndexType> offset;
  std::vector<LocalIndexType> count;

  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;

  typename SegmentationArray::iterator it;

  // We must use the active map of the graph to determine how
  // many features we have since there could be features without
  // vertices which we cannot discover otherwise.
  graph.createActiveMap(index_map);

  offset.resize(index_map.size()+1,0);
  count.resize(index_map.size(),0);


  // Now we scan the segmentation once to determine the counts
  // and offsets

  for (it=mSegmentation.begin();it!=mSegmentation.end();it++) {

    if (*it != GNULL) {

      if (compactify) { // If we need compactify the indices
        mIt = index_map.find(*it);
        sterror(mIt==index_map.end(),"Could not find segmentation index %d. Is this array already compactified ?", *it);

        *it = mIt->second;
        offset[mIt->second+1]++;
      }
      else {
        offset[*it+1]++;
      }
    }
  }

  for (GlobalIndexType i=1;i<offset.size();i++)
    offset[i] += offset[i-1];


  // Now we do a second pass to fill up a flat array of segmentation
  // indices. If given a map file we re-map the vertex indices
  // accordingly
  std::vector<GlobalIndexType> seg(offset.back());

  // And do a second scan in which we enter the vertices in the
  // correct order. If the map file is not NULL we use it to
  // determine the actual indices
  GlobalIndexType* mapping = NULL;

  if (map_file != NULL) {
    FILE* input = fopen(map_file,"r");
    mapping = new GlobalIndexType[mSegmentation.size()];
    fread(mapping,sizeof(GlobalIndexType),mSegmentation.size(),input);
    fclose(input);
  }

  GlobalIndexType i=0;
  for (it=mSegmentation.begin();it!=mSegmentation.end();it++) {

    if (*it != GNULL) {

       if (mapping != NULL)
        seg[offset[*it] + count[*it]] = mapping[i];
      else
        seg[offset[*it] + count[*it]] = it;

      count[*it]++;
    }
    i++;
  }


  sterror(clan.numFamilies()!=1,"Couldn't identify the family to write to.");
  sterror(!clan.family(0).providesSegmentation(),"Could not find segmentation handle.");


  TopologyFileFormat::SegmentationHandle& handle = clan.family(0).segmentation();

  handle.setOffsets(&offset);
  handle.setSegmentation(&seg);

  clan.write();


  if (mapping != NULL)
    delete[] mapping;

  return 1;
}


template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::splitByVertices(TopoGraphInterface& graph, uint32_t max_count,
                                       bool merge_tree)
{
  // Segments are lists of vertices with equal segmentatio id's. These lists
  // are *local* indices into the mSegmentation and mFunction array *not*
  // necessarily vertex id's.
  std::vector<std::vector<GlobalIndexType> > segments;
  typename SegmentationArray::iterator it;

  // The map to get from the segment id to its position in the segments
  // array
  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;
  Compare cmp(mFunction,merge_tree);

  for (it=mSegmentation.begin();it!=mSegmentation.end();it++) {
    mIt = index_map.find(*it);
    if (mIt == index_map.end()) {
      segments.push_back(std::vector<GlobalIndexType>());
      index_map[*it] = segments.size()-1;
      segments.back().push_back(it); // it gets cast into the point index of the given vertex
    }
    else
      segments[mIt->second].push_back(it); // it gets cast as above
  }

  for (mIt=index_map.begin();mIt!=index_map.end();mIt++) {
    if (segments[mIt->second].size() > max_count)
      splitSegment(graph,mIt->first,segments[mIt->second],max_count,cmp);
  }

  return 1;
}

template <class SegmentationArray, class FunctionArray>
int ArraySegmentation<SegmentationArray,FunctionArray>::splitSegment(TopoGraphInterface& graph, GlobalIndexType seg_id,
                                    std::vector<GlobalIndexType>& vertices, uint32_t max_count,
                                    const Compare& cmp)
{
  uint32_t i,j;
  uint32_t seg_count,seg_size;
  Node* top;
  Node* new_node;

  // Sort the vertices by function value according to the tree we are working with.
  // Note that we are sorting in *reverse* order. This allows us to change the
  // indices one sub-segment at a time.
  std::sort(vertices.rbegin(),vertices.rend(),cmp);

  top = graph.findActiveNode(mSegmentation.at(vertices[0]));

  // First we calculated how many pieces we need
  seg_count = vertices.size() / max_count;
  if (vertices.size() > seg_count*max_count)
    seg_count++;

  // and then how big each piece must be
  seg_size = vertices.size() / seg_count;
  if (vertices.size() > seg_size*seg_count)
    seg_size++;

  // Now we split the sequence into pieces adding the corresponding nodes to the graph
  // Note, that we also must watch out for equal function values since out sorting
  // did not take care of the SOS
  i = 0; // first vertex of the sub-segment
  j = seg_size-1; // Last vertex of the sub-segment
  while (j < vertices.size()-1) {

    // create the new node in the hierarchy. Note that splitArc returns the new node
    // which got entered "above" the old node.
    new_node = graph.splitArc(top,vertices[j],mFunction[vertices[j]]);

    // and for all vertices fix the segmentation
    for (uint32_t k=i;k<=j;k++)
      mSegmentation.at(vertices[k]) = new_node->id();

    i = j+1;
    j = i + seg_size-1;
  }


  return 1;
}


#endif

