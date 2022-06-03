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
  if (argc < 4) {
   fprintf(stderr,"Usage: %s <family> <val> <type> <species> <output>\n",argv[0]);

   exit(0);
  }


  ClanHandle clan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;
  StatHandle stat;

  clan.attach(argv[1]);
  family = clan.family(0);
  hierarchy.initialize(family.simplification(0));
  stat = clan.family(0).aggregate(argv[3],argv[4]);

  Data<float> data;
  stat.readData<float>(data);

  float p;

  p = atof(argv[2]);

  FeatureHierarchy::LivingIterator it;
  FILE* output = fopen(argv[5],"w");

  for (it=hierarchy.beginLiving(p);it!=hierarchy.endLiving();it++)
    fprintf(output,"%f\n",data[(*it)->id()]);

  fclose(output);
}


