/*
 * StatisticsDB.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: bremer5
 */

#include "StatisticsDB.h"

namespace Statistics {

using namespace TopologyFileFormat;

StatisticsDB::StatisticsDB() : mStat(""), mSpecies("")
{
}

StatisticsDB::~StatisticsDB()
{
  DataMap::iterator it;
  DataCollection::iterator dIt;

  for (it=mData.begin();it!=mData.end();it++) {
    for (dIt=it->second.begin();dIt!=it->second.end();dIt++) {
      delete dIt->second;
    }
  }

  HierarchyMap::iterator hIt;
  for (hIt=mHierarchies.begin();hIt!=mHierarchies.end();hIt++) {
    delete hIt->second;
  }
}

int StatisticsDB::initialize(const std::vector<std::string>& files)
{

  mHandles.initialize(files);

  sterror(mHandles.numClans()!=1,"Currently we only deal with one clan instead of %d.",mHandles.numClans());

  std::vector<ClanKey> clans;
  std::vector<FamilyKey> families;
  std::vector<FamilyKey>::iterator fIt;
  HierarchyMap::iterator hIt;

  mHandles.getClanKeys(clans);
  mHandles.getFamilyKeys(clans[0],families);

  for (fIt=families.begin();fIt!=families.end();fIt++) {
    mData[*fIt] = DataCollection();

    FeatureHierarchy* hierarchy = new FeatureHierarchy();
    hierarchy->initialize(mHandles.getFamilyHandle(*fIt).simplification(0));
    mHierarchies[*fIt] = hierarchy;

    mTime[mHandles.getFamilyHandle(*fIt).timeIndex()] = *fIt;
  }

  return 1;
}

int StatisticsDB::loadAttribute(const std::string& type_name, const std::string& attribute_name)
{
  FamilyHandle family;
  Attribute* data;
  Factory fac;
  StatMapKey key(type_name,attribute_name);

  mStat = type_name;
  mSpecies = attribute_name;

  DataMap::iterator fIt;

  // For all families
  for (fIt=mData.begin();fIt!=mData.end();fIt++) {
    family = mHandles.getFamilyHandle(fIt->first);

    if (!family.providesAggregate(mStat,mSpecies)) {
      fprintf(stderr,"Family %d does not provide the requested aggregate (%s,%s).\n",family.timeIndex(),
              type_name.c_str(),attribute_name.c_str());
      continue;
    }

    if (fIt->second.find(key) == fIt->second.end()) {

      data = fac.make_aggregator_array(mStat.c_str(),true);
      data->resize(family.aggregate(mStat,mSpecies).elementCount());
      family.aggregate(mStat,mSpecies).readData(data);

      fIt->second[key] = data;
    }
  }
  return 1;
}

int StatisticsDB::selectAttribute(const std::string& type_name, const std::string& attribute_name)
{
  mStat = type_name;
  mSpecies = attribute_name;

  return 1;
}

void StatisticsDB::getValues(double parameter, std::vector<TimeStep>* values, const std::string aggregator)
{
  DataMap::iterator fIt;
  DataCollection::iterator dIt;
  FeatureHierarchy *hierarchy;
  Factory fac;
  Attribute* agg = NULL;
  Attribute* data = NULL;
  StatMapKey key(mStat,mSpecies);

  if (aggregator != "")
    agg = fac.make_aggregator(aggregator);

  for (fIt=mData.begin();fIt!=mData.end();fIt++) {
    hierarchy = mHierarchies.find(fIt->first)->second;
    dIt = fIt->second.find(key);
    if (dIt == fIt->second.end()) {
      fprintf(stderr,"Family %d does not contain the requested data.\n",fIt->first.timeIndex());
      continue;
    }

    data = dIt->second;

    FeatureHierarchy::LivingIterator it;
    values->push_back(TimeStep());

    values->back().mTime = mHandles.getFamilyHandle(fIt->first).time();

    if (agg != NULL) {
      agg->reset();

      // For all the currently active features
      for (it=hierarchy->beginLiving(parameter);it!=hierarchy->endLiving();it++) {
        //fprintf(stderr,"Found feature with value %e\n",(*data)[(*it)->id()].value());
        agg->addVertex((*data)[(*it)->id()].value(),0);
      }

      values->back().mValues.push_back(agg->value());
    }
    else {
      //fprintf(stderr,"Time step %e with %d features at p = %e\n",values->back().mTime,hierarchy->livingSize(),parameter);
      for (it=hierarchy->beginLiving(parameter);it!=hierarchy->endLiving();it++) {
        //fprintf(stderr,"\tFound feature with value %e\n",(*data)[(*it)->id()].value());
        values->back().mValues.push_back((*data)[(*it)->id()].value());
        values->back().mIds.push_back((*it)->id());
      }
    }
  }

  std::sort(values->begin(),values->end());
}

double StatisticsDB::value(TopologyFileFormat::TimeIndexType time, GlobalIndexType id,
                           const std::string& type_name, const std::string& attribute_name) const
{
  TimeMap::const_iterator tIt;

  tIt = mTime.find(time);

  if (tIt == mTime.end()) {
    fprintf(stderr,"Database does not contain time index %d.\n",time);
    return 0;
  }

  DataMap::const_iterator dIt;

  dIt = mData.find(tIt->second);

  if (dIt == mData.end()) {
    fprintf(stderr,"Could not find family for time index %d.\n",time);
    return 0;
  }

  StatMapKey key;
  if ((type_name == "") || (attribute_name == ""))
    key = StatMapKey(mStat,mSpecies);
  else
    key = StatMapKey(type_name,attribute_name);


  DataCollection::const_iterator cIt;

  cIt = dIt->second.find(key);
  if (cIt == dIt->second.end()) {
    fprintf(stderr,"Family %d does not contain the requested data.\n",time);
    return 0;
  }

  return cIt->second->value(id);
}

void StatisticsDB::getSegment(double parameter, TopologyFileFormat::TimeIndexType time, std::vector<std::vector<GlobalIndexType> >* segments)
{
  TimeMap::const_iterator tIt;
  FeatureHierarchy *hierarchy;
  FeatureSegmentation segmentation;
  FamilyHandle family;


  tIt = mTime.find(time);

  if (tIt == mTime.end()) {
    fprintf(stderr,"Database does not contain time index %d.\n",time);
    return;
  }

  // The hierarchy is pre-loaded
  hierarchy = mHierarchies.find(tIt->second)->second;

  // The segmentation is not so we have to load it

  family = mHandles.getFamilyHandle(tIt->second);
  if (!family.providesSegmentation()) {
    fprintf(stderr,"The given feature family does not provide a segmentation.\n");
    return;
  }

  // Initialize the segmentation from the handle
  segmentation.initialize(family.segmentation());


  FeatureHierarchy::LivingIterator it;

  // Walk through all living features at the given parameter and collect the indices
  for (it=hierarchy->beginLiving(parameter);it!=hierarchy->endLiving();it++) {
    //fprintf(stderr,"\tFound feature with value %e\n",(*data)[(*it)->id()].value());

    // Create the next segment
    segments->push_back(std::vector<GlobalIndexType>());

    segmentation.segmentation(*it,segments->back());
  }
}



}
