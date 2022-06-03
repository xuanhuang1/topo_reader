#ifndef COMPUTEFAMILYCORRELATION_H
#define COMPUTEFAMILYCORRELATION_H

#include <map>
#include <unordered_map>
#include "Segment.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"
#include "FamilyKey.h"
#include "TrackingEdge.h"
#include "KdTree.h"
#include <algorithm>
//#include "AttributeHandle.h"
#include "StatHandle.h"
#include "StatKey.h"
//#include "AggregatorFactory.h"

namespace TopologyFileFormat{

typedef std::map<uint32_t, double> HashTable;
typedef std::vector<HashTable> ListMap;
typedef std::map<double, std::multimap<double, Edge*> > EdgeMap;

enum TalassAccumType {
    SUM = 1,
    MIN = 2,
    MAX = 3,
    MEAN = 4
};

struct Point3f {
    float mData[3];
    float x() { return mData[0]; }
    float y() { return mData[1]; }
    float z() { return mData[2]; }
    Point3f() {};
    Point3f(float x, float y, float z) {
        mData[0] = x;
        mData[1] = y;
        mData[2] = z;
    }
    Point3f(const Point3f& pt) {
        mData[0] = pt.mData[0];
        mData[1] = pt.mData[1];
        mData[2] = pt.mData[2];

    }
    float dist(Point3f pt) {
        return sqrt(pow((mData[0] - pt.mData[0]), 2) + pow((mData[1] - pt.mData[1]), 2) + pow((mData[2] - pt.mData[2]), 2));
    }
};

class ComputeFamilyCorrelation  
{
public:

  int timestep;
  std::map<LocalIndexType, std::set<LocalIndexType > > mEdges; 
  std::map<LocalIndexType, std::map<LocalIndexType,double> > mWeights; 

  ListMap   edgesOfNodes;

  ComputeFamilyCorrelation(FeatureHierarchy* &rh1, FeatureHierarchy* &rh2, 
    FeatureSegmentation* &rs1, FeatureSegmentation* &rs2, TrackingType type, double radius=-1, double param=NULL) : hierarchy1(rh1), hierarchy2(rh2), segmentation1(rs1), segmentation2(rs2), tracking_type(type), radius(radius), timestep(-1),mMaxDist(std::numeric_limits<double>::max()),parameter(param){}

  virtual ~ComputeFamilyCorrelation() {}

  //! Insert edges to the list of edges
  void insertEdge(LocalIndexType srcId, LocalIndexType desId, double edgeWeight, double edgeWeightAcc, bool accu);

  //! Accumulate edge weight over the hierarchy
  void accumulateEdgeWeight(Feature* pSrc, Feature* pDes, bool bDir1, bool bDir2, double weight);

  //! Get alive feature details for the given src and des
  void getAliveIds(Feature* &prSrc, Feature* &prDes, bool bDir1, bool bDir2);

  //! Combine correlations to a edge list
  void combineCorrelations(std::vector<HashTable> &rCorrelationMap);

  HashTable generateHashTable(FeatureHierarchy* &rHierarchy, FeatureSegmentation* &rSegmentation);
  void computeCorrelationsDistance();
  void computeCorrelationsOverlap();
  void computeCorrelationsGiven();


  void computeCorrelations();
  int generateKdTreeAll(FeatureHierarchy*& rHierarchy, FeatureSegmentation*& rSegmentation,kdtree*& tree);

  void initEdges(FeatureHierarchy* hierarchy);
  //void constructEdgeMap();
  TrackingType tracking_type;
  double radius;
  float mMaxDist;
  double parameter;

  int getFirstHierarchySize(){
    return hierarchy1->allFeatures().size();
  }
  std::map<int, std::vector<int> > assocation_map;
std::map<int, std::unordered_map<int, double> > association_weights;
private:
	FeatureHierarchy* hierarchy1; 
	FeatureHierarchy* hierarchy2;   
  FeatureSegmentation* segmentation1;
  FeatureSegmentation* segmentation2;
  TalassAccumType mAccumType;


};
}
#endif 
