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
    fprintf(stderr,"Usage: %s <family0> .... <familyN> <attribute> <value>\n",argv[0]);
    exit(0);
  }

  string attribute = string(argv[argc-2]);
  string value = string(argv[argc-1]);
  string dataset = string("Dataset");


  for (uint32_t i=1;i<argc-2;i++) {
    ClanHandle clan;

    clan.attach(argv[i]);

    if (attribute == dataset) {
      clan.dataset(value);
      clan.updateMetaData();
    }
    else {
      fprintf(stderr,"Attribute \"%s\" not recognized \n",attribute.c_str());
      exit(0);
    }
  }

  return 1;
}



