/*
 * SimplificationKey.h
 *
 *  Created on: Mar 14, 2012
 *      Author: bremer5
 */

#ifndef SIMPLIFICATIONKEY_H_
#define SIMPLIFICATIONKEY_H_

#include "SimplificationHandle.h"
#include "FamilyKey.h"

namespace TopologyFileFormat {

//! The simplification key
class SimplificationKey
{
public:

  //! Constructor
  SimplificationKey() {}

  //! Default constructor
  SimplificationKey(const std::string& metric, const FamilyKey& key) : mFamily(key), mMetric(metric) {}

  //! Copy constructor
  SimplificationKey(const SimplificationKey& key) : mFamily(key.mFamily), mMetric(key.mMetric) {}

  //! Constructor to construct a key from a handle
  SimplificationKey(const SimplificationHandle& handle, const FamilyKey& key);

  bool operator<(const SimplificationKey& key) const;

  std::string metric() const { return mMetric; }

  void metric(std::string s) { mMetric = s; }

  const ClanKey& clanKey() const {return mFamily.clanKey();}

  const FamilyKey& familyKey() const {return mFamily;}

  //! Read in a key from xml
  int parseXML(const XMLNode& node, uint8_t count=0);

  //! Write the attributes to xml
  int attachXML(XMLNode& node, uint8_t count=0) const;


private:

  //! The corresponding familky key
  FamilyKey mFamily;

  std::string mMetric;
};


}


#endif /* SIMPLIFICATIONKEY_H_ */
