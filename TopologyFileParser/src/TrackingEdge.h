#ifndef TRACKINGEDGE_H_
#define TRACKINGEDGE_H_

#include "DataHandle.h"

#include <vector>
#include <unordered_map>


namespace TopologyFileFormat{

struct Edge
{ 
  int srcId, desId;
  double edgeWeight, edgeWeightAcc;
  double birth, death;
  
  Edge() :
   srcId(-1), desId(-1), birth(-1), death(-1), edgeWeight(-1), edgeWeightAcc(-1) {} //TODO: include edge weight here? or just entire set of correlations, and use some sort of aggregator later?

  Edge(int srcId_, int desId_, double edgeBirth, double edgeDeath, double edgeWeight_, double edgeWeightAcc_=0) : 
   srcId(srcId_), desId(desId_), birth(edgeBirth), death(edgeDeath), edgeWeight(edgeWeight_), edgeWeightAcc(edgeWeightAcc_) {} //todo (maybe): pass in feature, extract id/birthdeath times in constructor

  ~Edge() {}	

  //! Operator==
  bool operator==(const Edge &rEdge)
  {
    if(rEdge.srcId==-1) return (desId==rEdge.desId);
    if(rEdge.desId==-1) return (srcId==rEdge.srcId);

    return (srcId==rEdge.srcId) && (desId==rEdge.desId);
  }

  bool isAlive(double param){
  	return (birth<=param && death>param);
  }
};


}
#endif