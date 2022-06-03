#ifndef DATAHANDLE_H
#define DATAHANDLE_H

#include <cstdio>
#include <iostream>
#include <cstring>

#include "FileHandle.h"
#include "FileData.h"

namespace TopologyFileFormat {

//! The common baseclass for all file handles
class DataHandle : public FileHandle
{
public:

  //! Constructor
  DataHandle(HandleType t=H_UNDEFINED); 
  
  //! Constructor
  DataHandle(const char* filename,HandleType t=H_UNDEFINED); 
  
  //! Copy constructor
  DataHandle(const DataHandle& handle);

  //! Destructor
  virtual ~DataHandle();

  //! Assignment operator
  DataHandle& operator=(const DataHandle& handle);
  
  //! Set in-memory data 
  /*! Set the given pointers as in memory data
   */
  void setData(FileData* data);
  
  //! Read data from file
  /*! Read the data from the stored file pointer into the buffer provided.
   */
  template <class ElementClass>
  void readData(Data<ElementClass>& data) const;

  //! Read the data from file into an array of individual elements
  void readData(FileData* data) const;

  //! Return the element count
  LocalIndexType elementCount() const {return mElementCount;}

  //! Return the handle's type
  HandleType type() const { return mType;}

  //! Return whether this handle is ascii or binary
  bool encoding() const {return mASCIIFlag;}

  //! Switch the encoding type
  void encoding(bool ascii_flag) {mASCIIFlag=ascii_flag;}

protected:

  //! The pointer to in memory data 
  FileData* mData;

  //! Number of element in the buffer
  LocalIndexType mElementCount;
  
  //! Flag to indicate whether we should encode binary=false or ascii=true
  bool mASCIIFlag;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Parse the information from the xml tree
  int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  int attachXMLInternal(XMLNode& node) const;

  //! Write the data to the given file stream
  /*! If appropriate this function will write the corresponding in memory data
   *  to the given file stream. The filename and offsets will be adjusted
   *  relative to the new file. The handle effectively becomes linked to the new
   *  file.
   *  @param output: A filepointer to the current end of the new file
   *  @return 1 if successful; 0 otherwise.
   */
  virtual int writeData(std::ofstream& output, const std::string& filename);
};

template <class ElementClass>
void DataHandle::readData(Data<ElementClass>& data) const
{
  std::ifstream file;

  if(mASCIIFlag) {
    openInputFile(this->mFileName,file,std::ios_base::in);
    //std::cout << "just opened ascii file, resizing data to " << mElementCount << std::endl;
  } else  {
    openInputFile(this->mFileName,file,std::ios_base::binary);
    //std::cout << "just opened binary file, resizing data to " << mElementCount << std::endl;
  }
  rewind(file);

  data.resize(mElementCount);
  if (mASCIIFlag)
    data.readASCII(file);
  else
    data.readBinary(file);

  file.close();
}



}
  
#endif
