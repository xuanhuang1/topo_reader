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
#include <cstdlib>
#include <ctype.h>
#include <vector>
#include <string.h>

using namespace std;

#include "Definitions.h"
#include "TopoTreeInterface.h"
#include "TopoGraphInterface.h"
#include "UnionTree.h"
#include "MergeTree.h"
#include "EnhancedMergeTree.h"
#include "AcceleratedMergeTree.h"
#include "EnhancedSegMergeTree.h"
#include "AcceleratedSegMergeTree.h"
#include "SplitTree.h"
#include "EnhancedSplitTree.h"
#include "AcceleratedSplitTree.h"
#include "EnhancedSegSplitTree.h"
#include "AcceleratedSegSplitTree.h"
#include "SegmentedUnionTree.h"
#include "SegmentedMergeTree.h"
#include "SortedMergeTree.h"
#include "SortedSplitTree.h"
#include "ContourTree.h"
//#include "SegmentedContourTree_TreeMerge.h"
//#include "SegmentedContourTree_FullTree.h"
//#include "ContourTree_TreeMerge.h"
#include "MTSegmentation.h"
#include "SegmentedSplitTree.h"
#include "TreeGather.h"
#include "TreeScatter.h"
#include "STSegmentation.h"
#include "GenericData.h"
#include "GridEdgeParser.h"
#include "PeriodicGridEdgeParser.h"
#include "ImplicitGridParser.h"
#include "GridParser.h"
#include "PeriodicGridParser.h"
#include "ImplicitPeriodicGridParser.h"
#include "SortedGridParser.h"
#include "SMAParser.h"
#include "SMBParser.h"
#include "BinaryParser.h"
#include "DistributedBinaryParser.h"
#include "CompactBinaryParser.h"
#include "CompactDistributedBinaryParser.h"
#include "HDF5GridParser.h"
#include "BlockDecomposition.h"
#include "GraphIO.h"
#include "ArrayIO.h"
#include "FileIO.h"
#include "MultiResGraph.h"
#include "ArcMetrics.h"
#include "AggregatorFactory.h"
#include "FeatureFamily.h"
#include "ManPage.h"
#include "ArraySegmentation.h"
#include "SegmentationHandle.h"
#include "ClanHandle.h"
#include "FeatureSegmentation.h"

using namespace TopologyFileFormat;
using namespace Statistics;

/********************************************************************************** 
 ****************** Primary Typedef Defining the Data Format **********************
 *********************************************************************************/ 
typedef GenericData<FunctionType> ParseType;

//!Number of available input options (size of gOptions)
#define NUM_OPTIONS 34

//!Array with the list of all available input options
static const char* gOptions[NUM_OPTIONS] = {
  
  "--help",
  
  "--i",
  "--input-format",
  "--o",
  "--output-format",
  
  "--graph-type",
  "--low-threshold",
  "--high-threshold",
  "--output-segmentation",
  "--output-extrema-hierarchy",

  "--simplify",
  "--arc-metric",
  "--noise-metric",
  "--noise-threshold",
  "--no-hierarchy",
  "--split-graph",

  "--aggregate",
  "--output-union-hierarchy",
  "--output-aggregate-family",
  "--attribute-names",
  "--nr-of-attributes",
  "--function",
  "--output-feature-family",
  "--test-streaming-aggregation",
  "--streaming-aggregation-threshold",
  "--raw-segmentation",
  "--accumulate-aggregators",
  "--additional-metrics",
  "--dataset-name",
  "--time-index",
  "--time",
  "--legacy-segmentation",
  "--geometry-attributes",
  "--simplex-dimension",
};

/********************************************************************************** 
 ****************** Input and output options **************************************
 *********************************************************************************/ 
//!Inout file stream. Use stdin as default input stream.

//FILE* gInputStream = stdin;
//const char* gInputFileName = NULL;

//!Number of supported input file formates (size of gInputFormatOptions)
#define NUM_INPUT_FORMATS 14
//!List of all available input formats as used in the input options
static const char* gInputFormatOptions[NUM_INPUT_FORMATS] = {
  "interleaveGrid",
  "sma",
  "binary",
  "compact",
  "sortedGrid",
  "grid",
  "hdf5grid",
  "distributed",
  "compact-distributed",
  "implicitGrid",
  "periodicGrid",
  "impPerGrid",
  "smb",
  "periodic"
};
//!Enumeration specifying the available input format. See also gInputFormatOptions.
enum InputFormat {
  IN_RAWGRID     = 0,
  IN_SMA         = 1,
  IN_BINARY      = 2,
  IN_COMPACT     = 3,
  IN_SORTED      = 4,
  IN_GRID        = 5,
  IN_HDF5GRID    = 6,
  IN_DISTRIBUTED = 7,
  IN_COMPDIST    = 8,
  IN_IMPLGRID    = 9,
  IN_PERGRID     = 10,
  IN_IMPPERGRID  = 11,
  IN_SMB         = 12,
  IN_PERIODIC    = 13,
};
//!The used input file format. Default is rawGrid (see gInputFormatOptions) / IN_RAWGRID (see InputFormat) .
InputFormat gInputFormat = IN_GRID;
GlobalIndexType gRawDimensions[3] = {0,0,0};
const char* gCompactIndexFileName = NULL;
FILE* gCompactIndexFile = NULL;
uint32_t gEmbeddingDimension = 0;
uint32_t gFunctionDimension = 0;
std::vector<const char*> gAttributeFileNames;
std::vector<FILE*> gAttributeFiles;
DomainType gDomainType = UNDEFINED_DOMAIN;
std::string gDomainDescription = "Unknown";
uint8_t gPeriod = 0; // In which dimensions is the grid periodic

//!Data stream the output is written to. Default is stdout.
FILE* gOutputStream = stdout;
//!Name of the output file. Default is NULL.
const char* gOutputFileName = NULL;
//!Number of supported output file formats. Length of gOutputFormatOptions.
#define NUM_OUTPUT_FORMATS 3
//!List of all available output formats as used as input options (see also OutputFormat) .
static const char* gOutputFormatOptions[NUM_OUTPUT_FORMATS] = {
  "noOutput",
  "ascii",
  "dot",
};
//!Enumeration of all available output formats (see also gOutputFormatOptions).
enum OutputFormat {
  //!Do not create an output
  OUT_NOOUTPUT = 0,
  //!Output graph(s) in plain ascii format
  OUT_ASCII    = 1,
  //!Output the (simplified) graph in dot format
  OUT_DOT      = 2,
};
//!Define the used output format. Default is no output (OUT_NOOUTPUT)
OutputFormat gOutputFormat = OUT_NOOUTPUT;
//!Z resolution of the dot graph. Default is 50.
int gOutputDotResolution = 50;
  
/********************************************************************************** 
 ****************** Computation related options ***********************************
 *********************************************************************************/ 
//!Number of graph types that can be computed (length of gGraphTypeOptions)
#define NUM_GRAPH_TYPES 11
//!List of all available graph types that can be generated as used as input option.
static const char* gGraphTypeOptions[NUM_GRAPH_TYPES] = {
  "mergeTree",
  "splitTree",
  "contourTree",
  "contourTreeTreeMerge",
  "contourTreeFullTree",
  "enhancedMergeTree",
  "enhancedSplitTree",
  "accMergeTree",
  "accSplitTree",
  "sortedMergeTree",
  "sortedSplitTree",
};
//!Enumeration of all available graph types that can be generated (see also gGraphTypeOptions).
enum GraphType {
  //!Compute the merge tree using the standard algorithm
  MERGE_TREE      = 0,
  //!Compute the split tree using the standard algorithm
  SPLIT_TREE      = 1,
  //!Compute the contour tree (TODO: Not fully implemented)
  CONTOUR_TREE    = 2,
  //!Compute the segmented contour tree using a more traditional approach by merging the merge- and split-tree
  CONTOUR_TREE_TREEMERGE    = 3,
  //!Compute the segmented contour tree using the fully augmented merge/split tree (slow)
  CONTOUR_TREE_FULLTREE = 4,
  //!Compute the merge tree using the enhanced algorithm (reduced search time)
  ENH_MERGE_TREE  = 5,
  //!Compute the split tree using the enhanced algorithm (reduced search time)
  ENH_SPLIT_TREE  = 6,
  //!Compute merge tree using the accelerated algorithm (TEST IMPLEMENTATION)
  ACC_MERGE_TREE  = 7,
  //!Compute merge tree using the accelerated algorithm (TEST IMPLEMENTATION)
  ACC_SPLIT_TREE  = 8,
  //!Compute the merge tree by sorting all input values and then traversing the sorted list.
  SORTED_MERGE    = 9,
  //!Compute the split tree by sorting all input values and then traversing the sorted list.
  SORTED_SPLIT    = 10,
};
//!Define which graph should be comuted using which algorithm. Default is ENH_MERGE_TREE (see GraphType).
GraphType gGraphType = ENH_MERGE_TREE;
//!Define whether a lower bound threshold should be used. All points smaller than gLowThreshold are excluded.
bool gUseLowThreshold = false;
//!The lower bound threshold (see also gUseLowThreshold)
double gLowThreshold = -gMaxValue;
//!Define whether an upper bound threshold should be used. All points larger than gHighThreshold are excluded.
bool gUseHighThreshold = false;
//!The upper bound threshold (see also gUseLowThreshold)
double gHighThreshold = gMaxValue;
//!The "input" tree
TopoTreeInterface* gTree = NULL;
//!The "output" graph
MultiResGraph<> gGraph;

//! Use additional segmentation information
bool gUseSegmentation = false;
//! Given a graph and segmentation write the corresponding one-parameter family to file
bool gTestStreamingAggregation = false;
double gAggregationTestThreshold = -gMaxValue;
const char* gSegmentationFileName = NULL; 
uint8_t gSegmentationBits = 4;
ArraySegmentation<FlexArray::BlockedArray<GlobalIndexType,LocalIndexType>, FlexArray::BlockedArray<FunctionType,LocalIndexType> >* gSegmentation = NULL;
bool gCompactSegmentationIndices = true;
bool gUseLegacySegmentation = false;

const char* gAggregatedFamilyFileName = NULL;
const char* gFeatureFamilyFileName = NULL;
bool gFeatureFamilyEncoding = true; // Whether the family file is written ascii or binary

/********************************************************************************** 
 ****************** Simplification related options ********************************
 *********************************************************************************/ 

bool gSimplifyGraph = false;
double gPersistence = 0;
double gNoiseThreshold = 0;
//! Number of available metrics for the arcs (length of gArcMetricOptions)
#define NUM_ARC_METRICS 10
//! List of all available arc metrics as used as input options.
static const char* gArcMetricOptions[NUM_ARC_METRICS] = {
  "absolutePersistence",
  "relativePersistence",
  "absoluteHighestSaddle",
  "relativeHighestSaddle",
  "absoluteLowestSaddle",
  "relativeLowestSaddle",
  "maximaRelevance",
  "minimaRelevance",
  "localThreshold",
  "logrelPersistence",
};
//! Enumeration of all available arc metrics (see also gArcMetricOptions).
enum ArcMetricType {
  ABSOLUTE_PERSISTENCE    = 0,
  RELATIVE_PERSISTENCE    = 1,
  ABSOLUTE_HIGHEST_SADDLE = 2,
  RELATIVE_HIGHEST_SADDLE = 3,
  ABSOLUTE_LOWEST_SADDLE  = 4,
  RELATIVE_LOWEST_SADDLE  = 5,
  MAXIMA_RELEVANCE        = 6,
  MINIMA_RELEVANCE        = 7,
  LOCAL_THRESHOLD         = 8,
  LOGREL_PERSISTENCE      = 9
};
ArcMetricType gArcMetric = ABSOLUTE_PERSISTENCE;
ArcMetricType gNoiseMetric = ABSOLUTE_PERSISTENCE;
ArcMetric<> *gPrimaryMetric = NULL;
std::vector<ArcMetricType> gAdditionalMetricTypes;
std::vector<ArcMetric<>* > gAdditionalMetrics;
bool gFilterNoise = false;


#define NUM_SPLIT_TYPES 5
static const char* gSplitTypeOptions[NUM_SPLIT_TYPES] = {
  "absolute",
  "relative",
  "logarithmic",
  "log-rel",
  "vertexCount",
};

double gGraphSplitDelta = -1;
SplitType gGraphSplitType = ABSOLUTE_SPLIT;

//! Flag to prevent any hierarchy from being computed
bool gNoHierarchy = false; 

//! Which types of leafs are we cancelling
HierarchyType gHierarchyType = MAXIMA_HIERARCHY;


std::vector<Statistics::Attribute*> gAggregators;
std::vector<std::vector<std::string> > gAggregatorAttributes;
std::vector<std::vector<int32_t> > gAggregatorIndices;
Statistics::Factory gAggFactory;
std::vector<std::string> gAttributeNames;
std::vector<int> gGeometryAttributes;
std::map<std::string, int> gAttributeNameMap;
bool gAccumulateAggregators = false;
uint8_t gPeriodicAggregatorCount = 0;
std::string gDatasetName = "";
uint32_t gTimeIndex = 0;
double gTime;
uint32_t gSimplexDimension = 2;


/*! \brief Open an input file
 *
 *  \param filename : Name of the input file
 *  \param options  : Additional options for the fopen command
 *  \return FILE* the new stream
 */
FILE* openFile(const char* filename,const char* options)
{
  FILE* f = fopen(filename,options);
  if (f == NULL) {
    fprintf(stderr,"Could not open file \"%s\"\n",filename);
    exit(0);
  }

  return f;
}

/*! \brief Constructe a new topological tree
 *
 * \param t     : Type of graph (and algorithm) to be constucted
 * \param graph : Data structure were the output graph is stored.
 * \param use_seg : Should a segmented or regular tree be created.
 * \param segmentation : Array were the segementation is stored.
 * \return The TopoTree.
 */
TopoTreeInterface* constructTree(GraphType t, TopoGraphInterface* graph, bool use_seg)
{
  switch(t) {
  case MERGE_TREE:
    if (use_seg) 
      return new SegmentedMergeTree(graph,gSegmentation);
    else
      return new MergeTree(graph);
    break;
  case SPLIT_TREE:
    if (use_seg) 
      return new SegmentedSplitTree(graph,gSegmentation);
    else
      return new SplitTree(graph);
    break;
  case CONTOUR_TREE:
    return new ContourTree(graph);
    break;
  case CONTOUR_TREE_TREEMERGE:
    /*
    if( use_seg && gSegmentationFileName!=NULL)
      return new SegmentedContourTree_TreeMerge<EnhancedSegUnionVertex<DataType> >(graph , segmentation );
    else
      return new ContourTree_TreeMerge<EnhancedSegUnionVertex<DataType> >(graph);
      */
    fprintf(stderr,"Option deprecated \n");
    exit(0);
    break;
  case CONTOUR_TREE_FULLTREE:
    //return new SegmentedContourTree_FullTree<EnhancedSegUnionVertex<DataType> >(graph , segmentation );
    fprintf(stderr,"Option deprecated \n");
    exit(0);
    break;
  case ENH_MERGE_TREE:
    if (use_seg) 
      return new EnhancedSegMergeTree<>(graph,gSegmentation);
    else
      return new EnhancedMergeTree(graph);
    break;
  case ENH_SPLIT_TREE:
    if (use_seg) 
      return new EnhancedSegSplitTree<>(graph,gSegmentation);
    else
      return new EnhancedSplitTree(graph);
    break;
  case ACC_MERGE_TREE:
    if (use_seg) 
      return new AcceleratedSegMergeTree(graph,gSegmentation);
    else
      return new AcceleratedMergeTree(graph);
    break;
  case ACC_SPLIT_TREE:
    if (use_seg) 
      return new AcceleratedSegSplitTree(graph,gSegmentation);
    else
      return new AcceleratedSplitTree(graph);
    break;
  case SORTED_MERGE:
    if (use_seg) 
      return new SortedMergeTree(graph,gSegmentation);
    else
      return new SortedMergeTree(graph);
    break;
  case SORTED_SPLIT:
    if (use_seg) 
      return new SortedSplitTree(graph,gSegmentation);
    else
      return new SortedSplitTree(graph);
    break;
  }
    
  return NULL;
}

/*! \brief Write a given graph to file
 *
 *  Note: The file format used is defined via gOutputFormat.
 *  \param output : The output stream to which the data should be written
 *  \param graph  : The graph that should be written to file
 */
template <class NodeDataClass>
void outputGraph(FILE* output,MultiResGraph<NodeDataClass>& graph)
{
  switch (gOutputFormat) {
    case OUT_NOOUTPUT:
    break;
    case OUT_ASCII:
        graph.saveASCII(output);
    break;
    case OUT_DOT:
        write_dot<NodeDataClass> (output, graph, gOutputDotResolution,
                                  graph.minF(), graph.maxF(), "shape=ellipse,fontsize=10");
    break;
    default:
    break;
  }
}

/*! \brief Parse the command line input.
 *
 * This function parses the command line input containing the various 
 * execution options and specifies the corresponding global variables used
 * during execution accordingly.
 * Note: The available execution options are defined in gOptions.
 * \param argc : The number of input arguments. (As given to main(...)).
 * \param argv : Array of lengths argc containing all input arguments.
 *               (As given to main(...)).
 * \return int : 0 in case of error and 1 in case of successs
 */
int parse_command_line(int argc, const char** argv)
{
  int i,j,option;

  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < NUM_OPTIONS;j++) {
      if(strcmp(gOptions[j],argv[i])==0)
        option= j;
      else if ((argv[i][0] == '\'') && (strncmp(gOptions[j],argv[i]+1,strlen(gOptions[j]))==0)) {
        option=j;
        fprintf(stderr,"Found \"%s\" to be option %d\n",argv[i],j);
      }
    }
    
    switch (option) {
      
    case -1:  // Wrong input parameter
      fprintf(stderr,"\nError: Wrong input parameter \"%s\"\nTry %s --help\n\n",argv[i],argv[0]);
      return 0;
    case 0:   // --help 
      print_help(stdout,argv[0]);
      return 0;
    case 1:   // --i

      gAttributeFileNames.push_back(argv[++i]);

      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) 
        gAttributeFileNames.push_back(argv[++i]);
      
      break;
    case 2:   // --input-format
      i++;
      for (j=0; j < NUM_INPUT_FORMATS;j++) {
        if(strcmp(gInputFormatOptions[j],argv[i])==0) {
          gInputFormat = (InputFormat)j;
          break;
        }
      }      
      if (j == NUM_INPUT_FORMATS) {
        fprintf(stderr,"Sorry, the input file format \"%s\"is not recognized .....\n",argv[i]);
        return 0;
      }
       
      switch (gInputFormat) {
      case IN_RAWGRID: //rawGrid
      case IN_SORTED: //sortedGrid
      case IN_IMPLGRID: //implicitGrid
      case IN_PERGRID:
      case IN_IMPPERGRID:
      case IN_GRID:
      case IN_PERIODIC:
        gRawDimensions[0] = atoi(argv[++i]);
        gRawDimensions[1] = atoi(argv[++i]);
        gRawDimensions[2] = atoi(argv[++i]);

        if ((gInputFormat == IN_PERGRID) || (gInputFormat == IN_IMPPERGRID) || (gInputFormat == IN_PERIODIC)) {
          gPeriod += atoi(argv[++i]);
          gPeriod += 2 * atoi(argv[++i]);
          gPeriod += 4 * atoi(argv[++i]);
        }

        if (((gInputFormat == IN_GRID) || (gInputFormat == IN_IMPLGRID) || (gInputFormat == IN_PERGRID)
            || (gInputFormat == IN_IMPPERGRID) || (gInputFormat == IN_PERIODIC) )
            && (i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
          gCompactIndexFileName = argv[++i];

        break;
      case IN_COMPACT:
      case IN_COMPDIST:
        if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
          gCompactIndexFileName = argv[++i];
        break;
      case IN_HDF5GRID:
         break;
      default:
        break;
      }
      
      break;
    case 3: // --o
      gOutputFileName = argv[++i];
      break;
    case 4: // --output-format
      i++;
      for (j=0; j < NUM_OUTPUT_FORMATS;j++) {
        if(strcmp(gOutputFormatOptions[j],argv[i])==0) {
          gOutputFormat = (OutputFormat)j;
          break;
        }
      }
      if (j == NUM_OUTPUT_FORMATS) {
        fprintf(stderr,"Sorry, the output file format is not recognized .....\n");
        return 0;
      }
      
      if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        switch (gOutputFormat) {
        case OUT_DOT:
          gOutputDotResolution = atoi(argv[++i]);
          break;
        default:
          break;
        }
      }
      break;
    case 5: // --graph-type
      i++;
      for (j=0; j < NUM_GRAPH_TYPES;j++) {
        if(strcmp(gGraphTypeOptions[j],argv[i])==0) {
          gGraphType = (GraphType)j;
          break;
        }
      }
      if(gGraphType == CONTOUR_TREE_TREEMERGE || gGraphType == CONTOUR_TREE_FULLTREE){
        gUseSegmentation=true;
      }
      if (j == NUM_GRAPH_TYPES) {
        fprintf(stderr,"Sorry, the graph type is not recognized .....\n");
        return 0;
      }
      break;
    case 6: // --low-threshold
      gUseLowThreshold = true;
      gLowThreshold = atof(argv[++i]);
      break;
    case 7: // --high-threshold
      gUseHighThreshold = true;
      gHighThreshold = atof(argv[++i]);
      break;
    case 8: // --output-segmentation
      gUseSegmentation = true;
      gSegmentationFileName = argv[++i];
      if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
        gSegmentationBits = atoi(argv[++i]);
      break;
    case 9: // --output-extrema-hierarchy
      fprintf(stderr,"Sorry, the \"%s\" option is deprecated.",argv[i]);
      return 0;
    case 10: // --simplify
      gSimplifyGraph = true;
      gPersistence = atof(argv[++i]);
      break;
    case 11: // --arc-metric
      i++;
      for (j=0; j < NUM_ARC_METRICS;j++) {
        if(strcmp(gArcMetricOptions[j],argv[i])==0) {
          gArcMetric = (ArcMetricType)j;
          break;
        }
      }
      if (j == NUM_ARC_METRICS) {
        fprintf(stderr,"Sorry, the arc metric is not recognized .....\n");
        return 0;
      }
      break;
    case 12: // --noise-metric
      gFilterNoise = true;
      i++;
      for (j=0; j < NUM_ARC_METRICS;j++) {
        if(strcmp(gArcMetricOptions[j],argv[i])==0) {
          gNoiseMetric = (ArcMetricType)j;
          break;
        }
      }
      if (j == NUM_ARC_METRICS) {
        fprintf(stderr,"Sorry, the noise metric is not recognized .....\n");
        return 0;
      }
      break;
    case 13: // --noise-threshold
      gNoiseThreshold = atof(argv[++i]);
      break;
    case 14: // --no-hierarchy
      gNoHierarchy = true;
      break;
    case 15: // -- split-graph
      gGraphSplitDelta =  atof(argv[++i]);
      if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;
        for (j=0; j < NUM_SPLIT_TYPES;j++) {
          if(strcmp(gSplitTypeOptions[j],argv[i])==0) {
            gGraphSplitType = (SplitType)j;
            break;
          }
        }
        if (j == NUM_SPLIT_TYPES) {
          fprintf(stderr,"Sorry, the graph split type is not recognized .....\n");
          return 0;
        }
      }

      // If we split by vertex count we must keep track of the segmentation
      if (gGraphSplitType == VERTEXCOUNT_SPLIT)
        gUseSegmentation = true;
      
      break;
    case 16: {// --aggregate
      char agg_name[100];
      gUseSegmentation = true;
      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;

        // Try to make an aggregator for the given name
        Attribute* aggregator;

        if (strncmp(argv[i],"periodic",8) == 0) {
          sprintf(agg_name,"%s %s %d %s",argv[i],argv[i+1],gPeriodicAggregatorCount,argv[i+2]);
          gPeriodicAggregatorCount++;
          i += 2;
        }
        else if (strncmp(argv[i],"weighted",8) == 0) {
          sprintf(agg_name,"%s %s",argv[i],argv[i+1]);
          i++;
        }
        else
          strcpy(agg_name,argv[i]);

        aggregator = gAggFactory.make_aggregator(agg_name);

        // If we got a valid aggregator
        if(aggregator != NULL) {

          gAggregators.push_back(aggregator);

          //if (aggregator->numAttributes() > 0) {
          // Create space for the aggregator indices
          gAggregatorIndices.push_back(std::vector<int32_t>(aggregator->numVariables()));

          // For all the necessary attributes
          for (j=0;j<aggregator->numVariables();j++) {

            // Advance to the next argument
            i++;

            // If the first character is a digit we assume we are given an
            // index rather than a name
            if (isdigit(argv[i][0])) {

              gAggregatorIndices.back()[j] = atoi(argv[i]);

              sterror(gAggregatorIndices.back()[j]<0,"Invalid attribute index %d.",gAggregatorIndices.back()[j]);

              // If we were given a name for this attribute
              if (gAggregatorIndices.back()[j] < gAttributeNames.size()) {
                aggregator->variableName(gAttributeNames[gAggregatorIndices.back()[j]],j);
              }
              else {
                char dummy_name[20];
                sprintf(dummy_name,"Attribute %d",gAggregatorIndices.back()[j]);
                aggregator->variableName(std::string("Unknown"),j);
              }
            }
            else { // Otherwise, try to find the attribute name

              aggregator->variableName(string(argv[i]),j);

              std::string name(argv[i]);
              uint8_t k;

              for (k=0;k<gAttributeNames.size();k++) {
                if (name == gAttributeNames[k]) {
                  //gAggregators.back()->attributeIndex(k);
                  gAggregatorIndices.back()[j] = k;
                  fprintf(stderr,"Found aggregation of %s of %s with index %d\n",argv[i-1],argv[i],gAggregatorIndices.back()[j]);
                  break;
                }
              }
              sterror(k==gAttributeNames.size(),"Could not find attribute name \"%s\" in list. Missing --attribute-names ?",name.c_str());
            }
          }
        }
        else {
          fprintf(stderr,"Sorry, the aggregator type %s is not recognized .....\n",argv[i]);
          return 0;
        }
      }
      break;
    }
    case 17:
      fprintf(stderr,"Sorry, the \"%s\" option is deprecated.",argv[i]);
      return 0;
    case 18:
      fprintf(stderr,"The --output-aggregated-family format is deprecated. Do not use.\n");
      exit(0);
      gAggregatedFamilyFileName = argv[++i];
      gUseSegmentation = true;
      break;
    case 19: // --attribute-names
      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;
        gAttributeNames.push_back(std::string(argv[i]));
      }
      if (gInputFormat == IN_IMPLGRID) {
        gAttributeNames.push_back("x-coord");
        gAttributeNames.push_back("y-coord");
        gAttributeNames.push_back("z-coord");
      }


      break;
    case 20: // --nr-of-attributes
      gEmbeddingDimension = atoi(argv[++i]);
      break;
    case 21: // --function
      i++;
      if (isdigit(argv[i][0])) {// If the first character is a number we assume
        // the user gave us an index
        gFunctionDimension = atoi(argv[i]);
      }
      else { // Otherwise, we assume the user provided an attribute name
        std::string name(argv[i]);

        if (gAttributeNames.empty()) {
          fprintf(stderr,"When using a named function the attribute names must come before specifying the funciton\n");
          exit(0);
        }

        for (uint8_t k=0;k<gAttributeNames.size();k++) {
          if (name == gAttributeNames[k])
            gFunctionDimension = k;
        }
      }
      break;
    case 22: // --output-feature-family
      //gUseSegmentation = true;
      gFeatureFamilyFileName = argv[++i];
      if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;
        if (strcmp("ascii",argv[i]) == 0)
          gFeatureFamilyEncoding = true;
        else if (strcmp("binary",argv[i]) == 0)
          gFeatureFamilyEncoding = false;
        else {
          fprintf(stderr,"Sorry: FeatureFamily encoding \"%s\" not recognized. Use either \"ascii\" or \"binary\"\n",argv[i+1]);
          exit(0);
        }
      }
      break;
    case 23: //--test-streaming-aggregation
      gTestStreamingAggregation = true;
      break;
    case 24: //--streaming-aggregation-threshold
      std::cout << "test streaming aggregation = true!" << std::endl;
      gAggregationTestThreshold = atof(argv[++i]);
    case 25: // --raw-segmentation
      gCompactSegmentationIndices = false;
      break;
    case 26: // --accumulate-aggregators
      gAccumulateAggregators = true;
    case 27: // --additional-metrics
      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;
        for (j=0; j < NUM_ARC_METRICS;j++) {
          if(strcmp(gArcMetricOptions[j],argv[i])==0) {
            gAdditionalMetricTypes.push_back((ArcMetricType)j);
            break;
          }
        }
        if (j == NUM_ARC_METRICS) {
          fprintf(stderr,"Sorry, the additional arc metric is not recognized .....\n");
          return 0;
        }
      }
      break;
    case 28: // --dataset-name
      gDatasetName = std::string(argv[++i]);
      break;
    case 29: // --time-index
      gTimeIndex = atoi(argv[++i]);
      break;
    case 30: // --time
      gTime = atof(argv[++i]);
      break;
    case 31: // --legacy-segmentation
      gUseLegacySegmentation = true;
      break;
    case 32: // --geometry-attributes
      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
        i++;
        if (isdigit(argv[i][0])) {// If the first character is a number we assume
          // the user gave us an index
          gGeometryAttributes.push_back(atoi(argv[i]));
        }
        else { // Otherwise, we assume the user provided an attribute name
          std::string name(argv[i]);

          for (uint8_t k=0;k<gAttributeNames.size();k++) {
            if (name == gAttributeNames[k])
              gGeometryAttributes.push_back(k);
          }
        }
      }
      break;
    case 33: // --simplex-dimension
      gSimplexDimension = atoi(argv[++i]);
      break;
    default:
      break;
    }
  }

  // Now we test some non-sensible parameter choices to alert the user to
  // problems before we start computing stuff

  if ((gFeatureFamilyFileName != NULL) && !gCompactSegmentationIndices) {
    fprintf(stderr,"A feature family relies on a compact segmentation index space.\n\
Cannot specify --raw-segmentation while writing feature families\n");
    return 0;
  }

  if ((gGraphSplitType == VERTEXCOUNT_SPLIT) && (gSimplifyGraph)) {
    fprintf(stderr,"Splitting a hierarchy by vertex count relies on an\n\
        unsimplified graph. Cannot specify both --simplify and\n\
        --split-graph vertexCount\n");
    return 0;
  }

  // If it wasn't provided we try to determine a decent data set name
  if (gDatasetName == "") {

    // If we were not given an input file name (meaning we work of
    // stdin) then we use a default name
    if (gAttributeFileNames.empty())
      gDatasetName = "Simulation";
    else { // Otherwise we'll try to construct a useful default
      std::string name = gAttributeFileNames[0];

      int32_t first,last;

      // Find the last appearance of a "/" to cut of the file path
      // For some reason this call can return -1 on some machines
      // which makes the tests below necessary
      first = name.rfind('/');
      if ((first < 0) || (first >= name.size())) // If we didn't find any
        first = 0; // We assume it is only the raw file name
      

      // Now find the last appearance of a "." to split off the extension
      last = name.rfind('.');

      fprintf(stderr,"name: %s   %d %d\n",name.c_str(),first,last);
      if ((last < 0) || (last >= name.size()))  // If we didn't find one
	last = name.size();
      
      gDatasetName.insert(gDatasetName.begin(),name.begin()+first,name.begin()+last);
    }
  }


  return 1;
}

/*! \brief Main function controlling the execution of the program
 *  \param argc : Number of arguments given on the command line
 *  \param argv : Array of length argc containing the command line arguments.
 *  \return int : Exit status
 */
int main(int argc, const char** argv)
{
  //Parse the command line input and define the execution settings
  if (parse_command_line(argc,argv) == 0)
    return EXIT_FAILURE;

  // Determine whether the user specified the number of attributes and/or their
  // names
  if (gEmbeddingDimension == 0) { // If the user has not specified anything
    if (gAttributeNames.size() != 0) { // If we were given names we assume these
                                       // were all attributes
      gEmbeddingDimension = gAttributeNames.size();
    }
    else { // Otherwise we assume the minimal number of attributes which is 1
      gEmbeddingDimension = 1;
      gAttributeNames.push_back("Unkown0");
    }
  }
  else if(gEmbeddingDimension != 0) { // If we have been given an embedding dimension
    if(gAttributeNames.size() == 0) { // But no attribute names
      for(uint8_t i=0; i < gEmbeddingDimension; i++) { // We create default names
        char defaultName[256];
        sprintf(defaultName, "Unkown%d", i);
        gAttributeNames.push_back(defaultName);
      }

      if (gInputFormat == IN_IMPLGRID) {
        gAttributeNames.push_back("x-coord");
        gAttributeNames.push_back("y-coord");
        gAttributeNames.push_back("z-coord");
      }
    }
    sterror(gEmbeddingDimension != gAttributeNames.size(), "Number of attribute names provided does not match the embedding dimension provided");
  } 
    
  // Now we need to compactify the attribute list since we may not need all
  // attributes later and we don't want to store stuff we don't need
  // build the attributeIndexMap
  std::set<uint8_t> used_attributes;
  std::set<uint8_t>::iterator uIt;
  std::vector<std::vector<int32_t> >::iterator sIt;
  std::vector<Attribute*>::iterator it;

  for (it=gAggregators.begin(), sIt = gAggregatorIndices.begin();it!=gAggregators.end();it++,sIt++) {
    for (uint8_t i=0;i<(*it)->numVariables();i++)
      if ((*sIt)[i] >= 0) {
        used_attributes.insert((*sIt)[i]);

      gAttributeNameMap[gAttributeNames[(*sIt)[i]]] = (*sIt)[i];
    }
  }

  for (uint8_t i=0;i<gGeometryAttributes.size();i++)
    used_attributes.insert(gGeometryAttributes[i]);


  if (gUseSegmentation || (gGraphType == SORTED_MERGE) || (gGraphType == SORTED_SPLIT))  {
    used_attributes.insert(gFunctionDimension);
    gAttributeNameMap[gAttributeNames[gFunctionDimension]] = gFunctionDimension;
  }

  // Now collect the vector of persistent attributes and setup the index_map 
  std::vector<uint32_t> persistent_attributes;
  std::map<uint32_t,uint32_t> index_map;
  std::map<uint32_t,uint32_t>::iterator mIt;
  for (uIt=used_attributes.begin();uIt!=used_attributes.end();uIt++) {
    index_map[*uIt] = persistent_attributes.size();
    persistent_attributes.push_back(*uIt);
  }

  // Correct the the attribute indices in the name map
  std::map<std::string, int>::iterator nmIt;
  for (nmIt=gAttributeNameMap.begin();nmIt!=gAttributeNameMap.end();nmIt++) {
    if (nmIt->second >= 0) {
      mIt = index_map.find(nmIt->second);
      sterror(mIt==index_map.end(),"Attribute index not found for aggregator %s.",nmIt->first.c_str());
      nmIt->second = mIt->second;
    }
    std::cout << " Name Map: " << nmIt->first << " = " << nmIt->second << std::endl;
  }

  // COrrect the attribute indices in the geomery attributes
  std::vector<int>::iterator iit;
  for (iit=gGeometryAttributes.begin();iit!=gGeometryAttributes.end();iit++) {
    mIt = index_map.find(*iit);
    sterror(mIt==index_map.end(),"Attribute index not found for geometry attribute %d.",*iit);
    *iit = mIt->second;
  }

  // Finally, use the update the indices
 for (uint8_t i=0;i<gAggregatorIndices.size();i++) {
   for (uint8_t j=0;j<gAggregatorIndices[i].size();j++) {

     mIt = index_map.find(gAggregatorIndices[i][j]);
     if (mIt==index_map.end()) {
       fprintf(stderr,"Could not find attribute index %d in map\n",gAggregatorIndices[i][j]);
       exit(0);
     }

     gAggregatorIndices[i][j] = mIt->second;
   }
 }


   // Finally, use the updated name map to fix the indices
  for (uint8_t i=0;i<gAggregatorAttributes.size();i++) {
    for (uint8_t j=0;j<gAggregatorAttributes[i].size();j++) {

      nmIt = gAttributeNameMap.find(gAggregatorAttributes[i][j]);
      if (nmIt==gAttributeNameMap.end()) {
        fprintf(stderr,"Could not find attribute name \"%s\" in map\n",gAggregatorAttributes[i][j].c_str());
        exit(0);
      }

      gAggregatorIndices[i][j] = nmIt->second;
    }
  }


  // First we setup the correct parser based on the input format
  Parser<ParseType>* parser = NULL;

  // Open all the attribute files
  if (!gAttributeFileNames.empty()) {
    gAttributeFiles.resize(gAttributeFileNames.size(),NULL);
    for (uint8_t i=0;i<gAttributeFileNames.size();i++) {
      gAttributeFiles[i] = openFile(gAttributeFileNames[i],"r");
    }
  }
  else {
    gAttributeFiles.resize(1,stdin);
  }


  // Open the map file for compactification is required
  if (gCompactIndexFileName != NULL)
    gCompactIndexFile = openFile(gCompactIndexFileName,"w");

  switch (gInputFormat) {
    case IN_RAWGRID:
      parser = new GridEdgeParser<ParseType>(gAttributeFiles[0], gRawDimensions[0], gRawDimensions[1], gRawDimensions[2], gEmbeddingDimension, gFunctionDimension, persistent_attributes);
      break;
    case IN_PERIODIC:
      parser = new PeriodicGridEdgeParser<ParseType>(gAttributeFiles[0], gRawDimensions[0], gRawDimensions[1], gRawDimensions[2], gEmbeddingDimension, gFunctionDimension, persistent_attributes, gPeriod);
      break;
    case IN_SMA:
      parser = new SMAParser<ParseType>(gAttributeFiles[0], gEmbeddingDimension, gSimplexDimension + 1, gFunctionDimension, persistent_attributes);
      break;
    case IN_SMB:
      parser = new SMBParser<ParseType>(gAttributeFiles[0], gEmbeddingDimension, gSimplexDimension + 1, gFunctionDimension, persistent_attributes);
      break;
    case IN_BINARY:
      parser = new BinaryParser<ParseType>(gAttributeFiles[0], gEmbeddingDimension, gFunctionDimension, persistent_attributes);
      break;
    case IN_DISTRIBUTED:
      parser = new DistributedBinaryParser<ParseType>(gAttributeFiles[0], gEmbeddingDimension, gFunctionDimension, persistent_attributes);
      break;
    case IN_COMPACT:
      parser = new CompactBinaryParser<ParseType>(gAttributeFiles[0], gCompactIndexFile, gEmbeddingDimension, gFunctionDimension, persistent_attributes);
      break;
    case IN_COMPDIST:
      parser = new CompactDistributedBinaryParser<ParseType>(gAttributeFiles[0], gCompactIndexFile,gEmbeddingDimension, gFunctionDimension, persistent_attributes);
      break;
    case IN_SORTED:

      if ((gGraphType == MERGE_TREE) || (gGraphType == ENH_MERGE_TREE) ||
          (gGraphType == ACC_MERGE_TREE) || (gGraphType == SORTED_MERGE)) {
        parser = new SortedGridParser<ParseType>(gAttributeFiles[0], gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
                                      gLowThreshold, gHighThreshold, gEmbeddingDimension, gFunctionDimension,
                                      persistent_attributes, true, gCompactIndexFile);
      }
      else if ((gGraphType == SPLIT_TREE) || (gGraphType == ENH_SPLIT_TREE) ||
          (gGraphType == ACC_SPLIT_TREE) || (gGraphType == SORTED_SPLIT)) {
        parser = new SortedGridParser<ParseType>(gAttributeFiles[0], gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
                                      gLowThreshold, gHighThreshold, gEmbeddingDimension, gFunctionDimension,
                                      persistent_attributes, false, gCompactIndexFile);
      }
      else {
        sterror(false, "Graph type not applicable to sorted grids");
      }
      break;
    case IN_GRID:
      parser = new GridParser<ParseType>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
                                         gFunctionDimension, persistent_attributes, (gCompactIndexFile != NULL),gCompactIndexFile);

      break;
    case IN_IMPLGRID:
      parser = new ImplicitGridParser<ParseType>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
                                                 gFunctionDimension, persistent_attributes, (gCompactIndexFile != NULL),gCompactIndexFile);

      break;
    case IN_PERGRID:
      parser = new PeriodicGridParser<ParseType>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2], gPeriod,
                                                 gFunctionDimension, persistent_attributes, (gCompactIndexFile != NULL),gCompactIndexFile);

      break;
    case IN_IMPPERGRID:
      parser = new ImplicitPeriodicGridParser<ParseType>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2], gPeriod,
                                                         gFunctionDimension, persistent_attributes, (gCompactIndexFile != NULL),gCompactIndexFile);

      break;
    case IN_HDF5GRID:
      int numGhostZones = 3;
      bool isPeriodic = true;
      parser = new HDF5GridParser<ParseType>( gAttributeFileNames[0] , string("/root/\\distribution.p0") , numGhostZones , isPeriodic);
      HDF5GridParser<ParseType>* temp = static_cast<HDF5GridParser<ParseType>*>( parser );
      vector<bool> periodicDimensions;
      periodicDimensions.resize(temp->getNumDimensions() , false );
      for(int i=0; i< temp->getNumActualDimensions()/2 ; ++i){
        periodicDimensions[periodicDimensions.size()-1-i] = true;
      }
      
      bool ok = temp->setPeriodicDimensions( periodicDimensions );
      if(!ok){
        sterror( 0 , "build_topo_tree: Error while setting the periodic dimensions.");
      }
      break;
  }

  // Now try to automatically determine the domain type
  if (gDomainType == UNDEFINED_DOMAIN) {

    switch (gInputFormat) {
      case IN_BINARY:
      case IN_COMPACT:
        dynamic_cast<BinaryParser<ParseType>*>(parser)->gridSize(gRawDimensions);
      case IN_RAWGRID:
      case IN_GRID:
      case IN_SORTED:
      case IN_HDF5GRID:
      case IN_IMPLGRID:
      case IN_PERGRID:
      case IN_IMPPERGRID:
      case IN_PERIODIC:
        gDomainType = REGULAR_GRID;

        if (gRawDimensions[0]*gRawDimensions[1]*gRawDimensions[2] > 0) {
          char descriptor[100];

          if ((gRawDimensions[1] == 1) && (gRawDimensions[2] == 1))
            sprintf(descriptor,"1 %d",gRawDimensions[0]);
          else if (gRawDimensions[2] == 1)
            sprintf(descriptor,"2 %d %d",gRawDimensions[0],gRawDimensions[1]);
          else
            sprintf(descriptor,"3 %d %d %d",gRawDimensions[0],gRawDimensions[1],
                    gRawDimensions[2]);

          gDomainDescription = std::string(descriptor);
        }
        break;
      case IN_SMA:
      case IN_SMB:
        gDomainType = POINT_SET;
        gDomainDescription = "Point set";
        break;



      default:
        break;
    }
  }



  // Set the minimum and maximum threshold
  parser->fMin(gLowThreshold);
  parser->fMax(gHighThreshold);


  // We need to create the segmentation
  if (gUseSegmentation) {

    // Now that the parser is initialized we must make sure that the function
    // dimension reflects the re-mapped attributes
    mIt = index_map.find(gFunctionDimension);

    switch (gGraphType) {
      case MERGE_TREE:
      case ENH_MERGE_TREE:
      case ACC_MERGE_TREE:
      case SORTED_MERGE:
        gSegmentation = new MTSegmentation(parser->attribute(mIt->second));
        break;
      case SPLIT_TREE:
      case ENH_SPLIT_TREE:
      case ACC_SPLIT_TREE:
      case SORTED_SPLIT:
        gSegmentation = new STSegmentation(parser->attribute(mIt->second));
        break;
      case CONTOUR_TREE:
        fprintf(stderr,"Sorry contour trees are not implemented yet.\n");
        return EXIT_FAILURE;
        break;
      case CONTOUR_TREE_TREEMERGE:
      case CONTOUR_TREE_FULLTREE:
        gSegmentation=NULL;
        break;
    }
  }


  // Second we setup the tree depending on which graph we want to
  // compute and whether we need the segmentation or not 
  gTree = constructTree(gGraphType,&gGraph,gUseSegmentation);

  // If we can ignore certain function values we must set the bounds
  // of the tree
  if (gUseLowThreshold) 
    gTree->setLowerBound(gLowThreshold);
  if (gUseHighThreshold) 
    gTree->setUpperBound(gHighThreshold);
  

  // Fourth, compute the first pass through the data
  FileToken token;
  FileToken last_token;
  uint32_t count = 0;
  uint32_t count2 = 0;

  token = parser->getToken();
  while (token != EMPTY) {

    switch (token) {
    case VERTEX:
      //if ((parser->getId() == 66659) || (parser->getId() % 1000000 == 0))
      //fprintf(stdout,"Introducing vertex %d, %f\n",parser->getId(),parser->getData().f());
      //if (parser->getId() > 13000000)
      gTree->addVertex(parser->getId(),parser->getData().f());
      count++;
      //if (count > 15)
      //  exit(0);
      break;
    case FINALIZE:
      //if (parser->getFinalized() == 86602)
      //fprintf(stdout,"Finalizing vertex %i   current %d\n",parser->getFinalized(),parser->getId());
      gTree->finalizeVertex(parser->getFinalized());
      //gTree->printTree();
      
      count2++;
      break;
    case EDGE:
      //fprintf(stdout,"Adding Edge %d %d \n",parser->getPath()[0],parser->getPath()[1]);
      //if ((parser->getPath()[0] == 86659) || (parser->getPath()[1] == 86659))
      //fprintf(stdout,"Adding Edge %d %d \n",parser->getPath()[0],parser->getPath()[1]);
      //  gTree->printTree();
      //  fprintf(stderr,"break\n");
      //}
      
      gTree->addEdge(parser->getPath()[0],parser->getPath()[1]);
      //gTree->printTree();
      break;
      
    case PATH: {
      //if (last_token != PATH)
      //  fprintf(stderr,"Active element count %d\n",tree.elementCo72unt());
      //const std::vector<GlobalIndexType>& path = parser->getPath();
      //fprintf(stdout,"%d %d %d\n",parser->getPath()[0],parser->getPath()[1],parser->getPath()[2]);
      //if (path[0] == 41)
      //  fprintf(stderr,"break\n");
      std::vector<GlobalIndexType>::const_iterator it;
      
      for (it=parser->getPath().begin();it!=parser->getPath().end()-1;it++) {
        ///if ((*it ==  86656) || (*(it+1) ==  86656))
        //  fprintf(stdout,"Adding Edge %d %d \n",*it,*(it+1));
        gTree->addEdge(*it,*(it+1));
        //gTree->printTree();
      }
      //if ((parser->getPath()[0] ==  86656) || (*it ==  86656))
      //  fprintf(stdout,"Adding Edge %d %d \n",parser->getPath()[0],*it);
      gTree->addEdge(parser->getPath()[0],*it);
      //gTree->printTree();
      break;
    }
    default:
      break;
    }
    
    last_token = token;
    token = parser->getToken();

  }
  
  fprintf(stderr,"Done reading data.\n");
  //exit(0);

  for (uint8_t i=0;i<gAttributeFileNames.size();i++) 
    fclose(gAttributeFiles[i]); 

  // If we saved an index compactification during input we must close
  // that stream as well
  if (gCompactIndexFile != NULL)
    fclose(gCompactIndexFile);

  // In case we used a non-streaming file or for other reasons not all
  // vertices were finalized we finalize them now.
  gTree->cleanup();

  //if we have contour tree then give it the parser
  if( gGraphType == CONTOUR_TREE_TREEMERGE || gGraphType == CONTOUR_TREE_FULLTREE){
    /*
    if( dynamic_cast<SegmentedContourTree_TreeMerge<>*>(gTree) )
      static_cast<SegmentedContourTree_TreeMerge<>*>(gTree)->finishTree(parser->attribute(gFunctionDimension));
    else if( dynamic_cast<ContourTree_TreeMerge<>*>(gTree) )
      static_cast<ContourTree_TreeMerge<>*>(gTree)->finishTree(parser->attribute(gFunctionDimension));
    else
      static_cast<SegmentedContourTree_FullTree<>*>(gTree)->finishTree(parser->attribute(gFunctionDimension));
      */
    fprintf(stderr,"Contour trees via merging is deprecated \n");
    exit(0);
  }

  fprintf(stderr,"Processed %d vertices finalized %d with %d unfinalized\n",count,count2,count-count2);


  /*if( gGraphType == CONTOUR_TREE ){
    if (gOutputFileName != NULL)
        gOutputStream = openFile(gOutputFileName,"w");

     outputGraph<DataType>(gOutputStream,gGraph);
  }*/



  // Before we start thinking about a hierarchy or noise removal we must makes
  // sure that we are using the correct hierarchy type

  switch (gGraphType) {
  case MERGE_TREE:
  case ENH_MERGE_TREE:
  case ACC_MERGE_TREE:
  case SORTED_MERGE:
    gHierarchyType = MAXIMA_HIERARCHY;
    break;
  case SPLIT_TREE:
  case ENH_SPLIT_TREE:
  case ACC_SPLIT_TREE:
  case SORTED_SPLIT:
    gHierarchyType = MINIMA_HIERARCHY;
    break;
  case CONTOUR_TREE:
  case CONTOUR_TREE_TREEMERGE:
  case CONTOUR_TREE_FULLTREE:
    gHierarchyType = MIXED_HIERARCHY;
    break;
    //fprintf(stderr,"Sorry contour trees are not implemented yet.\n");
    //return 0;
  }
  
  // If we want to remove noise from the data
  if (gFilterNoise) {

    ArcMetric<> *metric = NULL;
    switch (gNoiseMetric) {
    case ABSOLUTE_PERSISTENCE:
      metric = new AbsolutePersistence<>(&gGraph);
      break;
    case RELATIVE_PERSISTENCE:
      metric = new RelativePersistence<>(&gGraph);
      break;
    case ABSOLUTE_HIGHEST_SADDLE:
      metric = new HighestSaddleFirst<>(&gGraph);
      break;
    case RELATIVE_HIGHEST_SADDLE:
      metric = new HighestSaddleFirstRelative<>(&gGraph);
      break;
    case ABSOLUTE_LOWEST_SADDLE:
      metric = new LowestSaddleFirst<>(&gGraph);
      break;
    case RELATIVE_LOWEST_SADDLE:
      metric = new LowestSaddleFirstRelative<>(&gGraph);
      break;
    case MAXIMA_RELEVANCE:
      metric = new MaximaRelevance<>(&gGraph);
      break;
    case MINIMA_RELEVANCE:
      metric = new MinimaRelevance<>(&gGraph);
      break;
    case LOCAL_THRESHOLD:
      metric = new LocalThreshold<>(&gGraph);
      break;
    case LOGREL_PERSISTENCE:
      metric = new LogRelativePersistence<>(&gGraph);
      break;
    }
    
    // Construct a full hierarchy
    gGraph.constructHierarchy(*metric,gHierarchyType,gNoiseThreshold,RECORDED);
    
    // We no longer need the metric
    delete metric;
  }
  
  // If we are supposed to refine the graph according to the function value
  if ((gGraphSplitDelta > 0) && (gGraphSplitType != VERTEXCOUNT_SPLIT))
    gGraph.splitGraph(gGraphSplitDelta,gGraphSplitType);

  // Unless we specifically are asked not to create a hierarchy
  if (!gNoHierarchy) {
    // For absolute highest and lowest saddle metrics we need to
    // transform the persistence from a given value to the distance
    // from the extremum used by the metric
    if (gArcMetric == ABSOLUTE_HIGHEST_SADDLE)
      gPersistence = gGraph.maxF() - gPersistence;
    else if (gArcMetric == ABSOLUTE_LOWEST_SADDLE)
      gPersistence = gPersistence - gGraph.minF();

    switch (gArcMetric) {
    case ABSOLUTE_PERSISTENCE:
      gPrimaryMetric = new AbsolutePersistence<>(&gGraph);
      break;
    case RELATIVE_PERSISTENCE:
      gPrimaryMetric = new RelativePersistence<>(&gGraph);
      break;
    case ABSOLUTE_HIGHEST_SADDLE:
      gPrimaryMetric = new HighestSaddleFirst<>(&gGraph);
      break;
    case RELATIVE_HIGHEST_SADDLE:
      gPrimaryMetric = new HighestSaddleFirstRelative<>(&gGraph);
      break;
    case ABSOLUTE_LOWEST_SADDLE:
      gPrimaryMetric = new LowestSaddleFirst<>(&gGraph);
      break;
    case RELATIVE_LOWEST_SADDLE:
      gPrimaryMetric = new LowestSaddleFirstRelative<>(&gGraph);
      break;
    case MAXIMA_RELEVANCE:
      gPrimaryMetric = new MaximaRelevance<>(&gGraph);
      break;
    case MINIMA_RELEVANCE:
      gPrimaryMetric = new MinimaRelevance<>(&gGraph);
      break;
    case LOCAL_THRESHOLD:
      gPrimaryMetric = new LocalThreshold<>(&gGraph);
      break;
    case LOGREL_PERSISTENCE:
      gPrimaryMetric = new LogRelativePersistence<>(&gGraph);
      break;
    }
    
    fprintf(stderr,"Constructing hierarchy\n");

    // Construct a full hierarchy
    gGraph.constructHierarchy(*gPrimaryMetric,gHierarchyType);
    fprintf(stderr,"Done constructing hierarchy\n");

  }

  if (gSimplifyGraph)
    // Adapt the hierarchy to the given threshold
    gGraph.updatePersistence(gPersistence);
    
  
  
  // We may or may not have computed segmentation information which we
  // now want to dump
  if (gUseSegmentation) {

    if (gSegmentation != NULL) {
      fprintf(stderr,"Completing segmentation\n");

      gSegmentation->complete(gGraph);
    }
    else{
      //gPartialSegmentation = static_cast<SegmentedContourTree_TreeMerge<SegmentedUnionVertex<> >*> (gTree)->segmentation();
      fprintf(stderr,"ContourTree via merge deprecated\n");
      exit(0);
    }
  }
  
  if (gUseSegmentation)
    fprintf(stderr,"Completed segmentation\n");
  
  // IF we should split the hierarchy by vertex count
  if ((gGraphSplitDelta > 0) && (gGraphSplitType == VERTEXCOUNT_SPLIT)) {

    switch (gGraphType) {
    case MERGE_TREE:
    case ENH_MERGE_TREE:
    case ACC_MERGE_TREE:
    case SORTED_MERGE:
      gSegmentation->splitByVertices(gGraph,(uint32_t)gGraphSplitDelta,true);
      break;
    case SPLIT_TREE:
    case ENH_SPLIT_TREE:
    case ACC_SPLIT_TREE:
    case SORTED_SPLIT:
      gSegmentation->splitByVertices(gGraph,(uint32_t)gGraphSplitDelta,false);
      break;
    default:
      fprintf(stderr,"Split by vertex count not implemented for this graph type\n");
      break;
    }

    fprintf(stderr,"Clearing hierarchy");
    // Now we must redo the hierarchy since we just added some nodes
    gGraph.clearHierarchy();

    fprintf(stderr,"New Hierarchy");
    gGraph.constructHierarchy(*gPrimaryMetric,gHierarchyType);
    fprintf(stderr,"Done");

  }

  if ((gSegmentationFileName != NULL) && gUseLegacySegmentation) {
    
    if (gCompactSegmentationIndices)
      gSegmentation->compactify(gGraph);


    FILE* seg_stream = openFile(gSegmentationFileName,"w");
   
    gSegmentation->segmentation().dumpBinary(seg_stream);

    fclose(seg_stream);
  }
  else if (gSegmentationFileName != NULL) {

    ClanHandle clan(gSegmentationFileName);
    clan.dataset(gDatasetName);

    FamilyHandle family;
    family.timeIndex(gTimeIndex);
    family.time(gTime);
    family.variableName(gAttributeNames[gFunctionDimension]);

    SegmentationHandle segmentation;
    segmentation.domainType(gDomainType);
    segmentation.domainDescription(gDomainDescription);
    segmentation.encoding(false);

    segmentation.encoding(gFeatureFamilyEncoding); 

    family.add(segmentation);
    clan.add(family);

    //gSegmentation->compactify(gGraph);

    // Note that the segmentation file will use (by construction) un-compactified vertex indices
    // (How else would you represent a sparse grid like AMR)
    gSegmentation->writeSegmentationFile(clan,gGraph,gCompactIndexFileName,gCompactSegmentationIndices);


    // If the user wants to attach some attributes as point coordinates
    if (!gGeometryAttributes.empty()) {

      // Now we re-open the segmentation file
      clan.attach(gSegmentationFileName);

      // Get the segmentation
      segmentation = clan.family(0).segmentation();

      // And read it back in
      FeatureSegmentation seg;
      seg.initialize(segmentation);

      // Figure out the number of coordinates per point
      uint8_t dim = gGeometryAttributes.size();

      // And create an array big enough to hold them all
      const std::vector<Parser<ParseType>::CacheArray*>& attrs = parser->attributes();
      Data<FunctionType> coords(attrs[0]->size()*dim);

      // Create and initialize the bounding box
      std::vector<FunctionType> bbox(2*dim);
      for (uint8_t k=0;k<dim;k++) {
        bbox[2*k] = 1e34;
        bbox[2*k+1] = -1e34;
      }

      // IF we used a sparse index space then the indices in the segmentation
      // cannot be use to extract the geometry. Instead, we must perform the
      // reverse mapping
      std::map<GlobalIndexType,LocalIndexType> index_map;
      if (gCompactIndexFileName != NULL) {
        FILE* input = fopen(gCompactIndexFileName,"r");
        GlobalIndexType* mapping = new GlobalIndexType[gSegmentation->segmentation().size()];
        fread(mapping,sizeof(GlobalIndexType),gSegmentation->segmentation().size(),input);
        fclose(input);

        for (LocalIndexType i=0;i<gSegmentation->segmentation().size();i++)
          index_map[mapping[i]] = i;

      }


      GlobalIndexType count = 0;
      // Now go through the segmentation and copy over all the point coordinates
      for (GlobalIndexType f=0;f<seg.numFeatures();f++) { // For all features
        Segment s = seg.elementSegmentation(f);

        for (uint32_t i=0;i<s.size;i++) { // For all samples of the feature
          for (uint8_t d=0;d<dim;d++) {// For all geometry dimensions

            if (index_map.empty())
              coords[count] = (*attrs[gGeometryAttributes[d]])[s.samples[i]];
            else
              coords[count] = (*attrs[gGeometryAttributes[d]])[index_map.find(s.samples[i])->second];

            bbox[2*d] = std::min(bbox[2*d],coords[count]);
            bbox[2*d+1] = std::max(bbox[2*d+1],coords[count]);

            count++;
          }
        }
      }

      // Finally, create a geometry handle
      GeometryHandle geometry;

      geometry.dimension(dim);
      geometry.encoding(false);

      // Set its data
      geometry.setData(&coords);

      // and its bounding box
      std::stringstream ss(ios_base::out);
      ss << (int)dim << " ";
      for (uint8_t k=0;k<dim;k++)
        ss << bbox[2*k] << " " << bbox[2*k+1] << " ";

      // Update the domain description
      clan.family(0).segmentation().domainType(POINT_SET);
      clan.family(0).segmentation().domainDescription(ss.str());
      clan.family(0).segmentation().append(geometry);
    }
  }
   
 
  // If we are given additional metrics we need to allocate the correct types
  if (!gAdditionalMetricTypes.empty()) {
    std::vector<ArcMetricType>::iterator it;
    for (it=gAdditionalMetricTypes.begin();it!=gAdditionalMetricTypes.end();it++) {
      switch (*it) {
        case ABSOLUTE_PERSISTENCE:
          gAdditionalMetrics.push_back(new AbsolutePersistence<>(&gGraph));
          break;
        case RELATIVE_PERSISTENCE:
          gAdditionalMetrics.push_back(new RelativePersistence<>(&gGraph));
          break;
        case ABSOLUTE_HIGHEST_SADDLE:
          gAdditionalMetrics.push_back(new HighestSaddleFirst<>(&gGraph));
          break;
        case RELATIVE_HIGHEST_SADDLE:
          gAdditionalMetrics.push_back(new HighestSaddleFirstRelative<>(&gGraph));
          break;
        case ABSOLUTE_LOWEST_SADDLE:
          gAdditionalMetrics.push_back(new LowestSaddleFirst<>(&gGraph));
          break;
        case RELATIVE_LOWEST_SADDLE:
          gAdditionalMetrics.push_back(new LowestSaddleFirstRelative<>(&gGraph));
          break;
        case MAXIMA_RELEVANCE:
          gAdditionalMetrics.push_back(new MaximaRelevance<>(&gGraph));
          break;
        case MINIMA_RELEVANCE:
          gAdditionalMetrics.push_back(new MinimaRelevance<>(&gGraph));
          break;
        case LOCAL_THRESHOLD:
          gAdditionalMetrics.push_back(new LocalThreshold<>(&gGraph));
          break;
        case LOGREL_PERSISTENCE:
          gAdditionalMetrics.push_back(new LogRelativePersistence<>(&gGraph));
          break;
      }
    }
  }


  if (gFeatureFamilyFileName != NULL) {    

    ClanHandle clan(gFeatureFamilyFileName);
    clan.dataset(gDatasetName);

    FamilyHandle family;
    family.timeIndex(gTimeIndex);
    family.time(gTime);
    family.variableName(gAttributeNames[gFunctionDimension]);

    clan.add(family);

    //FILE* family_stream = openFile(gFeatureFamilyFileName,"wb");
    //write_feature_family(family_stream,gGraph,*gPartialSegmentation,parser->attributes(),gAggregators,gAttributeNameMap);
    write_feature_family(clan,gGraph,gSegmentation->segmentation(),parser->attributes(),gAggregators,
                         gAggregatorIndices,gAccumulateAggregators,gFeatureFamilyEncoding,
                         gHierarchyType,gAdditionalMetrics);
    
    //fclose(family_stream);
    
  }

  // And we are no done with the parser
  delete parser;
  
  if (gUseSegmentation)
    delete gSegmentation;
  
  delete gTree;
  
  for (uint16_t i=0;i<gAggregators.size();i++) 
    delete gAggregators[i];
    
  
  
  // Finally output the graph
  if (gOutputFormat != OUT_NOOUTPUT) {
    
    if (gOutputFileName != NULL)
      gOutputStream = openFile(gOutputFileName,"w");

    outputGraph<DefaultNodeData>(gOutputStream,gGraph);
    
    if (gOutputFileName != NULL)
      fclose(gOutputStream);
  }      
}

