#ifndef INDEXHANDLE_H
#define INDEXHANDLE_H

#include "DataHandle.h"

namespace TopologyFileFormat {

//! Handle for per-feature index map
class IndexHandle : public DataHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SimplificationHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SegmentationHandle;

  //! Constructor
  IndexHandle(HandleType t=H_INDEX) : DataHandle(t) {}

  //! Constructor
  IndexHandle(const char* filename) : DataHandle(filename,H_INDEX) {} 

  //! Copy Constructor
  IndexHandle(const IndexHandle& handle) : DataHandle(handle) {}

  //! Destructor
  virtual ~IndexHandle() {}

  //! Assignment operator
  IndexHandle& operator=(const IndexHandle& handle) {DataHandle::operator=(handle);return *this;}

protected:

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);

private:
  
  /*
  //! Overloaded call to disallow access to this function
  template <class ElementClass>
  void readData(ElementClass* data) {sterror(true,"This function should not be accessed.");}
*/
};

}

#endif
