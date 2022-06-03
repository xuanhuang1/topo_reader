/*
 * SimplificationKey.cpp
 *
 *  Created on: Mar 14, 2012
 *      Author: bremer5
 */

#include <iostream>
#include "SimplificationKey.h"

namespace TopologyFileFormat {


SimplificationKey::SimplificationKey(const SimplificationHandle& handle, const FamilyKey& key) :
mFamily(key), mMetric(handle.metric())
{

}


int SimplificationKey::parseXML(const XMLNode& node,uint8_t count)
{
  mFamily.parseXML(node,count);

  if (node.getAttribute("metric",0) == NULL) {
     mMetric = "Unknown";
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute("metric",0));
    mMetric = stringVal;
  }

  /*
  char attr_name[10];
  sprintf(attr_name,"id_%d",count);

  if (node.getAttribute(attr_name,0) == NULL) {
     this->id("Unknown");
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute(attr_name,0));
    this->id(stringVal);
  }
   */
  return 1;
}

//! Write the attributes to xml
int SimplificationKey::attachXML(XMLNode& node, uint8_t count) const
{
  mFamily.attachXML(node,count);

  node.addAttribute("metric",mMetric.c_str());

  return 1;
}


bool SimplificationKey::operator<(const SimplificationKey& key) const
{
  if (mFamily < key.familyKey())
    return true;
  else if (key.familyKey() < mFamily)
    return false;
  else
    return mMetric < key.mMetric;
}


}
