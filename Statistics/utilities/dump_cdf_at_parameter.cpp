#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include <stack>
#include <algorithm>

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
   fprintf(stderr,"Usage: %s <family0> ... <familyN> <val> <type> <species> <output>\n",argv[0]);

   exit(0);
  }

  vector<float> cdf;
  const char* type = argv[argc-3];
  const char* species = argv[argc-2];
  float p = atof(argv[argc-4]);

  for (uint32_t i=1;i<argc-4;i++) {


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

    for (it=hierarchy.beginLiving(p);it!=hierarchy.endLiving();it++)
      cdf.push_back(data[(*it)->id()]);

  }

  sort(cdf.begin(),cdf.end());

  FILE* output = fopen(argv[argc-1],"w");
  float sum=0;
  fprintf(output,"0 0\n");
  for (int i=0;i<cdf.size();i++) {
    sum += cdf[i];
    fprintf(output,"%f %f\n",cdf[i],(i+1)/(float)cdf.size());
  }

  fclose(output);
}


