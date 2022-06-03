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

#include <algorithm>
#include <string>
#include "ArraySegmentation.h"

typedef FlexArray::MappedArray<GlobalIndexType,GlobalIndexType,LocalIndexType> SegmentationArray;
typedef FlexArray::MappedArray<FlexArray::MappedFunctionElement<GlobalIndexType,LocalIndexType>,GlobalIndexType,LocalIndexType> FunctionArray;


template class ArraySegmentation<SegmentationArray,FunctionArray>;

#if 0
ArraySegmentation::ArraySegmentation(const FunctionArray& function)
: mFunction(function)
{
}

ArraySegmentation::~ArraySegmentation()
{
}

int ArraySegmentation::insert(GlobalIndexType vertex_id, GlobalIndexType seg_id)
{
  if (vertex_id >= mSegmentation.size())
    mSegmentation.resize(vertex_id+1);

  mSegmentation.at(vertex_id) = seg_id;

  return 1;
}


int ArraySegmentation::complete(TopoGraphInterface& graph)
{
  GlobalIndexType i,top;
  Node* node;
  std::vector<GlobalIndexType> path;
  uint64_t seg_size = mSegmentation.size();

  uint32_t jump_count = 0;

  for (i=0;i<mSegmentation.size();i++) {

    // If the i'th node is for some reason not part of the
    // segmentation we pass on its non-index and continue
    if (mSegmentation.at(i) == GNULL) {

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
    // happen if we start at top = i
    top = i;
    path.clear();

    //fprintf(stdout,"Path:\n\t%u\n",top);
    // While this vertex is not virtual nor critical. Note, that virtual nodes
    // by construction have an index larger than the largest index present in
    // the input file. Thus, once we reach an index leading outside the given
    // segmentation we know it must be virtual. Furthermore, a hierarchy can
    // entirely remove subtrees (e.g. below a persistence threshold) in which
    // case we might point to a GNULL segmentation index. This will be passed
    // on.
    while ((top < seg_size) && (top != mSegmentation.at(top)) && (mSegmentation.at(top) != GNULL)) {

      // Store this vertex and move upwards
      path.push_back(top);
      top = mSegmentation.at(top);

      jump_count++;
    }
    //fprintf(stdout,"\n");

    // If we found a valid index
    if (mSegmentation.at(top) != GNULL)
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
      mSegmentation.at(top) = node->id();
      while (!path.empty()) {
        mSegmentation.at(path.back()) = GNULL;
        path.pop_back();
      }
    }


    // Fix the segmentation id since top might be a simplified
    // critical point thus the change in id
    if (!node->isVirtual())
      mSegmentation.at(top) = node->id();


    //if (i == 31) {
    //  fprintf(stderr,"Vertex %d   to node id %d\n",top,node->id());
    //  fprintf(stderr,"Complete segmentation index %d  seg %d, starting from %d  %d vertices in path\n",i,mSegmentation.at(i),node->id(),path.size());
    //}

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
      if (path.back() < seg_size)
        mSegmentation.at(path.back()) = node->id();

      path.pop_back();
    }

  }

  fprintf(stderr,"Completed segmentation with jump count %u.\n",jump_count);

  return 1;
}

int ArraySegmentation::compactify(TopoGraphInterface& graph)
{
  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;

  graph.createActiveMap(index_map);

  for (GlobalIndexType i=0;i<mSegmentation.size();i++) {

    if (mSegmentation.at(i) != GNULL) {

      mIt = index_map.find(mSegmentation.at(i));
      sterror(mIt==index_map.end(),"Mesh index %llu not found in index map.",mSegmentation.at(i));

      mSegmentation.at(i) = mIt->second;

    }
  }

  return 1;
}

int ArraySegmentation::writeSegmentationFile(TopologyFileFormat::ClanHandle clan,
                                             TopoGraphInterface& graph,
                                             const char* map_file, bool compactify)
{

  std::vector<LocalIndexType> offset;
  std::vector<LocalIndexType> count;

  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;

  // We must use the active map of the graph to determine how
  // many features we have since there could be features without
  // vertices which we cannot discover otherwise.
  graph.createActiveMap(index_map);

  offset.resize(index_map.size()+1,0);
  count.resize(index_map.size(),0);


  // Now we scan the segmentation once to determine the counts
  // and offsets

  for (GlobalIndexType i=0;i<mSegmentation.size();i++) {

    if (mSegmentation.at(i) != GNULL) {

      if (compactify) { // If we need compactify the indices
        mIt = index_map.find(mSegmentation.at(i));
        sterror(mIt==index_map.end(),"Could not find segmentation index %d. Is this array already compactified ?", mSegmentation.at(i));

        mSegmentation.at(i) = mIt->second;
        offset[mIt->second+1]++;
      }
      else {
        offset[mSegmentation.at(i)+1]++;
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


  for (GlobalIndexType i=0;i<mSegmentation.size();i++) {

    if (mSegmentation.at(i) != GNULL) {

       if (mapping != NULL)
        seg[offset[mSegmentation.at(i)] + count[mSegmentation.at(i)]] = mapping[i];
      else
        seg[offset[mSegmentation.at(i)] + count[mSegmentation.at(i)]] = i;

      count[mSegmentation.at(i)]++;
    }
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


int ArraySegmentation::splitByVertices(TopoGraphInterface& graph, uint32_t max_count,
                                       bool merge_tree)
{
  // Segments are lists of vertices with equal segmentatio id's. These lists
  // are *local* indices into the mSegmentation and mFunction array *not*
  // necessarily vertex id's.
  std::vector<std::vector<GlobalIndexType> > segments;

  // The map to get from the segment id to its position in the segments
  // array
  std::map<GlobalIndexType,GlobalIndexType> index_map;
  std::map<GlobalIndexType,GlobalIndexType>::iterator mIt;
  Compare cmp(mFunction,merge_tree);

  for (GlobalIndexType i=0;i<mSegmentation.size();i++) {
    mIt = index_map.find(mSegmentation.at(i));
    if (mIt == index_map.end()) {
      segments.push_back(std::vector<GlobalIndexType>());
      index_map[mSegmentation.at(i)] = segments.size()-1;
      segments.back().push_back(i);
    }
    else
      segments[mIt->second].push_back(i);
  }

  for (mIt=index_map.begin();mIt!=index_map.end();mIt++) {
    if (segments[mIt->second].size() > max_count)
      splitSegment(graph,mIt->first,segments[mIt->second],max_count,cmp);
  }

  return 1;
}

int ArraySegmentation::splitSegment(TopoGraphInterface& graph, GlobalIndexType seg_id,
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
