#ifndef FILEELEMENT_H
#define FILEELEMENT_H

#include <vector>
#include <cassert>
#include <fstream>

namespace TopologyFileFormat {

//! An abstract baseclass for any element that should be written to a file
class FileElement
{
public:

  //! Constructor
  FileElement() {};
  
  //! Destructor
  virtual ~FileElement() {};

  //! Write the ascii version of the element to the file
  void writeASCII(std::ostream& output) const;

  //! Read the ascii version of the element from the file
  void readASCII(std::fstream& input);
};


}
#endif

