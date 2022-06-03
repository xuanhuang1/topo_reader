/*
 * AssociationKey.h
 *
 *  Created on: Mar 1, 2012
 *      Author: bremer5
 */

#ifndef ASSOCIATIONKEY_H_
#define ASSOCIATIONKEY_H_

#include "ClanKey.h"
#include "AssociationHandle.h"

namespace TopologyFileFormat {

class AssociationKey {

public:

  //! Constructor
  AssociationKey(const SimplificationKey& f0, const SimplificationKey& f1) : mClan(f0.clanKey()), mSource(f0), mDestination(f1) {}

  //! Constructor from a handle
  AssociationKey(const AssociationHandle& handle, ClanKey& key) :
    mClan(key), mSource(handle.source()), mDestination(handle.destination()) {}

  //! Copy Constructor
  AssociationKey(const AssociationKey& key) : mClan(key.mClan), mSource(key.mSource), mDestination(key.mDestination) {}

  //! Destructor
  ~AssociationKey() {}

  const ClanKey& clanKey() const {return mClan;}

  //! Return the source
  const SimplificationKey& source() const {return mSource;}

  //! Return the destination
  const SimplificationKey& destination() const {return mDestination;}

  //! Less then operator
  bool operator<(const AssociationKey& key) const {
    if (mClan < key.mClan)
      return true;
    else if (key.mClan < mClan)
      return false;
    else if (mSource < key.source()) 
      return true;
    else if (key.source() < mSource) 
      return false;
    else if (mDestination < key.destination()) 
      return true;
    else 
      return false;
}



private:

  //! The associated clan key
  ClanKey mClan;

  //! The source family
  SimplificationKey mSource;

  //! The destination family
  SimplificationKey mDestination;
};




}


#endif /* ASSOCIATIONKEY_H_ */
