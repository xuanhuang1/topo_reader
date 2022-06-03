#include "AnnotationHandle.h"

namespace TopologyFileFormat {

int AnnotationHandle::parseXML(const XMLNode& node)
{
  return parseXMLInternal(node);
}


int AnnotationHandle::parseXMLInternal(const XMLNode& node)
{
  DataHandle::parseXMLInternal(node);

  return 1;
}

int AnnotationHandle::attachXMLInternal(XMLNode& node) const
{
  DataHandle::attachXMLInternal(node);

  return 1;
}



}
