/*
 * tutorial_1.cpp
 *
 *  Created on: Nov 21, 2015
 *      Author: bremer5
 */

#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SegmentationHandle.h"
#include "SimplificationHandle.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"
#include "Segment.h"

/* This tutorial shows how to load a family and segmentation
 * file, set a threshold, and access all resulting features.
 *
 */
using namespace TopologyFileFormat;

int main(int argc, const char* argv[])
{
  if (argc < 3) {
    fprintf(stdout,"Usage: %s <family> <segmentation> <threshold>\n", argv[0]);
    exit(0);
  }

  ClanHandle clan;
  FamilyHandle family;
  SimplificationHandle simp;
  SegmentationHandle seg_handle;
  FeatureHierarchy hierarchy;
  FeatureSegmentation seg;
  float p;


  // Connect the family file with the given clan. Note that this
  // call only reads the xml footers not the actual data
  clan.attach(argv[1]);
  family = clan.family(0);
  simp = clan.family(0).simplification(0);

  // Initiate the actual file read and create the corresponding
  // hierarchy
  hierarchy.initialize(simp);

  // Now do the same thing again for the segmentation file. Note
  // that since the handles only store information on where to find
  // places in the file we can just re-use the same handles
  clan.attach(argv[2]);
  seg_handle = clan.family(0).segmentation();

  // Here is the actual file io that creates the segmentation
  seg.initialize(seg_handle);

  // Read the threshold
  p = atof(argv[3]);


  // An active iterator will cycle through all segments that
  // must be processed
  FeatureHierarchy::ActiveIterator it;

  // This is a collection of pointers to in memory data
  Segment s;
  uint32_t i;

  // For all the ones we have to process
  for (it=hierarchy.beginActive(p);it!=hierarchy.endActive();it++) {

    // Return the internal pointers into a const data structure
    s = seg.elementSegmentation(it->id());

    // This is the fetaure that should determine the color
    const Feature* color_feature = it->agent();

    for (i=0;i<s.size;i++) {
      // Draw all points, i.e. for 3d points
      // draw(s.coordinates[i*s.dim],s.coordinates[i*s.dim+1],s.coordinates[i*s.dim+2]);
    }

  }

  return 1;

}
