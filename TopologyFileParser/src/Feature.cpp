#include "Feature.h"

namespace TopologyFileFormat 
{

  FeatureFileType Feature::sFileType = SINGLE_REPRESENTATIVE;

  Feature::Feature() : mAgent(NULL), mId(LNULL), mFlags(0)
  {
    mLifeTime[0] = 0;
    mLifeTime[1] = 0;
  }

  Feature& Feature::operator=(const Feature& f)
  {
    mCon = f.mCon;
    mRep = f.mRep;
    mAgent = f.mAgent;
    mLifeTime[0] = f.mLifeTime[0];
    mLifeTime[1] = f.mLifeTime[1];
    mId = f.mId;
    mFlags = f.mFlags;

    return *this;
  }

  bool Feature::living(FunctionType p) const
  {
    if (direction()) 
    {
      if (mRep.empty()) // If we are a terminal node use a closed interval
        return ((p >= mLifeTime[0]) && (p <= mLifeTime[1]));
      else 
        return ((p > mLifeTime[0]) && (p <= mLifeTime[1]));
    }
    else 
    {
      if (mRep.empty()) // If we are a terminal node use a closed interval
        return ((p >= mLifeTime[0]) && (p <= mLifeTime[1]));
      else
        return ((p >= mLifeTime[0]) && (p < mLifeTime[1]));
    }
  }

  bool Feature::merged(FunctionType p) const
  {
    if (direction()) 
      return (p <= mLifeTime[0]);
    else
      return (p >= mLifeTime[1]);
  }

  Feature* Feature::con(uint32_t i)
  {
    sterror(i >= mCon.size(),"Constituent index out of range.");

    return mCon[i];
  }

  const Feature* Feature::con(uint32_t i) const
  {
    sterror(i >= mCon.size(),"Constituent index out of range.");

    return mCon[i];
  }

  void Feature::addCon(Feature* f)
  {
    sterror(f==NULL,"Constituent cannot be NULL.");

    mCon.push_back(f);
  }


Feature* Feature::rep(uint32_t i)
{
  sterror(i >= mRep.size(),"Representative index %u out of range.",i);

  return mRep[i];
}

const Feature* Feature::rep(uint32_t i) const
{
  sterror(i >= mRep.size(),"Representative index %u out of range.",i);

  return mRep[i];
}

void Feature::addRep(Feature* f)
  {
    sterror(f==NULL,"Representative cannot be NULL.");

    mRep.push_back(f);
  }

  void Feature::writeBinary(std::ofstream& output, const Feature* first) const
  {
    uint8_t dir;
    LocalIndexType id;
    uint16_t count = 1;

    output.write((const char*)mLifeTime,2*sizeof(FunctionType));
  
    if (direction())
      dir = 1;
    else
      dir = 0;

    output.write((const char*)&dir,sizeof(uint8_t));
 
    switch (sFileType) 
    {

    case MULTI_REPRESENTATIVE:
      count = mRep.size();
      output.write((const char*)&count,sizeof(uint16_t));
    
      if (count == 0)
        break;
    case SINGLE_REPRESENTATIVE:
      if (mRep.empty()) 
      {
        id = LNULL;
        output.write((const char*)&id,sizeof(LocalIndexType));
        break;
      }

      for (uint16_t i=0;i<count;i++) 
      {
        id = mRep[i] - first;
        output.write((const char*)&id,sizeof(LocalIndexType));
      }
      break;

    case MULTI_CONSTITUENT:    
      count = mCon.size();
      output.write((const char*)&count,sizeof(uint16_t));

      if (count == 0)
        break;

    case SINGLE_CONSTITUENT:

      if (mCon.empty()) 
      {
        id = LNULL;
        output.write((const char*)&id,sizeof(LocalIndexType));
        break;
      }

      for (uint16_t i=0;i<count;i++) 
      {
        id = mCon[i] - first;
        output.write((const char*)&id,sizeof(LocalIndexType));
      }
      break;
    }
  }
    

  void Feature::readBinary(std::ifstream& input, Feature* first)
  {
    uint8_t dir;
    LocalIndexType id;
    uint16_t count = 1;


    input.read((char*)mLifeTime,sizeof(FunctionType)*2);
    
    input.read((char*)&dir,sizeof(uint8_t));
    direction(dir==0);

    mId = this - first;

  
    switch (sFileType) 
    {
    
    case MULTI_REPRESENTATIVE:
      input.read((char*)&count,sizeof(uint16_t));

    case SINGLE_REPRESENTATIVE: 

      for (uint16_t i=0;i<count;i++) 
      {
        input.read((char*)&id,sizeof(LocalIndexType));
      
        if (id != LNULL) 
        {
          addRep(first + id);
          (first + id)->addCon(this);
        }
      }
      break;

    case MULTI_CONSTITUENT:
      input.read((char*)&count,sizeof(uint16_t));

    case SINGLE_CONSTITUENT:
      for (uint16_t i=0;i<count;i++) 
      {
        input.read((char*)&id,sizeof(LocalIndexType));
      
        if (id != LNULL) 
        {
          addRep(first + id);
          (first + id)->addCon(this);
        }
      }
      break;
    }
  }

  void Feature::writeASCII(std::ofstream& output, const Feature* first) const
  {
    uint16_t count = 1;
    int dir;

    if (direction())
      dir = 1;
    else
      dir = 0;

    output << mLifeTime[0] << " " << mLifeTime[1] << " " << dir;
   
    switch (sFileType) 
    {

    case MULTI_REPRESENTATIVE:
      output << " " << mRep.size();
    
      if (count == 0) 
      {
        output << "\n";
        break;
      }
    case SINGLE_REPRESENTATIVE:
      if (mRep.empty()) 
      {
        output << " " << LNULL << "\n";
        break;
      }

      for (uint16_t i=0;i<mRep.size();i++) 
        output << " " << (uint32_t)(mRep[i] - first);
   
      output << "\n";
      break;

    case MULTI_CONSTITUENT:    
      output << " " << mCon.size();
    
      if (mCon.empty()) 
      {
        output << "\n";
        break;
      }

    case SINGLE_CONSTITUENT:
      if (mCon.empty()) 
      {
        output << " "<< LNULL <<"\n";
        break;
      }

      for (uint16_t i=0;i<mCon.size();i++) 
        output << " " << mCon[i] - first;
   
      output << "\n";
      break;
    }
  }

  void Feature::readASCII(std::ifstream& input, Feature* first)
  {
    LocalIndexType id = LNULL;
    int count = 1;
    int dir;

    sterror(!input.good(),"Input stream corrupt.");

    input >> mLifeTime[0];
    input >> mLifeTime[1];
    input >> dir;

    direction(dir==1);

    mId = this - first;

    switch (sFileType) 
    {
    
    case MULTI_REPRESENTATIVE:
      input >> count;

    case SINGLE_REPRESENTATIVE: 
      for (uint16_t i=0;i<count;i++) 
      {
        input >> id;
      
        if (id != LNULL) 
        {
          addRep(first + id);
          (first + id)->addCon(this);
        }
      }
      break;

    case MULTI_CONSTITUENT:
      input >> count;

    case SINGLE_CONSTITUENT:
      for (uint16_t i=0;i<count;i++) 
      {
        input >> id;
      
        if (id != LNULL) 
        {
          addRep(first + id);
          (first + id)->addCon(this);
        }
      }
      break;
    }
  }


  std::istream& operator>>(std::istream& input, Feature& f)
  {
    // This is a dummy implementation since the compiler is not
    // smart enough to realize that FeatureData should never call
    // this operator since al; input functions are re-implemented
    sterror(true,"Reading features using >> is not possible since we need the first feature pointer.");
  return input;
  }

  std::ostream& operator<<(std::ostream& output, const Feature& f)
  {
    // This is a dummy implementation since the compiler is not
    // smart enough to realize that FeatureData should never call
    // this operator since al; input functions are re-implemented
    sterror(true,"Writing features using << is not possible since we need the first feature pointer.");

    return output;
  }


 }
