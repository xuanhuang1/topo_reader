#ifndef SEGMENTEDGRID_H
#define SEGMENTEDGRID_H

#include <vector>
#include <cstdlib>
#include <map>

#include "TopologyFileParser/FeatureHierarchy.h"
#include "TopologyFileParser/ClanHandle.h"
#include "StatisticsDefinitions.h"

using namespace TopologyFileFormat;
using namespace std;

const static int DIM = 2;

class SegmentedGrid 
{
public:
  class Cell 
  {
  public:
    uint16_t pos[DIM];
    
    Cell() {for(int i=0; i < DIM; i++) {pos[i] = 0;} }
    ~Cell() {}
    Cell(const Cell& c) { for(int i=0; i < DIM; i++) { pos[i] = c.pos[i]; } }
    Cell  operator+(const Cell & c) {
      Cell res = *this;
      for(int i=0; i < DIM; i++) {
        res.pos[i] += c.pos[i];
      }
      return res;
    }
    Cell  operator/(float val) {
      Cell res = *this;
      for(int i=0; i < DIM; i++) {
        res.pos[i] /= val;
      }
      return res;
    }

   	bool operator<(const Cell& c) const {
      for(int i=0; i < DIM; i++) {
			  if(pos[i] < c.pos[i]) return true;
			  if(pos[i] > c.pos[i]) return false;
		  }
		  return false;
    }
  };
  
  class Eigen
  {
    public:
    double vector[DIM];
    double value;

    Eigen() { for(int i=0; i < DIM; i++) vector[i] = 0; value=0; }
    ~Eigen() { }
    Eigen(const Eigen &e) {for(int i=0; i < DIM; i++) vector[i] = e.vector[i]; value=e.value; }
   	bool operator<(const Eigen& e) const {
      if(value < e.value) return true;
      if(value > e.value) return false;
      return false;
    }

  };

  class OOBB {
    public:
    Eigen eigens[DIM];
    Cell center;
  };

  class Segment 
  {
  public:
    std::vector<uint32_t> elements;
    Segment(){};
    Segment(const Segment& s){elements = s.elements;};
  };
  
  //! Default constructor
  SegmentedGrid() {};

  SegmentedGrid(LocalIndexType gDim[DIM], LocalIndexType tDim[DIM]) {
    for(int i=0; i < DIM; i++) {
      mGridDim[i] = gDim[i];
      mTileDim[i] = tDim[i];
    }
  }

  //! Destructor
  ~SegmentedGrid() { }
  void swapHierarchy(FeatureHierarchy *cur, string &baseFilename);
  void readSegmentation(string &baseFilename);

  map<Cell, LocalIndexType> getTileIDs(LocalIndexType featureID) const;
  map<LocalIndexType, map<Cell, LocalIndexType> > getTileIDs() const;
  OOBB getOBBs(LocalIndexType featureID) const;
  map<LocalIndexType, OOBB >getOBBs() const;



public:
  map<LocalIndexType, Segment> mSegments;

  //! Virtual Grid dimensions
  int mGridDim[DIM];
  //! Virtual Tile dimensions
  int mTileDim[DIM];

  FeatureHierarchy *mHierarchy;
};

#endif
