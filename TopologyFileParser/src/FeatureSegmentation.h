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

#ifndef FEATURESEGMENTATION_H
#define FEATURESEGMENTATION_H

#include <iostream>
#include <map>
#include "Feature.h"
#include "SegmentationHandle.h"
#include "Segment.h"

namespace TopologyFileFormat
{

//! Class that represents the segmentation corresponding to a feature hierarchy
/*!
 * A FeatureSegmentation store the segmentation wrt. to a regular grid corresponding
 * to a FeatureHierarchy. The interface will return a list of vertex indices for
 * each given feature.
 */
class FeatureSegmentation
{
public:

  //! Default constructor
  FeatureSegmentation();

  //! Destructor
  ~FeatureSegmentation() {}

  //! Initialize the segmentation from (a) file(s)
  /*!
   * Initialize the segmentation from a given file of indices. The segmentation file
   * is assumed to be a binary array of GlobalIndexTypes representing the feature ids
   * of vertices. If a map file is given it is assumed to be another array of indices
   * of the same size where each index represents the global index of the corresponding
   * vertex. If no map file is given the vertex indices are assumed to be given by the
   * order in the file starting at 0.
   * @param seg_file: open stream of the segmentation file
   * @param map_file: open stream of the map file
   * @return 1 if successful; 0 otherwise
   */
  int initialize(std::ifstream* seg_file, std::ifstream* map_file = NULL);

  //! Initialize the segmentation from a segmentation handle
  int initialize(SegmentationHandle& handle, bool build_geometry=true);

  //! Return the bounding box
  void boundingBox(double low[3], double high[3]) const;
  
  //! Determine whether the given feature id has vertices
  bool contains(LocalIndexType id) const;

  //! Return the number of features in the file
  int numFeatures() const {return mFeatureCount;}

  //! Determine whether there exists a non-trivial index map
  bool hasFeatureMap() const {return !mFeatures.empty();}

  bool hasLocations() const {return !mLocations.empty();}

  //! Return a reference to the set of available features if there exists a feature map
  const std::map<GlobalIndexType,LocalIndexType>& featureMap() const {return mFeatures;}

  //! Return a reference to all vertices belonging to an element
  Segment elementSegmentation(GlobalIndexType feature_id) const;

  //! Return the list of vertices part of a feature
  /*!
   * Given a feature this function returns a list of vertices that belong to this feature
   * or any of its constituents. As is described in the FeatureHierarchy only
   * constituents that are reached an uneven number of times are considered. If the given
   * feature does not exist in the segmentation the vertices array will be empty and the
   * function returns 0. Note that there can exist features without vertices in which case
   * the function will return 1 yet the vertices array is still empty.
   * @param feature: Reference of the feature in question
   * @param vertices: A vector of vertices belonging to the feature
   * @return 1 if successful and the feature exist; 0 otherwise.
   */
  int segmentation(const Feature* feature, std::vector<GlobalIndexType>& vertices) const;

  //! Return the list of segments part of a feature
  int segmentation(const Feature* feature, std::vector<Segment>& segments) const;

private:

  //! The set of valid feature ids and their respective indices in the list of segmentations
  std::map<GlobalIndexType,LocalIndexType> mFeatures;

  //! The number of features in this segmentation
  LocalIndexType mFeatureCount;

  //! The array of offsets
  std::vector<LocalIndexType> mOffsets;

  //! The array of samples for all segments
  std::vector<GlobalIndexType> mSegmentation;

  //! The number of dimensions of the coordinates
  uint32_t mDimension;

  //! The bounding box of the features if known
  double mLow[3];

  double mHigh[3];

  //! The array of coordinates for all samples
  std::vector<FunctionType> mCoordinates;

  //! The array of labels for all samples
  std::vector<std::string> mLabels;

  //! The array of locations for all samples
  std::vector<FunctionType> mLocations;
};

} // namespace

#endif /* FEATURESEGMENTATION_H */
