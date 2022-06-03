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

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;

string gDerivedName = "radius";

int gDimX;
int gDimY;
int gDimZ;

FunctionType eval(FunctionType* input)
{
  return sqrt(pow(input[0]-0.5*gDimX,2) + pow(input[1]-0.5*gDimY,2));
}


/*!
 * This executable is designed to add derived quantities to an existing
 * feature family. Since there is no easy way to compile code at runtime
 * in C++ the current strategy is to provide a simple to change function
 * for the user to modify.
 */

int main(int argc, const char* argv[])
{
  if (argc < 3) {
   fprintf(stderr,"Usage: %s <file> <family_id> <stat_1> <attribute_1> ... <stat_n> <attribute_n> <dimx> <dimy> <dimz>\n",argv[0]);
   fprintf(stderr,"\nFor example: %s mean x mean y\n",argv[0]);
   fprintf(stderr,"\tThis will make the mean x available as value[0] and mean y as value[1]\n\tin the code.\n\n");

   exit(0);
  }


  ClanHandle clan;
  uint32_t family_id;
  FamilyHandle family;
  vector<StatHandle> stats;
  vector<string> stat_names;
  vector<string> attr_names;
  Factory factory;

  clan.attach(argv[1]);
  family_id = atoi(argv[2]);
  gDimX = atoi(argv[argc-3]);
  gDimY = atoi(argv[argc-2]);
  gDimZ = atoi(argv[argc-1]);

  family = clan.family(family_id);

  for (int i=3;i<argc-4;i+=2) {

    if (family.providesAggregate(string(argv[i]),string(argv[i+1]))) {
      stats.push_back(family.aggregate(string(argv[i]),string(argv[i+1])));
    }
    else if (family.providesAttribute(string(argv[i]),string(argv[i+1]))) {
      stats.push_back(family.attribute(string(argv[i]),string(argv[i+1])));
    }
    else {
      fprintf(stderr,"Could not find statistic \"%s %s\" in the %d's family\n",argv[i],argv[i+1],family_id);
      exit(0);
    }
  }

  //vector<Attribute* > values(stats.size());
  vector<Attribute* > master(stats.size());

  for (uint32_t i=0;i<stats.size();i++) {
    master[i] = factory.make_aggregator_array(stats[i].stat(),stats[i].aggregated());
    master[i]->resize(stats[i].elementCount());
    //fprintf(stderr,"Before reding data\n");
    stats[i].readData(master[i]);
    //fprintf(stderr,"After reading data\n");
  }

  ValueArray derived(stats[0].elementCount());
  FunctionType *input;  
  input = new FunctionType[stats.size()];

  for (uint64_t i=0;i<derived.size();i++) {

    for (uint32_t k=0;k<stats.size();k++)
      input[k] = (*master[k])[i].value();

    //CHECK THIS: What is going on here?
    // set was removed from the base class in the switchover
    derived.addVertex(i, eval(input), i);
  }

  StatHandle derived_handle;

  derived_handle.aggregated(true);
  derived_handle.stat(derived[0].typeName());
  derived_handle.species(gDerivedName);
  derived_handle.encoding(false);

  derived_handle.setData(&derived);

  clan.family(family_id).append(derived_handle);

  // Now we need to clean up
  for (uint32_t i=0;i<stats.size();i++) {
    //master[i]->free(values[i]);
    delete master[i];
  }

}


