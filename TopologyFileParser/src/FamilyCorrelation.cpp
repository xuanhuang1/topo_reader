#include "FamilyCorrelation.h"


namespace TopologyFileFormat{

void FamilyCorrelation::initialize(EdgeHandle& handle){
  mFeatureCount = handle.featureCount();
  handle.readOffsets(mOffsets);
  handle.readNeighbors(mNeighbors);
  handle.readWeights(mWeights);

}

//overload []: key is srcid, returns vector of desIds
std::vector<LocalIndexType> FamilyCorrelation::edgesAt(LocalIndexType srcId){
  std::vector<LocalIndexType> edges;
  for(int i= mOffsets[srcId];i!=mOffsets[srcId+1];i++){ //i returns position to neighbor array
    edges.push_back(mNeighbors[i]);
}
  return edges;
}

std::vector<double> FamilyCorrelation::weightsAt(LocalIndexType srcId){
  std::vector<double> edges;
  for(int i= mOffsets[srcId];i!=mOffsets[srcId+1];i++){ //i returns position to neighbor array
    edges.push_back(mWeights[i]);
  }
  return edges;
}



}