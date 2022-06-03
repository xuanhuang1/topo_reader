#include "AttributeVariableList.h"

namespace Statistics {
std::ostream & operator<<(std::ostream &out, const Statistics::AttributeVariablePair &pair)  {
  out << pair.attribute() << " " << pair.variable();
  return out;
}

std::ostream & operator<<(std::ostream &out, const Statistics::AttributeVariableList &list)  {
  for(uint32_t i=0; i < list.mList.size(); i++) {
    out << list.mList[i] << std::endl;
  }
  return out;
}
};
  
