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

#include "Slopes.h"
#include "GradientComplex.h"
#include "ThreadedGradientComplex.h"
#include "MCAssembly.h"
#include "GridEdgeParser.h"
#include "ImplicitGridEdgeParser.h"
#include "ImplicitPMS2DParser.h"
#include "SMAParser.h"
#include "SMBParser.h"
#include "BinaryParser.h"
#include "CompactBinaryParser.h"
#include "MCVertex.h"
#include "MSCVertex.h"
#include "MCAssembly.h"
#include "MSCAssembly.h"
#include "AnalyticFunctions.h"
#include "Grids.h"
#include "EdgeConstructor.h"
#include "SlopeComplex.h"
#include "StableComplexAssembly.h"
#include "UnstableComplexAssembly.h"


//#ifndef DIMENSION
//#define DIMENSION 2
//#endif

typedef float DataType;

//! Number of coordinates of each vertex
static const uint32_t gEmbeddingDimension = DIMENSION+1;

typedef EmbeddedVertex<gEmbeddingDimension,DataType> BaseVertex;


#define NUM_OPTIONS 23

static const char* gOptions[NUM_OPTIONS] = {
  
  "--help",
  
  "--i",
  "--input-format",
  "--o",
  "--output-format",
  
  "--function",
  "--complex-type",
  "--threads",
  "--grid-type",
  "--low",
  "--high",
  "--edge-type",
  "--neighborhood",
  "--simplify",
  "--output-stat",
  "--no-hierarchy",
  "--max-stat",
  "--output-stable-ct",
  "--output-unstable-ct",
  "--output-stable-complex",
  "--output-unstable-complex",
  "--output-stable-label-hierarchy",
  "--output-unstable-label-hierarchy"
};



/********************************************************************************** 
 ****************** Input and output options **************************************
 *********************************************************************************/ 

FILE* gInputStream = stdin;
std::vector<FILE*> gAttributeFiles(1);
const char* gInputFileName = NULL;
#define NUM_INPUT_FORMATS 8
static const char* gInputFormatOptions[NUM_INPUT_FORMATS] = {
  "interleavedGrid",
  "implicitGrid",
  "sma",
  "binary",
  "compact",
  "smb",
  "implicitPMS2D",
  "grid",
};
enum InputFormat {
  IN_RAWGRID  = 0,
  IN_IMPLICIT = 1,
  IN_SMA      = 2,
  IN_BINARY   = 3,
  IN_COMPACT  = 4,
  IN_SMB      = 5,
  IN_IMPPMS2D = 6,
  IN_GRID     = 7,
};
InputFormat gInputFormat = IN_RAWGRID;
GlobalIndexType gRawDimensions[3] = {0,0,0};
const char* gCompactIndexFileName;
FILE* gCompactIndexFile = NULL;
uint32_t gFunctionDimension = gEmbeddingDimension-1;


FILE* gOutputStream = stdout;
const char* gOutputFileName = NULL;
#define NUM_OUTPUT_FORMATS 3
static const char* gOutputFormatOptions[NUM_OUTPUT_FORMATS] = {
  "noOutput",
  "ascii",
  "binary",
};
enum OutputFormat {
  OUT_NOOUTPUT = 0,
  OUT_ASCII    = 1, 
  OUT_BINARY   = 2,
};
OutputFormat gOutputFormat = OUT_NOOUTPUT;
const char* gStatisticsFileName = NULL;
const char* gStableCTFileName = NULL;
const char* gUnstableCTFileName = NULL;
const char* gStableComplexFileName = NULL;
const char* gUnstableComplexFileName = NULL;
const char* gStableLabelFileName = NULL;
const char* gUnstableLabelFileName = NULL;
bool gMaximaStat = false;

#define NUM_COMPLEX_TYPES 2
static const char* gComplexTypeOptions[NUM_COMPLEX_TYPES] = {
  "morse",
  "morse-smale",
};
enum ComplexType {
  MORSE_COMPLEX = 0,
  MORSE_SMALE_COMPLEX = 1,
};
ComplexType gComplexType = MORSE_COMPLEX;  
bool gBuildHierarchy = true;


#define NUM_SLOPE_TYPES 1
static const char* gSlopeTypeOptions[NUM_SLOPE_TYPES] = {
  "euclidian",
};
enum SlopeType {
  SLOPE_EUCLIDIAN = 0,
};
SlopeType gSlopeType = SLOPE_EUCLIDIAN;
Slope<gEmbeddingDimension,DataType>* gSlope = new EuclidianGradient<gEmbeddingDimension,DataType>;

#define NUM_GRID_TYPES 2
static const char* gGridTypeOptions[NUM_GRID_TYPES] = {
  "noGrid",
  "uniform",
};
enum GridType {
  GRID_EXPLICIT = 0,
  GRID_UNIFORM = 1,
};
GridType gGridType = GRID_EXPLICIT;

#define NUM_EDGE_TYPES 2
static const char* gEdgeTypeOptions[NUM_GRID_TYPES] = {
  "complete",
  "disjunct",
};
EdgeType gEdgeType = EDGE_COMPLETE;

// How many "levels" of neighbor do we create for implicit grids
uint16_t gNeighborhoodDegree = 1;

// The number of threads to use for computation
uint16_t gThreads = 1;

#define NUM_FUNCTION_TYPES 1
static const char* gFunctionTypeOptions[NUM_FUNCTION_TYPES] = {
  "friedman",
};
AnalyticFunction<gEmbeddingDimension,DataType>* gAnalyticFunction = analyticFunction<gEmbeddingDimension,DataType>(FRIEDMAN1);

GradientComplex<MCVertex<gEmbeddingDimension,DataType> >* gMorseComplex = NULL;
std::vector<AssemblyInterface*> gMCAssembly;

SlopeComplex<MSCVertex<gEmbeddingDimension,DataType> >* gMorseSmaleComplex = NULL;
std::vector<AssemblyInterface*> gMSCAssembly;

float gSimplify = -1;

//! The number of samples used for implicit grids
std::vector<uint32_t> gSamples(gEmbeddingDimension-1,1);

//! The lower left corner of the sampling range of implicit grids
std::vector<DataType> gLowRange(gEmbeddingDimension-1,0);

//! The right upper corner of the sampling range of implicit grids
std::vector<DataType> gHighRange(gEmbeddingDimension-1,1);

FILE* openFile(const char* filename,const char* options)
{
  FILE* f = fopen(filename,options);
  if (f == NULL) {
    fprintf(stderr,"Could not open file \"%s\"\n",filename);
    exit(0);
  }

  return f;
}

int parse_command_line(int argc, const char** argv)
{
  int i,option;
  uint32_t j;

  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < NUM_OPTIONS;j++) {
      if(strcmp(gOptions[j],argv[i])==0)
        option= j;
    }
    
    switch (option) {
      
    case -1:  // Wrong input parameter
      fprintf(stderr,"\nError: Wrong input parameter \"%s\"\nTry %s --help\n\n",argv[i],argv[0]);
      return 0;
    case 0:   // --help 
      //print_help(stdout,argv[0]);
      fprintf(stderr,"No help available yet.\n");
      return 0;
    case 1:   // --i
      gInputFileName = argv[++i];
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
      case IN_IMPLICIT: //implicit raw grid
      case IN_GRID:
        gRawDimensions[0] = atoi(argv[++i]);
        gRawDimensions[1] = atoi(argv[++i]);
        gRawDimensions[2] = atoi(argv[++i]);

        if ((gInputFormat == IN_GRID) && (i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
          gCompactIndexFileName = argv[++i];

        break;
      case IN_COMPACT:
        if ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
          gCompactIndexFileName = argv[++i];
        
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
        default:
          break;
        }
      }
      break;
    case 5: // --function
      gFunctionDimension = atoi(argv[++i]);
      BaseVertex::functionDimension(gFunctionDimension);

      if (gFunctionDimension >= gEmbeddingDimension) {
        fprintf(stderr,"Function dimension %d too high. Not enough function values in %d-dimensional data.",
                gFunctionDimension,gEmbeddingDimension);
        exit(0);
      }
        
      break;
    case 6:
      i++;
      for (j=0; j < NUM_COMPLEX_TYPES;j++) {
        if(strcmp(gComplexTypeOptions[j],argv[i])==0) {
          gComplexType = (ComplexType)j;
          break;
        }
      }
      if (j == NUM_COMPLEX_TYPES) {
        fprintf(stderr,"Sorry, the complex type is not recognized .....\n");
        return 0;
      }
      break;
    case 7: // --threads
      gThreads = atoi(argv[++i]);
      break;
    case 8: // --grid-type
      i++;
      for (j=0; j < NUM_GRID_TYPES;j++) {
        if(strcmp(gGridTypeOptions[j],argv[i])==0) {
          gGridType = (GridType)j;
          break;
        }
      }
      if (j == NUM_GRID_TYPES) {
        fprintf(stderr,"Sorry, the grid type is not recognized .....\n");
        return 0;
      }

      switch (gGridType) {

      case GRID_EXPLICIT:
        break;
      case GRID_UNIFORM:
        for (j=0;j<gEmbeddingDimension-1;j++)
          gSamples[j] = atoi(argv[++i]);
        break;
      }
      break;
    case 9: // --low
      for (j=0;j<gEmbeddingDimension-1;j++)
        gLowRange[j] = atof(argv[++i]);
      break;
    case 10: // --high
      for (j=0;j<gEmbeddingDimension-1;j++)
        gHighRange[j] = atof(argv[++i]);
      break;
      
    case 11: // --edge-type
      i++;
      for (j=0; j < NUM_EDGE_TYPES;j++) {
        if(strcmp(gEdgeTypeOptions[j],argv[i])==0) {
          gEdgeType = (EdgeType)j;
          break;
        }
      }
      if (j == NUM_EDGE_TYPES) {
        fprintf(stderr,"Sorry, the edge type is not recognized .....\n");
        return 0;
      }
      break;
    case 12: // --neighborhood
      gNeighborhoodDegree = atoi(argv[++i]);
      break;
    case 13: // --simplify
      gSimplify = atof(argv[++i]);
      break;
    case 14: // --output-stat
      gStatisticsFileName = argv[++i];
      break;
    case 15: // --no-hierarchy
      gBuildHierarchy = false;
      break;
    case 16: // --max-stat
      gMaximaStat = true;
      break;
    case 17: // --output-stable-ct
      gStableCTFileName = argv[++i];
      break;
    case 18: // --output-unstable-ct
      gUnstableCTFileName = argv[++i];
      break;
    case 19: // --output-stable-complex
      gStableComplexFileName = argv[++i];
      break;
    case 20: // --output-unstable-complex
      gUnstableComplexFileName = argv[++i];
      break;
    case 21: // --output-stable-label-hierarchy
      gStableLabelFileName = argv[++i];
      break;
    case 22: // --output-unstable-label-hierarchy
      gUnstableLabelFileName = argv[++i];
      break;
   default:
      break;
    } // end-switch
  } //end-for

  return 1;
}

int main(int argc, const char** argv)
{
  if (parse_command_line(argc,argv) == 0)
    return 0;

  // Create the complex we need
  switch (gComplexType) {
  case MORSE_COMPLEX:
    gMorseComplex = new GradientComplex<MCVertex<gEmbeddingDimension,DataType> >(*gSlope);
    gMCAssembly.resize(gThreads);
    for (uint16_t i=0;i<gThreads;i++) 
      gMCAssembly[i] = new MCAssembly<MCVertex<gEmbeddingDimension,DataType> >(*gMorseComplex);
    break;
  case MORSE_SMALE_COMPLEX:
    gMorseSmaleComplex = new SlopeComplex<MSCVertex<gEmbeddingDimension,DataType> >(*gSlope);
    gMSCAssembly.resize(gThreads);
    for (uint16_t i=0;i<gThreads;i++) 
      gMSCAssembly[i] = new MSCAssembly<MSCVertex<gEmbeddingDimension,DataType> >(*gMorseSmaleComplex);
    break;
  }
  



  // If we are given an input file
  if (gInputFileName != NULL) {
    gInputStream = openFile(gInputFileName,"r");

    gAttributeFiles[0] = gInputStream;

    // First we setup the correct parser based on the input format
    Parser<BaseVertex>* parser = NULL;

    switch (gInputFormat) {
    case IN_RAWGRID:
      parser = new GridEdgeParser<BaseVertex>(gInputStream,gRawDimensions[0],gRawDimensions[1],gRawDimensions[2],gEmbeddingDimension,gFunctionDimension);
      break;
    case IN_IMPLICIT:
      parser = new ImplicitGridEdgeParser<BaseVertex>(gAttributeFiles,gRawDimensions[0],gRawDimensions[1],gRawDimensions[2],gFunctionDimension);
      break;
    case IN_SMA:
      parser = new SMAParser<BaseVertex>(gInputStream,gEmbeddingDimension,3,gFunctionDimension);
      break;
    case IN_SMB:
      parser = new SMBParser<BaseVertex>(gInputStream,gEmbeddingDimension,2,gFunctionDimension);
      break;
    case IN_BINARY:
      parser = new BinaryParser<BaseVertex>(gInputStream,gEmbeddingDimension,gFunctionDimension);
      break;
    case IN_COMPACT: 
      if (gCompactIndexFileName != NULL)
        gCompactIndexFile = openFile(gCompactIndexFileName,"w");
      
      parser = new CompactBinaryParser<BaseVertex>(gInputStream,gCompactIndexFile,gEmbeddingDimension,gFunctionDimension);
      break;
    case IN_IMPPMS2D:
      parser = new ImplicitPMS2DParser<BaseVertex>(gAttributeFiles,gRawDimensions[0],gRawDimensions[1],gFunctionDimension);
      break;
    case IN_GRID:
      parser = new GridParser<BaseVertex>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
          gFunctionDimension);
      break;

     }


    // Fourth, compute the first pass through the data
    FileToken token;
    FileToken last_token;
    
    token = parser->getToken();
    while (token != EMPTY) {
      
      switch (token) {
      case VERTEX:
        //if ((parser->getId() != gMorseComplex->size()) || (parser->getId() % 1000000 == 0))
        //if (parser->getId() % 10000 == 0)
        //fprintf(stdout,"Introducing vertex %d,  %f %f %f %f\n",parser->getId(),parser->getData()[0],parser->getData()[1],parser->getData()[2],parser->getData()[3]);
        switch (gComplexType) {
        case MORSE_COMPLEX:
          gMorseComplex->addVertex(parser->getId(),parser->getData());
          break;
        case MORSE_SMALE_COMPLEX:
          gMorseSmaleComplex->addVertex(parser->getId(),parser->getData());
          break;
        }
        break;
      case FINALIZE:
        break;
      case EDGE:
        //if ((parser->getPath()[0] == 18) || (parser->getPath()[1] == 18)) {
        //fprintf(stderr,"Adding edge %d %d   steepest \n",parser->getPath()[0],parser->getPath()[1]);//,
        //gMorseComplex->vertex(18).stableInfo().steepest());
        //}
        
        switch (gComplexType) {
        case MORSE_COMPLEX:
          gMCAssembly[0]->add_edge(parser->getPath()[0],parser->getPath()[1]);
          break;
        case MORSE_SMALE_COMPLEX:
          gMSCAssembly[0]->add_edge(parser->getPath()[0],parser->getPath()[1]);
          break;
        }
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
          //fprintf(stdout,"Adding Edge %d %d \n",*it,*(it+1));
          switch (gComplexType) {
          case MORSE_COMPLEX:
            gMCAssembly[0]->add_edge(*it,*(it+1));
            break;
          case MORSE_SMALE_COMPLEX:
            gMSCAssembly[0]->add_edge(*it,*(it+1));
            break;
          }
          //gTree->printTree();
        }
        //fprintf(stdout,"Adding Edge %d %d \n",parser->getPath()[0],*it);
        switch (gComplexType) {
        case MORSE_COMPLEX:
          gMCAssembly[0]->add_edge(parser->getPath()[0],*it);
          break;
        case MORSE_SMALE_COMPLEX:
          gMSCAssembly[0]->add_edge(parser->getPath()[0],*it);
          break;
        }
        //gTree->printTree();
        break;
      }
      default:
        //sterror(true,"Token not recognized.");
        break;
      }
      
      last_token = token;
      token = parser->getToken();
    }
    
    //fprintf(stderr,"Finished read-in ! Read %d vertices\n",gMorseSmaleComplex->size());
    
    // If we read our data from file we must close the stream
    if (gInputFileName != NULL) 
      fclose(gInputStream);
  }
  else if (gGridType != GRID_EXPLICIT) {// If instead of reading a grid we were supposed to create one implicitly

    EdgeConstructor edge_maker;

    switch (gGridType) {

    case GRID_EXPLICIT:
      break;
    case GRID_UNIFORM: {
      switch (gComplexType) {
      case MORSE_COMPLEX:
        createUniformGrid<MCVertex<gEmbeddingDimension,DataType> >(gMorseComplex,gAnalyticFunction,gSamples,gLowRange,gHighRange);
        edge_maker.createEdges(gEdgeType,gNeighborhoodDegree,gSamples,gMCAssembly);
        break;
      case MORSE_SMALE_COMPLEX:
        createUniformGrid<MSCVertex<gEmbeddingDimension,DataType> >(gMorseSmaleComplex,gAnalyticFunction,gSamples,gLowRange,gHighRange);
        edge_maker.createEdges(gEdgeType,gNeighborhoodDegree,gSamples,gMSCAssembly);
        break;
      }
      break;
    }
    }

    
 
  }


  // Now finalize the complex we need
  switch (gComplexType) {
  case MORSE_COMPLEX:
    gMorseComplex->computeStableSegmentation();
    break;
  case MORSE_SMALE_COMPLEX:
    gMorseSmaleComplex->computeStableSegmentation();
    gMorseSmaleComplex->computeUnstableSegmentation();
    break;
  }

  if (gBuildHierarchy) {
    
    AssemblyInterface* stable = NULL;
    AssemblyInterface* unstable = NULL;

    switch (gComplexType) {
    case MORSE_COMPLEX:
      stable = new StableComplexAssembly<MCVertex<gEmbeddingDimension,DataType> >(*gMorseComplex);
      break;
    case MORSE_SMALE_COMPLEX:
      stable = new StableComplexAssembly<MSCVertex<gEmbeddingDimension,DataType> >(*gMorseSmaleComplex);
      unstable = new UnstableComplexAssembly<MSCVertex<gEmbeddingDimension,DataType> >(*gMorseSmaleComplex);
      break;
    }
   

    // If we are given an input file
    if (gInputFileName != NULL) {
      gInputStream = openFile(gInputFileName,"r");
      
      // Needed to satisfy the new implicit parser
      gAttributeFiles[0] = gInputStream;

      // First we setup the correct parser based on the input format
      Parser<BaseVertex>* parser = NULL;
      
      switch (gInputFormat) {
      case IN_RAWGRID:
        parser = new GridEdgeParser<BaseVertex>(gInputStream,gRawDimensions[0],gRawDimensions[1],gRawDimensions[2],gEmbeddingDimension,gFunctionDimension);
        break;
      case IN_IMPLICIT:
        parser = new ImplicitGridEdgeParser<BaseVertex>(gAttributeFiles,gRawDimensions[0],gRawDimensions[1],gRawDimensions[2],gFunctionDimension);
        break;
      case IN_SMA:
        parser = new SMAParser<BaseVertex>(gInputStream,gEmbeddingDimension,3,gFunctionDimension);
        break;
      case IN_SMB:
        parser = new SMBParser<BaseVertex>(gInputStream,gEmbeddingDimension,2,gFunctionDimension);
        break;
      case IN_BINARY:
        parser = new BinaryParser<BaseVertex>(gInputStream,gEmbeddingDimension,gFunctionDimension);
        break;
      case IN_COMPACT: 
        if (gCompactIndexFileName != NULL)
          gCompactIndexFile = openFile(gCompactIndexFileName,"w");
      
        parser = new CompactBinaryParser<BaseVertex>(gInputStream,gCompactIndexFile,gEmbeddingDimension,gFunctionDimension);
        break;
      case IN_IMPPMS2D:
        parser = new ImplicitPMS2DParser<BaseVertex>(gAttributeFiles,gRawDimensions[0],gRawDimensions[1],gFunctionDimension);
        break;
      case IN_GRID:
        parser = new GridParser<BaseVertex>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
            gFunctionDimension);
        break;

      }


      // Fourth, compute the first pass through the data
      FileToken token;
    
      token = parser->getToken();
      while (token != EMPTY) {
      
        switch (token) {
        case VERTEX:
         break;
        case FINALIZE:
          break;
        case EDGE:
          //if ((parser->getPath()[0] == 18) || (parser->getPath()[1] == 18)) {
          //  fprintf(stderr,"Adding edge %d %d   steepest  \n",parser->getPath()[0],parser->getPath()[1]);//,
          //gMorseComplex->vertex(18).stableInfo().steepest());
          //}
        
          stable->add_edge(parser->getPath()[0],parser->getPath()[1]);
          if (unstable != NULL)
            unstable->add_edge(parser->getPath()[0],parser->getPath()[1]);
         break;
        
        case PATH: {
          std::vector<GlobalIndexType>::const_iterator it;

          for (it=parser->getPath().begin();it!=parser->getPath().end()-1;it++) {
            stable->add_edge(*it,*(it+1));
            if (unstable != NULL)
              unstable->add_edge(*it,*(it+1));
          }

          break;
        }
        default:
          break;
        }
        token = parser->getToken();
      }
    
      fprintf(stderr,"Finished processing edges for the second time\n");
    
      // If we read our data from file we must close the stream
      if (gInputFileName != NULL) 
        fclose(gInputStream);
    }
    else if (gGridType != GRID_EXPLICIT) {// If instead of reading a grid we were supposed to create one implicitly

      EdgeConstructor edge_maker;
      std::vector<AssemblyInterface*> s(1,stable);
      std::vector<AssemblyInterface*> u(1,unstable);
      

      switch (gGridType) {

      case GRID_EXPLICIT:
        break;
      case GRID_UNIFORM: {
        switch (gComplexType) {
        case MORSE_COMPLEX:
          edge_maker.createEdges(gEdgeType,gNeighborhoodDegree,gSamples,s);
          break;
        case MORSE_SMALE_COMPLEX:
          edge_maker.createEdges(gEdgeType,gNeighborhoodDegree,gSamples,s);
          edge_maker.createEdges(gEdgeType,gNeighborhoodDegree,gSamples,u);
          break;
        }
        break;
      }
      }
    }

    switch (gComplexType) {
    case MORSE_COMPLEX:
      fprintf(stderr,"Stable size %p  %p\n",dynamic_cast<ComplexAssembly<MCVertex<gEmbeddingDimension,DataType> >*>(stable),stable);
      gMorseComplex->createStableComplex(static_cast<ComplexAssembly<MCVertex<gEmbeddingDimension,DataType> >*>(stable));
      gMorseComplex->simplifyStableComplex(gSimplify);
      break;
    case MORSE_SMALE_COMPLEX:
      gMorseSmaleComplex->createStableComplex(dynamic_cast<ComplexAssembly<MSCVertex<gEmbeddingDimension,DataType> >*>(stable));
      gMorseSmaleComplex->createUnstableComplex(dynamic_cast<ComplexAssembly<MSCVertex<gEmbeddingDimension,DataType> >*>(unstable));

      gMorseSmaleComplex->simplifyStableComplex(gSimplify);
      gMorseSmaleComplex->simplifyUnstableComplex(gSimplify);
      break;
    }
 
    
  }
   
    


  if (gOutputFormat != OUT_NOOUTPUT) {

    if (gOutputFileName != NULL)
      gOutputStream = openFile(gOutputFileName,"w");
    
    if (gComplexType == MORSE_COMPLEX) {

      switch (gOutputFormat) {
        case OUT_NOOUTPUT:
          break;
        case OUT_ASCII:
          gMorseComplex->outputStableSegmentation(gOutputStream,1);
          break;
        case OUT_BINARY:
          gMorseComplex->outputStableSegmentation(gOutputStream,0);
          break;
      }
    }
    else {
      switch (gOutputFormat) {
        case OUT_NOOUTPUT:
          break;
        case OUT_ASCII:
          gMorseSmaleComplex->outputSlopeSegmentation(gOutputStream,1);
          break;
        case OUT_BINARY:
          gMorseSmaleComplex->outputSlopeSegmentation(gOutputStream,0);
          break;
      }

    }
    
    if (gOutputFileName != NULL)
      fclose(gOutputStream);
  }

  if (gStableCTFileName != NULL) {

    FILE* output = fopen(gStableCTFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      gMorseComplex->outputStableCancellationTree(output);
    else
      gMorseSmaleComplex->outputStableCancellationTree(output);

    fclose(output);
  }

  if (gUnstableCTFileName != NULL) {

    FILE* output = fopen(gUnstableCTFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      fprintf(stderr,"Sorry cannot output unstable cancellation tree when computing Morse complexes.\n");
    else
      gMorseSmaleComplex->outputUnstableCancellationTree(output);

    fclose(output);
  }


  if (gStableComplexFileName != NULL) {

    FILE* output = fopen(gStableComplexFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      gMorseComplex->outputStableComplex(output);
    else
      gMorseSmaleComplex->outputStableComplex(output);

    fclose(output);
  }

  if (gUnstableComplexFileName != NULL) {

    FILE* output = fopen(gUnstableComplexFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      fprintf(stderr,"Sorry cannot output unstable cancellation tree when computing Morse complexes.\n");
    else
      gMorseSmaleComplex->outputUnstableComplex(output);

    fclose(output);
  }


  if (gStableLabelFileName != NULL) {

    FILE* output = fopen(gStableLabelFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      gMorseComplex->outputStableLabelHierarchy(output);
    else
      gMorseSmaleComplex->outputStableLabelHierarchy(output);

    fclose(output);
  }

  if (gUnstableLabelFileName != NULL) {

    FILE* output = fopen(gUnstableLabelFileName,"w");

    if (gComplexType == MORSE_COMPLEX)
      fprintf(stderr,"Sorry cannot output unstable label hierarchy when computing Morse complexes.\n");
    else
      gMorseSmaleComplex->outputUnstableLabelHierarchy(output);

    fclose(output);
  }



  if (gStatisticsFileName != NULL) {

    FILE* output = fopen(gStatisticsFileName,"w");

    if (gInputFileName != NULL)
      fprintf(output,"@with g0\n@ title \"%s\"\n",gInputFileName);

    switch (gComplexType) {
    case MORSE_COMPLEX:
      gMorseComplex->outputExtremaVsPersistence(output);
      break;
    case MORSE_SMALE_COMPLEX:
      if (!gMaximaStat)
        gMorseSmaleComplex->outputExtremaVsPersistence(output);
      else
        gMorseSmaleComplex->outputMaximaVsPersistence(output);
        
      break;
    }
    
    fclose(output);
  }
    
  
  return 1;
}
