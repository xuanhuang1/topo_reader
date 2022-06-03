#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>

#include "ValueElement.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"

using namespace std;
using namespace TopologyFileFormat;

/*!
 *
 */

int main(int argc, const char* argv[])
{
  if (argc < 3) {
   fprintf(stderr,"Usage: %s <family> <segmentation> <fill-value> [<output.bof>]\n",argv[0]);

   exit(0);
  }


  ClanHandle clan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;

  SegmentationHandle seg_handle;
  std::string domain;
  FeatureSegmentation seg;

  float fill = atof(argv[3]);

  int dim;
  int dimx;
  int dimy;
  int dimz;


  clan.attach(argv[1]);
  family = clan.family(0);
  hierarchy.initialize(family.simplification(0));


  clan.attach(argv[2]);
  seg_handle = clan.family(0).segmentation();
  seg.initialize(seg_handle);

  if (seg_handle.domainType() != REGULAR_GRID) {

   fprintf(stderr,"So far this only works for regualr grids\n");
   assert(false);
  }

  dimx = dimy = dimz = 1;
  domain = seg_handle.domainDescription();

  sscanf(domain.c_str(),"%d %d %d %d",&dim,&dimx,&dimy,&dimz);
  fprintf(stderr,"Domain %d x %d x %d\n",dimx,dimy,dimz);

  float* volume = new float[dimx*dimy*dimz];

  for (int i=0;i<dimx*dimy*dimz;i++)
    volume[i] = fill;

  std::vector<Feature>::const_iterator it;
  Segment s;
  float val;

  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    val = it->lifeTime()[0];

    s = seg.elementSegmentation(it->id());

    for (LocalIndexType i=0;i<s.size;i++) {
      assert(volume[s.samples[i]] == fill);
      volume[s.samples[i]] = val;
    }
  }

  FILE* output;
  if (argc > 4)
    output = fopen(argv[4],"w");
  else
    output = fopen("output.raw","w");

  for (int i=0;i<dimz;i++)
    fwrite(volume+i*dimx*dimy,sizeof(float),dimx*dimy,output);

  fclose(output);
}


