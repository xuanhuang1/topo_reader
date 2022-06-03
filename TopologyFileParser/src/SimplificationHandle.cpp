#include "SimplificationHandle.h"

namespace TopologyFileFormat {

SimplificationHandle::SimplificationHandle() : 
  DataHandle(H_SIMPLIFICATION), mMetric(""), mFileType(SINGLE_REPRESENTATIVE), mTimeIndex(0), mTime(0)
{
  mRange[0] = mRange[1] = 0;
}

SimplificationHandle::SimplificationHandle(const SimplificationHandle& handle) :
  DataHandle(handle), mMetric(handle.mMetric), mFileType(handle.mFileType), mIndex(handle.mIndex),
  mTimeIndex(handle.mTimeIndex), mTime(handle.mTime)
{
  mRange[0] = handle.mRange[0];
  mRange[1] = handle.mRange[1];
}

SimplificationHandle::~SimplificationHandle() 
{
}

SimplificationHandle::SimplificationHandle(const char* filename) : DataHandle(filename,H_SIMPLIFICATION)
{
}

SimplificationHandle& SimplificationHandle::operator=(const SimplificationHandle& handle)
{
  DataHandle::operator=(handle);

  mMetric = handle.mMetric;
  mFileType = handle.mFileType;
  mRange[0] = handle.mRange[0];
  mRange[1] = handle.mRange[1];
  mTimeIndex = handle.mTimeIndex;
  mTime = handle.mTime;
  mIndex = handle.mIndex;

  return *this;
}

void SimplificationHandle::getRange(FunctionType& low, FunctionType& high) const
{
  low = mRange[0];
  high = mRange[1];
}

void SimplificationHandle::setRange(FunctionType low, FunctionType high) 
{
  mRange[0] = low;
  mRange[1] = high;
}

void SimplificationHandle::clear()
{
  DataHandle::clear();

  mMetric = std::string("");
  mFileType = SINGLE_REPRESENTATIVE;
  mRange[0] = mRange[1] = 0;
}

int SimplificationHandle::parseXML(const XMLNode& node)
{
  FileHandle* handle;
 
  parseXMLInternal(node);

  if (node.nChildNode() > 0) {
    handle = this->constructHandle(node.getChildNode(0).getName(),this->mFileName);
    handle->topHandle(clanHandle());
    sterror(handle->type() != H_INDEX,"Only index map nodes can be children of a simplification node.");
    
    handle->parseXML(node.getChildNode(0));
    mIndex = *dynamic_cast<IndexHandle*>(handle);

    delete handle;
  }

  return 1;
}

int SimplificationHandle::parseXMLInternal(const XMLNode& node)
{
  DataHandle::parseXMLInternal(node);


  if (node.getAttribute("metric",0) == NULL) 
    fprintf(stderr,"Warning: no \"metric\" attribute found in simplification handle.");
  else 
    mMetric = std::string(node.getAttribute("metric",0));
   
  if (node.getAttribute("filetype",0) == NULL) 
    fprintf(stderr,"Warning: no \"filetype\" attribute found in simplification handle.");
  else 
    mFileType = (FeatureFileType)atoi(node.getAttribute("filetype",0));
 
  if (node.getAttribute("range",0) == NULL) 
    fprintf(stderr,"Warning: no \"range\" attribute found in simplification handle.");
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
  
  return 1;
}

int SimplificationHandle::attachXMLInternal(XMLNode& node) const
{
  DataHandle::attachXMLInternal(node);

  node.addAttribute("metric",mMetric.c_str());
  node.addAttribute("filetype",(uint8_t)mFileType);

  char range[40];
  
  sprintf(range,"%.10e %.10e",mRange[0],mRange[1]);
  node.addAttribute("range",range);

  if (mIndex.elementCount() > 0) {
    XMLNode child;
    
    child = node.addChild(mIndex.name());
    mIndex.attachXMLInternal(child);
  }

  return 1;
}
  
int SimplificationHandle::writeData(std::ofstream& output, const std::string& filename)
{
  // First write your own data
  DataHandle::writeData(output,filename);

  // The if necessary the index map
  if (mIndex.elementCount() > 0) 
    mIndex.writeData(output,this->mFileName);

  return 1;
}



}
