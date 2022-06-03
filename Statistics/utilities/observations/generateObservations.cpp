#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Engine.h"
#include "SegmentedGrid.h"

using namespace std;
using namespace Statistics;

int main(int argc, char *argv[]) {

  if (argc < 5 || argc %2== 1) {
   fprintf(stderr,"Usage: %s <inputFilename> <threshold> <numAttr> <attribute_1> <variable_1> ... <attribute_numAttr> <variable_numAttr> |<dimx> <dimy> <tilex> <tiley>|\n",argv[0]);
   fprintf(stderr,"\ndimx dimy tilex tiley are optional parameters that can be used to decompose the data domain into tilex*tiley number of tiles\n");
   fprintf(stderr,"\tEach feature will generate an observation for each tile that contains it.\n\n");
   exit(0);
  }
  
  float threshold;
  int numAttributes;
  bool dumpTileInfo = false;
  SegmentedGrid *grid;
  string inputFilename;
  
  inputFilename = string(argv[1]);
  threshold = atof(argv[2]);
  numAttributes = atoi(argv[3]);
  vector<string> attribute;
  vector<string> variable;

  for(int i=4; i < 4+2*numAttributes; i+=2) {
    attribute.push_back(string(argv[i]));
    variable.push_back(string(argv[i+1]));
  }

  LocalIndexType gridDim[DIM], tileDim[DIM];
  if(argc > 4+2*numAttributes) {
    gridDim[0] = atoi(argv[4+2*numAttributes]);
    gridDim[1] = atoi(argv[4+2*numAttributes+1]);
    tileDim[0] = atoi(argv[4+2*numAttributes+2]);
    tileDim[1] = atoi(argv[4+2*numAttributes+3]);
    dumpTileInfo = true;
  } 

  Engine<float> *engine = new Engine<float>(inputFilename);

  engine->printAvailableAttributeVariables();

  // load all filter feature/attributes
  for(int i=0; i < numAttributes; i++) {
    if(variable[i] == string("NA")) {
      variable[i] = string("");
    }

    engine->loadAttributeVariable(attribute[i], variable[i]); 
  }

  int simpID = 0;
  engine->setParameter(threshold, simpID);

  // generate outputfilename from inputfilename
  string outputName(inputFilename);
  int pos = outputName.rfind(".");
  outputName.replace(pos, 4, ".out");
  ofstream outfile(outputName.c_str());

  vector< map<LocalIndexType, vector<float> > > observations;
  engine->getObservations(simpID, observations);


  // write to output file
  outfile << "parameter " << threshold << endl;
  outfile << "fileID\tfeatureID";

  vector<string> orderedNameList = engine->getActiveAttributeList();

  for(uint32_t i=0; i < orderedNameList.size(); i++) {
    outfile << "\t" << orderedNameList[i];
  }

  if(dumpTileInfo) {
    for(int i=0; i < DIM; i++) {
      outfile << "\ttileDim[" << i << "]";
    }
    grid = new SegmentedGrid(gridDim, tileDim);
    outfile << "\ttileVertexCount";
    for(int i=0; i < DIM; i++) {
      outfile << "\tcenter[" << i << "]";
    }
    for(int i=0; i < DIM; i++) {
      outfile << "\teigenvalue[" << i << "]";
      for(int j=0; j < DIM; j++) {
       outfile << "\teigenvector[" << i << "][" << j << "]";
      }
    }
  }
  outfile << endl; 

  for(uint32_t fileID = 0; fileID < observations.size(); fileID++) {
    if(dumpTileInfo) {
      string fName=engine->getBaseFilename(fileID); 
      grid->swapHierarchy(engine->getHierarchy(fileID, simpID), fName);
      map<LocalIndexType, map<SegmentedGrid::Cell, LocalIndexType> > tileIDs;
      tileIDs = grid->getTileIDs();
      
      map<LocalIndexType, SegmentedGrid::OOBB > obbs = grid->getOBBs();

      map<LocalIndexType, map<SegmentedGrid::Cell, LocalIndexType> >::iterator idItr = tileIDs.begin();
      for(; idItr != tileIDs.end(); idItr++) {
        map<SegmentedGrid::Cell, LocalIndexType>::iterator cItr = (idItr->second).begin();
        for(; cItr != (idItr->second).end(); cItr++) {
          // output file id and feature id
          outfile << fileID << "\t" << idItr->first; 
          for(uint32_t i=0; i < observations[fileID][idItr->first].size(); i++) {
            outfile << "\t" << observations[fileID][idItr->first][i]; 
          }
          for(int i=0; i < DIM; i++) {
            outfile << "\t" << cItr->first.pos[i]; 
          }
          outfile << "\t" << cItr->second;

          SegmentedGrid::OOBB obb = obbs[idItr->first];

          for(int i=0; i < DIM; i++) {
            outfile << "\t" <<  obb.center.pos[i];
          }

          for(int i=0; i < DIM; i++) {
            outfile << "\t" << obb.eigens[i].value;
            for(int j=0; j < DIM; j++) {
              outfile << "\t" << obb.eigens[i].vector[j]; 
            }
          }
           
          outfile << endl;
        }
      }
    } else {
      std::map<LocalIndexType, std::vector<float> >::iterator itr;
      for(itr=observations[fileID].begin(); itr != observations[fileID].end(); itr++) {
        // output file id and feature id
        outfile << fileID << "\t" << itr->first; 
        // output data info
        for(std::vector<float>::iterator dItr = (itr->second).begin(); dItr != (itr->second).end(); dItr++) {
          outfile << "\t" << *dItr; 
        }
        outfile << endl;
      }
    }
  }
  outfile.close();
  if(dumpTileInfo) delete grid;

  return 0;
}
