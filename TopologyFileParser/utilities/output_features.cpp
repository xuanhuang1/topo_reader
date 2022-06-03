/*
 * output_feature_geometry.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: bremer5
 */

#include <vector>

#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SegmentationHandle.h"
#include "SimplificationHandle.h"
#include "StatHandle.h"
#include "FileData.h"
#include "ValueElement.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"

using namespace std;
using namespace TopologyFileFormat;


//! This executable will output a list of lists with vertex indices per feature

int main(int argc,const char *argv[])
{
  if (argc < 4) {
    fprintf(stdout,"Usage: %s <threshold> <family> <segmentation> [<output-file>]", argv[0]);
    exit(0);
  }

  ClanHandle clan;
  FamilyHandle family;
  SimplificationHandle simp;
  SegmentationHandle seg_handle;
  FeatureHierarchy hierarchy;
  FeatureSegmentation seg;
  float p;

  clan.attach(argv[2]);
  family = clan.family(0);
  simp = clan.family(0).simplification(0);

  hierarchy.initialize(simp);

  clan.attach(argv[3]);
  seg_handle = clan.family(0).segmentation();
  seg.initialize(seg_handle);

  p = atof(argv[1]);


  FILE *output = stdout;
  if (argc > 4) {
    output = fopen(argv[4],"w");
  }

  FeatureHierarchy::LivingIterator it;
  vector<Segment> segments;

  for (it=hierarchy.beginLiving(p);it!=hierarchy.endLiving();it++) {


    // Get a set of subsegments
    seg.segmentation(*it,segments);

    for (uint32_t i=0;i<segments.size();i++) {

      for (uint32_t k=0;k<segments[i].size;k++) {

        fprintf(output," %d",segments[i].samples[k]);
      }
    }

   fprintf(output,"\n");
  }

  if (argc > 4)
    fclose(output);

  return 1;
}



