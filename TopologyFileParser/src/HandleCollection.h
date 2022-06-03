/*
 * HandleCollection.h
 *
 *  Created on: Feb 26, 2012
 *      Author: bremer5
 */

#ifndef HANDLECOLLECTION_H_
#define HANDLECOLLECTION_H_

#include <map>
#include <string>
#include <set>
#include "HandleKeys.h"
#include "ClanKey.h"
#include "FamilyKey.h"
#include "AssociationKey.h"
#include "ClanHandle.h"

namespace TopologyFileFormat {

//! The map type to find a family in a clan
typedef std::map<FamilyKey, FamilyHandle> FamilyMapType;

//! The map type to find an association in a clan
typedef std::map<AssociationKey, AssociationHandle> AssociationMapType;

//! A class to read in the meta information of a collection of files
/*! A HandleCollection reads and maintains the meta information from a
 *  collection of files. It will aggregate information from multiple
 *  files into a unified look-up structure. This means that handles from
 *  different physical files will end up grouped in the same clan and
 *  family handles. For reading the information that is unproblematic.
 *  However, care should be taken when appending data to handles in a
 *  collection.
 *
 *  So far there is no checking for adding the identical handle multiple
 *  times.
 */
class HandleCollection
{
private:

  class Clan;
public:


  //! Constructor
  HandleCollection();

  //! Destructor
  ~HandleCollection() {}

  //! Initialize the collection from a set of file names
  int initialize(const char* files[], int count);

  //! Initialize the collection from a set of file names
  int initialize(const std::vector<std::string> &filenames);

  //! insert a clan into the clan map
  int insertClan(Clan& c, ClanHandle& handle, const char* fn);

  //! return the number of clans
  int numClans() const { return mClanMap.size(); }
  
  //! return the number of families
  int numFamilies(const ClanKey& key ) const;

  //! return the number of associations
  int numAssociations(const ClanKey& key ) const;

  //! Return the current bounding box
  void boundingBox(double low[3], double high[3]) const;

  //! get a vector of the ClanKeys in the clanMap
  void getClanKeys(std::vector<ClanKey> &keys) const;

  //! get a vector of the FamilyKeys associated with a particular clan
  void getFamilyKeys(const ClanKey& cKey, std::vector<FamilyKey> &fKeys) const;

  //! get a vector of the FamilyHandles associated with a particular clan
  void getFamilyHandles(const ClanKey& cKey, std::vector<FamilyHandle> &fHandles) const;

  //! get a vector of the AssociationKeys associated with a particular clan
  void getAssociationKeys(const ClanKey& cKey, std::vector<AssociationKey> &aKeys) const;

  //! Find a FamilyHandle or return FamilyHandle()
  FamilyHandle findFamilyHandle(const FamilyKey &fKey) const;

  //! Get an existing family handle and assert otherwise
  const FamilyHandle& getFamilyHandle(const FamilyKey &fKey) const;

  //! Find a AssociationHandle or return FamilyHandle()
  AssociationHandle findAssociationHandle(const AssociationKey &aKey) const;

  //! get an association handle
  const AssociationHandle& getAssociationHandle(const AssociationKey &aKey) const;

  //! get the first time index associated with a family 
  TimeIndexType getFirstTime(const ClanKey &key, const std::string &varName) const;
  
  //! get the end time index associated with a family 
  TimeIndexType getEndTime(const ClanKey &key, const std::string &varName) const;

  //! The set of datasets that contributed to the collection
  const std::set<std::string>& datasets() const {return mDatasets;}

  //! The set of variable names that the collection has families for
  const std::set<std::string>& variables() const {return mVariables;}

  //! The set of metrics the collection has simplifications for
  const std::set<std::string>& metrics() const {return mMetrics;}

  //! The set of statistics that the collection has StatHandles for
  const std::set<std::pair<std::string, std::string> >& statistics() const {return mStatistics;}

  const char*  getFileFromFamily(const FamilyKey fkey);

private:


  class Clan {
  public:

    //! Constructor
    Clan(ClanKey k) : key(k) {}

    //! The corresponding clan key
    ClanKey key;

    //! The set of families of this clan
    FamilyMapType families;

    //! The set of associations of this clan
    AssociationMapType associations;
  };

  //! The map type to find the clan
  typedef std::map<ClanKey, Clan> ClanMapType;

  //! A map into the array of clans
  ClanMapType mClanMap;

  //! The set of datasets
  std::set<std::string> mDatasets;

  //! The set of variables
  std::set<std::string> mVariables;

  //! The set of metrics
  std::set<std::string> mMetrics;

  //! The set of statistics
  std::set<std::pair<std::string, std::string> > mStatistics;

  std::map<FamilyKey, const char *> mFilenames;

  //! The bounding box of any segmentation handles
  double mLow[3];
  double mHigh[3];



  //! Internal call to initialize
  int initialize(const char* file);

};

}
#endif /* HANDLECOLLECTION_H_ */
