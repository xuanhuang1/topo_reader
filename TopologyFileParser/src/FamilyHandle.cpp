#include "FamilyHandle.h"

namespace TopologyFileFormat {

FamilyHandle::FamilyHandle() : FileHandle(H_FAMILY), mTimeIndex(0), mTime(0), mVariableName("Attribute 0")
{
  mRange[0] = 0;
  mRange[1] = 0;
}

FamilyHandle::FamilyHandle(const FamilyHandle& handle) : FileHandle(handle)
{
  mSimplifications = handle.mSimplifications;

  mAttributes = handle.mAttributes;
  mAttributeMap = handle.mAttributeMap;

  mAggregates = handle.mAggregates;
  mAggregateMap = handle.mAggregateMap;

  mRange[0] = handle.mRange[0];
  mRange[1] = handle.mRange[1];

  mTimeIndex = handle.mTimeIndex;
  mTime = handle.mTime;

  mSegmentation = handle.mSegmentation;
  mVariableName = handle.mVariableName;
}

FamilyHandle::~FamilyHandle()
{
}

FamilyHandle& FamilyHandle::operator=(const FamilyHandle& handle)
{
  FileHandle::operator=(handle);

  mSimplifications = handle.mSimplifications;

  mAttributes = handle.mAttributes;
  mAttributeMap = handle.mAttributeMap;
  
  mAggregates = handle.mAggregates;
  mAggregateMap = handle.mAggregateMap;

  mRange[0] = handle.mRange[0];
  mRange[1] = handle.mRange[1];
  
  mTimeIndex = handle.mTimeIndex;
  mTime = handle.mTime;

  mSegmentation = handle.mSegmentation;
  mVariableName = handle.mVariableName;

  return *this;
}

FileHandle& FamilyHandle::add(const FileHandle& handle)
{
  HandleKey key;

  switch (handle.type()) {

  case H_SIMPLIFICATION:
    mSimplifications.push_back(dynamic_cast<const SimplificationHandle&>(handle));
    mSimplifications.back().topHandle(clanHandle());
    return mSimplifications.back();
    break;
  case H_STAT: {
    const StatHandle& stat = dynamic_cast<const StatHandle&>(handle);
    key.type_name = stat.stat();
    key.name = stat.species();

    //fprintf(stderr,"Adding handle %s %s\n",key.type_name.c_str(),key.name.c_str());

    if (stat.aggregated()) {
      mAggregates.push_back(stat);
      mAggregateMap[key] = mAggregates.size()-1;
      mAggregates.back().topHandle(clanHandle());
      return mAggregates.back();
    }
    else {
      mAttributes.push_back(stat);
      mAttributeMap[key] = mAttributes.size()-1;
      mAttributes.back().topHandle(clanHandle());
      return mAttributes.back();
    }
    break;
  }
  case H_SEGMENTATION:

    mSegmentation.push_back(dynamic_cast<const SegmentationHandle&>(handle));
    mSegmentation.back().topHandle(clanHandle());
    return mSegmentation.back();
    break;

  case H_FAMILY: {
    const FamilyHandle& handleRef = dynamic_cast<const FamilyHandle&>(handle);

    if(handleRef.providesSegmentation())
      add(handleRef.segmentation());

    for(uint32_t i=0; i < handleRef.numSimplifications(); i++)
      add(handleRef.simplification(i));

    for(uint32_t i=0; i < handleRef.numAggregates(); i++)
      add(handleRef.aggregate(i));

    for(uint32_t i=0; i < handleRef.numAttributes(); i++)
      add(handleRef.attribute(i));
    break;
  }
  default:
    sterror(true,"Unexpected node type.");
  }
         
  return *this;
}

bool FamilyHandle::providesSimplification(const std::string& metric) const
{
  std::vector<SimplificationHandle>::const_iterator it;

  for (it=mSimplifications.begin();it!=mSimplifications.end();it++) {
    if (it->metric() == metric)
      return true;
  }

  return false;
}

const SimplificationHandle& FamilyHandle::simplification(const std::string& metric) const
{
  std::vector<SimplificationHandle>::const_iterator it;

  for (it=mSimplifications.begin();it!=mSimplifications.end();it++) {
    if (it->metric() == metric)
      return *it;
  }

  sterror(true,"Could not find simplification for metric %s",metric.c_str());

  return mSimplifications[0];
}

bool FamilyHandle::providesAttribute(const std::string& type_name, 
                                     const std::string& attribute_name) const
{
  HandleKey key(type_name,attribute_name);
  StatMapType::const_iterator mIt;

  mIt = mAttributeMap.find(key);
  
  return (mIt != mAttributeMap.end());
}


const StatHandle& FamilyHandle::attribute(const std::string& type_name,
                                          const std::string& attribute_name) const
{
  HandleKey key(type_name,attribute_name);
  StatMapType::const_iterator mIt;

  mIt = mAttributeMap.find(key);
  
  sterror(mIt==mAttributeMap.end(),"Could not find %s, %s aggregate combination.",
          type_name.c_str(),attribute_name.c_str());

  return mAttributes[mIt->second];
}

std::vector<std::pair<std::string, std::string> > FamilyHandle::attributes() const
{
  StatMapType::const_iterator mIt;
  std::vector<std::pair<std::string, std::string> > pairs;

  for (mIt=mAttributeMap.begin();mIt!=mAttributeMap.end();mIt++) 
    pairs.push_back(std::pair<std::string, std::string>(mIt->first.type_name,mIt->first.name));
  
  return pairs;
}

bool FamilyHandle::providesAggregate(const std::string& type_name, 
                                     const std::string& attribute_name) const
{
  HandleKey key(type_name,attribute_name);
  StatMapType::const_iterator mIt;

  mIt = mAggregateMap.find(key);
  
  return (mIt != mAggregateMap.end());
}

const StatHandle& FamilyHandle::aggregate(const std::string& type_name,
                                          const std::string& attribute_name) const
{
  HandleKey key(type_name,attribute_name);
  StatMapType::const_iterator mIt;

  mIt = mAggregateMap.find(key);
  
  sterror(mIt==mAggregateMap.end(),"Could not find %s, %s aggregate combination.",
          type_name.c_str(),attribute_name.c_str());

  return mAggregates[mIt->second];
}

std::vector<std::pair<std::string, std::string> > FamilyHandle::aggregates() const
{
  StatMapType::const_iterator mIt;
  std::vector<std::pair<std::string, std::string> > pairs;

  for (mIt=mAggregateMap.begin();mIt!=mAggregateMap.end();mIt++) 
    pairs.push_back(std::pair<std::string, std::string>(mIt->first.type_name,mIt->first.name));
  
  return pairs;
}


void FamilyHandle::clear()
{
  FileHandle::clear();

  mSimplifications.clear();

  mAttributes.clear();
  mAttributeMap.clear();
  
  mAggregates.clear();
  mAggregateMap.clear();

  mRange[0] = mRange[1] = 0; 
}

void FamilyHandle::topHandle(FileHandle* top)
{
  std::vector<SimplificationHandle>::iterator sIt;
  std::vector<StatHandle>::iterator it;

  mTopHandle = top;

  for (sIt=mSimplifications.begin();sIt!=mSimplifications.end();sIt++)
    sIt->topHandle(clanHandle());

  for (it=mAttributes.begin();it!=mAttributes.end();it++)
    it->topHandle(clanHandle());

  for (it=mAggregates.begin();it!=mAggregates.end();it++)
    it->topHandle(clanHandle());
}



int FamilyHandle::parseXML(const XMLNode& family)
{
  FileHandle* handle;
  HandleKey key;

  parseXMLInternal(family);
  
  for (int i=0;i<family.nChildNode();i++) {
    
    handle = this->constructHandle(family.getChildNode(i).getName(),mFileName);
    handle->topHandle(clanHandle());
    handle->parseXML(family.getChildNode(i));
    
    switch (handle->type()) {

    case H_SIMPLIFICATION:
      mSimplifications.push_back(*dynamic_cast<SimplificationHandle*>(handle));

      // Now pass on the time step information to the handle
      mSimplifications.back().timeIndex(mTimeIndex);
      mSimplifications.back().time(mTime);

      break;
    case H_STAT: {
      StatHandle* stat = dynamic_cast<StatHandle*>(handle);

      if (!stat->aggregated()) {
        mAttributes.push_back(*stat);
        key.type_name = stat->stat();
        key.name = stat->species();
        mAttributeMap[key] = mAttributes.size()-1;
      }
      else {
        mAggregates.push_back(*stat);
        key.type_name = stat->stat();
        key.name = stat->species();
        mAggregateMap[key] = mAggregates.size()-1;
      }
      break;
    }
    case H_SEGMENTATION:
      mSegmentation.push_back(*dynamic_cast<SegmentationHandle*>(handle));
      break;
    default:
      sterror(true,"Unexpected node type.");
    }

    delete handle;
  }
  
  return 1;
}

int FamilyHandle::parseXMLInternal(const XMLNode& node)
{
  FileHandle::parseXMLInternal(node);

  if (node.getAttribute("range",0) == NULL)
    fprintf(stderr,"Warning: \"range\" attribute nor found in family node.");
  else {

    const char* range = node.getAttribute("range",0);

    // We need to guard against the fact that sscanf only reads floats but
    // never doubles. However, if FunctionType is a double than we will read
    // garbage.
    float range_val[2];
    sscanf(range,"%e %e",range_val,range_val+1);
    mRange[0] = range_val[0];
    mRange[1] = range_val[1];
  }

  if (node.getAttribute("time-index",0) == NULL)
    mTimeIndex = 0;
  else {
    std::stringstream input(std::string(node.getAttribute("time-index",0)));
    input >> mTimeIndex;
  }

  if (node.getAttribute("time",0) == NULL)
    mTime = 0;
  else {
    std::stringstream input(std::string(node.getAttribute("time",0)));
    input >> mTime;
  }

  if (node.getAttribute("variable",0) != NULL) {
    std::stringstream input(std::string(node.getAttribute("variable",0)));
    input >> mVariableName;
  }


  return 1;
}

int FamilyHandle::attachXMLInternal(XMLNode& node) const
{
  char range[40];
  XMLNode child;

  FileHandle::attachXMLInternal(node);

  sprintf(range,"%.10e %.10e",mRange[0],mRange[1]);

  node.addAttribute("range",range);
  node.addAttribute("time-index",mTimeIndex);
  node.addAttribute("time",mTime);
  node.addAttribute("variable",mVariableName.c_str());

  for (uint16_t i=0;i<mSimplifications.size();i++) {

    child = node.addChild(mSimplifications[i].name());
    mSimplifications[i].attachXMLInternal(child);
  }

  for (uint16_t i=0;i<mAttributes.size();i++) {

    child = node.addChild(mAttributes[i].name());
    mAttributes[i].attachXMLInternal(child);
  }

  for (uint16_t i=0;i<mAggregates.size();i++) {

    child = node.addChild(mAggregates[i].name());
    mAggregates[i].attachXMLInternal(child);
  }

  for (uint16_t i=0;i<mSegmentation.size();i++) {

    child = node.addChild(mSegmentation[i].name());
    mSegmentation[i].attachXMLInternal(child);
  }

  return 1;
}

int FamilyHandle::writeData(std::ofstream& output, const std::string& filename)
{
  this->mFileName = filename;

  mOffset = static_cast<FileOffsetType>(output.tellp());

  for (uint16_t i=0;i<mSimplifications.size();i++) 
    mSimplifications[i].writeData(output,filename);


  for (uint16_t i=0;i<mSegmentation.size();i++)
    mSegmentation[i].writeData(output,filename);

  for (uint16_t i=0;i<mAttributes.size();i++)  
    mAttributes[i].writeData(output,filename);


  for (uint16_t i=0;i<mAggregates.size();i++)
    mAggregates[i].writeData(output,filename);



  return 1;

}

}
 
