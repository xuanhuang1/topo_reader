/*
 * FamilyKey.h
 *
 *  Created on: Mar 1, 2012
 *      Author: bremer5
 */

#ifndef FAMILYKEY_H_
#define FAMILYKEY_H_

#include "HandleKeys.h"
#include "ClanKey.h"
#include "xmlParser.h"

namespace TopologyFileFormat {


class FamilyKey {

public:

  friend struct FamilyKeyHash;

  //! Constructor
  FamilyKey() : mVariable(""), mTimeIndex(0) {}

  //! Constructor
  FamilyKey(const std::string& id, TimeIndexType t, ClanKey c) :
    mClan(c), mVariable(id), mTimeIndex(t) {}

   //! Copy Constructor
  FamilyKey(const FamilyKey& key) : mClan(key.mClan),
      mVariable(key.mVariable), mTimeIndex(key.mTimeIndex) {}

  //! Destructor
  ~FamilyKey() {}

  //! Less then operator
  bool operator<(const FamilyKey& key) const;

  //! Get the time index
  TimeIndexType timeIndex() const {return mTimeIndex;}

  //! Set the time index
  void timeIndex(TimeIndexType t) { mTimeIndex = t;}

  //! Get the variable name 
  const std::string& variableName() const { return mVariable; }
 
  //! Set the variable name 
  void variableName(std::string s) { mVariable = s;}

  //! Return the corresponding clankey
  const ClanKey& clanKey() const {return mClan;}

  //! Read in a key from xml
  int parseXML(const XMLNode& node, uint8_t count=0);

  //! Write the attributes to xml
  int attachXML(XMLNode& node, uint8_t count=0) const;

private:

  //! The associated clan key
  ClanKey mClan;

  //! The variable name
  std::string mVariable;

  //! The time index
  uint32_t mTimeIndex;
};
}

namespace std
{
template<>
struct hash<TopologyFileFormat::FamilyKey> {
    size_t operator()(const TopologyFileFormat::FamilyKey& key) const {
      std::size_t seed = 0;
      seed = TopologyFileFormat::combine_hash_key(seed,std::hash<TopologyFileFormat::ClanKey>()(key.clanKey().clan()));
      seed = TopologyFileFormat::combine_hash_key(seed,std::hash<std::string>()(key.variableName()));
      seed = TopologyFileFormat::combine_hash_key(seed,std::hash<uint32_t>()(key.timeIndex()));

      return seed;
    }
};
}



#endif /* FAMILYKEY_H_ */
