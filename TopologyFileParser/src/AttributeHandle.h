#ifndef ATTRIBUTEHANDLE_H
#define ATTRIBUTEHANDLE_H

#include "StatHandle.h"

namespace TopologyFileFormat {

//! Handle for per-feature attributes
class AttributeHandle : public StatHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class FamilyHandle;

  //! Constructor
  AttributeHandle(HandleType t=H_ATTRIBUTE) : StatHandle(t) {}

  //! Constructor
  AttributeHandle(const char* filename) : StatHandle(filename,H_ATTRIBUTE) {} 

  //! Copy Constructor
  AttributeHandle(const AttributeHandle& handle) : StatHandle(handle) {}

  //! Des::tructor
  virtual ~AttributeHandle() {}

protected:

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);
};

}

#endif
