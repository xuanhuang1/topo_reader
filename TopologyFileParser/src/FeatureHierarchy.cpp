#include <math.h>
#include <algorithm>
#include <queue>
#include <stack>

#include <iostream>

#include "FeatureHierarchy.h"
#include "FeatureData.h"
#include "IntervalTree.h"


namespace TopologyFileFormat {

const std::string FeatureHierarchy::sPropertyNames[sPropertyCount] = {
    std::string("featureCount"),
    std::string("minParamWithCount"),
    std::string("maxParamWithCount"),
};

FeatureHierarchy::LivingIterator::LivingIterator(const LivingIterator& it)
{
  mParameter = it.mParameter;
  mInitialized = it.mInitialized;
  mIt = it.mIt;
  mEnd = it.mEnd;
}

FeatureHierarchy::LivingIterator& FeatureHierarchy::LivingIterator::operator=(const LivingIterator& it)
{
  mParameter = it.mParameter;
  mInitialized = it.mInitialized;
  mIt = it.mIt;
  mEnd = it.mEnd;

  return *this;
}

void FeatureHierarchy::LivingIterator::operator++(int i)
{
  // If we are not initialized we do nothing
  if (!mInitialized)
    return;

  // Advance at least once
  mIt++;

  // Now skip all other features that are not actually alive
  while ((mIt != mEnd) && !(*mIt)->living(mParameter))
    mIt++;

  // Now make sure that when we have reach the end we
  // count as uninitialized
  if (mIt == mEnd)
    mInitialized = false;
}

bool FeatureHierarchy::LivingIterator::operator==(const LivingIterator& it) const
{
  // Special rules apply when comparing to an uninitialized iterator

  // If both iterators are not initialized they are considered equal
  if (!mInitialized && !it.mInitialized)
    return true;

  if (mInitialized && it.mInitialized) {

    // If both are initialized do a standard comparison
    return ((mParameter == it.mParameter) && (mIt == it.mIt) && (mEnd == it.mEnd));
  }

  // If one is initialized and one is not than we always compare as false
  return false;
 }

FeatureHierarchy::LivingIterator::LivingIterator(FunctionType parameter,
                                                 std::vector<const Feature*>::const_iterator start,
                                                 std::vector<const Feature*>::const_iterator stop)
{
  mParameter = parameter;
  mIt = start;
  mEnd = stop;

  if (mIt == mEnd)
    mInitialized = false;
  else
    mInitialized = true;
}


FeatureHierarchy::ActiveIterator::ActiveIterator()
{
}

FeatureHierarchy::ActiveIterator::ActiveIterator(const ActiveIterator& it) :
          mFront(it.mFront),mRootIt(it.mRootIt)
{
}

FeatureHierarchy::ActiveIterator::~ActiveIterator()
{
}


FeatureHierarchy::ActiveIterator& FeatureHierarchy::ActiveIterator::operator=(const ActiveIterator& it)
{
  mFront = it.mFront;

  mRootIt = it.mRootIt;

  return *this;
}

void FeatureHierarchy::ActiveIterator::operator++(int i)
{
  // If there is nothing left to process
  if (mFront.empty())
    return; // There is nothing to do

  sterror(mFront.empty(),"The front should not be empty().");

  const Feature* top = mFront.front();
  Feature::iterator it;

  mFront.pop_front();

  for (it=top->constituents().begin();it!=top->constituents().end();it++) {
    mFront.push_back(*it);
  }


  // If we need to switch subtree
  if (mFront.empty()) {
    mRootIt++;

    // If there is no subtree left
    if (!mRootIt.valid())
      return;
    else
      mFront.push_back(*mRootIt);
  }
}


bool FeatureHierarchy::ActiveIterator::operator==(const  ActiveIterator& it) const
{
  if (mRootIt != it.mRootIt)
    return false;

  if (mFront.size() != it.mFront.size())
    return false;

  if (mFront != it.mFront)
    return false;

  return true;
}

const Feature* FeatureHierarchy::ActiveIterator::feature() const
{
  if (mFront.empty())
    return NULL;

  return mFront.front();
}

const Feature* FeatureHierarchy::ActiveIterator::rep() const
{
  if (!mRootIt.valid())
    return NULL;

  return *mRootIt;
}

const Feature* FeatureHierarchy::ActiveIterator::agent() const
{
  if (rep() == NULL)
    return NULL;

  return rep()->agent();
}

FeatureHierarchy::ActiveIterator::ActiveIterator(LivingIterator it) :
         mRootIt(it)
{
  if (mRootIt.valid())
    mFront.push_back(*mRootIt);
}

FeatureHierarchy::FeatureHierarchy() :
        mMinPar(10e34), mMaxPar(-10e34)
{
  //fprintf(stderr,"FeatureHierarchy::FeatureHierarchy()\n");
}

FeatureHierarchy::~FeatureHierarchy()
{
}

void FeatureHierarchy::clear()
{
  mFeatures.clear();
  mParameter = 0;
  mMinPar = 10e34;
  mMaxPar = -10e34;
  mIndexMap.clear();
}

int FeatureHierarchy::initialize(const SimplificationHandle& handle)  {
  return initialize(handle, false, -1.0);
}

int FeatureHierarchy::initialize(const SimplificationHandle& handle, bool useInputParameter, FunctionType param)
{
  Feature::iterator it;
  std::vector<Feature>::iterator fIt;
  Data<LocalIndexType> indexMap;
  IndexHandle indexHandle;

  // First we make sure that we clear any old information
  clear();

  Feature::fileType(handle.fileType());

  handle.getRange(mMinPar,mMaxPar);
  mTimeIndex = handle.timeIndex();
  mTime = handle.time();


  FeatureData data(&mFeatures);
  handle.readData(data);

  indexHandle = handle.indexHandle();
  if (indexHandle.elementCount() > 0) {
    sterror(indexHandle.elementCount() != handle.elementCount(),"Invalid File. There should be the same number of indices as there are features.");

    indexMap.resize(indexHandle.elementCount());
    indexHandle.readData(indexMap);

    mIndexMap.clear();
    for (LocalIndexType i=0;i<handle.elementCount();i++)  {
      mFeatures[i].id(indexMap[i]);
      mIndexMap[indexMap[i]] = i;
    }

  }
  else {
    for (LocalIndexType i=0;i<handle.elementCount();i++)
      mFeatures[i].id(i);
  }



  // Now assemble the agent information. Iterate through all features
  for (fIt=mFeatures.begin();fIt!=mFeatures.end();fIt++) {

    // For all "roots" recursively assign agents
    if (fIt->representatives().empty())
      initializeAgent(&(*fIt));
  }

  // Double check that we have assigned all agents
  for (fIt=mFeatures.begin();fIt!=mFeatures.end();fIt++)
    sterror(fIt->agent()==NULL,"Found uninitialized agent for id %d.",fIt->id());


  // Finally, create the look-up structure to quickly find features
  if (!mFeatures[0].direction()) {
    // If we have a split tree
    mIntervalTree = new IntervalTreeSpecific<SplitTreeInterval>(mFeatures);
  }
  else {
    // Otherwise we assume we have a merge tree
    mIntervalTree = new IntervalTreeSpecific<MergeTreeInterval>(mFeatures);
  }


  return 1;
}




void FeatureHierarchy::initializeIndexMap(FILE* input, LocalIndexType feature_count, bool binary)
{
  sterror(mFeatures.size() != feature_count,"Number of features and number of indices should match.");

  LocalIndexType index;

  for (LocalIndexType i=0;i<feature_count;i++) {

    if (binary)
      fread(&index,sizeof(LocalIndexType),1,input);
    else
      fscanf(input,"%u",&index);

    mFeatures[i].id(index);

    mIndexMap[index] = i;
  }


}

/*
FunctionType FeatureHierarchy::minParameterWithCount(uint32_t count, uint32_t resolution)
{
  FunctionType org_p = parameter(); // Save the current parameter value
  FunctionType p = minParameter();
  //FunctionType delta = (maxParameter() - minParameter()) / resolution;
  std::vector<Feature*> sequence(mFeatures.size());
  LifeTimeCmp cmp(1);
  uint32_t i;

  for (i=0;i<mFeatures.size();i++)
    sequence[i] = &mFeatures[i];

  sort(sequence.begin(),sequence.end(),cmp);

  fprintf(stderr,"FeatureHierarchy::minParameterWithCount(%d, %d)\n",count,resolution);
  parameter(p); // Start at the very bottom which means the last count should
  // be considered zero

  // Check whether we already fulfill our criterion
  if (livingSize() > count) // If so we are done
    return p;

  // For resolution many intervals check whether we now have more than count
  // many features
  for (i=0;i<sequence.size();i++) {

    p = sequence[i]->lifeTime()[1];
    parameter(p);

    if (livingSize() > count) // If so we are done
      break;
  }

  // restore the initial state of the hierarchy
  parameter(org_p);

  if (i < sequence.size()) // If we stopped early
    return p; // We found a valid parameter
  else
    return minParameter(); // Otherwise, we return the default
}

FunctionType FeatureHierarchy::maxParameterWithCount(uint32_t count, uint32_t resolution)
{
  FunctionType org_p = parameter(); // Save the current parameter value
  FunctionType p = maxParameter();
  FunctionType delta = (maxParameter() - minParameter()) / resolution;
  uint32_t i;

  parameter(p); // Start at the very bottom which means the last count should
  // be considered zero

  // For resolution many intervals check whether we now have more than count
  // many features
  for (i=0;i<resolution;i++) {

    if (livingSize() >= count) // If so we are done
      break;

    p -= delta;
    parameter(p);
  }

  // restore the initial state of the hierarchy
  parameter(org_p);


  if (i < resolution) // If we stopped early
    return p; // We found a valid parameter
  else
    return maxParameter(); // Otherwise, we return the default
}
*/

FeatureHierarchy::LivingIterator FeatureHierarchy::beginLiving(FunctionType parameter) const
{
  return LivingIterator(parameter,mIntervalTree->begin(parameter),mIntervalTree->end(parameter));
}

FeatureHierarchy::ActiveIterator FeatureHierarchy::beginActive(FunctionType parameter)
{
  return ActiveIterator(beginLiving(parameter));
}

LocalIndexType FeatureHierarchy::livingSize(FunctionType parameter) const
{
  LivingIterator it = beginLiving(parameter);
  LocalIndexType count = 0;

  while (it != endLiving()) {
    count++;
    it++;
  }

  return count;
}

std::vector<Feature*> FeatureHierarchy::FeatureHierarchy::leafs()
{
  std::vector<Feature*> leafs;

  for (LocalIndexType i=0;i<mFeatures.size();i++) {
    if (mFeatures[i].conSize() == 0)
      leafs.push_back(&mFeatures[i]);
  }

  return leafs;
}

std::vector<Feature*> FeatureHierarchy::roots()
{
  std::vector<Feature*> roots;

  for (LocalIndexType i=0;i<mFeatures.size();i++) {
    if (mFeatures[i].repSize() == 0)
      roots.push_back(&mFeatures[i]);
  }

  return roots;
}


/*
int32_t FeatureHierarchy::hierarchyPropertyIndex(const std::string& property)
{
  for (uint32_t i=0;i<sPropertyCount;i++) {
    if (sPropertyNames[i] == property)
      return i;
  }

  return -1;
}


FunctionType FeatureHierarchy::hierarchyProperty(int32_t index, const std::vector<float>& parameters)
{
  switch (index) {
    case 0:
      return (FunctionType)mSubSelect.size();
    case 1:
      if (parameters.empty()) {
        stwarning("Hierarchy property %s needs at least one input <parameterValue>",sPropertyNames[index].c_str());
        return 0;
      }
      if (parameters.size() == 1)
        return (FunctionType)minParameterWithCount(round(parameters[0]));
      else
        return (FunctionType)minParameterWithCount(round(parameters[0]), round(parameters[1]));
    case 2:
      if (parameters.empty()) {
        stwarning("Hierarchy property %s needs at least one input <parameterValue>",sPropertyNames[index].c_str());
        return 0;
      }
      if (parameters.size() == 1)
        return (FunctionType)maxParameterWithCount(round(parameters[0]));
      else
        return (FunctionType)maxParameterWithCount(round(parameters[0]), round(parameters[1]));
    default:
      stwarning("Hierarchy property index %u out of range [0,%u]",index,sPropertyCount);
      return 0;
  }

  return 0;
}
*/


void FeatureHierarchy::initializeAgent(Feature* f)
{
  Feature::iterator it;
  std::stack<Feature*> front;
  Feature* top;

  front.push(f);

  while (!front.empty()) {

    top = front.top();

    if (top->constituents().empty()) { // If I have not constituent
      top->agent(top); // I am my own agent
      front.pop(); // And I don't need to look at this feature again
      continue;
    }

    // Now check the agents of all constituents
    for (it=top->constituents().begin();it!=top->constituents().end();it++) {
      if ((*it)->agent() == NULL)
        front.push(*it);
    }

    // If all my constituents have an agent
    if (front.top() == top) {
      // This feature is done
      front.pop();

      // And we pick our actual agent from those of our constituents
      for (it=top->constituents().begin();it!=top->constituents().end();it++) {

        if ((top->agent() == NULL)  // If this is the first constituent
            || (top->direction() && (top->agent()->lifeTime()[0] > (*it)->agent()->lifeTime()[0])) // or if it provides a better one
            || (!top->direction() && (top->agent()->lifeTime()[1] < (*it)->agent()->lifeTime()[1]))) {
          top->agent((*it)->agent());
        }
      }
    }
  }
}

LocalIndexType FeatureHierarchy::mappedIndex(LocalIndexType i)
{
  if (mIndexMap.empty())
    return i;

  std::map<LocalIndexType,LocalIndexType>::iterator it = mIndexMap.find(i);
  sterror(it==mIndexMap.end(),"Index - %d not found in index map .", i);
  return it->second;
}

}

