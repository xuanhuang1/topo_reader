/*
 * FamilyKey.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: bremer5
 */

#include "FamilyKey.h"
#include <iostream>

namespace TopologyFileFormat {

//! Less then operator
bool FamilyKey::operator<(const FamilyKey& key) const
{
  if (mClan < key.mClan)
    return true;
  else if (key.mClan < mClan)
    return false;
  else if (mTimeIndex < key.mTimeIndex)
    return true;
  else if (mTimeIndex > key.mTimeIndex)
    return false;
  else
    return (mVariable < key.mVariable);
}


int FamilyKey::parseXML(const XMLNode& node,uint8_t count)
{
  mClan.parseXML(node,count+2);

  if (node.getAttribute("variable",0) == NULL) {
     mVariable = "Unknown";
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute("variable",0));
    mVariable = stringVal;
  }

  if (node.getAttribute("timeindex",0) == NULL) {
     this->mTimeIndex = -1;
     return 0;
  }
  else {
    std::stringstream input(std::string(node.getAttribute("timeindex",0)));
    input >> this->mTimeIndex;
  }


  /*
  char attr_name[10];
  sprintf(attr_name,"id_%d",count+1);

  if (node.getAttribute(attr_name,0) == NULL) {
     this->id("Unknown");
     return 0;
  }
  else {
    std::string stringVal(node.getAttribute(attr_name,0));
    this->id(stringVal);
  }

  sprintf(attr_name,"id_%d",count);
  if (node.getAttribute(attr_name,0) == NULL) {
     this->mTimeIndex = -1;
     return 0;
  }
  else {
    std::stringstream input(std::string(node.getAttribute(attr_name,0)));
    input >> this->mTimeIndex;
  }
  */
  return 1;

}

int FamilyKey::attachXML(XMLNode& node, uint8_t count) const
{
  mClan.attachXML(node,count+2);
  char tIndex[10];

  node.addAttribute("variable",mVariable.c_str());

  sprintf(tIndex, "%d", mTimeIndex);
  node.addAttribute("timeindex",tIndex);
  /*
  char attr_name[10], tIndex[10];

  sprintf(attr_name,"id_%d",count+1);
  node.addAttribute(attr_name,this->id().c_str());


  sprintf(attr_name,"id_%d",count);
  sprintf(tIndex, "%d", mTimeIndex);
  node.addAttribute(attr_name, tIndex);
   */

  return  1;
}


}
