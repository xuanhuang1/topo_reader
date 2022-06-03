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
   fprintf(stderr,"Usage: %s <family0> ... <familyN> <min-p> <max-p> <steps> <aggregate> <type> <species> <output>\n",argv[0]);
   exit(0);
  }

  float min,max,delta;
  int step;

  min = atof(argv[argc-7]);
  max = atof(argv[argc-6]);
  step = atoi(argv[argc-5]);

  const char* aggregate = argv[argc-4];
  const char* type = argv[argc-3];
  const char* species = argv[argc-2];

  delta = (max - min) / (step - 1);

  Factory fac;
  Attribute* derived = fac.make_aggregator_array(aggregate,true);
  derived->resize(step);

  for (uint32_t i=1;i<argc-7;i++) {


    ClanHandle clan;
    FamilyHandle family;
    FeatureHierarchy hierarchy;
    StatHandle stat;

    clan.attach(argv[i]);
    family = clan.family(0);
    hierarchy.initialize(family.simplification(0));
    stat = clan.family(0).aggregate(type,species);

    Data<float> data;
    stat.readData<float>(data);


    FeatureHierarchy::LivingIterator it;

    for (int i=0;i<step;i++) {

      for (it=hierarchy.beginLiving(min + i*delta);it!=hierarchy.endLiving();it++) {
        //fprintf(stderr,"adding vertex %d %f\n",(*it)->id(),data[(*it)->id()]);

        (*derived)[i].addVertex(data[(*it)->id()],(*it)->id());
      }
    }
  }

  FILE* output = fopen(argv[8],"w");

  for (int i=0;i<step;i++)
    fprintf(output,"%f %e\n",min+i*delta,(*derived)[i].value());

  fclose(output);
}


