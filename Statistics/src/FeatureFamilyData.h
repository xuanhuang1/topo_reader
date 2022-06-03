#ifndef FEATUREFAMILYDATA_H
#define FEATUREFAMILYDATA_H

#include "FamilyHandle.h"
#include "FeatureHierarchy.h"
#include "Attribute.h"
#include "AggregatorFactory.h"
#include "Range.h"
#include "StatisticsDefinitions.h"
#include "AttributeVariableList.h"
#include <iostream>
#include <fstream>


using namespace TopologyFileFormat;
namespace Statistics {


class FeatureFamilyData
{
  public:
  FeatureFamilyData() { };
  ~FeatureFamilyData();

  bool initializeFamily(const FamilyHandle &reader, bool useThreshold, FunctionType threshold, const LocalIndexType familyID=0);
  bool loadAttributeVariables(const StatHandle &reader, LocalIndexType attributeID, Range &range, LocalIndexType simpID);
  bool makeAccessors(LocalIndexType attributeID, LocalIndexType derivedAttributeID, const std::string &attribute, const std::string &derivedAttribute, Range &range, LocalIndexType simpID);
  bool unloadAttributeVariables(LocalIndexType attributeID);
  void updateParameter(double parameter, LocalIndexType simpID);
  void accumulateAttributes(LocalIndexType attributeID, LocalIndexType simpID);
  void getFilteredFeatures(LocalIndexType plotAttributeID, std::map<LocalIndexType, Range > &filters,  std::vector<FunctionType> &features, LocalIndexType simpID);
  void filterFeatures(LocalIndexType plotAttributeID, std::map<LocalIndexType, Range > &filters,  LocalIndexType simpID);
  void writeSelectedFeaturesToFile(LocalIndexType plotAttributeID, std::vector< Range > &filters,  const std::string &filename, LocalIndexType simpID);
  FunctionType getParameter(LocalIndexType simpID) const { return mHierarchies[simpID]->parameter(); }
  void clearSubSelectedIDs();
  void writeSubSelectedIDsToFile(const std::string &filename, const LocalIndexType simpID);
  void getObservations(LocalIndexType simpID, std::map<LocalIndexType, std::vector<FunctionType> > & observations) const;
  FeatureHierarchy * getHierarchy(LocalIndexType simpID) const { return mHierarchies[simpID]; }
  FunctionType getAggregatedAttributes(LocalIndexType attributeID, LocalIndexType featureID, LocalIndexType simpID) const;

  public:
  // one attribute array per attribute-variable pair
  // index is the id into global attribute variable list
  std::map<LocalIndexType, Attribute*> mAttributes;
 
  // aggregated attributes
  std::map<LocalIndexType, Attribute*> mAggregatedAttributes;

  std::map<LocalIndexType, bool> mIsAccessor;
 
  // allow for one Hierarchy per simplificaton sequence
  // although right now there is only one per family in practice
  std::vector<FeatureHierarchy *> mHierarchies;


};


};
#endif

