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
#include "ValueElement.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;


int main(int argc, const char* argv[])
{
  if (argc < 9) {
   fprintf(stderr,"Usage: %s <family> <min> <max> <steps> <aggregate> <type> <species> <output>\n",argv[0]);
   exit(0);
  }


  ClanHandle clan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;
  StatHandle stat;
  Factory fac;

  clan.attach(argv[1]);
  family = clan.family(0);
  hierarchy.initialize(family.simplification(0));
  stat = clan.family(0).aggregate(argv[6],argv[7]);

  Data<float> data;
  stat.readData<float>(data);

  float min,max,delta;
  int step;

  min = atof(argv[2]);
  max = atof(argv[3]);
  step = atoi(argv[4]);

  delta = (max - min) / (step - 1);

  Attribute* agg = fac.make_aggregator(argv[5]);

  FeatureHierarchy::LivingIterator it;
  FILE* output = fopen(argv[8],"w");
  for (int i=0;i<step;i++) {
    hierarchy.parameter(min + i*delta);

    agg->reset();
    for (it=hierarchy.beginLiving();it!=hierarchy.endLiving();it++)
      agg->addVertex(data[(*it)->id()],(*it)->id());
    fprintf(output,"%f %e\n",min+i*delta,agg->value());
  }

  fclose(output);
}


