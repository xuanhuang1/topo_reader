#include "StatHandle.h"

namespace TopologyFileFormat 
{

StatHandle::StatHandle(HandleType t) : DataHandle(t), mDirect(true), mAggregated(false),
    mStat("Unkown"), mSpecies("Unknown")
{
}

StatHandle::StatHandle(const StatHandle& handle) : DataHandle(handle), mStat(handle.mStat),
                                                   mSpecies(handle.mSpecies)
{
  mDirect = handle.mDirect;
  mAggregated = handle.mAggregated;
}

StatHandle::~StatHandle()
{
}

StatHandle::StatHandle(const char* filename,HandleType t) : DataHandle(filename,t), mDirect(true), 
    mAggregated(false), mStat("Unkown"),mSpecies("Unknown")
{
}

StatHandle& StatHandle::operator=(const StatHandle& handle)
{
  DataHandle::operator=(handle);

  mDirect =  handle.mDirect;
  mAggregated = handle.mAggregated;
  mStat = handle.mStat;
  mSpecies = handle.mSpecies;

  return *this;
}


void StatHandle::clear()
{
  DataHandle::clear();

  mDirect = true;
  mAggregated = false;

  mSpecies = std::string("Unknown");
  mStat = std::string("Unknown");
}

int StatHandle::parseXMLInternal(const XMLNode& node)
{
  DataHandle::parseXMLInternal(node);

  sterror(node.getAttribute("mapping",0)==NULL,"Could not find \"mapping\" attribute for stat handle.");

  const char* tmp;

  tmp = node.getAttribute("mapping",0);

  if (strcmp(tmp,"direct")==0) 
    mDirect = true;
  else if (strcmp(tmp,"indirect")==0) 
    mDirect = false;
  else
    sterror(true,"Mapping attribute must be either \"direct\" or \"indirect\".");

  tmp = node.getAttribute("aggregated",0);

  // The aggregated attribute was added later. For backward compatibility
  // we allow  it to be missing in which case we assume it is set to false.
  if (tmp != NULL) {

    if (strcmp(tmp,"yes")==0)
      mAggregated = true;
    else if (strcmp(tmp,"no")==0)
      mAggregated = false;
    else
      sterror(true,"Mapping attribute must be either \"yes\" or \"no\".");
  }
  else
    mAggregated = false;


  sterror(node.getAttribute("attribute",0)==NULL,"Could not find \"attribute\" attribute for stat handle.");
  tmp = node.getAttribute("attribute",0);

  mSpecies = std::string(tmp);

  if (node.getAttribute("statistic",0) == NULL) 
    fprintf(stderr,"Warning: no \"statistic\" attribute found in statistics handle.");
  else 
    mStat = std::string(node.getAttribute("statistic",0));


  return 1;
}

int StatHandle::attachXMLInternal(XMLNode& node) const
{
  DataHandle::attachXMLInternal(node);

  if (mDirect)
    node.addAttribute("mapping","direct");
  else
    node.addAttribute("mapping","indirect");

  if (mAggregated)
    node.addAttribute("aggregated","yes");
  else
    node.addAttribute("aggregated","no");

  node.addAttribute("attribute",mSpecies.c_str());
  node.addAttribute("statistic",mStat.c_str());

  return 1;
}

}

