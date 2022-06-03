#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include <stack>

#include "Value.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;


void accumulate(const Feature* f, Attribute& attr)
{
  vector<const Feature*> order;
  vector<const Feature*>::reverse_iterator it;
  stack<const Feature*> front;
  const Feature* top;


  front.push(f);

  while (!front.empty()) {
    top = front.top();
    front.pop();

    order.push_back(top);

    for (int i=0;i<top->conSize();i++)
      front.push(top->con(i));
  }

  for (it=order.rbegin();it!=order.rend();it++) {
    if ((*it)->repSize() > 0)
      attr.addSegment((*it)->rep(0)->id(),(*it)->id());
  }

}


/*!
 *
 */

int main(int argc, const char* argv[])
{
  if (argc < 5) {
   fprintf(stderr,"Usage: %s <family> <segmentation> [<field.bof> | x-coord | y-coord | z-coord | void] <metric> [<name>]\n",argv[0]);

   exit(0);
  }


  ClanHandle fclan,sclan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;

  SegmentationHandle seg_handle;
  std::string domain;
  FeatureSegmentation seg;

  int dim;
  int dimx;
  int dimy;
  int dimz;


  fclan.attach(argv[1]);
  family = fclan.family(0);
  hierarchy.initialize(family.simplification(0));

  hierarchy.parameter(1.0);
  //exit(0);

  sclan.attach(argv[2]);
  seg_handle = sclan.family(0).segmentation();
  seg.initialize(seg_handle);

  if (seg_handle.domainType() != REGULAR_GRID) {

   fprintf(stderr,"So far this only works for regualr grids\n");
   assert(false);
  }

  dimx = dimy = dimz = 1;
  domain = seg_handle.domainDescription();

  sscanf(domain.c_str(),"%d %d %d %d",&dim,&dimx,&dimy,&dimz);
  fprintf(stderr,"Dimensions %d x %d x %d of %s\n",dimx,dimy,dimz,argv[3]);

  int metric_arg = 4;
  float*  field = NULL;

  if (strcmp(argv[3],"void") && strcmp(argv[3],"x-coord") && strcmp(argv[3],"y-coord") && strcmp(argv[3],"z-coord")) {

    field = new float[dimx*dimy*dimz];

    //for (int k=0;k<dimx*dimy*dimz;k++)
    //  field[k] = 42;

    ifstream file (argv[3], ios::in|ios::binary);
    if (file.is_open()) {

      for (int k=0;k<dimz;k++)
        file.read ((char *)(field + k*dimx*dimy), dimx*dimy*sizeof(float));
      file.close();
    }
  }

  std::vector<Feature>::const_iterator it;
  Segment s;
  Attribute* derived;
  ValueArray vals(hierarchy.featureCount());

  if (strcmp(argv[metric_arg],"volume") == 0)
    derived = new VertexCountArray(hierarchy.featureCount());
  else if (strcmp(argv[metric_arg],"mean") == 0)
    derived = new MeanArray(hierarchy.featureCount());
  else if (strcmp(argv[metric_arg],"min") == 0)
    derived = new MinimumArray(hierarchy.featureCount());
  else if (strcmp(argv[metric_arg],"max") == 0)
    derived = new MaximumArray(hierarchy.featureCount());
  else if (strcmp(argv[metric_arg],"variance") == 0)
    derived = new VarianceArray(hierarchy.featureCount());
  else if (strcmp(argv[metric_arg],"skewness") == 0)
    derived = new SkewnessArray(hierarchy.featureCount());
  else {
    fprintf(stderr,"Metric not recognized %s \n",argv[metric_arg]);
    exit(0);
  }

  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    s = seg.elementSegmentation(it->id());

    if (field != NULL) { // If we read in a function value
      for (LocalIndexType k=0;k<s.size;k++)
        (*derived)[it->id()].addVertex(field[s.samples[k]],s.samples[k]);

      //fprintf(stderr,"Value %f\n",(*derived)[it->id()].value());
    }
    else if (strcmp(argv[3],"x-coord") == 0) {
      for (LocalIndexType k=0;k<s.size;k++)
        (*derived)[it->id()].addVertex(s.samples[k] % dimx,s.samples[k]);
    }
    else if (strcmp(argv[3],"y-coord") == 0) {
      for (LocalIndexType k=0;k<s.size;k++)
        (*derived)[it->id()].addVertex((int)(s.samples[k] % (dimx*dimy)) / dimx,s.samples[k]);
    }
    else if (strcmp(argv[3],"z-coord") == 0) {
      for (LocalIndexType k=0;k<s.size;k++)
        (*derived)[it->id()].addVertex((int)(s.samples[k] / (dimx*dimy)),s.samples[k]);
    }
    else {

      for (LocalIndexType k=0;k<s.size;k++)
        (*derived)[it->id()].addVertex(1,s.samples[k]);
    }
  }

  fprintf(stderr,"Done computing ... starting accumulating\n");


  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    // For all roots
    if (it->repSize() == 0) {
      accumulate(&(*it),*derived);
      //fprintf(stderr,"Value %f [%f,%f] [%f,%f] %d\n",(*derived)[it->id()].value(),
      //       it->lifeTime()[0],it->lifeTime()[1],it->agent()->lifeTime()[0],it->agent()->lifeTime()[1],it->living(1));
    }
  }


  fprintf(stderr,"Done accumulating... starting output\n");


  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++)
    vals[it->id()].addVertex((*derived)[it->id()].value(),0);


  StatHandle derived_handle;

  derived_handle.aggregated(true);
  derived_handle.stat(vals[0].typeName());

  if (argc > metric_arg+1)
    derived_handle.species(argv[metric_arg+1]);
  else
    derived_handle.species(argv[metric_arg]);
  derived_handle.encoding(false);
  derived_handle.setData(&vals);

  fclan.family(0).append(derived_handle);

  //if (field != NULL)
  //  delete[] field;

}


