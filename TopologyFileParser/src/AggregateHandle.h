#ifndef AGGREGATEHANDLE_H
#define AGGREGATEHANDLE_H

#include "StatHandle.h"

namespace TopologyFileFormat {

//! Handle for per-feature pre-aggregated attributes
class AggregateHandle : public StatHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class FamilyHandle;

  //! Constructor
  AggregateHandle(HandleType t=H_AGGREGATE) : StatHandle(t) {}

  //! Constructor
  AggregateHandle(const char* filename) : StatHandle(filename,H_AGGREGATE) {} 

  //! Copy Constructor
  AggregateHandle(const AggregateHandle& handle) : StatHandle(handle) {}

  //! Des::tructor
  virtual ~AggregateHandle() {}

protected:

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);
};

}

#endif
