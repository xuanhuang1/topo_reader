/*
 * add_circumference.cpp
 *
 *
 *  Created on: Nov 12, 2013
 *      Author: bremer5
 */


#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>

#include "TalassConfig.h"
#include "ValueElement.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;




int main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr,"Usage: %s <family> <family_id> <parameter> [attribute_0 statistics_0 ... attribute_N statistics_N] \n", argv[0]);
    exit(0);
  }

  ClanHandle clan;
  uint32_t family_id;
  FamilyHandle family;
  FeatureHierarchy hierarchy;
  //FeatureHierarchy::LivingIterator it;
  vector<Feature>::const_iterator it;
  FunctionType param;
  vector<Attribute*> attributes;
  Factory fac;

  clan.attach(argv[1]);
  family_id = atoi(argv[2]);
  param = atof(argv[3]);


  family = clan.family(family_id);

  for (int i=4;i<argc;i+=2) {

    if (family.providesAggregate(argv[i+1],argv[i])) {
      fprintf(stderr,"Warning: Family \"%s\" does not contain the attribute \"%s\" or statistics \"%s\"\n",family.variableName().c_str(),argv[i],argv[i+1]);
      //continue;
    }

    Attribute* data = fac.make_aggregator_array(argv[i+1],true);
    StatHandle stat = family.aggregate(argv[i+1],argv[i]);
    data->resize(stat.elementCount());
    stat.readData(data);

    attributes.push_back(data);
  }

  hierarchy.initialize(family.simplification(0));
  hierarchy.parameter(param);

  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    if (it->conSize() == 0) { // For all leafs

      const Feature* f = &(*it);

      char name[40];
      sprintf(name,"track_%d.dat",f->id());
      FILE* output = fopen(name,"w");

      int count = 0;
      while ((f != NULL) && (f->agent()->id() == it->id())) {


        fprintf(output,"%e ",f->lifeTime()[1]);
        for (uint8_t i=0;i<attributes.size();i++)
          fprintf(output,"%e ",(*attributes[i])[f->id()].value());
        fprintf(output,"%d\n",f->id());
        if (f->repSize() > 0)
          f = f->rep(0);
        else
          f = NULL;

        count++;
      }
      fclose(output);

      if (count < 3)
        remove(name);

    }
  }

  return 1;
}
