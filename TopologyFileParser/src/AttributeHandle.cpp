#include "AttributeHandle.h"

namespace TopologyFileFormat {

int AttributeHandle::parseXML(const XMLNode& node)
{
  return parseXMLInternal(node);
}

}
