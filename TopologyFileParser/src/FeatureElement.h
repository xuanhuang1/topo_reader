#ifndef FEATUREELEMENT_H
#define FEATUREELEMENT_H

#include <vector>

#include "FileElement.h"
#include "SimplificationHandle.h"

namespace TopologyFileFormat {

//! A FeatureElement provides a bare-bones interface to a feature of a feature family
/*! A FeatureElement forms a convinient file interface for applications that
 *  want to use the FileHandle infrastructure to write out feature families and
 *  simplification specifically. A FeatureElement stores only the id, life-time,
 *  direction and a collection of indices that represent either representatives
 *  or constituents depending on the FeatureFileType
 */
class FeatureElement : public FileElement
{
public:

  //! Default constructor
  FeatureElement();
  FeatureElement(FeatureFileType type);

  //! Copy constructor
  FeatureElement(const FeatureElement& element);

  //! Destructor
  virtual ~FeatureElement() {}

  //! Assignment operator
  FeatureElement& operator=(const FeatureElement& element);

  //! Offset operator to provide access to arrays represented only as FileElement*
  virtual FeatureElement& operator[](unsigned int i) {return *(this+i);}
  
  //! Return the vector of links
  const std::vector<LocalIndexType>& links() const {return mLinks;}

  //! Return the i'th link
  LocalIndexType link(LocalIndexType i) const {return mLinks[i];}

  //! Add a new index to the list
  void addLink(LocalIndexType index);

  //! Set the life time
  void lifeTime(FunctionType low, FunctionType high);

  //! Set the direction
  void direction(bool ascending);

  //! Write the ascii version of the element to the file
  virtual void writeASCII(std::ofstream& output) const;

  //! Read the ascii version of the element from the file
  virtual void readASCII(std::ifstream& input);

  //! Write the binary version of the element to the file
  virtual void writeBinary(std::ofstream& output) const;

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input);

  //! Return the size in number of bytes
  virtual uint32_t size() const {return sizeof(FeatureElement);}

private:

  //! The feature file type
  FeatureFileType mFileType;

  //! The life time range
  FunctionType mLifeTime[2];

  //! A flag to store the direction
  uint8_t mDirection;

  //! A vector of indices
  std::vector<LocalIndexType> mLinks;
};

//! ASCII output
std::ofstream& operator<<(std::ofstream& output, const FeatureElement& elem);

//! ASCII input
std::ifstream& operator>>(std::ifstream& input, FeatureElement& elem);

class FeatureElementData : public Data<FeatureElement>
{
public:
  //! Constructor to use internal array
  FeatureElementData(GlobalIndexType size=1) : Data<FeatureElement>(size) {}

  //! Constructor to use external array
  FeatureElementData(std::vector<FeatureElement>* data) : Data<FeatureElement>(data) {}

  virtual ~FeatureElementData() {}


  //! Write the binary version of the element to the file
  virtual void writeBinary(std::ofstream& output) const;

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input);
};

}
  
#endif
