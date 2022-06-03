#include "FeatureData.h"

namespace TopologyFileFormat
{

void FeatureData::writeBinary(std::ofstream& output) const
{
  std::vector<Feature>::const_iterator it;

  for (it=this->mElements->begin();it!=this->mElements->end();it++)
    it->writeBinary(output,&this->mElements->at(0));
}

void FeatureData::readBinary(std::ifstream& input)
{
  std::vector<Feature>::iterator it;

  for (it=this->mElements->begin();it!=this->mElements->end();it++)
    it->readBinary(input,&this->mElements->at(0));
}

void FeatureData::writeASCII(std::ofstream& output) const
{
  std::vector<Feature>::const_iterator it;

  for (it=this->mElements->begin();it!=this->mElements->end();it++)
    it->writeASCII(output,&this->mElements->at(0));
}

void FeatureData::readASCII(std::ifstream& input)
{
  std::vector<Feature>::iterator it;

  for (it=this->mElements->begin();it!=this->mElements->end();it++)
    it->readASCII(input,&this->mElements->at(0));
}

}
