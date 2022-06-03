/*
 * ClanKey.h
 *
 *  Created on: Mar 1, 2012
 *      Author: bremer5
 */

#ifndef CLANKEY_H_
#define CLANKEY_H_

#include "HandleKeys.h"

namespace TopologyFileFormat {

//! Function to combine hash values (stolen from boost)
inline size_t combine_hash_key( size_t lhs, size_t rhs ) {
  lhs^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
  return lhs;
}

//! The unique key to identify a piece of data
class ClanKey {

public:

  friend struct ClanKeyHash;

  //! Constructor
  ClanKey() : mClan("unkown") {}

  //! Constructor
  ClanKey(const std::string& key) : mClan(key) {}

  //! Copy constructor
  ClanKey(const ClanKey& key) : mClan(key.mClan) {}

  //! Destuctor
  ~ClanKey() {}

  //! less than operator for the map
  bool operator<(const ClanKey& key) const {return mClan < key.clan();}

  //! Comparison operation
  bool operator==(const ClanKey& key) const {return mClan == key.clan();}

  //! Get the clan key
  const std::string& clan() const {return mClan;}

  //! Set the clan key
  void clan(const std::string key) {mClan = key;}


  //! Read in a key from xml
  int parseXML(const XMLNode& node, uint8_t count=0);

  //! Write the attributes to xml
  int attachXML(XMLNode& node, uint8_t count=0) const;

private:

  //! The name of the data set
  std::string mClan;
};
}

namespace std
{
template<>
struct hash<TopologyFileFormat::ClanKey> {
    size_t operator()(const TopologyFileFormat::ClanKey &key) const {
        return std::hash<std::string>()(key.clan());
    }
};
}



#endif /* CLANKEY_H_ */
