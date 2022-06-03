#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "PointIndex.h"
#include "Box.h"
#include "Patch.h"
#include "SerialControl.h"
#include "ModuloGather.h"
#include "ModuloScatter.h"


using namespace std;
int main(int argc, char** argv) {

  if(argc < 8) {
    printf("Usage: %s inputfile gXDim gYDim gZDim nBucketsX nBucketsY nBucketsZ factor <additional inputfiles> \n", argv[0]);
    return 0;
  }

  // initialize based off of parameters
  vector<string> filenames;
  PointIndex globalDimensions(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
  PointIndex numBuckets(atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
  int factor(atoi(argv[8])); 

  GlobalIndexType globalSize = globalDimensions.size(); 
  int numFiles = 1;
  if(argc > 9) numFiles += argc-9;

  filenames.resize(numFiles);
  filenames[0] = string(argv[1]);

  if(argc > 9) {
    for(int i=9; i < argc; i++) filenames[i-9+1] = string(argv[i]);
  }


  // compute block sizes
  // Note: if the block size is not evenly divisible by numBuckets, the processing 
  // code below will handle this with the last bucket containing the addtitional elements 
  PointIndex blockSize;
  for(int i=0; i < 3; i++) {
    blockSize[i] = globalDimensions[i] / numBuckets[i]; 
  }

  // read in global data
  vector<float*> globalData(numFiles); 
  for(int i=0; i < numFiles; i++) {
    FILE* inputFile = fopen(filenames[i].c_str(), "rb");
    globalData[i] = new FunctionType[globalSize];
    fread(globalData[i], sizeof(FunctionType), globalSize, inputFile);
    fclose(inputFile);
  }

  // compute vector of patches
  vector<Patch*> patches;
  PointIndex lowerLeft, upperRight;
  // only add ghost cells on upper right hand side of buckets
  for(int x=0; x < numBuckets[0]; x++) {
    lowerLeft[0] = x * blockSize[0];
    if(x != numBuckets[0]-1)  upperRight[0] = (x+1) * blockSize[0] + 1;
    else upperRight[0] = globalDimensions[0];
    for(int y=0; y < numBuckets[1]; y++) {
      lowerLeft[1] = y * blockSize[1];
      if(y != numBuckets[1]-1) upperRight[1] = (y+1) * blockSize[1] + 1;
      else upperRight[1] = globalDimensions[1];
      for(int z=0; z < numBuckets[2]; z++) {
        lowerLeft[2] = z * blockSize[2];
        if(z != numBuckets[2]-1) upperRight[2] = (z+1) * blockSize[2] + 1;
        else upperRight[2] = globalDimensions[2];

        Box box(lowerLeft, upperRight, globalDimensions);
        vector<const FunctionType * > patchData;

        /*
        cout << "box Dimensions: " << endl;
        cout << "lower left: ";
        for(int z=0; z < 3; z++) {
          cout << lowerLeft[z] << " ";
        }
        cout << endl;
        cout << "upper right: ";
        for(int z=0; z < 3; z++) {
          cout << upperRight[z] << " ";
        }
        cout << endl;
        cout << "global dimensions: ";
        for(int z=0; z < 3; z++) {
          cout << globalDimensions[z] << " ";
        }
        cout << endl;
        */

        // need to compute to make sure to account for ghost cells and 
        // varying block sizes (i.e. globalDimension%numBuckets > 0)
        PointIndex curBlockSize;
        for(int z=0; z < 3; z++) curBlockSize[z] = upperRight[z]-lowerLeft[z];

        for(int b=0; b < numFiles; b++) {
          //cout << "data size = " << curBlockSize.size() << endl;
          FunctionType* data = new FunctionType[curBlockSize.size()];

          int count=0;
          for(int k=lowerLeft[2]; k < upperRight[2]; k++) {
            for(int j=lowerLeft[1]; j < upperRight[1]; j++) {
              for(int i=lowerLeft[0]; i < upperRight[0]; i++) {
                // note since lowerLeft & upperRight are with 
                // respect to the globalDimensisions, we need 
                // to use the local flat indexing function
                PointIndex curIndex(i, j, k);
                data[count++] = globalData[b][curIndex.local(globalDimensions)];
              }
            }
          }
          patchData.push_back(data);
        }
        patches.push_back(new Patch(patchData, box));

      }
    }
  }

  // operate on vector of patches
  SerialControl controller;
  controller.localAlgorithm(LOCAL_SORTED_UF);
  controller.scatterAlgorithm(SCATTER_ITERATIVE_TRAVERSAL);
  controller.gatherAlgorithm(GATHER_STREAMING_SORT);

  controller.gatherFlow(new ModuloGather(numBuckets.size(), factor));
  controller.scatterFlow(new ModuloScatter(numBuckets.size(), factor));

  controller.initialize(patches, numBuckets[0]);
  controller.compute();
  controller.save();

  for(int i=0; i < patches.size(); i++) delete [] patches[i];

  return 1;
};

