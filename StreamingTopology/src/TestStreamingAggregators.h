/*********************************************************************** *
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov, jcbenne@sandia.gov 
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


#ifndef TESTSTREAMINGAGGREGATORS_H
#define TESTSTREAMINGAGGREGATORS_H

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "FlexArray/OOCArray.h"
#include "MultiResGraph.h"
#include "GraphIO.h"
#include "Node.h"
#include "Parser.h"
#include "Statistics/SegmentAggregator.h"

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
template <class NodeData>
int test_streaming_aggregation_functions(MultiResGraph<NodeData>& graph, HierarchyType hierarchy_type,
                                         const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                                         const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& functions,
                                         std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                                         std::map<std::string, int>& attributeNameMap,
                                         FunctionType threshold)
{
  std::map<GlobalIndexType,LocalIndexType> index_map;
  std::map<GlobalIndexType,LocalIndexType>::iterator mIt,mIt2;
  typename MultiResGraph<NodeData>::NodeArrayType::const_iterator gIt;
  std::vector<SegmentAggregator<FunctionType>*>::iterator aIt;
  std::vector<const typename MultiResGraph<NodeData>::NodeType*> active_nodes;
  std::vector< std::vector<SegmentAggregator<FunctionType>*> > attributes;

  std::vector< std::vector< std::vector<FunctionType> > > attributeValues;
  std::map<std::string, int>::iterator amIt;


  uint32_t i,j,k;
  uint32_t node_count; 
  const typename MultiResGraph<NodeData>::NodeType* child;

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
    sterror(true,"test_streaming_aggregation_functions not implemented for mixed hierarchies yet. Sorry.");
    break;
  }

  // Setup the index map from the global mesh index of the nodes to
  // the new local index within the array
  for (i=0;i<node_count;i++) 
    index_map[active_nodes[i]->id()] = i;

  // For each active segment we setup a list of aggregators
  attributes.resize(node_count);
  attributeValues.resize(node_count);
  for (i=0;i<node_count;i++) {
    attributes[i].resize(aggregators.size());
    attributeValues[i].resize(attributeNameMap.size());

    for (j=0;j<aggregators.size();j++) 
      attributes[i][j] = aggregators[j]->clone();
  }

  // Now we go through the complete segmentation and aggregate all
  // necessary values
  for (GlobalIndexType i=0;i<segmentation.size();i++) {

    if (segmentation[i] != GNULL) {

      mIt = index_map.find(segmentation[i]);

      // We assume the segmentation has been adapted to the current
      // graph
      sterror(mIt==index_map.end(),"Could not find segmentation index %llu in index map.",segmentation[i]);

      // Add the info for this element to the aggregators
      std::set<std::string> currentAttributes;
      for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
        if(!(*aIt)->attributeNameSet()) {
          // we don't which function we use - we are aggregating something that doesn't depend on function value
          (*aIt)->addVertex((functions[0])->at(i) ,i);
        }
        else {
          (*aIt)->addVertex((functions[attributeNameMap[(*aIt)->attributeName()]])->at(i) ,i);
        }
        currentAttributes.insert((*aIt)->attributeName());
      }
      for(std::set<std::string>::iterator caIt = currentAttributes.begin(); caIt != currentAttributes.end(); caIt++) {
        amIt = attributeNameMap.find(*caIt);
        sterror(amIt==attributeNameMap.end(),"Could not find attribute name %s in attribute map.", (*caIt).c_str());
        if(strcmp((*caIt).c_str(), SegmentAggregator<float>::sDefaultName) != 0) {
          attributeValues[mIt->second][amIt->second].push_back(functions[attributeNameMap[*caIt]]->at(i));
        }
      }
    }
  }

  // do initial testing
  std::cout << "doing initial testing" << std::endl;
  int numFailed = 0;
  int numSkipped = 0;
  for (i=0;i<node_count;i++) {
    for (aIt=attributes[i].begin();aIt!=attributes[i].end();aIt++) {
      amIt = attributeNameMap.find((*aIt)->attributeName());
      sterror(amIt==attributeNameMap.end(),"Could not find attribute name %s in attribute map.",(*aIt)->attributeName().c_str());

      if(amIt->second != -1 && attributeValues[i][amIt->second].size() == 0) {
        numSkipped++;
      }
      else {
        if(amIt->second == -1) {
          if(!(*aIt)->compareToStreamingComputation(attributeValues[i][0], threshold)) {
            numFailed++;
          }
        }
        else {
          if(!(*aIt)->compareToStreamingComputation(attributeValues[i][amIt->second], threshold)) {
            numFailed++;
          }
        }
      }
    }
  }

  std::cout << "done with initial testing and numFailed = " << numFailed << " out of " << node_count << std::endl;
  std::cout << "done with initial testing and numSkipped = " << numSkipped << " out of " << node_count << std::endl;


  // Unlike the union-hierarchy we want to completely aggregate the
  // attributes. To do this we simply sweep through the nodes (in the
  // appropriate order see the sort above). 
  
  // do additional testing
  std::cout << "doing additional testing" << std::endl;
  numFailed = 0;
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
      sterror(mIt==index_map.end(),"Could not find child index %llu in index map.",child->id());
      
      // Add the attributes of the parent to the attribute of the child
      for (j=0;j<aggregators.size();j++) 
        attributes[mIt->second][j]->addSegment(attributes[i][j]);



      int numParent;
      int numChild;


      for(j=0; j < attributeNameMap.size(); j++) {
        numParent = attributeValues[i][j].size();
        numChild = attributeValues[mIt->second][j].size();
	for(k=0; k < attributeValues[i][j].size(); k++) {
          attributeValues[mIt->second][j].push_back(attributeValues[i][j][k]);
	}
      }


      for (aIt=attributes[mIt->second].begin();aIt!=attributes[mIt->second].end();aIt++) {
          amIt = attributeNameMap.find((*aIt)->attributeName());
          sterror(amIt==attributeNameMap.end(),"Could not find attribute name %s in attribute map.",(*aIt)->attributeName().c_str());

	  if(amIt->second != -1) {
            if(!(*aIt)->compareToStreamingComputation(attributeValues[mIt->second][amIt->second], threshold)) {
	      numFailed++;
	    }
          } else {
            if(!(*aIt)->compareToStreamingComputation(attributeValues[mIt->second][0], threshold)) {
	      numFailed++;
	    }
	  }
      }

    }
     
  }
  std::cout << "numFailed = " << numFailed << endl;
  return 1;
}


#endif
