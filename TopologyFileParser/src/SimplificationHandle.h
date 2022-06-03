#ifndef SIMPLIFICATIONHANDLE_H
#define SIMPLIFICATIONHANDLE_H

#include "DataHandle.h"
#include "IndexHandle.h"

namespace TopologyFileFormat {

enum FeatureFileType {
  
  SINGLE_REPRESENTATIVE = 0,
  SINGLE_CONSTITUENT    = 1,
  MULTI_REPRESENTATIVE  = 2,
  MULTI_CONSTITUENT     = 3
};

//! Type for the (hopefully) unique id of each SimplificationHandle
typedef uint64_t HierarchyId;

//! Handle to a simplification sequence
class SimplificationHandle : public DataHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class FamilyHandle;

  //! Constructor
  SimplificationHandle();

  //! Constructor
  SimplificationHandle(const char* filename); 
  
  //! Copy constructor
  SimplificationHandle(const SimplificationHandle& handle);
  
  //! Destructor
  virtual ~SimplificationHandle();

  //! Assignment operator
  SimplificationHandle& operator=(const SimplificationHandle& handle);
  
  //! Return the name of the metric
  std::string metric() const {return mMetric;}

  //! Return the file type
  FeatureFileType fileType() const {return mFileType;}

  //! Return the range
  void getRange(FunctionType& low, FunctionType& high) const;

  //! Return the range
  void range(FunctionType* range) const {range[0] = mRange[0],range[1] = mRange[1];}

  //! Set the metric name
  void metric(const std::string& m) {mMetric = m;}

  //! Set the file type
  void fileType(FeatureFileType type) {mFileType = type;}

  //! Set the range
  void setRange(FunctionType low, FunctionType high);

  //! Return the time index
  uint32_t timeIndex() const {return mTimeIndex;}

  //! Set the time index
  void timeIndex(uint32_t index) {mTimeIndex = index;}

  //! Return the physical time
  double time() const {return mTime;}

  //! Set the physical time
  void time(double t) {mTime = t;}

  //! Return the index handle
  const IndexHandle& indexHandle() const {return mIndex;}

  //! Set the index handle
  void indexHandle(const IndexHandle& index) {mIndex = index; mIndex.topHandle(clanHandle());}

protected:

  //! The name of the metric
  std::string mMetric;

  //! The file type of this set of features
  FeatureFileType mFileType;

  //! The range of the simplification parameter
  FunctionType mRange[2];

  //! The handle to the index map if it exists
  IndexHandle mIndex;

  //! The time index of this hierarchy
  uint32_t mTimeIndex;

  //! The real time value of this hierarchy
  double mTime;



  //! Reset all values to their default uninitialized values
  virtual void clear();

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

#endif
