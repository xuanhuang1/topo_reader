#ifndef CLANHANDLE_H
#define CLANHANDLE_H

#include <iostream>

#include "FileHandle.h"
#include "FamilyHandle.h"
#include "AssociationHandle.h"

namespace TopologyFileFormat {

class ClanHandle : public FileHandle
{
public:

  //! Current major version number
  static const uint16_t sMajorVersion = 1;

  //! Current minor version number
  static const uint16_t sMinorVersion = 0;
  
  //! ASCII floating point precision
  static const uint8_t sPrecision = 8;

  //! The default dataset name
  static const std::string sDefaultName;

  //! Default constructor
  ClanHandle();
  
  //! Constructor 
  ClanHandle(const char* filename);

  //! Constructor
  ClanHandle(std::string& filename);

  //! Copy constructor 
  ClanHandle(const ClanHandle& handle);

  //! Destructor
  virtual ~ClanHandle() {}

  //! Assignment operator
  ClanHandle& operator=(const ClanHandle& handle);

  //! Return the number of families in the file
  uint32_t numFamilies() const {return mFamilies.size();}
  
  //! Return the i'th family
  FamilyHandle& family(int i) {return mFamilies[i];}

  //! Return the number of associations in the file
  uint32_t numAssociations() const {return mAssociations.size();}

  //! Return the i'th associations
  AssociationHandle& association(int i) {return mAssociations[i];}

  //! Return the dataset name
  std::string dataset() const {return mDataset;}

  //!Set the dataset name
  void dataset(const std::string& name) {mDataset = name;}

  /*******************************************************************************************
   **************************************  File I/O  *****************************************
   ******************************************************************************************/
 
  //! Connect a handle to the given filename
  int attach(const std::string& filename) {return attach(filename.c_str());}

  //! Connect a handle to the given filename
  int attach(const char* filename);

  //! Write the family to the given filename
  void write(const char* filename = NULL);

  //! Append the given handle (and its data) to the file
  void append(FamilyHandle& handle) {add(handle);appendData(mFamilies.back());}
  
  //! Append the given handle (and its data) to the file
  void append(AssociationHandle& handle) {add(handle);appendData(mAssociations.back());}

  //! Add the given handle to the internal data structure but don't write 
  virtual FileHandle& add(const FileHandle& handle);

  //! Update the XML-Footer
  void updateMetaData(const char* filename=NULL);

private:

  //! The "name" of the dataset used to distinguish simulations
  std::string mDataset;

  //! A list of Aggregates
  std::vector<FamilyHandle> mFamilies;

  //! A list of associations
  std::vector<AssociationHandle> mAssociations;

  //! The major version number
  uint16_t mMajor;

  //! The minor version number
  uint16_t mMinor;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Append the data of the given handle to the file and re-write the footer
  void appendData(FileHandle& handle);

  //! Parse the xml tree
  int parseXML(const XMLNode& node);

  //! Parse the local information from the xml tree
  int parseXMLInternal(const XMLNode& node);

  //! Attach the local variables to the given node
  int attachXMLInternal(XMLNode& node) const;

  //! Write the XML-footer to the given stream
  int attachXMLFooter(std::ofstream& output);
};

}

#endif
