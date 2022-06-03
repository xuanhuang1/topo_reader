#include "FeatureElement.h"

namespace TopologyFileFormat {

FeatureElement::FeatureElement() :
  FileElement(), mFileType(SINGLE_REPRESENTATIVE), mDirection(false)
{
  mLifeTime[0] = mLifeTime[1] = 0;
}

FeatureElement::FeatureElement(FeatureFileType type) :
  FileElement(), mFileType(type), mDirection(false)
{
  mLifeTime[0] = mLifeTime[1] = 0;
}

FeatureElement::FeatureElement(const FeatureElement& element) 
  : FileElement(element), mFileType(element.mFileType), mDirection(element.mDirection),
    mLinks(element.mLinks)                                      
{
  mLifeTime[0] = element.mLifeTime[0];
  mLifeTime[1] = element.mLifeTime[1];
}

FeatureElement& FeatureElement::operator=(const FeatureElement& element) 
{
  mFileType = element.mFileType;
  mLifeTime[0] = element.mLifeTime[0];
  mLifeTime[1] = element.mLifeTime[1];
  mDirection = element.mDirection;
  mLinks = element.mLinks;

  return *this;
}

void FeatureElement::addLink(LocalIndexType index)
{
  mLinks.push_back(index);
}

void FeatureElement::lifeTime(FunctionType low, FunctionType high)
{
  sterror(low>high,"Life time should be a non-inverted interval but I got [%f,%f].",low,high);

  mLifeTime[0] = low;
  mLifeTime[1] = high;
}

void FeatureElement::direction(bool ascending)
{
  if (ascending)
    mDirection = 1;
  else
    mDirection = 0;
}

void FeatureElement::writeASCII(std::ofstream& output) const
{
  output << mLifeTime[0] << " " << mLifeTime[1] << " " << (int)mDirection;

  switch (mFileType) {
    
  case SINGLE_REPRESENTATIVE:
  case SINGLE_CONSTITUENT:
    
    sterror(mLinks.size() >= 2,"Invalid number of representatives.");

    if (mLinks.empty())
      output << " " << LNULL;
    else 
      output << " " << mLinks[0];
              
    break;

  case MULTI_REPRESENTATIVE:
  case MULTI_CONSTITUENT:

    if (mLinks.empty())
      output << " 0\n";
    else {
      output << " " << mLinks.size() << "\n";

      for (std::vector<LocalIndexType>::const_iterator it=mLinks.begin();it!=mLinks.end();it++) 
        output << " " << *it;
      
      output << "\n";
    }
    break;
  }
}

void FeatureElement::readASCII(std::ifstream& input)
{
  input >> mLifeTime[0];
  input >> mLifeTime[1];
  input >> mDirection;

  switch (mFileType) {
    
  case SINGLE_REPRESENTATIVE:
  case SINGLE_CONSTITUENT:
    mLinks.resize(1);
    
    input >> mLinks[0];
    break;

  case MULTI_REPRESENTATIVE:
  case MULTI_CONSTITUENT: {
    uint16_t count;

    input >> count;

    mLinks.resize(count);

    for (uint16_t i=0;i<count;i++) 
      input >> mLinks[i];

    break;
  }
  }
}


void FeatureElement::writeBinary(std::ofstream& output) const
{
  output.write((const char*)mLifeTime,sizeof(FunctionType)*2);
  output.write((const char*)&mDirection,sizeof(uint8_t));

  switch (mFileType) {

    case SINGLE_REPRESENTATIVE:
    case SINGLE_CONSTITUENT:

      sterror(mLinks.size() >= 2,"Invalid number of representatives.");

      if (mLinks.empty())
        output.write((const char*)&LNULL,sizeof(LocalIndexType));
      else
        output.write((const char*)&mLinks[0],sizeof(LocalIndexType));

      break;

    case MULTI_REPRESENTATIVE:
    case MULTI_CONSTITUENT: {
      uint16_t count = mLinks.size();
      output.write((const char*)&count,sizeof(uint16_t));

      if (!mLinks.empty())
        output.write((const char*)&mLinks[0],sizeof(LocalIndexType)*count);

      break;
    }
  }
}

void FeatureElement::readBinary(std::ifstream& input)
{
  input.read((char *)mLifeTime,sizeof(FunctionType)*2);
  input.read((char *)&mDirection,sizeof(uint8_t));

  switch (mFileType) {

    case SINGLE_REPRESENTATIVE:
    case SINGLE_CONSTITUENT:
      mLinks.resize(1);

      input.read((char *)&mLinks[0],sizeof(LocalIndexType));
      break;

    case MULTI_REPRESENTATIVE:
    case MULTI_CONSTITUENT: {
      uint16_t count;

      input.read((char *)&count,sizeof(uint16_t));

      mLinks.resize(count);

      input.read((char *)&mLinks[0],sizeof(LocalIndexType)*count);
      break;
    }
  }

}


std::ofstream& operator<<(std::ofstream& output, const FeatureElement& elem)
{
  elem.writeASCII(output);return output;
}

std::ifstream& operator>>(std::ifstream& input, FeatureElement& elem)
{
  elem.readASCII(input);return input;
}

void FeatureElementData::writeBinary(std::ofstream& output) const
{
  std::vector<FeatureElement>::iterator it;

  for (it=mElements->begin();it!=mElements->end();it++)
    it->writeBinary(output);

}

void FeatureElementData::readBinary(std::ifstream& input)
{
  std::vector<FeatureElement>::iterator it;

  for (it=mElements->begin();it!=mElements->end();it++)
    it->readBinary(input);

}


}
