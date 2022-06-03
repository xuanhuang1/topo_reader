#include "FamilyAggregator.h"

namespace Statistics {

// JCB moved from .h -- crashed before main when there
const Statistics::Factory FamilyAggregator::sFactory;


FamilyAggregator::FamilyAggregator() : mType(UNDEFINED_AGG), mStat(NULL), mHierarchyProperty(-1),
mPropertyParameters(std::vector<float>(1,1))
{
}
FamilyAggregator::FamilyAggregator(const std::string& name, const std::vector<float>& params)
                                                 : mType(UNDEFINED_AGG), mStat(NULL), mHierarchyProperty(-1)
{
  set(name,params);
}


FamilyAggregator::~FamilyAggregator()
{
  if (mStat != NULL)
    delete mStat;
}

FamilyAggregator& FamilyAggregator::operator=(const FamilyAggregator& agg)
{
  if(&agg == this) return *this;

  if (mStat != NULL) {
    delete mStat;
    mStat = NULL;
  }

  if (agg.mStat != NULL)
    mStat = agg.mStat->clone();

  mHierarchyProperty = agg.mHierarchyProperty;
  mPropertyParameters = agg.mPropertyParameters;
  mType = agg.mType;

  return *this;
}

bool FamilyAggregator::operator==(const FamilyAggregator& agg) const
{
  if (mType != agg.mType) {
    return false;
  }

  if (mStat != agg.mStat) {
    return false;
  }

  if (mHierarchyProperty != agg.mHierarchyProperty) {
    return false;
  }

  if (mPropertyParameters != agg.mPropertyParameters) {
    return false;
  }

  return true;
}

std::string FamilyAggregator::name() const
{
  switch (mType) {

    case FEATURE_BASED:
      return std::string(mStat->typeName());

    case HIERARCHY_BASED: {
      std::string id = TopologyFileFormat::FeatureHierarchy::propertyName(mHierarchyProperty);
      std::ostringstream param;

      if(mPropertyParameters.size() > 1) {
        for (uint32_t i=0;i<mPropertyParameters.size();i++)
          param << " " << mPropertyParameters[i];
      }

      return id + param.str();
    }
    case UNDEFINED_AGG:
      return std::string("Undefined");
  }

  return std::string("Undefined");
}

int FamilyAggregator::set(const std::string& name, const std::vector<float>& params)
{
  Attribute* agg;

  // First we clear the old data
  if (mStat != NULL) {
    delete mStat;
    mStat = NULL;
  }

  mHierarchyProperty = -1;
  mPropertyParameters.clear();

  // First we check to see if this is a hierarchy property
  if (TopologyFileFormat::FeatureHierarchy::hierarchyPropertyIndex(name) != -1) {
    mHierarchyProperty = TopologyFileFormat::FeatureHierarchy::hierarchyPropertyIndex(name);
    mPropertyParameters = params;
    mType = HIERARCHY_BASED;
    mStat = NULL;
  } else {
    // otherwise try to construct an aggregator
    agg = sFactory.make_aggregator(name);
    if (agg != NULL) { // If there is such a statistic
      mStat = agg;
      mType = FEATURE_BASED;
    }// If there is no such stat function it might be a property instead
    else {
      mType = UNDEFINED_AGG;
      return 0;
    }
  }

  return 1;
}

FunctionType FamilyAggregator::evaluate(const std::vector<FunctionType>& features) const
{
  if (mType != FEATURE_BASED) {
    stwarning("FamilyAggregator could not be evaluated feature based. Type mistmatch.");
    return 0;
  }

  sterror(mStat==NULL,"Type mismatch mStat == NULL but type is FEATURE_BASED");

  //(*mStat)[0].reset();
  (*mStat).reset();

  std::vector<FunctionType>::const_iterator it;

  for (it=features.begin();it!=features.end();it++) {
    //(*mStat)[0].addVertex(*it,0);
    (*mStat).addVertex(*it,0);
  }

  //return (*mStat)[0].value();
  return (*mStat).value();
}

/*
FunctionType FamilyAggregator::evaluate(TopologyFileFormat::FeatureHierarchy* hierarchy) const
{
  if (mType != HIERARCHY_BASED) {
    stwarning("FamilyAggregator could not be evaluated feature based. Type mistmatch.");
    return 0;
  }

  sterror(mHierarchyProperty==-1,"Type mismatch mHierarchyProperty == -1 but type is HIERARCHY_BASED");

  return hierarchy->hierarchyProperty(mHierarchyProperty,mPropertyParameters);
}
*/
}

