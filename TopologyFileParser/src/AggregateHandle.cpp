#include "AggregateHandle.h"

namespace TopologyFileFormat {

int AggregateHandle::parseXML(const XMLNode& node)
{
  return parseXMLInternal(node);
}

}
