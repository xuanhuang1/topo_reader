#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SegmentationHandle.h"
#include "SimplificationHandle.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"
#include "FeatureData.h"
#include "Segment.h"
#include "FeatureElement.h"

const char *filename = "TestOutput.family";

using namespace std;
using namespace TopologyFileFormat;


// Example file to demonstrate how to output a one-parameter
// hierarchy as .family and .seg files
int main(int argc, const char* argv[])
{

  // Data in the topology file format is ultimately stored in types
  // derived from FileData. Most often these end up being std::vectors

  // Lets define a hierarchy of 7 nodes. Note that we tell the constructor that all
  // our features will have a single representative/parent which is what we normally
  // encounter (everybody has a single parent but maybe multiple children)
  vector<FeatureElement> features(7,FeatureElement(SINGLE_REPRESENTATIVE));


  // Lets assume we have a binary clustering hierarchy where all 4 leafs
  // exist at 0 and the threshold is some sort of distance criterion

  //                    threshold
  // 0     1   2   3       0
  //  \   /     \ /        1
  //   \ /       5         2
  //    4       /          3
  //     \     /           4
  //      \   /            5
  //        6              6

  // Now we set the intervals at which these features are alive
  // and all the other information

  // The node/feature "0" is alive if the threshold is [0,3)
  features[0].lifeTime(0,3); // We set this lifetime

  // See Feature.h:Line 89 : These features are "born" at their low value and
  // "die" at the high value which is considered an ascending hierarchy
  features[0].direction(true);


  // Now we do the same for all other nodes
  features[1].lifeTime(0,3);
  features[1].direction(true);

  features[2].lifeTime(0,2);
  features[2].direction(true);

  features[3].lifeTime(0,2);
  features[3].direction(true);

  features[4].lifeTime(3,6);
  features[4].direction(true);

  features[5].lifeTime(2,6);
  features[5].direction(true);

  features[6].lifeTime(6,7); // Note we could hvae picked any value here larger than 6
  features[6].direction(true);


  // Now we link these up. Note that we only need one direction
  features[0].addLink(4);
  features[1].addLink(4);

  features[2].addLink(5);
  features[3].addLink(5);

  features[4].addLink(6);
  features[5].addLink(6);



  // Now we save this to a file

  // The name of the family file
  ClanHandle clan(filename);

  // The name of the data set (all time steps must have
  // the same data set name)
  clan.dataset("TestDataSet");

  // Create the family which is the internal name for the collection
  // of all the information related to a time step
  FamilyHandle family;

  // The index of this time step
  family.timeIndex(0);

  // The actual floating point real time. Often this is identical
  // to the index but it does not have to be. Also this allows to
  // accept time series with unevenly distributed time steps
  family.time(1.0);

  // The name of the threshold parameter we are using
  family.variableName("ParameterName");

  // The range of the parameter
  family.range(0,1);

  // Now we create the actual hierarchy in form of a simplification
  // sequence. As before we first create the handle
  SimplificationHandle simplification;

  // Set some more meta data
  simplification.metric("Threshold");
  simplification.fileType(SINGLE_REPRESENTATIVE);
  simplification.setRange(0,7);
  simplification.encoding(true); // For testing purposes we write this in ascii

  // Now we assign the data to this handle. To comply with the API we need to
  // have a type derived from FileData
  Data<FeatureElement> data(&features); // This does not copy the data but is simply a pointer

  simplification.setData(&data); // Again this only passes pointers


  // Now we attach the hierarchy to the family
  family.add(simplification);

  // and the family to the clan
  clan.add(family);

  // And finally we write the clan to file
  clan.write();


  // Now let's figure out how to add some geometric points to act as elements of each cluster


  // First we create a 10x10 grid of points (standard row major layout)
  vector<FunctionType> coords(100*3);

  for (int j=0;j<10;j++) {
    for (int i=0;i<10;i++) {
      coords[3*(10*j+i)] = i;     // x-coord
      coords[3*(10*j+i) + 1] = j; // y-coord
      coords[3*(10*j+i) + 2] = 0; // z-coord
    }
  }

  // And we add those to a geometry handle
  GeometryHandle geometry;
  geometry.dimension(3); // We could have done this in 2 dimensions
  geometry.encoding(true); // Make things ascii for testing

  Data<FunctionType> points(&coords);
  geometry.setData(&points);


  // Now we create the segmentation. One way of doing this is to store a set of sets
  // style representation. Here "segment" 0 will correspond to all point indices part
  // of feature/node 0 etc.
  vector<vector<GlobalIndexType> > segments(7);

  // In this case let's assume only the leafs contain data (like in a clustering hierarchy)
  segments[0].resize(25);
  segments[1].resize(25);
  segments[2].resize(25);
  segments[3].resize(25);

  // And for convenience let's just do things by stripes
  for (int i=0;i<25;i++) {
    segments[0][i] = i;
    segments[1][i] = 25+i;
    segments[2][i] = 50+i;
    segments[3][i] = 75+i;
   }


  // And just like usual we now add this data to a segmentation handle
  SegmentationHandle segmentation;
  segmentation.encoding(true); // Make things ascii for testing
  segmentation.domainType(POINT_SET);

  char domain_description[100];
  sprintf(domain_description,"3 %f %f %f %f %f %f", 0.0,9.0,0.0,9.0,0.0,0.0);
  segmentation.domainDescription(domain_description);


  segmentation.setSegmentation(&segments);

  //Now we add the geometry to the segmentation
  segmentation.add(geometry);


  // Just for demonstration purposes we re-open the earlier file
  clan.attach(filename);

  // We happen to know that this is the first family in the file and we "append" our information
  // Note that this call will automatically write this information to file and update the meta data
  clan.family(0).append(segmentation);


  // We are done ... have a look at the bottom of the file to better understand what is happening





  return 1;
}
