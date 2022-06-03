#include <cstdio>
#include <cstdlib>
#include <algorithm>
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
  if (argc < 4) {
   fprintf(stderr,"Usage: %s <family> <min> <max> <steps> <output>\n",argv[0]);

   exit(0);
  }


  ClanHandle clan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;
  StatHandle stat;
  FunctionType range[2];

  clan.attach(argv[1]);
  family = clan.family(0);
  hierarchy.initialize(family.simplification(0));
  stat = clan.family(0).aggregate("Value","volume_Q");

  Data<float> data;
  stat.readData<float>(data);

  float min,max,delta;
  int step;

  min = atof(argv[2]);
  max = atof(argv[3]);
  step = atoi(argv[4]);

  family.simplification(0).range(range);

  min = std::max(range[0],min);
  max = std::min(range[1],max);

  delta = (max - min) / (step - 1);

  FeatureHierarchy::LivingIterator it;
  FILE* output = fopen(argv[5],"w");
  for (int i=0;i<step;i++) {
    hierarchy.parameter(min + i*delta);
    fprintf(output,"%f %d\n",min+i*delta,hierarchy.livingSize());

  }

  fclose(output);
}


