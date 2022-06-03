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

#ifndef FEATUREFAMILY_H
#define FEATUREFAMILY_H

#include <vector>
#include <string>
#include <stack>

#include "Definitions.h"
#include "MultiResGraph.h"
#include "Parser.h"
#include "OOCArray.h"
#include "ClanHandle.h"
#include "FamilyHandle.h"
#include "SimplificationHandle.h"
#include "FeatureElement.h"
#include "xmlParser.h"
#include "Aggregator.h"
#include "VertexCount.h"
#include "GenericData.h"

using namespace Statistics;

/*
static const char* sFileVersion = "0.0.1"; // The current version of the file format
static const char* sTopName = "FeatureFamily"; // The string identifying the top node
static const char* sFamilyName = "Family"; // The string identifying the simplification sequence
*/
/*
int write_feature_attribute(std::fstream& output, XMLNode& xml,
                            const std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                            const std::vector< std::vector<SegmentAggregator<FunctionType>*> >& values,
                            LocalIndexType k, bool ascii);
*/
//! Compute and output the attributes for all features
/*! Given a segmentation and an array of attribute values for each vertex this
 *  function will accumulate the attributes according to the given
 *  aggregators. It will create the correct XMLNode and attach it to the given
 *  one.
 *  @param output: The file to which the data should be written (the xml is written by the caller)
 *  @param xml: The parent xml node to attach the attribute nodes
 *  @param num_of_features: The number of features we will find in the
 *                          segmentation file. Note that this parameter implies
 *                          a compactified segmentation.
 *  @param segmentation: List of segmentation indices. Note that this function
 *                       assumes that the segmentation is compactified. 
 *  @param attributes: Array of arrays containing the different attributes for each vertex
 *  @param aggregators: List of aggregators describing which statistics should be collected
 *  @param ascii: Flag to indicate whether the statistics should be written in
 *                ascii (True) or binary (False).
 */
/*
int write_feature_attributes(std::fstream& output, XMLNode& xml, LocalIndexType num_of_features,
                             const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                             const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes,
                             std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                             std::map<std::string, int>& attributeNameMap,
                             bool ascii = true);
*/
/*
//! Function to write a feature family: simplification sequence + attributes
template <class NodeData>
int write_feature_family(std::fstream& output, MultiResGraph<NodeData>& graph,
                         const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                         const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes,
                         std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                         std::map<std::string, int>& attributeNameMap,
                         bool ascii = true)
{
  FileOffsetType start;
  XMLNode xml,family,node;
  LocalIndexType number_of_features = 1;
  char token[500];
  std::vector<SegmentAggregator<FunctionType>*>::iterator it;

  for (it=aggregators.begin();it!=aggregators.end();it++) {
    fprintf(stderr,"Found aggregator %s  %s\n",(*it)->typeName(),(*it)->attributeName().c_str());
  }

  // Create the top-level node with the version information
  xml = XMLNode::createXMLTopNode(sTopName);
  xml.addAttribute("version", sFileVersion);
  
  // Create the Family node
  family = xml.addChild(sFamilyName);

  if(ascii) family.addAttribute("encoding", "ascii");
  else family.addAttribute("encoding", "binary");
  family.addAttribute("globalsize",(uint32_t)sizeof(GlobalIndexType));
  family.addAttribute("localsize",(uint32_t)sizeof(LocalIndexType));

  switch (sizeof(FunctionType)) {

  case 4:    
    family.addAttribute("precision","float");
    break;
  case 8:
    family.addAttribute("precision","double");
    break;
  default:
    stwarning("Could not determine precision of simplification parameters.");
  }
  
  sprintf(token,"%.10e %.10e",graph.minF(),graph.maxF());
  family.addAttribute("range",token);

  start = static_cast<FileOffsetType>(output.tellp());
  family.addAttribute("addr",(uint64_t)start);

  // Write out the simplification sequence meaning both the necessary xml info
  // as well as the actual content
  number_of_features = graph.writeSimplificationSequence(output,family,ascii);
  
  family.addAttribute("familysize",number_of_features);

  if (!aggregators.empty())
    write_feature_attributes(output,family,number_of_features,segmentation,attributes,aggregators,attributeNameMap,ascii);
  
  start = static_cast<FileOffsetType>(output.tellp());

  XMLSTR header = xml.createXMLString();
  output << header;

  output.write((const char*)&start,sizeof(FileOffsetType));
  return 1;
}
*/

//! Compute and output the attributes for all features
/*! Given a segmentation and an array of attribute values for each vertex this
 *  function will accumulate the attributes according to the given
 *  aggregators.
 *  @param values: A list of accumulation functions one for each features and each aggregator
 *  @param feature_count: The global number of features
 *  @param segmentation: List of segmentation indices. Note that this function
 *                       assumes that the segmentation is compactified. 
 *  @param attributes: Array of arrays containing the different attributes for each vertex
 *  @param aggregators: List of aggregators describing which statistics should be collected
 *  @param attributeNameMap: A map to compute the attribute index from the name stored
 *                           in the aggregators
 */
int collect_attributes(std::vector<Attribute* >& values,
                       LocalIndexType feature_count,
                       const FlexArray::BlockedArray<GlobalIndexType,LocalIndexType>& segmentation,
                       const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes,
                       const std::vector<vector<int32_t> >& attribute_index);

//! Given a list of features with statistics accumulated the values
/*!
 * Given a list of features and associated statistics in form of segment aggregators
 * pre-accumulated all values. This function will do a recursive traversal through
 * the hierarchy to collect the values of subtrees. Note that right now only trees
 * are handled correctly and we assume the tree pointers to be child pointers
 * @param features: A list of features and their representatives pointers
 * @param values: A list of associated statistics values
 * @return 1 if successful; 0 otherwise
 */
int aggregate_attributes(TopologyFileFormat::Data<TopologyFileFormat::FeatureElement>& features,
                         std::vector<Attribute* >& values);


template <class NodeData>
int write_feature_family(TopologyFileFormat::ClanHandle clan,MultiResGraph<NodeData>& graph,
                         const FlexArray::BlockedArray<GlobalIndexType,LocalIndexType>& segmentation,
                         const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes = std::vector<Parser<GenericData<FunctionType> >::CacheArray*>(),
                         std::vector<Attribute*> aggregators = std::vector<Attribute*>(),
                         const std::vector<std::vector<int32_t> >& attribute_index = std::vector<std::vector<int32_t> >(),
                         bool aggregate = false, bool ascii = true,
                         HierarchyType hierarchy_type = MAXIMA_HIERARCHY,
                         std::vector<ArcMetric<NodeData> *> additional_metrics = std::vector<ArcMetric<NodeData> *>())
{
  TopologyFileFormat::FamilyHandle& family = clan.family(0);
  
  // Set the overall function range of the clan
  family.range(graph.minF(),graph.maxF());
  
  // Now assemble the simplification sequence
  TopologyFileFormat::FeatureElementData features;
  TopologyFileFormat::FeatureElementData extra_features;
  std::vector<GlobalIndexType> index_map;
  TopologyFileFormat::SimplificationHandle simp;
  TopologyFileFormat::IndexHandle index;
  FunctionType range[2];

  // Create the feature elements of the hierarchy
  graph.createSimplificationSequence(features,range[0],range[1],hierarchy_type);
  
  // Set the features as data from the simplification handle 
  simp.setData(&features);

  // Set the range 
  simp.setRange(range[0],range[1]);
 
  // For the moment we assume that we have merge or split trees in which case we
  // always only have a single represetative
  simp.fileType(TopologyFileFormat::SINGLE_REPRESENTATIVE);

  // Set the name of the metric we use
  if (graph.hierarchyMetric() != NULL)
    simp.metric(std::string(graph.hierarchyMetric()->name()));

  // Set the encoding
  simp.encoding(ascii);


#if 0
  // This code add the index list which at the moment creates more problems
  // since we assume the one-to-one map

  // Get the list of global ids for the active features
  graph.createGlobalIndices(index_map);

  TopologyFileFormat::Data<GlobalIndexType> index_data(&index_map);

  // Set the indices as data of the index handle
  index.setData(&index_data);

  // Set the encoding
  index.encoding(ascii);

  // Add the index map to the simplification handle
  simp.indexHandle(index);
#endif

  // Add the simplification handle to the family
  family.add(simp);
 
  
  // If there is an aggregator at all
  if (!aggregators.empty()) {
    
    // Now we compute all attributes
    collect_attributes(aggregators,features.size(),segmentation,attributes,attribute_index);

    // If we are supposed to pre-aggregate the statistics
    if (aggregate) {
      aggregate_attributes(features,aggregators);
    }

    
    // Going forward we will create handles and data for all attributes
    TopologyFileFormat::StatHandle handle;
    
    handle.encoding(ascii);
    
    if (aggregate)
      handle.aggregated(true);

    for (uint8_t i=0;i<aggregators.size();i++) {
      
      handle.setData(aggregators[i]);
      handle.stat(std::string(aggregators[i]->typeName()));

      std::string species("");
      for (uint8_t k=0;k<aggregators[i]->numVariables();k++)
        species += aggregators[i]->variableName(k);
      handle.species(species);

      family.add(handle);
    }
  }

  sterror(additional_metrics.size() > 1,"Not implemented yet .. porblems with FeatureData constructor\n");
  // If we are given additional metrics to store in the file we must create the
  // appropriate simplification sequences and add them to the family

  // Note that we need to create extra vectors for the features since the information
  // is only written to the file at the very end
  for (uint8_t i=0;i<additional_metrics.size();i++) {
    TopologyFileFormat::SimplificationHandle extra_simp;

    graph.clearHierarchy();
    graph.constructHierarchy(*additional_metrics[i],hierarchy_type);

    fprintf(stderr,"Creating extra metric %s\n",graph.hierarchyMetric()->name());
    // Create the feature elements of the hierarchy
    graph.createSimplificationSequence(extra_features,range[0],range[1],hierarchy_type);

    // Set the features as data from the simplification handle
    extra_simp.setData(&extra_features);

    // Set the range
    extra_simp.setRange(range[0],range[1]);

    // For the moment we assume that we have merge or split trees in which case we
    // always only have a single represetative
    extra_simp.fileType(TopologyFileFormat::SINGLE_REPRESENTATIVE);

    // Set the name of the metric we use
    if (graph.hierarchyMetric() != NULL)
      extra_simp.metric(std::string(graph.hierarchyMetric()->name()));

    // Set the encoding
    extra_simp.encoding(ascii);

    // Add the simplification handle to the family
    family.add(extra_simp);
  }


  // and write the file
  clan.write();

  
  return 1;
}

template <class NodeData>
int write_feature_family(const char* filename, MultiResGraph<NodeData>& graph,
                         HierarchyType hierarchy_type)
{
  // Create a handle to a family
  TopologyFileFormat::ClanHandle clan;
  TopologyFileFormat::FamilyHandle family;

  // Set the overall function range of the clan
  family.range(graph.minF(),graph.maxF());

  // Now assemble the simplification sequence
  TopologyFileFormat::Data<TopologyFileFormat::FeatureElement> features;
  std::vector<LocalIndexType> indexMap;
  TopologyFileFormat::SimplificationHandle simp;
  TopologyFileFormat::IndexHandle index;
  FunctionType range[2];

  // Create the feature elements of the hierarchy
  graph.createSimplificationSequence(features,range[0],range[1],hierarchy_type);

  // Set the features as data from the simplification handle
  simp.setData(&features);

  // Set the range
  simp.setRange(range[0],range[1]);

  // For the moment we assume that we have merge or split trees in which case we
  // always only have a single represetative
  simp.fileType(TopologyFileFormat::SINGLE_REPRESENTATIVE);

  // Set the name of the metric we use
  if (graph.hierarchyMetric() != NULL)
    simp.metric(std::string(graph.hierarchyMetric()->name()));

  // Set the encoding
  simp.encoding(false);

  // Add the simplification handle to the family
  family.add(simp);

  // Finally, we attach the family to the clan
  clan.add(family);

  // and write the file
  clan.write(filename);


  return 1;

}


#endif

