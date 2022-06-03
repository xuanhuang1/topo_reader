/*
 * SegmentationHandle.h
 *
 *  Created on: Feb 15, 2012
 *      Author: bremer5
 */

#ifndef SEGMENTATIONHANDLE_H_
#define SEGMENTATIONHANDLE_H_

#include <vector>
#include "IndexHandle.h"
#include "GeometryHandle.h"
#include "AnnotationHandle.h"
#include "LocationHandle.h"

namespace TopologyFileFormat {

enum DomainType {
  REGULAR_GRID = 0,
  POINT_SET = 1, 
  LABEL_SET = 2,
  UNDEFINED_DOMAIN = 3
};

//! A class that encapsulates the handle to a segmentation
class SegmentationHandle : public FileHandle
{
public:

  //! Friend declaration to allow access to protected members
  friend class FamilyHandle;

  //! Default constructor
  SegmentationHandle();


  //! Constructor
  SegmentationHandle(const char* filename);

  //! Copy constructor
  SegmentationHandle(const SegmentationHandle& handle);

  //! Destructor
  virtual ~SegmentationHandle();

  //! Assignment operator
  SegmentationHandle& operator=(const SegmentationHandle& handle);

  /***********************************************************************
   ************************  Access to members ***************************
   ***********************************************************************/

  //! Gte the domain type
  DomainType domainType() const {return mDomainType;}

  //! Set the domain type
  void domainType(DomainType type) {mDomainType = type;}

  //! Get the domain description
  std::string domainDescription() const {return mDomainDescription;}

  //! Set the domain descriptor
  void domainDescription(const std::string& description) {mDomainDescription = description;}

  //! Return whether this handle is ascii or binary
  bool encoding() const {return mASCIIFlag;}

  //! Switch the encoding type
  void encoding(bool ascii_flag) {mASCIIFlag=ascii_flag;}

  //! Return the number of features
  LocalIndexType featureCount() const {return mFeatureCount;}

  //! Set the segmentation data as set of sets
  void setSegmentation(std::vector<std::vector<GlobalIndexType> >* segmentation);

  //! Set the offsets
  void setOffsets(std::vector<LocalIndexType>* offsets);

  //! Set the segmentation as flat array
  void setSegmentation(std::vector<GlobalIndexType>* segmentation);

  /*******************************************************************************************
   ****************************  Interface to internal handles *******************************
   ******************************************************************************************/

  //! Add the given handle to the internal data structure but don't write
  virtual FileHandle& add(const FileHandle& handle);

  //! Return the index handle
  IndexHandle& indexHandle() {return mIndex;}

  //! Set the index handle
  void indexHandle(const IndexHandle& index) {mIndex = index; mIndex.topHandle(clanHandle());}

  //! Return the geometry handle
  GeometryHandle& geometryHandle() {return mGeometry;}

  //! Set the geometry handle
  void geometryHandle(const GeometryHandle& geometry) {mGeometry = geometry; mGeometry.topHandle(clanHandle());}

  //! Return the annotation handle
  AnnotationHandle& annotationHandle() {return mAnnotation;}

  //! Set the annotation handle
  void annotationHandle(const AnnotationHandle& annotation) {mAnnotation = annotation; mAnnotation.topHandle(clanHandle());}

  //! Return the location handle
  LocationHandle& locationHandle() {return mLocation;}

  //! Set the location handle
  void locationHandle(const LocationHandle& location) {mLocation = location; mLocation.topHandle(clanHandle());}

  /*******************************************************************************************
   ****************************  Interface to read the data    *******************************
   ******************************************************************************************/

  //! Read the offset data
  int readOffsets(std::vector<LocalIndexType>& offsets);

  //! Read the segmentation data
  int readSegmentation(std::vector<GlobalIndexType>& segmentation);

protected:

  //! The domain type
  DomainType mDomainType;

  //! The domain descriptor
  std::string mDomainDescription;

  //! Data encoding
  bool mASCIIFlag;

  //! The handle to the index map if it exists
  IndexHandle mIndex;

  //! The handle to the geometry data if it exists
  GeometryHandle mGeometry;

  //! The handle to the geometry data if it exists
  LocationHandle mLocation;

  //! The handle to the text data if it exists
  AnnotationHandle mAnnotation;

  //! The number of features should be identical to mSegmentation->size()
  LocalIndexType mFeatureCount;

  //! The segmentation information as sets of sets
  std::vector<std::vector<GlobalIndexType> >* mSegmentation;

  //! The segmentation information as flat array
  std::vector<GlobalIndexType>* mFlatSegmentation;

  //! The array of offsets corresponding to the flat array
  std::vector<LocalIndexType>* mOffsets;

  //! Set the top handle
  virtual void topHandle(FileHandle* top) {mTopHandle = top;mIndex.topHandle(top);}

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);

  //! Parse the information from the xml tree
  virtual int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  virtual int attachXMLInternal(XMLNode& node) const;

  //! Write the local data
  virtual int writeData(std::ofstream& output, const std::string& filename);
};



}

#endif /* SEGMENTATIONHANDLE_H_ */
