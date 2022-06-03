/*
 * TalassDB.h
 *
 *  Created on: Nov 20, 2013
 *      Author: bremer5
 */

#ifndef STATISTICSDB_H_
#define STATISTICSDB_H_


#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <cmath>
#include <stack>
#include <algorithm>
#include <map>


#include "Value.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"
#include "ValueElement.h"
#include "HandleCollection.h"
#include "StatHandle.h"

namespace Statistics {

struct TimeStep {
  double mTime;
  std::vector<double> mValues;
  std::vector<int> mIds;

  bool operator<(const TimeStep& step) const {return mTime < step.mTime;}
};

class StatisticsDB
{
public:

  //! Typedef for a data collection as pair of StatKeys,Attribute*
  typedef std::map<TopologyFileFormat::StatMapKey,Attribute*> DataCollection;

  //! Typedef for the family to data collection map
  typedef std::map<TopologyFileFormat::FamilyKey,DataCollection> DataMap;

  //! Typedef for the family to hierarchy map
  typedef std::map<TopologyFileFormat::FamilyKey,TopologyFileFormat::FeatureHierarchy*> HierarchyMap;

  //! Typedef for the time-step to FamilyKey Map
  typedef std::map<TopologyFileFormat::TimeIndexType, TopologyFileFormat::FamilyKey> TimeMap;

  //! Default constructor
  StatisticsDB();

  //! Destructor
  ~StatisticsDB();

  //! Initialize the DB with a collection of files
  int initialize(const std::vector<std::string>& files);

  //! Load a particular attribute into memory
  int loadAttribute(const std::string& type_name, const std::string& attribute_name);

  int selectAttribute(const std::string& type_name, const std::string& attribute_name);

  //! Get a list of values per-timestep potentially aggregated
  void getValues(double parameter, std::vector<TimeStep>* values, const std::string aggregator = "");

  //! Return the attribute values for the given time step, type, and attribute_name
  /*! Return the attribute values for the given time step, type, and attribute_name.
   *  If the type and/or the attribute_name are empty the currently loaded values
   *  will be used.
   *
   * @param time Time step
   * @param id Feature id
   * @param type_name Aggregator name
   * @param attribute_name attribute name
   * @return the value of this attribute
   */
  double value(TopologyFileFormat::TimeIndexType time, GlobalIndexType id,
               const std::string& type_name = "", const std::string& attribute_name = "") const;

  //! Return a list of segmentation indices for all segments currently active
  /*! Return a list of indices for all features alive at the current threshold
   * @param parameter The current simplification threshold
   * @param time The time step in question
   * @param segments A vector of vectors with vertex ids for all features
   */
  void getSegment(double parameter, TopologyFileFormat::TimeIndexType time, std::vector<std::vector<GlobalIndexType> >* segments);


private:

  //! The HandleCollection corresponding to the current list of files
  TopologyFileFormat::HandleCollection mHandles;

  //! Currently selected statistics type
  std::string mStat;

  //! Currently selected species name
  std::string mSpecies;

  //! The map from all family keys to the data collection
  DataMap mData;

  //! The map from all family keys to the hierachies
  HierarchyMap mHierarchies;

  //! The map from time index to family key
  TimeMap mTime;

};

} // End of namespace Statistics

#endif /* TALASSDB_H_ */
