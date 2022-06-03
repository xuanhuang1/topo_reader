/*
 * FamilyKey.h
 *
 *  Created on: Mar 1, 2012
 *      Author: bremer5
 */

#ifndef STATKEY_H_
#define STATKEY_H_

#include "HandleKeys.h"
#include "ClanKey.h"
#include "FamilyKey.h"
#include "xmlParser.h"

namespace TopologyFileFormat {


class StatKey {

public:

  //! Constructor
  StatKey() : mStat(""), mSpecies("") {}

  //! Constructor
  StatKey(const std::string& stat, const std::string& species, FamilyKey f) :
    mFamily(f), mStat(stat), mSpecies(species) {}

   //! Copy Constructor
  StatKey(const StatKey& key) : mFamily(key.mFamily),
      mStat(key.mStat), mSpecies(key.mSpecies) {}

  //! Destructor
  ~StatKey() {}

  //! Less then operator
  bool operator<(const StatKey& key) const;

  //! Get the name of the statistics
  const std::string& species() const { return mSpecies; }

  //! Set the statistics
  void species(const std::string& s) { mSpecies = s;}

  //! Get the name of the statistics
  const std::string& statistic() const { return mStat; }
 
  //! Set the statistics
  void statistics(const std::string& s) { mStat = s;}

  //! Return the corresponding clankey
  const FamilyKey& familyKey() const {return mFamily;}

  //! Read in a key from xml
  int parseXML(const XMLNode& node, uint8_t count=0);

  //! Write the attributes to xml
  int attachXML(XMLNode& node, uint8_t count=0) const;

private:

  //! The associated clan key
  FamilyKey mFamily;

  //! The statistics name
  std::string mStat;

  //! The species name
  std::string mSpecies;
};




}

#endif /* FAMILYKEY_H_ */
