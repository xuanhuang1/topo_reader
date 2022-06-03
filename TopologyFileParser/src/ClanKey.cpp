/*
 * ClanKey.cpp
 *
 *  Created on: Mar 14, 2012
 *      Author: bremer5
 */

#include <iostream>
#include "ClanKey.h"

namespace TopologyFileFormat {

int ClanKey::parseXML(const XMLNode& node, uint8_t count)
{
  if (node.getAttribute("clan-name",0) == NULL) {
     mClan = "Unknown";
     return 0;
  }
  else {
    std::stringstream input(std::string(node.getAttribute("clan-name",0)));
    input >> mClan;
  }

  return 1;
}

int ClanKey::attachXML(XMLNode& node, uint8_t count) const
{
  node.addAttribute("clan-name",mClan.c_str());

  return 1;

}


}
