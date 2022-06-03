
#include <iostream>
#include <cstring>
#include <vector>
#include "TopologyFileParser/ClanHandle.h"
#include "TopologyFileParser/FeatureHierarchy.h"
#include "TopologyFileParser/FeatureSegmentation.h"
#include "Mean.h"
#include "AggregatorFactory.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;


class IndexConversion
{
public:

  // Constructor to set the global grid dimension
  IndexConversion(uint32_t dimx, uint32_t dimy, uint32_t dimz) : mDimX(dimx), mDimY(dimy), mDimZ(dimz) {}

  ~IndexConversion() {}

  void operator()(uint32_t index, uint32_t i[3]) {
    i[0] = index % mDimX;
    i[1] = (index % (mDimX*mDimY)) / mDimX;
    i[2] = index / (mDimX*mDimY);
  }

  const uint32_t mDimX;
  const uint32_t mDimY;
  const uint32_t mDimZ;
};


void accumulate(Feature* f,MeanArray& distance)
{
  if (f->conSize() == 0) // If I have no constituents
    return; // My value is correct
  else { // Otherwise
    for (uint32_t k=0;k<f->conSize();k++) {
      accumulate(f->con(k),distance); // accumulated the constituent
      //TODO: Which below is correct?
      //distance.addSegment(f->id(), f->id() + f->con(k)->id()); // and add its contribution
      distance.addSegment(f->id(), f->con(k)->id()); // and add its contribution
      //ORIG:
      //distance[f->id()].addSegment(distance + f->con(k)->id()); // and add its contribution
    }
  }
}

int main(int argc, const char* argv[])
{
  ClanHandle clan;
  FamilyHandle family;
  Feature* f;
  FeatureHierarchy hierarchy;
  FeatureSegmentation segmentation;
  StatHandle dist_handle;
  std::ifstream seg_stream;
  std::ifstream map_stream;
  uint32_t dimx,dimy,dimz;

  if (argc < 7) {
    fprintf(stderr,"Usage: %s <family> <segmentation> <indexmap> <dimx> <dimy> <dimz> \n",
            argv[0]);
    exit(0);
  }

  // First read the clan
  clan.attach(argv[1]);

  // Open the segmentation and index map file
  seg_stream.open(argv[2],std::ios_base::in);
  if (!seg_stream) {
    fprintf(stderr,"Could not read segmemtation file \"%s\"\n",argv[2]);
    exit(0);
  }

  map_stream.open(argv[3],std::ios_base::in);
  if (!map_stream) {
    fprintf(stderr,"Could not read index map \"%s\"\n",argv[3]);
    exit(0);
  }

  // Read the grid dimensions
  dimx = atoi(argv[4]);
  dimy = atoi(argv[5]);
  dimz = atoi(argv[6]);

  // Find the family
  family = clan.family(0);

  // Read the hierarchy
  hierarchy.initialize(family.simplification(0));

  // Allocate enough memory for all new thickness values
  //MeanArray* distance = new MeanArray[hierarchy.featureCount()];
  MeanArray distance; 
  distance.resize(hierarchy.featureCount());

  // Read the segmentation
  segmentation.initialize(&seg_stream,&map_stream);

  // Create an index conversion
  IndexConversion index_map(dimx,dimy,dimz);
  // and a place to put the 3D indices
  LocalIndexType coord[3];

  // Now go through all features of the hierarchy and compute their average distances
  for (LocalIndexType i=0;i<hierarchy.featureCount();i++) {
    f = hierarchy.feature(i);

    // If this feature is part of the segmentation
    if (segmentation.contains(f->id())) {

      // Get a reference to all vertices
      Segment seg= segmentation.elementSegmentation(f->id());

      // Now go through all vertices and add their distance to the appropriate mean
      for (LocalIndexType i=0;i<seg.size;i++) {
        // Get the 3D coordinates
        index_map(seg.samples[i],coord);

        distance[i].addVertex(sqrt(pow(coord[0] - 0.5*dimx,2) + pow(coord[1] - 0.5*dimy,2)),0);
      }
    }
  }

  // Now we would like to accumulate the means of all features
  for (LocalIndexType i=0;i<hierarchy.featureCount();i++) {
    f = hierarchy.feature(i);

    // For each feature without a representative we start a recursive traversal
    // to accumulate the distances of its constituents. Note, that this will
    // only work for tree type hierarchies without loops in the graph
    if (f->repSize() == 0)
      accumulate(f,distance);
  }

  // Set the handle information
  dist_handle.aggregated(true);
  dist_handle.stat(distance.typeName());
  dist_handle.species("radius");

  // Assign the data
  dist_handle.setData(&distance);

  fprintf(stderr,"Done computing\n");
  // Add the stat handle to the file
  clan.family(0).append(dist_handle);

}





