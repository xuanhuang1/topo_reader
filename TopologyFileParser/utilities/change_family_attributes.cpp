/*
 * change_clan_attributes.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: bremer5
 */

#include <cstdio>
#include <cstdlib>
#include "ClanHandle.h"

using namespace std;
using namespace TopologyFileFormat;


int main(int argc, const char* argv[])
{
  if (argc < 4) {
    fprintf(stderr,"Usage: %s <family0> <familiy-id> <attribute> <value>\n",argv[0]);
    exit(0);
  }

  string attribute = string(argv[3]);
  int family_id = atoi(argv[2]);

  ClanHandle clan;
  clan.attach(argv[1]);


  if (attribute == string("time-index")) {
    uint32_t time_index = atoi(argv[4]);


    clan.family(family_id).timeIndex(time_index);
    clan.updateMetaData();
  }
  else if (attribute == string("time")) {
    double time = strtod(argv[4],NULL);

    clan.family(family_id).time(time);
    clan.updateMetaData();
  }


  return 1;
}



