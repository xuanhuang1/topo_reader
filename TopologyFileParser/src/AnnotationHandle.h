#ifndef ANNOTATIONHANDLE_H
#define ANNOTATIONHANDLE_H

#include "DataHandle.h"

namespace TopologyFileFormat {

//! Handle for per vertex geometry information (ie. poinnt coordinates)
class AnnotationHandle : public DataHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SegmentationHandle;

  //! Default constructor
  AnnotationHandle(HandleType t=H_ANNOTATION) : DataHandle(t) {}

  //! Constructor
  AnnotationHandle(const char* filename) : DataHandle(filename,H_ANNOTATION) {}

  //! Copy Constructor
  AnnotationHandle(const AnnotationHandle& handle) : DataHandle(handle) {}

  //! Destructor
  virtual ~AnnotationHandle() {}

  //! Assignment operator
  AnnotationHandle& operator=(const AnnotationHandle& handle) {DataHandle::operator=(handle);return *this;}

protected:

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);

  //! Parse the information from the xml tree
  virtual int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  virtual int attachXMLInternal(XMLNode& node) const;
};

}


#endif
