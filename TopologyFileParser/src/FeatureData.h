#ifndef FEATUREDATA_H
#define FEATUREDATA_H

#include "FileData.h"
#include "Feature.h"

namespace TopologyFileFormat {

class FeatureData : public Data<Feature>
{
public:

  //! Constructor to use internal array
  FeatureData(GlobalIndexType size=1) : Data<Feature>(size) {}

  //! Constructor to use external array
  FeatureData(std::vector<Feature>* data) : Data<Feature>(data) {}

  //! Write the binary version of the feature to the file
  virtual void writeBinary(std::ofstream& output) const;

  //! Read the binary version of the feature from the file
  virtual void readBinary(std::ifstream& input);

  //! Write the ascii version of the feature to the file
  virtual void writeASCII(std::ofstream& output) const;

  //! Read from a binary file
  void readASCII(std::ifstream& input);

};

}

#endif /* FEATUREDATA_H_ */
