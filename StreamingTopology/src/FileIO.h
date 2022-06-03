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


#ifndef FILEIO_H
#define FILEIO_H

#include <map>
#include <vector>

#include "BlockedArray.h"
#include "Parser.h"
#include "MultiResGraph.h"
#include "GraphIO.h"
#include "Node.h"
#include "Attribute.h"

const float MAX_VALUE = 1.0e+38;

//! Given a graph and its corresponding segmentation write an extrema hierarchy
/*! Given a graph and its correpsonding segmentation this function
 *  writes the hierarchy of extrema including the vertex count for
 *  each corresponding component the file format (described for a
 *  merge tree of maxima) is the following
 * 
 *  <float32> <float32> <float32> "fMin fMax persistence"
 *  <int32> "# of maxima"
 *  <int32> "area sampling desity"
 *  
 *  "a list of"
 *  <int32> <int32> <float32> <float32> 
 *  :
 *  :
 *  <int32> <int32> <float32> <float32> 
 *
 *  "for each maximum/stable manifold this list contains the
 *  <mesh-index of the max>  <index in the list of its parent> <function value> <persistence> "
 *
 *  "finally a (#of max+1)x(sampling density) grid of <float32> where the first row stores the function intervalls as
 *  <f0> <f1> <f2> .... <fn>
 *  and row i+1 stores the areas of the i'th stable manifold covering in a given function interval
 *  <area in [f0,f1)> <area in [f1,f2)> ... <area in [fn,inf]> 
 *
 */
template <class NodeData>
int write_extrema_vertex_count_hierarchy(FILE* output, MultiResGraph<NodeData>& graph,
                                         const FlexArray::BlockedArray<GlobalIndexType,LocalIndexType>& segmentation,
                                         const FlexArray::BlockedArray<FunctionType,LocalIndexType>& function,
                                         int32_t sampling_density, MorseType type)
{
  std::map<GlobalIndexType,LocalIndexType> index_map;
  std::map<GlobalIndexType,LocalIndexType>::iterator mIt;
  typename MultiResGraph<NodeData>::NodeArrayType::const_iterator gIt;
  int32_t leaf_count = 0;
  int32_t indices[2];
  float function_values[2];
  const typename MultiResGraph<NodeData>::NodeType* node;
  
  sterror((type != MAXIMUM) && (type != MINIMUM),"This function only outputs maxima or minima hierarchies.");

  // First we collect and count all maxima
  for (gIt=graph.nodes().begin();gIt!=graph.nodes().end();gIt++) {
    if (gIt->isActive() && ((gIt->morseType() == type) || (gIt->type() == ROOT)))
      index_map[gIt->id()] = leaf_count++;
  }
  
  // Now we setup the map for all other active nodes
  for (gIt=graph.nodes().begin();gIt!=graph.nodes().end();gIt++) {
    if (gIt->isActive() && (gIt->morseType() != type) && (gIt->type() != ROOT)) {
      
      if (gIt->downSize() == 0) 
        node = graph.findActiveLeaf(gIt->up(0));
      else if (gIt->upSize() == 0) 
        node = graph.findActiveLeaf(gIt->down(0));
      else
        node = graph.findActiveLeaf(gIt->id());
    
      sterror(node==NULL,"Could not find active leaf for node %d.",gIt->id());

      mIt = index_map.find(node->id());
      
      sterror(mIt==index_map.end(),"Could not find active leaf in index map.");
      
      index_map[gIt->id()] = mIt->second;
    }
  }
  
  // Write the header information

  float tmp;

  tmp = graph.minF();  fwrite(&tmp,sizeof(float),1,output);
  tmp = graph.maxF();  fwrite(&tmp,sizeof(float),1,output);
  tmp = graph.persistence();  fwrite(&tmp,sizeof(float),1,output);

  fprintf(stderr,"minF %f   maxF %f  persistence %f\n",graph.minF(),graph.maxF(),0.0);
  
  fwrite(&leaf_count,sizeof(int32_t),1,output);
  fwrite(&sampling_density,sizeof(int32_t),1,output);

  fprintf(stderr,"Number of buckets %d  resolution %d\n",leaf_count,sampling_density);
  
  // Write the hierarchy information
  leaf_count = 0;
  for (gIt=graph.nodes().begin();gIt!=graph.nodes().end();gIt++) {
    if (gIt->isActive() && ((gIt->morseType() == type) || (gIt->type() == ROOT))) {
      
      // set the comon values
      indices[0] = gIt->id();
      function_values[0] = gIt->f();
      leaf_count++;

      // if this leaf/root is never cancelled
      if (gIt->parent() == NULL) {
        indices[1] = leaf_count-1;
        function_values[1] = MAX_VALUE;
      }
      else {
        mIt = index_map.find(gIt->parent()->id());
        
        sterror(mIt==index_map.end(),"Could not find active leaf in index map.");
        
        indices[1] = mIt->second;
        function_values[1] = gIt->persistence() - graph.maxF() + gIt->f();
      }
      
      // write the four values to the file
      fwrite(indices,sizeof(int32_t),2,output);
      fwrite(function_values,sizeof(float),2,output);
      
      //fprintf(stderr,"Region %d (%d)  Parent %d  Func %f  Per %f\n",indices[0],gIt->id(),indices[1],
      //        function_values[0],function_values[1]);
    }
  }
  
  // Now allocate an array of vertex counts
  vector<float> count_grid((leaf_count+1)*sampling_density,0);

  // Compute the sampling boundaries
  for (int i=0;i<sampling_density;i++) 
    count_grid[i] = graph.minF() + i*(graph.maxF() - graph.minF()) / (float)(sampling_density - 1);
  

  int32_t bucket;
  float delta = (graph.maxF() - graph.minF()) / (float)(sampling_density - 1);

  for (GlobalIndexType i=0;i<segmentation.size();i++) {
    
    if (segmentation[i] != GNULL) {
      
      mIt = index_map.find(segmentation[i]);
      
      sterror(mIt==index_map.end(),"Could not find segmentation index in index map.");
      
      bucket = (int32_t)((function[i] - graph.minF()) / delta);
      
      count_grid[sampling_density*(mIt->second+1) + bucket] += 1;
    }
  }
    
  
  // Finally we write out the count grid
  fwrite(&count_grid[0],sizeof(float),count_grid.size(),output);
  
  return 1;
}

//! Given a segmented graph output a union-style feature hierarchy
/*! Given a graph and a corresponding segmentation this function outputs a
 *  union-style hierarchy. Note, that the function assumes that the segmentation
 *  has been pre-adapted to the given graph, i.e. all id's found in the
 *  segmentation are currently active nodes in the graph. The function will
 *  iterate through the segmentation and aggregate all given attributes on a per
 *  node basis. The resulting information is written out in the following
 *  (ascii) format (only the non #'ed lines are written).
 *
 *  # Header information
 *  NodeCount <int>
 *  Range <float> <float>
 *  Attributes <int> <name1> <name2> .... <nameN>
 * 
 *  # Node information: one line per node
 *  <Node-ID> <Node-Type> <Local-Parent-ID> <Global-Parent-ID> <Function-Value> <Persistence> <Attribute0> ...
 *  :
 *  :
 */
/*
template <class NodeData>
int write_union_hierarchy(FILE* output, MultiResGraph<NodeData>& graph,
                          const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                          const FlexArray::BlockedArray<FunctionType>& function,
                          std::vector<Statistics::Attribute*>& attributes)
{
  std::map<GlobalIndexType,LocalIndexType> index_map;
  std::map<GlobalIndexType,LocalIndexType>::iterator mIt,mIt2;
  typename MultiResGraph<NodeData>::NodeArrayType::const_iterator gIt;
  typename std::vector<Statistics::Attribute*>::iterator aIt;
  typename MultiResGraph<NodeData>::NodeType* node;
  uint32_t leaf_count = 0;
  uint32_t i,j;
  GlobalIndexType parent_id,parent_local_id;

  // First we collect and count all active nodes
  for (gIt=graph.nodes().begin();gIt!=graph.nodes().end();gIt++) {
    if (gIt->isActive()) 
      index_map[gIt->id()] = leaf_count++;
  }

  // Now we make sure that there is enough space for all attributes
  for (aIt=attributes.begin();aIt!=attributes.end();aIt++)
    (*aIt)->resize(leaf_count);

  
  // Now we go through the complete segmentation and aggregate all
  // necessary values
  for (GlobalIndexType i=0;i<segmentation.size();i++) {
    
    if (segmentation[i] != GNULL) {
      
      mIt = index_map.find(segmentation[i]);
      
      // We assume the segmentation has been adapted to the current
      // graph
      sterror(mIt==index_map.end(),"Could not find segmentation index %llu in index map.",segmentation[i]);
      
      // Add the info for this element to the aggregators
      for (aIt=attributes.begin();aIt!=attributes.end();aIt++) {
        (*aIt)[mIt->second]->addVertex(function[i],i);
      }
    }
  }

  
  // Finally we write the file
  fprintf(output,"NodeCount %d\n",leaf_count);
  fprintf(output,"Range %f %f\n",graph.minF(),graph.maxF());
  fprintf(output,"Attributes %d",(int)aggregators.size());
  
  for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
    fprintf(output," ");
    (*aIt)->typeToASCII(output);
  }
  fprintf(output,"\n");
    
  for (mIt=index_map.begin();mIt!=index_map.end();mIt++) {
    node = graph.findElement(mIt->first);

    if (node->parent() == NULL) {
      parent_id = GNULL;
      parent_local_id = GNULL;
    }
    else {
      mIt2 = index_map.find(node->parent()->id());
      sterror(mIt2==index_map.end(),"Could not find parent index in map.");
      
      parent_id = mIt2->first;
      parent_local_id = mIt2->second;
    }
    
    fprintf(output,"%u %u %u %u %f %f",node->id(),node->morseType(),
            parent_local_id,parent_id,node->f(),node->persistence());
    
    for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
      
      fprintf(output," ");
      (*aIt)->valueToASCII(output);
    }

    fprintf(output,"\n");
  }

  return 1;
}
*/
//! Given a graph and segmentation write the corresponding one-parameter family to file
/*! Given a multi-resolution graph and its corresponding segmentation
 *  this function will output the one-parameter family of
 *  segmentations in ascii format. Unlike the union-hierarchy this
 *  format is less dependent on the nature of the hierarchy and
 *  instead relies more heavily on the life span of features. In
 *  particular, the family will be "aggregated" meanning that merged
 *  features will store already aggregated attributes. This requires a
 *  slightly different view on the roles of graph nodes wrt. to the
 *  features they encode. Given a typical cancellation u-v-w, u and w
 *  each represent their own feature while v now represents the
 *  combined u,v,w feature. Nevertheless, it is still useful to record
 *  the highest maximum in each feature and we do so in form of a
 *  representative. The format is the following with the list of
 *  features/nodes sorted by decreasing/increasing (for max/min
 *  hierarchies) order of birth points.
 * 
 *  NodeCount <int>
 *  Range <float> <float>
 *  Attributes <int> <name1> ... <namen>
 *  <node-id> <child-id> <representative-id> <min-life-span> <max-life-span> <a0> <a1> ... <an>
 * 
 *  node-id : the (global) id of the node creating this feature (i.e. an extremum *or* a saddle)
 *  child-id: the (local) id of the child of this node (the next lower vertex)
 *  rep-id  : the (global) id of the node representing this feature (must be an extremum)
 *  life-span: two floats describing the life-span of this feature
 *  a_i     : the attribute information for each aggregated attribute 
 * 
 * For the moment the function is hardcoded for merge hierarchies
 */
/*
template <class NodeData>
int write_aggregated_family(FILE* output, MultiResGraph<NodeData>& graph, HierarchyType hierarchy_type,
                            const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                            const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& functions,
                            std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                            std::map<std::string, int>& attributeNameMap)
{
  std::map<GlobalIndexType,LocalIndexType> index_map;
  std::map<GlobalIndexType,LocalIndexType>::iterator mIt,mIt2;
  typename MultiResGraph<NodeData>::NodeArrayType::const_iterator gIt;
  typename std::vector<SegmentAggregator<FunctionType>*>::iterator aIt;
  typename std::vector<const typename MultiResGraph<NodeData>::NodeType*> active_nodes;
  typename std::vector< std::vector<SegmentAggregator<FunctionType>*> > attributes;
  uint32_t i,j;
  uint32_t node_count; 
  GlobalIndexType parent_id;
  const typename MultiResGraph<NodeData>::NodeType* child;

  fprintf(stderr,"This function is deprecated since the aggregators no longer have a FILE* interface \
(See the uncommented at the end of the function).\n");
  assert(false);

  // First we collect all active nodes
  for (gIt=graph.nodes().begin();gIt!=graph.nodes().end();gIt++) {
    if (gIt->isActive()) 
      active_nodes.push_back(gIt);
  }
  node_count = active_nodes.size();

  // Depending on the hierarchy type we sort the nodes by "birth" value. This is
  // crucial to perform the aggregation in a single sweep since we must
  // guarantee that features only pass on their aggregated values if they have
  // collected all their corresponding values themselfs. This obviously does not
  // work for mixed hierarchies (you would need a graph sort) which explains the
  // assert.
  switch (hierarchy_type) {
  case MAXIMA_HIERARCHY:
    // In a maximum hierarchy features get accumulated top to bottom and thus we
    // sort the nodes by decreasing function value
    sort(active_nodes.rbegin(),active_nodes.rend(),nodeCmp);
    break;
  case MINIMA_HIERARCHY:
    // In a minimum hierarchy features get accumulated bottom to top and thus we
    // sort the nodes by increasing function value
    sort(active_nodes.begin(),active_nodes.end(),nodeCmp);
    break;
  case MIXED_HIERARCHY:
    sterror(true,"write_aggregated_family not implemented for mixed hierarchies yet. Sorry.");
    break;
  }

  // Setup the index map from the global mesh index of the nodes to
  // the new local index within the array
  for (i=0;i<node_count;i++) 
    index_map[active_nodes[i]->id()] = i;

  // For each active segment we setup a list of aggregators
  attributes.resize(node_count);
  for (i=0;i<node_count;i++) {
    
    attributes[i].resize(aggregators.size());
    for (j=0;j<aggregators.size();j++) 
      attributes[i][j] = aggregators[j]->clone();
  }

  
  // Now we go through the complete segmentation and aggregate all
  // necessary values
  for (GlobalIndexType i=0;i<segmentation.size();i++) {
    
    if (segmentation[i] != GNULL) {
      
      fprintf(stderr,"Trying to fix segmentation index %d\n",i);

      mIt = index_map.find(segmentation[i]);
      
      // We assume the segmentation has been adapted to the current
      // graph
      sterror(mIt==index_map.end(),"Could not find segmentation index %llu  for vertex %llu in index map.",segmentation[i],i);
      
      // Add the info for this element to the aggregators
      for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
        if(!(*aIt)->attributeNameSet() ) {
          // we don't which function we use - we are aggregating something that doesn't depend on function value
          (*aIt)->addVertex((functions[0])->at(i) ,i);
        }
        else {
          (*aIt)->addVertex((functions[attributeNameMap[(*aIt)->attributeName()]])->at(i) ,i);
        }
      }
    }
  }

  // Prepare to output the file by writing the header information
  fprintf(output,"NodeCount %d\n",node_count);
  fprintf(output,"Range %0.8f %0.8f\n",graph.minF(),graph.maxF());
  fprintf(output,"Attributes %d",(int)aggregators.size());
  
  for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
    fprintf(output," ");
    (*aIt)->typeToASCII(output);
  }
  fprintf(output,"\n");


  // Unlike the union-hierarchy we want to completely aggregate the
  // attributes. To do this we simply sweep through the nodes (in the
  // appropriate order see the sort above). 
  for (i=0;i<node_count;i++) {

    switch (hierarchy_type) {
    case MAXIMA_HIERARCHY:
      if (active_nodes[i]->downSize() > 0) {
        child = active_nodes[i]->down(0);
        sterror(active_nodes[i]->downSize() != 1,"Currently, maxima hierarchies require pure merge trees. Sorry.");
      }
      else
        child = NULL;
      break;
    case MINIMA_HIERARCHY:
      if (active_nodes[i]->upSize() > 0) {
        child = active_nodes[i]->up(0);
        sterror(active_nodes[i]->upSize() != 1,"Currently, minima hierarchies require pure split trees. Sorry.");
      }
      else
        child = NULL;
      break;
    case MIXED_HIERARCHY:
      sterror(true,"Not implemented yet.");
      break;
    }

    // If we need to pass on our aggregated function values
    if (child != NULL) {
            
      // Find the local index of its child  
      mIt = index_map.find(child->id());

      // Add the attributes of the parent to the attribute of the child
      for (j=0;j<aggregators.size();j++) 
        attributes[mIt->second][j]->addSegment(attributes[i][j]);
    }
     
    if (active_nodes[i]->parent() == NULL)
      parent_id = active_nodes[i]->id();
    else
      parent_id = active_nodes[i]->parent()->id();

    if (child != NULL) {
      if (hierarchy_type == MAXIMA_HIERARCHY) {
        fprintf(output,"%u %u %u %0.8f %0.8f",active_nodes[i]->id(),mIt->second,parent_id, 
                child->f(),active_nodes[i]->f());
      }
      else {
        fprintf(output,"%u %u %u %0.8f %0.8f",active_nodes[i]->id(),mIt->second,parent_id, 
                active_nodes[i]->f(),child->f());        
      }
    }
    else {
      fprintf(output,"%u %u %u %0.8f %0.8f",active_nodes[i]->id(),LNULL,parent_id, 
              active_nodes[i]->f(),active_nodes[i]->f());
    }

    for (aIt=attributes[i].begin();aIt!=attributes[i].end();aIt++) {
      
      fprintf(output," ");
      // This call no longer exists and to make things compile has been commented out
      //(*aIt)->writeASCII(output);
    }

    fprintf(output,"\n");
  }

  return 1;
}
*/

#endif
