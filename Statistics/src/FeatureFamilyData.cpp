#include "FeatureFamilyData.h"
namespace Statistics {



FeatureFamilyData::~FeatureFamilyData() {
  while(mAttributes.size()) {
    unloadAttributeVariables(mAttributes.begin()->first);
  }
  for(LocalIndexType simpID = 0; simpID < mHierarchies.size(); simpID++) {
    delete mHierarchies[simpID];
  }
}


bool FeatureFamilyData::initializeFamily(const FamilyHandle &reader, bool useThreshold, FunctionType threshold, const LocalIndexType familyID) {
  // initialize hierarchies
  std::cout << "initializing hierarchies, numSimp = "  << reader.numSimplifications() << std::endl;
  for(LocalIndexType simpID = 0; simpID < reader.numSimplifications(); simpID++) {
     FeatureHierarchy *hierarchy = new FeatureHierarchy();

     hierarchy->initialize(reader.simplification(simpID), useThreshold, threshold);

     mHierarchies.push_back(hierarchy);
  }

  return true;
}



bool FeatureFamilyData::loadAttributeVariables(const StatHandle &reader, LocalIndexType attributeID,
                                                            Range &range, LocalIndexType simpID)
{

  Factory factory;
  Attribute* attributes = factory.make_aggregator_array(reader.stat(), reader.aggregated());

  attributes->resize(reader.elementCount());
  // Read the data from file
  reader.readData(attributes);
  //std::cout << "done reading and attributes.size() = " << attributes->size() << std::endl;
  // Update the global range
  for(LocalIndexType j=0; j < reader.elementCount(); j++)
    //range.UpdateRange((*attributes)[j].value());
    range.UpdateRange(attributes->value(j));

  attributes->preAggregated(reader.aggregated());
  //std::cout << "attributes->aggregated = " << attributes->preAggregated() << std::endl;
  mAttributes[attributeID] = attributes;
  mIsAccessor[attributeID] = false;

  if(reader.aggregated()) {
    //std::cout << "no accum reader->aggregated = " << reader.aggregated() << std::endl;
    mAggregatedAttributes[attributeID] = mAttributes[attributeID];
  } else {
    //std::cout << "accum reader->aggregated = " << reader.aggregated() << std::endl;
    mAggregatedAttributes[attributeID] = factory.make_aggregator_map(reader.stat(), reader.aggregated());
    accumulateAttributes(attributeID, simpID);
  }


  return true;

}


bool FeatureFamilyData::makeAccessors(LocalIndexType baseAttributeID, LocalIndexType derivedAttributeID,
                                                    const std::string &baseAttribute, const std::string &derivedAttribute,
                                                    Range &range, LocalIndexType simpID)
{


  Factory factory;
  Attribute* attributes = factory.make_accessor_array(mAttributes[derivedAttributeID], baseAttribute.c_str(), derivedAttribute.c_str());

  for(LocalIndexType j=0; j < attributes->size(); j++) {
    range.UpdateRange((*attributes)[j].value());
  }

  attributes->preAggregated(mAttributes[derivedAttributeID]->preAggregated());
  mAttributes[baseAttributeID] = attributes;
  mIsAccessor[baseAttributeID] = true;

  if(mAttributes[baseAttributeID]->preAggregated()) {
    mAggregatedAttributes[baseAttributeID] = mAttributes[baseAttributeID];
  } else {
    mAggregatedAttributes[baseAttributeID] = factory.make_accessor_map(mAggregatedAttributes[derivedAttributeID], baseAttribute.c_str(), derivedAttribute.c_str());
  }

  return true;
}


bool FeatureFamilyData::unloadAttributeVariables(LocalIndexType attributeID)
{
  std::cout << "mAggregatedAttributes.size() == " << mAggregatedAttributes.size() << std::endl;
  std::cout << "mAttributes.size() == " << mAttributes.size() << std::endl;
  if(!(mAggregatedAttributes[attributeID]->preAggregated())) {
    std::cout << "deleting aggregator" << std::endl;
    delete mAggregatedAttributes[attributeID];
  }
  mAggregatedAttributes.erase(attributeID);

  delete mAttributes[attributeID];
  mAttributes.erase(attributeID);

  mIsAccessor.erase(attributeID);

  return true;
}


void FeatureFamilyData::accumulateAttributes(LocalIndexType attributeID, LocalIndexType simpID) {

  //std::cout << "mAggregatedAttributes[attributeID]->preAggregated()
  // skip if already accumulated or an accessor
  if(!mAggregatedAttributes[attributeID]->preAggregated() && !mIsAccessor[attributeID]) {
    // clear
    mAggregatedAttributes[attributeID]->clear();

    // go through active iterators to accumulate
    for(FeatureHierarchy::ActiveIterator activeItr=mHierarchies[simpID]->beginActive(); activeItr!=mHierarchies[simpID]->endActive(); activeItr++) {
      LocalIndexType mappedID = mHierarchies[simpID]->mappedIndex(activeItr->id());
      LocalIndexType mappedRepID = mHierarchies[simpID]->mappedIndex((activeItr.rep())->id());

      mAggregatedAttributes[attributeID]->addSegment(mappedRepID, mappedID);
    }
  }

}


void FeatureFamilyData::updateParameter(double param, LocalIndexType simpID) {
  // update hierarchy
  std::cout << "ffam update param " << param << " on simpid = " << simpID << " of " << mHierarchies.size() << std::endl;
  mHierarchies[simpID]->parameter(param);

  // get accumulated aggregators

  for(std::map<LocalIndexType, Attribute*>::iterator funcItr = mAttributes.begin(); funcItr != mAttributes.end(); funcItr++) {
    accumulateAttributes(funcItr->first, simpID);
  }
}


void FeatureFamilyData::getFilteredFeatures(LocalIndexType plotAttributeID, std::map<LocalIndexType, Range > &filters,  std::vector<FunctionType> &features, LocalIndexType simpID) {
  LocalIndexType filterID;
  LocalIndexType localFeatureID;

  //std::cout << "in get filtered feature!" << std::endl;
  //std::cout << "simpID = " << simpID << std::endl;
  //std::cout << "mHierarchies.size() = " << mHierarchies.size() << std::endl;
  std::vector<Feature *> subselected;
  for(FeatureHierarchy::LivingIterator liveItr=mHierarchies[simpID]->beginLiving(); liveItr!=mHierarchies[simpID]->endLiving(); liveItr++) {
   // std::cout << "blerg "<< std::endl;
    //std::cout << "on feature " << (*liveItr)->id() << std::endl;
    localFeatureID = mHierarchies[simpID]->mappedIndex((*liveItr)->id());
    //std::cout << "localFeatureID = " << localFeatureID << std::endl;
    bool containedInAllFilters = true;
    //std::cout << "there are " << filters.size() << " filters" << std::endl;
    for(std::map<LocalIndexType, Range >::iterator filterItr = filters.begin(); filterItr != filters.end(); filterItr++) {
     // std::cout << "checking filter ";
      filterID = filterItr->first;
      //std::cout << filterID << std::endl;
      //std::cout << "aggAttr.size = " << mAggregatedAttributes.size() << std::endl;
      //std::cout << "aggAttr[filterID].size = " << mAggregatedAttributes[filterID]->size() << std::endl;
      //std::cout << "accessing aggattr[filterID][localFeature] = " << std::endl;
      //(*mAggregatedAttributes[filterID])[localFeatureID];
      //std::cout << "aggattr[filterID][localFeature] = " << (*mAggregatedAttributes[filterID])[localFeatureID].value() << std::endl;
      if(!(filterItr->second).Contains((*mAggregatedAttributes[filterID])[localFeatureID].value())) {
        containedInAllFilters = false;
        break;
      }
    }
    if(containedInAllFilters) {
      //std::cout << "contained in all filters!" << std::endl;
      features.push_back((*mAggregatedAttributes[plotAttributeID])[localFeatureID].value());
      //std::cout << "features.back = " << features.back() << std::endl;
      subselected.push_back(mHierarchies[simpID]->feature(localFeatureID));
    }
  }
  //std::cout << "there are " << subselected.size() << " subselected features" << std::endl;
  mHierarchies[simpID]->subselect(subselected);
}

//this is useful in plot selection when you don't want to change the hierachy

void FeatureFamilyData::writeSelectedFeaturesToFile(LocalIndexType plotAttributeID, std::vector< Range > &filters,  const std::string &filename, LocalIndexType simpID) {
  LocalIndexType localFeatureID;

  std::vector<Feature *> initialList = mHierarchies[simpID]->getSubselect(), subSelected;

  for(uint32_t i=0; i < initialList.size(); i++) {
    localFeatureID = mHierarchies[simpID]->mappedIndex(initialList[i]->id());
    for(uint32_t j=0; j < filters.size(); j++) {
      if(filters[j].Contains((*mAggregatedAttributes[plotAttributeID])[localFeatureID].value())) {
        subSelected.push_back(initialList[i]);
      }
    }
  }
  mHierarchies[simpID]->writeSubselectedToFile(filename.c_str(), subSelected);

}




void FeatureFamilyData::filterFeatures(LocalIndexType plotAttributeID, std::map<LocalIndexType, Range > &filters,  LocalIndexType simpID) {
  LocalIndexType filterID;
  LocalIndexType localFeatureID;

  // If there are no filters we simply select all living features.
  if (filters.empty()) {
    mHierarchies[simpID]->inverseSubselect();
  }
  else {
    std::vector<Feature *> subselected;
    for(FeatureHierarchy::LivingIterator liveItr=mHierarchies[simpID]->beginLiving(); liveItr!=mHierarchies[simpID]->endLiving(); liveItr++) {

      localFeatureID = mHierarchies[simpID]->mappedIndex((*liveItr)->id());
      bool containedInAllFilters = true;
      for(std::map<LocalIndexType, Range >::iterator filterItr = filters.begin(); filterItr != filters.end(); filterItr++) {
        filterID = filterItr->first;
        if(!(filterItr->second).Contains((*mAggregatedAttributes[filterID])[localFeatureID].value())) {
          containedInAllFilters = false;
          break;
        }
      }
      if(containedInAllFilters) {
        subselected.push_back(mHierarchies[simpID]->feature(localFeatureID));
      }
    }
    //std::cout << "there are " << subselected.size() << " subselected features" << std::endl;
    mHierarchies[simpID]->subselect(subselected);
  }
}


void FeatureFamilyData::clearSubSelectedIDs() {
  for(LocalIndexType i=0; i < mHierarchies.size(); i++) {
    mHierarchies[i]->initializeSubselect();
  }
}


void FeatureFamilyData::writeSubSelectedIDsToFile(const std::string &filename, const LocalIndexType simpID) {
  mHierarchies[simpID]->writeSubselectedToFile(filename.c_str());
}


FunctionType FeatureFamilyData::getAggregatedAttributes(LocalIndexType attributeID, LocalIndexType featureID, LocalIndexType simpID) const {
  std::map<LocalIndexType, Attribute* >::const_iterator mItr;
  if((mItr = mAggregatedAttributes.find(attributeID)) == mAggregatedAttributes.end()) {
    sterror(true, "trying to find an aggregator for a attributeID that is not loaded %d", attributeID);
  }
  LocalIndexType id = mHierarchies[simpID]->mappedIndex(featureID);
  return (*mItr->second)[id].value();
}


void FeatureFamilyData::getObservations(LocalIndexType simpID, std::map<LocalIndexType, std::vector<FunctionType> > & observations) const {
  LocalIndexType localFeatureID;
  for(FeatureHierarchy::LivingIterator liveItr=mHierarchies[simpID]->beginLiving(); liveItr!=mHierarchies[simpID]->endLiving(); liveItr++) {
    localFeatureID = mHierarchies[simpID]->mappedIndex((*liveItr)->id());
    std::map<LocalIndexType, Attribute* >::const_iterator itr;
    std::vector<FunctionType> values;
    for(itr=mAggregatedAttributes.begin(); itr != mAggregatedAttributes.end(); itr++) {
      values.push_back(getAggregatedAttributes(itr->first, localFeatureID, simpID));
    }
    observations.insert(std::map<LocalIndexType, std::vector<FunctionType> >::value_type(localFeatureID, values));
  }
}


}
