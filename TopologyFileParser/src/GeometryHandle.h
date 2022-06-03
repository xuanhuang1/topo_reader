#ifndef GEOMETRYHANDLE_H
#define GEOMETRYHANDLE_H

#include "DataHandle.h"

namespace TopologyFileFormat {

//! Handle for per vertex geometry information (ie. poinnt coordinates)
class GeometryHandle : public DataHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SegmentationHandle;

  //! Default constructor
  GeometryHandle(HandleType t=H_GEOMETRY) : DataHandle(t), mDim(3) {}

  //! Constructor
  GeometryHandle(const char* filename) : DataHandle(filename,H_GEOMETRY), mDim(3) {}

  //! Copy Constructor
  GeometryHandle(const GeometryHandle& handle) : DataHandle(handle), mDim(handle.mDim) {}

  //! Destructor
  virtual ~GeometryHandle() {}

  //! Assignment operator
  GeometryHandle& operator=(const GeometryHandle& handle) {mDim=handle.mDim;DataHandle::operator=(handle);return *this;}

  /*******************************************************************************************
   **************************************  Access to members *********************************
   ******************************************************************************************/

  //! Get the number of coordinates per vertex
  uint32_t dimension() const {return mDim;}

  //! Set the number of dimensions per vertex
  void dimension(uint32_t dim) {mDim = dim;}

protected:

  //! The number of dimensions/coordinates per vertex
  uint32_t mDim;

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);

  //! Parse the information from the xml tree
  virtual int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  virtual int attachXMLInternal(XMLNode& node) const;
};

}


#endif
