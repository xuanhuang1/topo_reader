#ifndef FAMILYHANDLE_H
#define FAMILYHANDLE_H

#include <vector>
#include <map>
#include <string>

#include "xmlParser.h"
#include "FileHandle.h"
#include "StatHandle.h"
#include "SimplificationHandle.h"
#include "SegmentationHandle.h"

namespace TopologyFileFormat {

//! A class encapsulating the file handle to a feature family
/*! A FamilyHandle encapsulates the file handle to a feature family file. In
 *  particular, it provides an interface to create and read handles for the
 *  different handle types. 
 */
class FamilyHandle : public FileHandle {

public:

  friend class ClanHandle;

  //! Default constructor
  FamilyHandle();
  
  //! Constructor 
  FamilyHandle(const char* filename) : FileHandle(filename,H_FAMILY) {}

  //! Copy constructor 
  FamilyHandle(const FamilyHandle& handle);

  //! Destructor
  virtual ~FamilyHandle();

  //! Assignment operator
  FamilyHandle& operator=(const FamilyHandle& handle);
  
  /*******************************************************************************************
   **************************************  Access to members *********************************
   ******************************************************************************************/

  //! Return the range of the function
  void range(FunctionType* range) const {range[0] = mRange[0],range[1] = mRange[1];}

  //! Set the range (of the overall function)
  void range(FunctionType low, FunctionType high) {mRange[0]=low;mRange[1]=high;}

  //! Return the time index
  uint32_t timeIndex() const {return mTimeIndex;}

  //! Set the time index
  void timeIndex(uint32_t index) {mTimeIndex = index;}

  //! Return the physical time
  double time() const {return mTime;}

  //! Set the physical time
  void time(double t) {mTime = t;}

  //! Return the variable name
  std::string variableName() const {return mVariableName;}

  //! Set the variable name
  void variableName(const std::string name) {mVariableName = name;}

  /*******************************************************************************************
   ****************************  Interface to internal handles *******************************
   ******************************************************************************************/
  
  //! Add the given handle to the internal data structure but don't write
  FileHandle& add(const FileHandle& handle);

  //! Indicate whether the family contains a specific attribute handle
  bool providesAttribute(const std::string& type_name, const std::string& attribute_name) const;

  //! Return the number of attributes
  uint32_t numAttributes() const {return mAttributes.size();}

  //! Return the i'th attribute
  const StatHandle& attribute(uint32_t i) const {return mAttributes[i];}

  //! Return a reference to all attributes
  const std::vector<StatHandle>& attributeList() const {return mAttributes;}

  //! Return a handle to a given attribute
  const StatHandle& attribute(const std::string& type_name, const std::string& attribute_name) const;
  
  //! Return a list of type_name attribute_name pairs of the attributes
  std::vector<std::pair<std::string, std::string> > attributes() const;

  //! Indicate whether the family contains a specific aggregate handle
  bool providesAggregate(const std::string& type_name, const std::string& attribute_name) const;

  //! Return the number of aggregates
  uint32_t numAggregates() const {return mAggregates.size();}

  //! Return the i'th aggregate
  const StatHandle& aggregate(uint32_t i) const {return mAggregates[i];}

  //! Return a reference to all aggregates
  const std::vector<StatHandle>& aggregateList() const {return mAggregates;}

  //! Return a handle to a given aggregate
  const StatHandle& aggregate(const std::string& type_name, const std::string& attribute_name) const;
  
  //! Return a list of type_name attribute_name pairs of the aggregates
  std::vector<std::pair<std::string, std::string> > aggregates() const;

  //! Return the number of simplifications
  uint16_t numSimplifications() const {return mSimplifications.size();}

  //! Indicate whether the family contains at least i simplification sequences
  bool providesSimplification(uint8_t i) const {return (i < mSimplifications.size());}
  
  //! Indicate whether the family contains a simplification for the given metric
  bool providesSimplification(const std::string& metric) const;

  //! Return a handle to the i'th simplification sequence
  const SimplificationHandle& simplification(uint8_t i) const {return mSimplifications[i];}

  //! Return a handle to the simplification for the given metric
  const SimplificationHandle& simplification(const std::string& metric) const;

  //! Indicate whether this family contains a segmentation
  bool providesSegmentation() const {return !mSegmentation.empty();}

  //! Return the segmentation handle if it exists
  const SegmentationHandle& segmentation() const {return mSegmentation[0];}
  SegmentationHandle& segmentation() {return mSegmentation[0];}

protected:
  
  //! Private class to compare typeName / attributeName pairs
  class HandleKey {
    
  public:
    //! The default constructor
    HandleKey() {}

    //! Convinient constructor
    HandleKey(const std::string& t, const std::string& n) : type_name(t), name(n) {}

    //! Destructor
    ~HandleKey() {}

    //! Comparison operator
    bool operator<(const HandleKey& key) const {return ((type_name < key.type_name) 
                                                        || ((type_name == key.type_name) 
                                                            && (name < key.name)));}

    //! The type name 
    std::string type_name;
    
    //! The attribute name
    std::string name;
  };

  //! Typedef for the statistics map
  typedef std::map<HandleKey,uint32_t> StatMapType;
  
  //! A list of simplifications
  std::vector<SimplificationHandle> mSimplifications;

  //! A list of unaccumulated attributes
  std::vector<StatHandle> mAttributes;

  //! The typeName / attributeName to index map 
  StatMapType mAttributeMap;

  //! A list of aggregate attributes
  std::vector<StatHandle> mAggregates;

  //! A typeName / attributeName to index map
  StatMapType mAggregateMap;

  //! A segmentation handle. WE are using a vector since there
  //! might not be a segmentation handle
  std::vector<SegmentationHandle> mSegmentation;

  //! The global range of the function values
  FunctionType mRange[2];

  //! The time index in the sequence
  uint32_t mTimeIndex;

  //! The physical time
  double mTime;

  //! The name of the primary variable of this family
  std::string mVariableName;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Set the top handle
  void topHandle(FileHandle* top);

  //! Parse the xml tree
  int parseXML(const XMLNode& family);

  //! Parse the local information from the xml tree
  int parseXMLInternal(const XMLNode& family);

  //! Attach the local variables to the given node
  int attachXMLInternal(XMLNode& node) const;
  
  //! Write the data of children to the given file stream
  int writeData(std::ofstream& output, const std::string& filename);

};

}

#endif

