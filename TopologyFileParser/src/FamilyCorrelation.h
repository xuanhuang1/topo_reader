#ifndef TRACKING_H_
#define TRACKING_H_

#include "EdgeHandle.h"

namespace TopologyFileFormat{




class FamilyCorrelation
{
public:


  FamilyCorrelation(){}

  void initialize(EdgeHandle& handle);

  //overload []: key is srcid, returns vector of desIds
  std::vector<LocalIndexType> edgesAt(LocalIndexType src);
  std::vector<double> weightsAt(LocalIndexType srcId);

  LocalIndexType featureCount() {return mFeatureCount;}


private:
  std::vector<LocalIndexType> mNeighbors;
  std::vector<LocalIndexType> mOffsets;
  LocalIndexType mFeatureCount;
  std::vector<double> mWeights;

};  





}
#endif