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
#include <algorithm>
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

#define NUM_OPTIONS 2


static const char* BBoxFeatureOptions[NUM_OPTIONS] = {
    "volume",
    "maxDim",
};

enum BBoxFeature {
  VOLUME = 0,
  MAXDIM = 1,
  UNDEFINED = 2,
};

int main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr,"Usage: %s <family> <family-id> [volume | maxDim]\n", argv[0]);
    exit(0);
  }

  ClanHandle clan;
  uint32_t family_id;
  FamilyHandle family;
  StatHandle bbox[6];
  Factory fac;

  clan.attach(argv[1]);
  family_id = atoi(argv[2]);

  BBoxFeature feature = UNDEFINED;
  for (uint8_t i=0;i<NUM_OPTIONS;i++) {
    if (strcmp(BBoxFeatureOptions[i],argv[3]) == 0) {
      feature = (BBoxFeature)i;
      break;
    }
  }

  if (feature == UNDEFINED) {
    fprintf(stderr,"Unknown option \"%s\"\n",argv[3]);
    return 0;
  }

  family = clan.family(family_id);

  if (!family.providesAggregate("min","x-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain min-x\n",argv[1]);
    return 0;
  }
  bbox[0] = family.aggregate("min","x-coord");

  if (!family.providesAggregate("max","x-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain max-x\n",argv[1]);
    return 0;
  }
  bbox[1] = family.aggregate("max","x-coord");

  if (!family.providesAggregate("min","y-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain min-y\n",argv[1]);
    return 0;
  }
  bbox[2] = family.aggregate("min","y-coord");

  if (!family.providesAggregate("max","y-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain max-y\n",argv[1]);
    return 0;
  }
  bbox[3] = family.aggregate("max","y-coord");

  if (!family.providesAggregate("min","z-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain min-z\n",argv[1]);
    return 0;
  }
  bbox[4] = family.aggregate("min","z-coord");

  if (!family.providesAggregate("max","z-coord")) {
    fprintf(stderr,"Warning: Family \"%s\" does not contain max-z\n",argv[1]);
    return 0;
  }
  bbox[5] = family.aggregate("max","z-coord");


  Attribute* data[6];

  data[0] = fac.make_aggregator_array("min",true);
  data[1] = fac.make_aggregator_array("max",true);
  data[2] = fac.make_aggregator_array("min",true);
  data[3] = fac.make_aggregator_array("max",true);
  data[4] = fac.make_aggregator_array("min",true);
  data[5] = fac.make_aggregator_array("max",true);


  for (uint8_t i=0;i<6;i++) {
    data[i]->resize(bbox[i].elementCount());
    bbox[i].readData(data[i]);
  }

  ValueArray derived(bbox[0].elementCount());
  StatHandle derived_handle;


  switch (feature) {

    case VOLUME: {
      for (uint32_t i=0;i<bbox[0].elementCount();i++) {

        //fprintf(stderr,"bbox[%d]  %f %f %f   %f %f %f\n",i,(*data[0])[i].value(),(*data[2])[i].value(),(*data[4])[i].value(),
        //        (*data[1])[i].value(),(*data[3])[i].value(),(*data[5])[i].value());
        derived[i].addVertex((ceil((*data[1])[i].value()) - floor((*data[0])[i].value()))
                             *(ceil((*data[3])[i].value()) - floor((*data[2])[i].value()))
                             *(ceil((*data[5])[i].value()) - floor((*data[4])[i].value())),0);
      }
      derived_handle.species("bbox-volume");
      break;
    }


    case MAXDIM: {
      for (uint32_t i=0;i<bbox[0].elementCount();i++) {

        FunctionType val;

        val = std::max((ceil((*data[1])[i].value()) - floor((*data[0])[i].value())),
                       (ceil((*data[3])[i].value()) - floor((*data[2])[i].value())));
        val = std::max(val,(FunctionType)(ceil((*data[5])[i].value()) - floor((*data[4])[i].value())));
        derived[i].addVertex(val,0);

        fprintf(stderr,"bbox[%d]  %f %f %f   %f %f %f      max = %f\n",i,(*data[0])[i].value(),(*data[2])[i].value(),(*data[4])[i].value(),
                (*data[1])[i].value(),(*data[3])[i].value(),(*data[5])[i].value(),val);


      }
      derived_handle.species("max-dim");
      break;
    }

    case UNDEFINED:
      assert(false);
      break;
  }



  derived_handle.aggregated(true);
  derived_handle.stat(derived.typeName());
  derived_handle.encoding(false);
  derived_handle.setData(&derived);

  clan.family(family_id).append(derived_handle);
  return 1;
}
