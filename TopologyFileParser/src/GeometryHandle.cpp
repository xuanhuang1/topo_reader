#include "GeometryHandle.h"

namespace TopologyFileFormat {

int GeometryHandle::parseXML(const XMLNode& node)
{
  return parseXMLInternal(node);
}


int GeometryHandle::parseXMLInternal(const XMLNode& node)
{
  DataHandle::parseXMLInternal(node);

  if (node.getAttribute("dimension",0) == NULL)
    fprintf(stderr,"Warning: \"dimension\" attribute nor found in family node.");
  else {
    std::stringstream input(std::string(node.getAttribute("dimension",0)));
    input >> mDim;
  }

  return 1;
}

int GeometryHandle::attachXMLInternal(XMLNode& node) const
{
  DataHandle::attachXMLInternal(node);

  node.addAttribute("dimension",mDim);

  return 1;
}



}
