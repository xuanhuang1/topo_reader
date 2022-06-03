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


#include <cstdlib>
#include <cstdio>
#include <vector>
#include "SubGridParser.h"
#include "ParallelMergeTree.h"
#include "MultiResGraph.h"
#include "GridGather.h"
#include "GraphIO.h"
#include "EnhancedMergeTree.h"
#include "FeatureFamily.h"
#include "ArcMetrics.h"

int main(int argc, const char* argv[])
{
  if (argc < 8) {
    fprintf(stderr,"Usage: %s <filename> <dimx> <dimy> <dimy> <nx> <ny> <nz",argv[0]);
    return 0;
  }

  const char* filename = argv[1];
  uint32_t dim[3];
  uint32_t sub[3];
  uint32_t p_count; // parser count

  dim[0] = atoi(argv[2]);
  dim[1] = atoi(argv[3]);
  dim[2] = atoi(argv[4]);

  sub[0] = atoi(argv[5]);
  sub[1] = atoi(argv[6]);
  sub[2] = atoi(argv[7]);
  p_count = sub[0]*sub[1]*sub[2];

  std::vector<FILE*> attributes(1);
  std::vector<uint32_t> persistence;

  //attributes[0] = fopen(filename,"r");


  // Create an array holding the parsers for each subgrid
  SubGridParser<>* parser[p_count];


  // Populate the parsers
  for (uint32_t k=0;k<sub[2];k++) {
    for (uint32_t j=0;j<sub[1];j++) {
      for (uint32_t i=0;i<sub[0];i++) {
        attributes[0] = fopen(filename,"r");
        parser[(k*sub[1] + j)*sub[0] + i] = new SubGridParser<>(attributes,dim[0],dim[1],dim[2],
            sub[0],sub[1],sub[2],i,j,k,0,persistence,false);
      }
    }
  }

  //parser[0] = new SubGridParser<>(attributes,dim[0],dim[1],dim[2],sub[0],sub[1],sub[2],0,0,0,0,persistence,false);


  MultiResGraph<> merge_graph; // The final merge graph
  EnhancedMergeTree root_tree(&merge_graph); // The compute tree gathering the inputs from the sub pieces
  GridGather collector(&root_tree, // Minimal interface to translate the graph output to tree output
                       0,0,0,
                       dim[0],dim[1],dim[2],
                       dim[0],dim[1],dim[2],
                       sub[0],sub[1],sub[2]);
  //ParallelMergeTree* sub_tree[p_count]; // An array of trees for the sub pieces
  EnhancedMergeTree* sub_tree[p_count]; // An array of trees for the sub pieces


  // Create all sub trees each using the collector as fake output
  for (uint32_t p=0;p<p_count;p++) {
    //sub_tree[p] = new ParallelMergeTree(&collector);
    //sub_tree[p] = new ParallelMergeTree(&merge_graph);
    sub_tree[p] = new EnhancedMergeTree(&collector);
  }

  // For now process the parsers serially
  for (uint32_t p=0;p<p_count;p++) {
  //for (uint32_t p=0;p<2;p++) {

    FileToken token;
    token = parser[p]->getToken();
    while (token != EMPTY) {

      switch (token) {
        case VERTEX:

          if (parser[p]->getId() % 93125 == 0)
            fprintf(stderr,"break\n");

          sub_tree[p]->addVertex(parser[p]->getId(),parser[p]->getData().f());


          break;
        case FINALIZE:
          //if (parser[p]->getRestricted())
          if (parser[p]->getFinalized() == 63812)
            fprintf(stderr,"Finalize %d\n",parser[p]->getFinalized());
          sub_tree[p]->finalizeVertex(parser[p]->getFinalized(),parser[p]->getRestricted());
          break;
        case EDGE:
          //sub_tree[p]->printTree();
          //fprintf(stderr,"Edge  %u %u\n",parser[p]->getPath()[0],parser[p]->getPath()[1]);
          sub_tree[p]->addEdge(parser[p]->getPath()[0],parser[p]->getPath()[1]);
          break;
        case PATH:
          fprintf(stderr,"Deprecated\n");
          exit(0);
          break;
        default:
          break;
      }

      token = parser[p]->getToken();
    }

    // Make sure the sub tree is really finished
    sub_tree[p]->cleanup();

    // This parser is no longer needed
    //delete parser[p];
  }

  root_tree.printTree();
  // No that all sub trees are finished so should be the root tree
  root_tree.cleanup();

  HighestSaddleFirst<> metric(&merge_graph);
  merge_graph.constructHierarchy(metric,MAXIMA_HIERARCHY);

  // For test purposes we write out a dot file with the graph
  FILE* output = fopen("output.dot","w");
  write_dot<>(output, merge_graph, 20, merge_graph.minF(),merge_graph.maxF(), "shape=ellipse,fontsize=10");
  fclose(output);

  write_feature_family("output.family",merge_graph,MAXIMA_HIERARCHY);

  return 1;
}
