#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <cstdio>
#include <string>


#include "FileElement.h"
#include "xmlParser.h"

namespace TopologyFileFormat {

typedef uint64_t FileOffsetType;

#define HANDLE_COUNT 13

enum HandleType {
  H_CLAN = 0,
  H_FAMILY = 1,
  H_SIMPLIFICATION = 2,
  H_INDEX = 3,
  H_STAT = 4,
  H_ASSOC = 5,
  H_EDGE = 6,
  H_SEGMENTATION = 7,
  H_GEOMETRY = 8,
  H_ANNOTATION = 9,
  H_LOCATION = 10,
  H_FIELD = 11,
  H_UNDEFINED = 12
};
enum TrackingType {
    VERTEX_OVERLAP = 0,
    DISTANCE = 1,
    GIVEN=2,
    tNONE=3
};
//! The common baseclass for all file handles
class FileHandle
{
public:

  //! Convinience variable defining an empty string
  static const std::string sEmptyString;

  //! Return the name of the given handle type
  static const char* name(HandleType t);

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SegmentationHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class SimplificationHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class FamilyHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class AssociationHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class ClanHandle;

  //! Static function to create various types of handles
  static FileHandle* constructHandle(const char* name, const char* filename);

  //! Static function to create various types of handles
  static FileHandle* constructHandle(const char* name, const std::string& filename);

  //! Constructor
  FileHandle(HandleType t=H_UNDEFINED); 
  
  //! Constructor
  FileHandle(const char* filename,HandleType t=H_UNDEFINED); 
  
  //! Copy constructor
  FileHandle(const FileHandle& handle);

  //! Destructor
  virtual ~FileHandle();

  //! Assignment operator
  FileHandle& operator=(const FileHandle& handle);

  //! Determine whether this handle has been initialized
  bool valid() const {return (mFileName != "");}
  
  //! Return the handle's type
  HandleType type() const { return mType;}

  //! Return the string identifying this handle
  const char* name() const;

  //! Append the given handle and re-write the file
  void append(FileHandle& handle) {
      FileHandle& h = add(handle);
      clanHandle()->appendData(h);}

  //! Add the given handle to the internal data structure but don't write
  virtual FileHandle& add(const FileHandle& handle);


protected:

  //! The type of this handle
  const HandleType mType;

  //! The filename we are connected to 
  std::string mFileName;

  //! The file offset of the data
  std::streamoff mOffset;

  //! Pointer to the top most handle. Careful, special rules apply in the copy constructor or "="
  FileHandle* mTopHandle;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Return the top handle
  const FileHandle* clanHandle() const {return mTopHandle;}

  //! Return the top handle
  FileHandle* clanHandle() {return mTopHandle;}

  //! Set the top handle
  virtual void topHandle(FileHandle* top) {mTopHandle = top;}

  //! Append the data of the given handle to the file and re-write the footer
  virtual void appendData(FileHandle& handle);

  //! Stub to allow writing data to file
  virtual int writeData(std::ofstream& output,const std::string& filename) {return 1;}

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& family) = 0;

  //! Parse the local information from the xml tree
  int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  int attachXMLInternal(XMLNode& node) const;

  //! Set the file pointer to the correct offset
  int rewind(std::ifstream& file) const;

  //! Open the given file for output in the mode provided
  int openOutputFile(const std::string& filename, std::ofstream& file,
                     bool binary) const;

  //! Open the given file for output in the mode provided
  int openOutputFile(const char* filename, std::ofstream& file,
                     bool binary) const;

  //! Open the given file for input in the mode provided
  int openInputFile(const std::string& filename, std::ifstream& file,
                    bool binary) const;

  //! Open the given file for input in the mode provided
  int openInputFile(const char* filename, std::ifstream& file,
                    bool binary) const;
};

}
  
#endif
