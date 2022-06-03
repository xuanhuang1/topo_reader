/*
 * StatKey.cpp
 *
 *  Created on: Sep 14th, 2016
 *      Author: bremer5
 */

#include "StatKey.h"
#include <iostream>

namespace TopologyFileFormat {

//! Less then operator
bool StatKey::operator<(const StatKey& key) const
{
  if (mFamily < key.mFamily)
    return true;
  else if (key.mFamily < mFamily)
    return false;
  else if (mSpecies < key.mSpecies)
    return true;
  else if (mSpecies > key.mSpecies)
    return false;
  else
    return (mStat < key.mStat);
}


int StatKey::parseXML(const XMLNode& node,uint8_t count)
{
  mFamily.parseXML(node,count+2);

  if (node.getAttribute("statistic",0) == NULL) {
     mStat = "Unknown";
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute("statistic",0));
    mStat = stringVal;
  }

  if (node.getAttribute("species",0) == NULL) {
     mSpecies = "Unknown";
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute("species",0));
    mSpecies = stringVal;
  }

  return 1;
}

int StatKey::attachXML(XMLNode& node, uint8_t count) const
{
  mFamily.attachXML(node,count+2);

  node.addAttribute("statistic",mStat.c_str());
  node.addAttribute("species",mSpecies.c_str());

  return  1;
}


}
