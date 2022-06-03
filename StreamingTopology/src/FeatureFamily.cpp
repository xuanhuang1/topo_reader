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

#include "FeatureFamily.h"

const char* sAttributeName = "Attribute"; // The string identifying the attribute node

/*
int write_feature_attribute(std::fstream& output, XMLNode& xml,
                            const std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                            const std::vector< std::vector<SegmentAggregator<FunctionType>*> >& values,
                            LocalIndexType k, bool ascii)
{

  XMLNode attribute;
  uint32_t i;
  FileOffsetType offset;

  // Create a new xml node for this attribute
  attribute = xml.addChild(sAttributeName);

  attribute.addAttribute("name",aggregators[k]->typeName());

  if (aggregators[k]->attributeName() != SegmentAggregator<FunctionType>::sDefaultName)
    attribute.addAttribute("attribute",aggregators[k]->attributeName().c_str());
  else
    attribute.addAttribute("attribute","none");



  if (ascii)
    attribute.addAttribute("encoding","ascii");
  else
    attribute.addAttribute("encoding","binary");


  // Similarly, we assume for the moment that all our attributes are complete
  // sets of features with a one-to-one corresponding to the family
  attribute.addAttribute("mapping","direct");

  offset = static_cast<FileOffsetType>(output.tellp());
  attribute.addAttribute("addr",(uint64_t)offset);


  if (ascii) {

    for (i=0;i<values.size();i++) {
      values[i][k]->writeASCII(output);
    }
  }
  else {
    stwarning("Binary attributes not implemented yet ... sorry.\n");

    return 0;
  }


  return 1;
}

int write_feature_attributes(std::fstream& output, XMLNode& xml, LocalIndexType num_of_features,
                             const FlexArray::BlockedArray<GlobalIndexType>& segmentation,
                             const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes,
                             std::vector<SegmentAggregator<FunctionType>*>& aggregators,
                             std::map<std::string, int>& attributeNameMap,
                             bool ascii)
{
  LocalIndexType i,j;
  GlobalIndexType k;
  uint32_t num_of_aggregators = aggregators.size();
  std::vector<uint32_t> attribute_index(num_of_aggregators);
  std::vector<SegmentAggregator<FunctionType>*>::iterator aIt;

  // For each feature this vector stores a vector of aggregators
  std::vector< std::vector<SegmentAggregator<FunctionType>*> > values(num_of_features);

  for (i=0;i<num_of_aggregators;i++) {

    if(!aggregators[i]->attributeNameSet()) {
      // If the user did not specify which function to aggregate, the
      // aggregator does not depend on the function value. Thus we simply
      // pass the first one.
      attribute_index[i] = 0;
    }
    else {
      // Otherwise pass the corresponding attribute to accumulate
      attribute_index[i] = attributeNameMap[aggregators[i]->attributeName()];
    }

    //fprintf(stderr,"Found attribute index %d\n",attribute_index[i]);
  }


  // Setup an aggregator for each feature and each statistics that should be
  // accumulated
  for (i=0;i<num_of_features;i++) {

    values[i].resize(aggregators.size());
    for (j=0;j<aggregators.size();j++)
      values[i][j] = aggregators[j]->clone();
  }

  // Now go through the complete segmentation and aggregate all necessary values
  for (k=0;k<segmentation.size();k++) {

    // Only if this vertex is assigned to a feature do we need to consider it
    if (segmentation[k] != GNULL) {

      sterror(segmentation[k]>=num_of_features,"Number of features was supposed to be %llu but we found a segmentation index %llu. Did you use --raw-segmentation ?.",
              (uint64_t)num_of_features,(uint64_t)segmentation[k]);


      // Add the info for this element to the aggregators
      for (i=0,aIt=values[segmentation[k]].begin();aIt!=values[segmentation[k]].end();aIt++,i++) {
        //fprintf(stderr,"Processing vertex %d with feature %d at aggregator \"%s\" with aname %s\n",k,segmentation[k],(*aIt)->typeName(),(*aIt)->attributeName());
        (*aIt)->addVertex(attributes[attribute_index[i]]->at(k),k);

        //if (segmentation[k] == 1)
        //fprintf(stderr,"Adding vertex %d with %f value to feature %d. New avg %f\n",k,attributes[attributeNameMap[(*aIt)->attributeName()]]->at(k),segmentation[k],(*aIt)->value());
      }
    }
  }

  // By now we have accumulated all necessary statistics and we now will write them one by one
  for (i=0;i<aggregators.size();i++)
    write_feature_attribute(output,xml,aggregators,values,i,ascii);

  return 1;
}
*/


int collect_attributes(std::vector<Attribute* >& values,
                       LocalIndexType feature_count,
                       const FlexArray::BlockedArray<GlobalIndexType,LocalIndexType>& segmentation,
                       const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes,
                       const std::vector<std::vector<int32_t> >& attribute_index)
{
  LocalIndexType i;
  GlobalIndexType k;
  std::vector<Attribute*>::iterator aIt;

  sterror(values.size()!=attribute_index.size(),"Number of statistics does not match the number of attribute indices.");

  // Make sure there is enough space for each value array
  for (aIt=values.begin();aIt!=values.end();aIt++)
    (*aIt)->resize(feature_count);


   // Now go through the complete segmentation and aggregate all necessary values
  for (k=0;k<segmentation.size();k++) {

    // Only if this vertex is assigned to a feature do we need to consider it
    if (segmentation[k] != GNULL) {

      sterror(segmentation[k]>=feature_count,"Number of features was supposed to be %llu but we found a segmentation index %llu. Did you use --raw-segmentation ?.",
              (uint64_t)(feature_count),(uint64_t)segmentation[k]);

      for (i=0;i<values.size();i++) {
        if (values[i]->numVariables() == 1) {
          (*values[i])[segmentation[k]].addVertex(attributes[attribute_index[i][0]]->at(k),k);
        }
        else if (values[i]->numVariables() == 1) {
          (*values[i])[segmentation[k]].addVertex(0,k);
        }
        else {
          (*values[i])[segmentation[k]].addVertex(attributes[attribute_index[i][0]]->at(k),
                                                  attributes[attribute_index[i][1]]->at(k),k);
        }
      }

    }
  }

  return 1;
}

int aggregate_attributes(TopologyFileFormat::Data<TopologyFileFormat::FeatureElement>& features,
                         std::vector<Attribute* >& values)
{
  // Since unfortunately features store representative pointers it is not obvious how
  // to know whether all constituents of a node have been processed. In order to enable
  // this we first count how many (direct) constituent pointers every feature has.

  std::vector<uint32_t> counters(features.size(),0);
  uint32_t i,top;

  for (i=0;i<features.size();i++)  {
    if (!features[i].links().empty()) {
      counters[features[i].link(0)]++;
    }
  }


  // Now we create a stack to keep track of the traversal and initialize
  // it with all features that have no constituents
  std::stack<uint32_t> s;

  for (uint32_t i=0;i<features.size();i++)  {
    if (counters[i] == 0)
      s.push(i);
  }

  // Now we process all features
  while (!s.empty()) {

    top = s.top();
    s.pop();

    // If the top most feature has a representative
    if (!features[top].links().empty()) {

      // Pass on the values for all aggregators
      for (i=0;i<values.size();i++)
        values[i]->addSegment(features[top].link(0),top);

      // The current counter of the representative should be not zero
      sterror(counters[features[top].link(0)]==0,"Invalid counter in accumulation. Counter should not be zero.");

      // Decrease the representatives counter
      counters[features[top].link(0)]--;

      // If this caused the counter to become zero
      if (counters[features[top].link(0)] == 0)
        s.push(features[top].link(0)); // The representative is ready to be processed
    }
  }

  return 1;
}




