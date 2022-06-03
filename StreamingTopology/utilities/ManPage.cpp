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


#include <cstdio>

#include "ManPage.h"



void print_help(FILE* output,const char* exec)
{
  fprintf(output,"Usage: %s [options]\nWhere options can be any of the following:\n\n",exec);

  print_file_help(output);
  
  print_compute_help(output);

  print_hierarchy_help(output);
}

void print_file_help(FILE* output)
{
  fprintf(output,"--i <filename> ... <filename>\n\
\tFilename(s) of one or multiple input input fields. Some binary parsers (most notably grid)\n\
\tcan read multiple fields from multiple files rather than the standard interleaved format.\n\
\tFor all other formats only the first file is considered\n");
  fprintf(output,"--input-format <format-string> [<map-file>]\tdefault: grid\n\
\tIf a map file is given the index map between global input indices\n\
\tand the local indices is written as flat binary file.\n\
\tinterleavedGrid <dim-x> <dim-y> <dim-z> : binary grid of float with 1,2, or 3 dimensions\n \
\t                                          read vertex by vertex and triangulated/tetrahedralized\n\
\t                                          in the standard fashion.[DEPRECATED use grid instead]\n\
\tgrid <dim-x> <dim-y> <dim-z>    : binary grid of float with 1,2, or 3 dimensions\n \
\t                                  read vertex by vertex and triangulated/tetrahedralized\n\
\t                                  in the standard fashion (changeably in the header).\n\
\timplicitGrid <dx> <dy> <dz>     : implicit grid which attaches the coordinates as the last\n\
\t                                  attributes under the names [x|y|z]-coord\n\
\tperiodicGrid <dx> <dy> <dz> <px> <py> <pz> : periodic grid that connects the grid in the \n\
\t                                             specified dimensions p* = 1 means periodic\n\
\t                                             p* = 0 means not periodic\n\
\timpPerGrid <dx> <dy> <dz> <px> <py> <pz> : an implicit periodic grid\n\
\tsma                             : triangulated surface stored in sma format. Currently\n\
\t                                  reckognized tokens\n \
\t                                  v <float> ... <float>: Vertex with k coordinates\n\
\t                                  f <index> ... <index>: Triangle with 3 vertices \n\
\t                                  e <index> ... <index>: Edge with 2 vertices\n\
\t                                  negative indices are interpreted as finalization\n\
\t                                  token for the last-index vertex read. All positive\n\
\t                                  indices are 1 based\n\
\t                                  x <index>: Finalization of a vertex\n\
\tsmb                             : same as sma but in binary format\n\
\tbinary                          : streaming binary mesh containing tokens \"v\" for\n\
\t                                  verices, \"e\" for edges (two indices), and \"x\" for\n\
\t                                  finalization.\n\
\tdistributed                     : distributed streaming format with binary char tokens\n\
\t                                  \"v <id> <uint8_t> <f0> ... <fn>\" count being the number\n\
\t                                  of expected edges or -1 if unknown. Note that there must be\n\
\t                                  at least one copy of a vertex with a positive count\n\
\t                                  \"e <id0> <id1>\" an edge between the two vertices\n\
\t                                  edges are finalized implicitly after enough edges have been seen\n\
\tcompact <filename>              : binary read that compactifies the input index space and\n\
\tcompact-distributed <filename>  : distributed read that compatcifies the index space\n\
\t                                  stores the corresponding index map in the given file\n\n\
\thdf5grid                        : Binary hdf5 grid file \n\n");

  fprintf(output,"--nr-of-attributes <uint8>\tdefault: 1\n\
\tthe number of attributes given for each vertex in the interleaved formats.\n");

  fprintf(output,"--attribute-names <string> ... <string>\tdefault: \"Unknown\"\n\
\tthe names of all attributes. These names can be used to specify the function\n\
\t(see below) as well as the aggregation attributes. This list should come before\n\
\tits first use. If no --nr-of-attributes is given the number of names given here is used\n\
\t--geometry-attributes <string|index> ... <string|index>\t default:\"Unknown\"\n\
\t                      The attributes that will be written into a potential segmentation as point coordinates\n\n");

  fprintf(output,"--o <filename>\n\
\tFilename to write the output file to. If no filename is given the output will be\n\
\twritten to stdout.\n");
  fprintf(output,"--output-format <format-string>\tdefault noOutput\n\
\tnoOutput           : no output\n\
\tascii              : graph(s) in plain ascii format\n\
\tdot [z-resolution] : outputs the (simplified) graph in dot format. If a resolution\n\
\t                     parameter is given the file will use z-resolution many levels\n\
\t                     to layout the graph. The default resolution is 50.\n");

  fprintf(output,"--dataset-name <string>\n\
\tThe name of the dataset in the output files. If not provided the\n\
\tprefix of the filename is used as default\n");

  fprintf(output,"--time-index <int>\n\
\tAn integer time step usually the index in the sequence of files\n\
\tIf not provided 0 is used as default\n");

  fprintf(output,"--time <double>\n\
\tThe physical time of this piece of data. If not provided the time\n\
\tindex is used as default.\n");

  fprintf(output,"--simplex-dimension <uint32_t>\t default: 2\n\
\tThe dimension of the simplices in the mesh, e.g. for edges it is 1, for triangle mesh it is 2.\n\n");
}

void print_compute_help(FILE* output)
{
  fprintf(output,"--function <string>|<uint8>\t default: 0\n\
\tname or index of the attribute that should be used as function\n");

  fprintf(output,"--graph-type <type-string>\t default enhancedMergeTree\n\
\tmergeTree        : compute the merge tree of the data\n\
\tsplitTree        : compute the split tree of the data\n\
\tcontourTree      : compute the contour tree of the data (not correct yet)\n\
\tcontourTreeTreeMerge : compute the contour tree by merging and the segmented merge and split-tree (not correct yet)\n\
\taccMergeTree     : compute the merge tree using the search accelerated algorithm\n\
\taccSplitTree     : compute the split tree using the search accelerated algorithm\n\
\tenhancedMergeTree: compute the merge tree using the enhanced algorithm (deprecated)\n\
\tenhancedSplitTree: compute the split tree using the enhanced algorithm (deprecated)\n\
");

  fprintf(output,"--low-threshold <threshold>\n\
\tIgnore all input vertices with function value strictly below the given threshold.\n");            
  fprintf(output,"--high-threshold <threshold>\n\
\tIgnore all input vertices with function value strictly above the given threshold.\n");            

  fprintf(output,"--aggregate <type-string> [string|uint8]\t default None\n\
\tvertexCount          : collect the number of vertices per segment\n\
\tmean <attribute>     : collect the mean of the given attribute per segment\n\
\tperiodic mean low high <attribute>n\
\t                     : collect the mean of a periodic value with core range [low,high]\n\
\tweighted mean <attribute> <weight_attribute>\n\
\t                     : collected the weighted mean using the second attribute as weight\n\
\tvariance <attribute> : collect the variance of the given attribute per segment\n\
\tcovariance <attribute> <attribute>\n\
\t                     : covariance of two attributes\n\
\tskewness <attribute> : collect the skewness of the given attribute per segment\n\
\tkurtosis <attribute> : collect the kurtosis of the given attribute per segment\n\
\tmax <attribute>      : collect the maximal attribute value per segment\n\
\tmin <attribute>      : collect the minimal attribute value per segment\n\
\tsum <attribute>      : collect the sum of the attribute values per segment\n");


  fprintf(output,"--output-segmentation <filename> [bits]\t default 4\n\
\tCompute the segmentation corresponding to the graph type that is computed\n\
\tand store the result in binary format into the given filename. Unless the\n\
\t--raw-segmentation flag is used the indices will be compactified wrt. the order\n\
\tof active nodes in the hierarchy, meaning without nodes below the simplification\n\
\tthreshold but with potential virtual nodes that were introduced through splits.\n\
\tIf the number of bits is explicitly given a signed integer of the given size will\n\
\tbe used to store the result. \n\
\tNEW CHANGE: While the default format is binary with 4bits, \n\
\tbased gFeatureFamilyEncoding which is specified in --output-feature-family as [ascii|binary], \n\
\tthe segmentation file format will also be changed. \n");
  fprintf(output,"--raw-segmentation\n\
\tDo not compactify the  indices space when writing a segmentation but instead write\n\
\tthe original mesh indices as segmentation indices. Has no effect if no segmentation\n\
\t is stored.\n");

  fprintf(output,"--output-extrema-hierarchy <filename> [resolution]\t default 100\n\
\tCompute and output the leaf hierarchy including the vertex counts per segment\n\
\tand function range. For merge trees this file is in .mrh format.\n");

  fprintf(output,"--output-union-hierarchy <filename>\n\
\tCompute and output a union-type hierarchy of all branches including the attributes\n\
\tper branch. For merge trees this file is in .union format.\n\
\tThis format is DEPRACATED.");

  fprintf(output,"--output-feature-family <filename> [ascii|binary]\n\
\tCompute the feature family and all its statistical attributes and store\n\
\tthe results in the TopologyFileFormat. Optionally, one can specify to store\n\
\tthe numerical data in ascii or binary\n");


  fprintf(output,"--streaming-aggregation-threshold <float>\n\
\tThe threshold value for comparing equivalence between streaming and non-streaming aggregation \n\
functions.\n");
};

void print_hierarchy_help(FILE* output)
{
  fprintf(output,"--simplify <float>\n\tSimplify the resulting graph by the given threshold.\n");
  fprintf(output,"--arc-metric <metric-string>\n\
\tabsolutePersistence   : simplify the graph by absolute persistence\n\
\trelativePersistence   : simplify the graph by relative persistence\n\
\tabsoluteHighestSaddle : simplify by cancelling the highest saddle first. Unless the relative\n\
\t                        version is used the threshold is interpreted as the lowest function\n\
\t                        at which saddles should still be cancelled.\n\
\trelativeHighestSaddle : simplify by cancelling the highest saddle first.\n\
\tabsoluteLowestSaddle  : simplify by cancelling the lowest saddle first. Unless the relative\n\
\t                        version is used the threshold is interpreted as the highest function\n\
\t                        at which saddles should still be cancelled.\n\
\trelativeLowestSaddle  : simplify by cancelling the lowest saddle first.\n\
\tlocalThreshold        : simplify by local relevance which is distance in function value\n\
\t                      : scaled by the local function range (e.g local max - global min).\n\
\tNote that the persistence metrics are not generally compatible with computing and adapting\n\
\ta segmentation. The resulting segmentations and corresponding statistics may produce unexpected\n\
\tresults are fail all together\n.");

  fprintf(output,"--noise-metric <metric-string>\n\
\tsame options as --arc-metric.\n");

  fprintf(output,"--noise-threshold <float>\n\tPermanently simplify the graph to this threshold\n\
\tusing the noise-metric.\n");

  fprintf(output,"--additional-metrics <metric-string> ... <metric-string>\n\tAdditional metrics used to create extra\n\
\tsimplficiation sequences in the output feature families.\n");

  fprintf(output,"--split-graph <float> [<split-type>] \n\
\tSplit the graph so that no arc is longer (in function value) than the given delta. The split type can be:\n\
\tabsolute   : Arcs are split by absolute function value (default)\n\
\trelative   : Arcs are split by function value relative to the global range (i.e. delta*(global_max - global_min))\n\
\tlogarithmic: Arcs are split by first transforming the function into log-space\n\
\tlog-rel    : Arcs are split in log space but relative to the new function range\n");
}
