#ifndef STATHANDLE_H
#define STATHANDLE_H

#include "DataHandle.h"

namespace TopologyFileFormat 
{

//! Class to compare typeName / attributeName pairs
class StatMapKey
{
public:
  //! The default constructor
  StatMapKey() {}

  //! Convinient constructor
  StatMapKey(const std::string& t, const std::string& n) : type_name(t), name(n) {}

  //! Destructor
  ~StatMapKey() {}

  //! Comparison operator
  bool operator<(const StatMapKey& key) const {return ((type_name < key.type_name)
      || ((type_name == key.type_name) && (name < key.name)));}

  //! The type name
  std::string type_name;

  //! The attribute name
  std::string name;
};



//! The generic handle for per-feature information
/*! A StatHandle implements a generic handle for per-feature
 *  information. Compared to a FileHandle it also allows for a handle to have a
 *  sub-node that stores the mapping if it has been specified as indirect
 */
class StatHandle : public DataHandle
{
public:

  //! Friend declaration to allow access to the hidden parsing functions
  friend class FamilyHandle;

  //! Friend declaration to allow access to the hidden parsing functions
  friend class AssociationHandle;

  //! Constructor
  StatHandle(HandleType t=H_STAT);

  //! Constructor
  StatHandle(const char* filename, HandleType t=H_STAT);
  
  //! Copy Constructor
  StatHandle(const StatHandle& handle);

  //! Destructor
  virtual ~StatHandle();

  //! Assignment operator
  StatHandle& operator=(const StatHandle& handle);
  
  //! Return whether this statistics is aggregated
  bool aggregated() const {return mAggregated;}

  //! Return the name of the statistic
  const std::string& stat() const {return mStat;}

  //! Return the species
  const std::string& species() const {return mSpecies;}
  
  //! Set the name of the statistics
  void stat(const std::string& s) {mStat = s;}

  //! Set the species name
  void species(const std::string& s) {mSpecies = s;}

  //! Set the aggregation flag
  void aggregated(bool flag) {mAggregated = flag;}

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node) {return parseXMLInternal(node);}

protected:

  //! The flag that stores the mapping type
  bool mDirect;

  //! The flag that stores whether the values are aggregated
  bool mAggregated;

  //! The name of the statistic represented
  std::string mStat;

  //! The name of the attribute (default Function)
  std::string mSpecies;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Parse the information from the xml tree
  int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  int attachXMLInternal(XMLNode& node) const;
};

}

#endif
