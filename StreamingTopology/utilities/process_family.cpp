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


#include <iostream>
#include <cstring>
#include "ClanHandle.h"
#include "FamilyHandle.h"
#include "StatHandle.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"
#include "AggregatorFactory.h"

using namespace TopologyFileFormat;
using namespace Statistics;



int main(int argc, const char* argv[])
{
  ClanHandle clan;
  FamilyHandle family;
  Feature* f;
  FeatureHierarchy hierarchy;
  FeatureSegmentation segmentation;
  Factory factory;
  std::vector<GlobalIndexType> vertices;
  StatHandle stat;
  std::ifstream seg_stream;
  std::ifstream map_stream;
  uint32_t dimx,dimy,dimz;

  if (argc != 7) {
    fprintf(stderr,"Usage: %s <family-file> <seg-file> <map-file> <dimx> <dimy> dimz>\n",argv[0]);
    return 0;
  }


  // First read the clan
  clan.attach(argv[1]);

  // Open the segmentation and index map file
  seg_stream.open(argv[2],std::ios_base::in);
  if (!seg_stream) {
    fprintf(stderr,"Could not read segmemtation file \"%s\"\n",argv[2]);
    exit(0);
  }

  map_stream.open(argv[3],std::ios_base::in);
  if (!map_stream) {
    fprintf(stderr,"Could not read index map \"%s\"\n",argv[3]);
    exit(0);
  }

  // Read the grid dimensions
  dimx = atoi(argv[4]);
  dimy = atoi(argv[5]);
  dimz = atoi(argv[6]);

  // Get the first family which describes the tree plus statistics
  family = clan.family(0);

  // Read the hierarchy, i.e. initialize the split tree
  hierarchy.initialize(family.simplification(0));

  // Read the segmentation information corresponding to the split tree
  segmentation.initialize(&seg_stream,&map_stream);


  // See whether the family supplies a certain aggregate
  if (!family.providesAggregate(std::string("variance"),std::string("TEMP"))) {
    fprintf(stderr,"This family does not contain variance of temperature as statistic.\n");
    return 0;
  }

  // If it does we create an array to hold all statistic values.q
  // Get the statistics handle to this aggregate
  stat = family.aggregate(std::string("variance"),std::string("TEMP"));

  // Then we create a vector of variances with enough elements
  VarianceArray values(stat.elementCount());

  // And finally actually read the data from disk
  stat.readData(&values);


  // Now we can start working with the family. For example:



  FeatureHierarchy::LivingIterator it;

  // And iterate through all features alive at this threshold
  for (it=hierarchy.beginLiving(-0.5);it!=hierarchy.endLiving();it++) {

    // For a given feature (a node / branch in the tree) we can now find
    // and print is temperature variance
    f = *it;
    fprintf(stderr,"Feature %d contains %llu vertices and has mean temperature %f and temperature variance %f\n",
            f->id(),values[f->id()].vertexCount(),values[f->id()].mean(),values[f->id()].variance());

    // We can also get all the vertices of the mesh that make up this
    // feature
    segmentation.segmentation(f,vertices);

    // To try things out we break here
    //break;
  }

  return 1;
}




