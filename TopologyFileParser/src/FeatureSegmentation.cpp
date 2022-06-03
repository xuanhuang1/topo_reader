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

#include <set>
#include <stack>
#include <ios>
#include <cassert>
#include <algorithm>

#include "FeatureSegmentation.h"

namespace TopologyFileFormat {

FeatureSegmentation::FeatureSegmentation() : mFeatureCount(0), mDimension(0)
{
  mLow[0] = mLow[1] = mLow[2] = 0;
  mHigh[0] = mHigh[1] = mHigh[2] = 0;
}

int FeatureSegmentation::initialize(std::ifstream* seg_file, std::ifstream* map_file)
{
  sterror(!seg_file,"Invalid segmentation stream during the initialization.");

  LocalIndexType nr_of_vertices;
  LocalIndexType i;
  std::map<GlobalIndexType,LocalIndexType>::iterator mIt;

  // Figure out how large the file is
  seg_file->seekg (0, std::ios::end);
  nr_of_vertices = seg_file->tellg() / sizeof(LocalIndexType);
  seg_file->seekg (0, std::ios::beg);


  GlobalIndexType* segmentation = new GlobalIndexType[nr_of_vertices];
  seg_file->read((char*)segmentation,nr_of_vertices*sizeof(GlobalIndexType));

  // Given a segmentation and a map file rather than a segmentation handle
  // virtually guarantees that we need a feature map. So we construct one
  // right here. However, initially we misapproritate it to also count the
  // number of samples per feature

  for (i=0;i<nr_of_vertices;i++) {
    mIt = mFeatures.find(segmentation[i]);

    if (mIt == mFeatures.end())
      mFeatures[segmentation[i]] = 1;
    else
      mIt->second += 1;
  }

  // Now we are ready to construct the offset map
  LocalIndexType count = 0;
  mOffsets.resize(mFeatures.size() + 1); // One more to hold size of final segment
  mFeatureCount = mFeatures.size();

  for (i=0,mIt=mFeatures.begin();i<nr_of_vertices;i++,mIt++) {

    mOffsets[i] = count;
    count += mIt->second;
  }
  mOffsets[i] = count;

  // Now we convert the counts in the feature map to ids
  count = 0;
  for (mIt=mFeatures.begin();mIt!=mFeatures.end();mIt++)
    mIt->second = count++;

  // Create an array of counts to keep track of how many samples have been added to a segment
  std::vector<LocalIndexType> counts(mFeatures.size(),0);

  // Allocate enough memory for the segmentations
  mSegmentation.resize(nr_of_vertices);

  GlobalIndexType* mapping = NULL;
  // If we have a valid map file
  if (map_file != NULL) {

    // Allocate memory for it
    mapping = new GlobalIndexType[nr_of_vertices];

    // Read the mapping
    map_file->read((char*)mapping,nr_of_vertices*sizeof(GlobalIndexType));
  }

  // Go through the segmentation once again and sort the vertices by segment
  for (i=0;i<nr_of_vertices;i++) {
    mIt = mFeatures.find(segmentation[i]);

    if (mapping == NULL)
      mSegmentation[mOffsets[mIt->second] + counts[mIt->second]] = i;
    else
      mSegmentation[mOffsets[mIt->second] + counts[mIt->second]] = mapping[i];

    counts[mIt->second]++;
  }

  if (mapping != NULL)
    delete[] mapping;

  delete[] segmentation;

  return 1;
}

int FeatureSegmentation::initialize(SegmentationHandle& handle, bool build_geometry)
{
  handle.readOffsets(mOffsets);
  handle.readSegmentation(mSegmentation);
  mFeatureCount = handle.featureCount();

  build_geometry = true;
  /*
  std::cout << "initializing and offsets = " << std::endl;
  for(int i=0; i < mOffsets.size(); i++)
    std::cout << mOffsets[i] << " ";
  std::cout << std::endl;
  std::cout << "initializing and segmentation = " << std::endl;
  for(int i=0; i < mSegmentation.size(); i++)
    std::cout << mSegmentation[i] << " ";
  std::cout << std::endl;
   */

  IndexHandle index = handle.indexHandle();
  if (index.elementCount() > 0) {
    Data<GlobalIndexType> mapping(index.elementCount());

    index.readData(&mapping);

    for (LocalIndexType i=0;i<mapping.size();i++)
      mFeatures[mapping[i]] = i;
  }

  if (!build_geometry)
    return 1;

  std::stringstream input(handle.domainDescription(),std::ios_base::in);
  
  // Decode the bounding box
  switch (handle.domainType()) {
  case REGULAR_GRID:

    uint32_t dim[4];

    // The first number is the dimension of the grid
    input >> dim[0];

    // For now we assume mLow = {0,0,0}

    if (dim[0] > 0)  {
      input >> dim[1];
      mHigh[0] = std::max((double)dim[1],mHigh[0]);
    }

    if (dim[0] > 1)  {
      input >> dim[2];
      mHigh[1] = std::max((double)dim[2],mHigh[1]);
    }

    if (dim[0] > 2)  {
      input >> dim[3];
      mHigh[2] = std::max((double)dim[3],mHigh[2]);
    }
    break;

  case POINT_SET:
    double v;

    uint32_t num_dim;

    input >> num_dim;

    for (int i=0;i<num_dim;i++) {
      input >> v;
      mLow[i] = std::min(mLow[i],v);
      input >> v;
      mHigh[i] = std::max(mHigh[i],v);
    }
    break;

  default:
    break;
  }

  GeometryHandle geometry = handle.geometryHandle();
  if (geometry.elementCount() > 0) {
    mDimension = geometry.dimension();
    mCoordinates.resize(geometry.elementCount());

    Data<FunctionType> coords(&mCoordinates);
    geometry.readData(&coords);
  }
  else if (handle.domainType() == REGULAR_GRID) {
    std::stringstream input(handle.domainDescription(),std::ios_base::in);

    uint32_t dims[3];

    input >> mDimension;

    sterror((mDimension <= 0) && (mDimension > 3),"Invalid grid dimension %d",mDimension);
    mCoordinates.resize(mDimension*mSegmentation.size());

    switch (mDimension) {
      case 1:
        input >> dims[0];
        //dims[0]--;
        for (LocalIndexType i=0;i<mSegmentation.size();i++)
          mCoordinates[i] = (FunctionType)mSegmentation[i];
        break;
      case 2:
        input >> dims[0];
        input >> dims[1];
        //dims[0]--;
        //dims[1]--;

        for (LocalIndexType i=0;i<mSegmentation.size();i++) {
          mCoordinates[2*i]   = (FunctionType)(mSegmentation[i] % dims[0]);
          mCoordinates[2*i+1] = (FunctionType)((mSegmentation[i] - mCoordinates[2*i]) / dims[0]);

          fprintf(stderr,"%f.0 %f.0\n",mCoordinates[2*i],mCoordinates[2*i+1]);
        }
        break;
      case 3:
        input >> dims[0];
        input >> dims[1];
        input >> dims[2];

        for (LocalIndexType i=0;i<mSegmentation.size();i++) {
          mCoordinates[3*i]   = (FunctionType)(mSegmentation[i] % dims[0]);
          mCoordinates[3*i+1] = (FunctionType)((LocalIndexType)((mSegmentation[i] - mCoordinates[3*i]) / dims[0]) % dims[1]);
          mCoordinates[3*i+2] = (FunctionType)((((mSegmentation[i] - mCoordinates[3*i]) / dims[0]) - mCoordinates[3*i+1]) / dims[1]);
        }
        break;
      default:
        break;
    }
  }

  //read AnnotationHandle if available
  AnnotationHandle annotation = handle.annotationHandle();
  if (annotation.elementCount() > 0) {
    mLabels.resize(annotation.elementCount());
    Data<std::string> labels(&mLabels);
    annotation.readData(&labels);
  }

  //read LocationHandle if available
  LocationHandle location = handle.locationHandle();
  if (location.elementCount() > 0) {
    mLocations.resize(location.elementCount());
    Data<FunctionType> locations(&mLocations);
    location.readData(&locations);
  }

  return 1;
}

void FeatureSegmentation::boundingBox(double low[3], double high[3]) const
{
  memcpy(low,mLow,3*sizeof(double));
  memcpy(high,mHigh,3*sizeof(double));
}



bool FeatureSegmentation::contains(LocalIndexType id) const
{
  // If there is no index map present
  if (mFeatures.empty())
    return (id < mFeatureCount);
  else
    return mFeatures.find(id) != mFeatures.end();
}

Segment FeatureSegmentation::elementSegmentation(GlobalIndexType feature_id) const
{
  Segment seg;

  if (mFeatures.empty()) {

    //std::cout << " feature map not exists: fid = " << feature_id << ", featureCount " << mFeatureCount << std::endl;
    if (feature_id >= mFeatureCount)
      return seg;

    if(mOffsets[feature_id]>=mSegmentation.size())
    {
      //std::cout << "incorrect seg Id " << mOffsets[feature_id] << " " << mSegmentation.size() << std::endl;
      return seg;
    }

    seg.samples = &mSegmentation[mOffsets[feature_id]];
    seg.size = mOffsets[feature_id+1] - mOffsets[feature_id];
    seg.dim = mDimension;

    if (!mCoordinates.empty()) {
      seg.coordinates = &mCoordinates[seg.dim*mOffsets[feature_id]];      
    }

    if (!mLabels.empty()) {
      seg.labels = &mLabels[mOffsets[feature_id]];
    }

    if (!mLocations.empty()) {
      seg.locations = &mLocations[seg.dim*mOffsets[feature_id]];
    }

    //std::cout << "featureMap not exists: " << seg.size << std::endl;

    return seg;
  }
  else {
    std::map<GlobalIndexType,LocalIndexType>::const_iterator mIt;
    //std::cout << " featureMap exists: fid = " << feature_id << ", featureCount " << mFeatureCount << std::endl;

    mIt = mFeatures.find(feature_id);

    if (mIt==mFeatures.end())
      return seg;

    seg.samples = &mSegmentation[mOffsets[mIt->second]];
    seg.size = mOffsets[mIt->second+1] - mOffsets[mIt->second];
    //std::cout << "featureMap exists: " << seg.size << std::endl;

    if (!mCoordinates.empty()) {
      seg.coordinates = &mCoordinates[mDimension*mOffsets[mIt->second]];
      seg.dim = mDimension;
    }

    return seg;
  }
}



int FeatureSegmentation::segmentation(const Feature* feature, std::vector<GlobalIndexType>& vertices) const
{
  std::set<LocalIndexType> feature_ids;
  std::set<LocalIndexType>::iterator sIt;
  std::stack<const Feature*> front; // Breadth first traversal of all constituents
  const Feature* top;
  Segment seg;

  front.push(feature);

  // First we collect the ids of all constituents that are reached an uneven number
  // of times
  while (!front.empty()) {
    top = front.top();
    front.pop();

    // See whether we have seen this feature id before since we want ignore
    // all features that have an even count
    sIt = feature_ids.find(top->id());

    if (sIt != feature_ids.end()) // If we have seen this feature already
      feature_ids.erase(top->id()); // Erase it again
    else
      feature_ids.insert(top->id());

    for (uint32_t i=0;i<top->constituents().size();i++)
      front.push(top->con(i));
  }


  vertices.clear();
  // Now collect the vertices for all features we have see

  //std::cout << "found " << feature_ids.size() << " features" << std::endl;
  for (sIt=feature_ids.begin();sIt!=feature_ids.end();sIt++) {

    seg = elementSegmentation(*sIt);

    LocalIndexType size = vertices.size();
    vertices.resize(size + seg.size);

    memcpy(&vertices[size],seg.samples,sizeof(GlobalIndexType)*seg.size);
  }

  return 1;
}

int FeatureSegmentation::segmentation(const Feature* feature, std::vector<Segment>& segments) const
{
  std::set<LocalIndexType> feature_ids;
  std::set<LocalIndexType>::iterator sIt;
  std::stack<const Feature*> front; // Breadth first traversal of all constituents
  const Feature* top;

  front.push(feature);

  // First we collect the ids of all constituents that are reached an uneven number
  // of times
  while (!front.empty()) {
    top = front.top();
    front.pop();

    // See whether we have seen this feature id before since we want ignore
    // all features that have an even count
    sIt = feature_ids.find(top->id());

    if (sIt != feature_ids.end()) // If we have seen this feature already
      feature_ids.erase(top->id()); // Erase it again
    else
      feature_ids.insert(top->id());

    for (uint32_t i=0;i<top->constituents().size();i++)
      front.push(top->con(i));
  }


  segments.clear();
  // Now collect the segments for all features we have see

  //std::cout << "found " << feature_ids.size() << " features" << std::endl;
  for (sIt=feature_ids.begin();sIt!=feature_ids.end();sIt++) {

    segments.push_back(elementSegmentation(*sIt));
  }

  return 1;
}

}

