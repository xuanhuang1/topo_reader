/*
 * HandleCollection.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: bremer5
 */

#include <algorithm>

#include "HandleCollection.h"

namespace TopologyFileFormat {

HandleCollection::HandleCollection()
{
  mLow[0] = mLow[1] = mLow[2] = 0;
  mHigh[0] = mHigh[1] = mHigh[2] = 1;
}


int HandleCollection::initialize(const std::vector<std::string> &filenames)
{
  mClanMap.clear();
  mDatasets.clear();
  mVariables.clear();
  mMetrics.clear();
  mStatistics.clear();

  for (uint32_t i=0;i<filenames.size();i++) {
    initialize(filenames[i].c_str());
  }

  return 1;
}

int HandleCollection::initialize(const char* files[], int count)
{
  mClanMap.clear();
  mDatasets.clear();
  mVariables.clear();
  mMetrics.clear();
  mStatistics.clear();

  for (int i=0;i<count;i++) {
    initialize(files[i]);
  }

  return 1;
}

int HandleCollection::initialize(const char* file)
{
  ClanHandle handle;
  ClanMapType::iterator cIt;

  if (handle.attach(file)) {

    ClanKey key(handle.dataset());

    cIt = mClanMap.find(key);
    if (cIt == mClanMap.end())
      cIt = mClanMap.insert(std::pair<ClanKey,Clan>(key,Clan(key))).first;

    mDatasets.insert(handle.dataset());
    insertClan(cIt->second,handle, file);
  }

  return 1;
}


int HandleCollection::insertClan(Clan& c, ClanHandle& handle, const char* file)
{
  FamilyMapType::iterator fIt;

  //std::cout << "inserting handle into clanMap" << std::endl;
  for (uint32_t i=0;i<handle.numFamilies();i++) {
    FamilyHandle family = handle.family(i);

    FamilyKey f_key(family.variableName(), family.timeIndex(),
                    ClanKey(handle.dataset()));

    mVariables.insert(family.variableName());
    if(mFilenames[f_key]==NULL)
      mFilenames[f_key]=file;
    fIt = c.families.find(f_key);
    if (fIt == c.families.end()) {
     // std::cout << "setting family key " << std::endl;
      c.families[f_key] = family;
    }
    else {
      c.families[f_key].add(family);
    }

    // For all the sipmlifications we want to add the metric
    for (int i=0;i<family.numSimplifications();i++) {
      mMetrics.insert(family.simplification(i).metric());
    }

    for (int i=0;i<family.numAggregates();i++) {
      mStatistics.insert(std::pair<std::string,std::string>(family.aggregate(i).stat(),family.aggregate(i).species()));
    }

    // If this family had a segmentation we want to update our bounding box
    if (family.providesSegmentation()) {
      std::stringstream input(family.segmentation().domainDescription(),std::ios_base::in);

      // Decode the bounding box
      switch (family.segmentation().domainType()) {
        case TopologyFileFormat::REGULAR_GRID:

          uint32_t dim[4];

          input >> dim[0];

          if (dim[0] > 0)  {
            input >> dim[1];
            mHigh[0] = std::max((double)dim[1],mHigh[0]);
            mLow[0] = std::min(0.0,mLow[0]);
          }

          if (dim[0] > 1)  {
            input >> dim[2];
            mHigh[1] = std::max((double)dim[2],mHigh[1]);
            mLow[1] = std::min(0.0,mLow[1]);
          }

          if (dim[0] > 2)  {
            input >> dim[3];
            mHigh[2] = std::max((double)dim[3],mHigh[2]);
            mLow[2] = std::min(0.0,mLow[2]);
          }
          break;

        case TopologyFileFormat::POINT_SET:
          double v;

          uint32_t count;

          input >> count;


          for (int i=0;i<count;i++) {
            input >> v;
            mLow[i] = std::min(mLow[i],v);
            input >> v;
            mHigh[i] = std::max(mHigh[i],v);
          }
          break;

        default:
          break;
      }
    }
  }

  AssociationMapType::iterator aIt;

  for (uint32_t i=0;i<handle.numAssociations();i++) {
    //std::cout << "on assocation " << i <<  std::endl;
    AssociationKey a_key(handle.association(i).source(),
                         handle.association(i).destination());

    aIt = c.associations.find(a_key);
    if (aIt == c.associations.end()) {
      c.associations[a_key] = handle.association(i);
    } 
  }

  return 1;
}

int HandleCollection::numFamilies(const ClanKey& key ) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(key);
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  return cIt->second.families.size();
}

int HandleCollection::numAssociations(const ClanKey& key ) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(key);
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  return cIt->second.associations.size();
}
    
void HandleCollection::boundingBox(double low[3], double high[3]) const
{
  memcpy(low,mLow,3*sizeof(double));
  memcpy(high,mHigh,3*sizeof(double));
}


void HandleCollection::getClanKeys(std::vector<ClanKey> &keys) const
{
  for(ClanMapType::const_iterator cIt=mClanMap.begin(); cIt != mClanMap.end(); cIt++) {
    keys.push_back(cIt->first);
  }
}

void HandleCollection::getFamilyKeys(const ClanKey& cKey, std::vector<FamilyKey> &fKeys) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(cKey);
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  for(FamilyMapType::const_iterator fIt=cIt->second.families.begin(); fIt != cIt->second.families.end(); fIt++)
  {
    fKeys.push_back(fIt->first);
  }
}

void HandleCollection::getFamilyHandles(const ClanKey& cKey, std::vector<FamilyHandle> &fHandles) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(cKey);
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  for(FamilyMapType::const_iterator fIt=cIt->second.families.begin(); fIt != cIt->second.families.end(); fIt++)
  {
    fHandles.push_back(fIt->second);
  }
}


void HandleCollection::getAssociationKeys(const ClanKey& cKey, std::vector<AssociationKey> &aKeys) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(cKey);
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  for(AssociationMapType::const_iterator aIt=cIt->second.associations.begin(); aIt != cIt->second.associations.end(); aIt++)
  {
    aKeys.push_back(aIt->first);
  }
}

FamilyHandle HandleCollection::findFamilyHandle(const FamilyKey &fKey) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(fKey.clanKey());
  if (cIt == mClanMap.end())
    return FamilyHandle();
  
  FamilyMapType::const_iterator fIt = cIt->second.families.find(fKey);
  if (fIt == cIt->second.families.end())
    return FamilyHandle();
  
  return fIt->second;
}

const FamilyHandle& HandleCollection::getFamilyHandle(const FamilyKey &fKey) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(fKey.clanKey());
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  FamilyMapType::const_iterator fIt = cIt->second.families.find(fKey);
  sterror(fIt == cIt->second.families.end(), "FamilyKey not contained in familyMap");
  return fIt->second;
}

AssociationHandle HandleCollection::findAssociationHandle(const AssociationKey &aKey) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(aKey.clanKey());
  if (cIt == mClanMap.end())
    return AssociationHandle();
  
  AssociationMapType::const_iterator aIt = cIt->second.associations.find(aKey);
  if (aIt == cIt->second.associations.end())
    return AssociationHandle();
  
  return aIt->second;

}

const AssociationHandle& HandleCollection::getAssociationHandle(const AssociationKey &aKey) const
{
  ClanMapType::const_iterator cIt = mClanMap.find(aKey.clanKey());
  sterror(cIt == mClanMap.end(), "ClanKey not contained in clanMap");
  AssociationMapType::const_iterator aIt = cIt->second.associations.find(aKey);
  sterror(aIt == cIt->second.associations.end(), "AssociationKey not contained in AssociationMap");
  return aIt->second;
}

TimeIndexType HandleCollection::getFirstTime(const ClanKey &key, const std::string &varName) const
{
  TimeIndexType time=0;
  bool first = true;
  ClanMapType::const_iterator cIt = mClanMap.find(key);
  for(FamilyMapType::const_iterator fIt = cIt->second.families.begin(); fIt != cIt->second.families.end(); fIt++) {
    if(fIt->first.variableName() == varName) {
      if(first) {
        time = fIt->first.timeIndex();
        first = false;
      }
      else if(time > fIt->first.timeIndex()) time = fIt->first.timeIndex();
    }
  }
  return time;
}
  
TimeIndexType HandleCollection::getEndTime(const ClanKey &key, const std::string &varName) const
{
  TimeIndexType time=0;
  bool first = true;
  ClanMapType::const_iterator cIt = mClanMap.find(key);
  for(FamilyMapType::const_iterator fIt = cIt->second.families.begin(); fIt != cIt->second.families.end(); fIt++) {
    if(fIt->first.variableName() == varName) {
      if(first) {
        time = fIt->first.timeIndex();
        first = false;
      }
      else if(time < fIt->first.timeIndex()) time = fIt->first.timeIndex();
    }
  }
  return time;
}

const char* HandleCollection::getFileFromFamily(const FamilyKey fKey){
  return mFilenames[fKey];
}

}
