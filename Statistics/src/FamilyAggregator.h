#ifndef FAMILYAGGREGATOR_H
#define FAMILYAGGREGATOR_H

#include <vector>
#include <string>
#include <ostream>
#include "Attribute.h"
#include "AggregatorFactory.h"
#include "FeatureHierarchy.h"

enum FamilyAggregationType {
  UNDEFINED_AGG   = 0,
  FEATURE_BASED   = 1,
  HIERARCHY_BASED = 2,
};

namespace Statistics {

/*! A FamilyAggregator encapsulates any functionality that condenses
 * a feature family at its current stage into a single value. Depending
 * on the type this can mean taking statistics of a given list of
 * features or a property of a feature hierarchy
 */
class FamilyAggregator
{
public:

  //! Default constructor
  FamilyAggregator();

  //! Convinience constructor
  FamilyAggregator(const std::string& name, const std::vector<float>& params = std::vector<float>(1,1));

  //! Copy constructor
  FamilyAggregator(const FamilyAggregator& agg) {*this = agg;}

  //! Destructor
  ~FamilyAggregator();

  //! Assignment operator
  FamilyAggregator& operator=(const FamilyAggregator& agg);

  //! Comparison operator
  bool operator==(const FamilyAggregator& agg) const;

  //! Comparison operator
  bool operator!=(const FamilyAggregator& agg) const {return !(*this == agg);}

  //! Return the type of aggregation currently defined
  FamilyAggregationType type() const {return mType;}

  //! Return a string describing this aggregator
  std::string name() const;

  //! Set the aggregator based on a string and (potentially) some parameter
  int set(const std::string& name, const std::vector<float>& params = std::vector<float>());

  //! Evaluate the aggregation for a given set of features
  FunctionType evaluate(const std::vector<FunctionType>& features) const;

  //! Evaluate the aggregation for a given hierarchy
  //FunctionType evaluate(TopologyFileFormat::FeatureHierarchy* hierarchy) const;

private:

  //! A single copy of an aggregator factory
  static const Statistics::Factory sFactory;

  //! The current aggregation type
  FamilyAggregationType mType;

  //! The current statistics operator
  Attribute* mStat;

  //! The current hierarchy property
  int32_t mHierarchyProperty;

  //! The current set of property parameters
  std::vector<float> mPropertyParameters;
};

//template <typename FunctionType>
//const Statistics::Factory<FunctionType> FamilyAggregator<FunctionType>::sFactory;


}

#endif /* FAMILYAGGREGATOR_H_ */
