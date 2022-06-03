#ifndef FEATUREHIERARCHY_H
#define FEATUREHIERARCHY_H

#include <cmath>
#include <set>
#include <deque>
#include <map>
#include "Feature.h"
#include "SimplificationHandle.h"

namespace TopologyFileFormat {

//! Forward declaration to avoid unnecessary include
class IntervalTree;

//! Feature hierarchy describes a hierarchical feature graph
/*! The FeatureHIerarchy class implements a generic feature hierarchy in which
 *  each feature may have any number of "constituents" and "representatives."
 *  Furthermore, each feature has a (birth,death] parameter range in which it is
 *  considered "alive." The range is half-open half closed with a flag
 *  indicating which side is closed and which is open. If a feature is alive
 *  none of its representatives are alive and all of its constituents are
 *  considered merged. A feature that is alive can switch status by either
 *  merging in which case its representatives come to life or by dying in which
 *  case its constituents become alive. We will use the following
 *  nomenclature. Features are:
 *
 *  Alive: A feature is alive if the parameter is within its personal
 *  range. These are the nodes just above the graph cut representing the current
 *  list of features.
 *
 *  Subselected: This is a subset of features that are alive subselected based
 *  on an arbitrary criterion.
 * 
 *  Active: These are all features that are either subselected or constituents
 *  of subselected features. 
 */
class FeatureHierarchy 
{
public:

  //! Number of valid hierarchy properties
  static const uint32_t sPropertyCount = 3;

  //! List of property names that can be computed only based on the current state
  static const std::string sPropertyNames[sPropertyCount];
  
  //! Iterator type for all features that are currently alive and subselected
  typedef std::vector<const Feature*>::const_iterator SubSelectedIterator;

  //! An iterator that visits all features that are alive
  /*! A living iterator will be initialized from an IntervalTree
   *  and thus guaranteed to visit all living features.
   */
  class LivingIterator
  {
  public:
    //! Friend declaration to allow the hierarchy access to the private constructor
    friend class FeatureHierarchy;

    //! Default constructor
    LivingIterator() : mParameter(0), mInitialized(false) {}

    //! Copy constructor
    LivingIterator(const LivingIterator& it);

    //! Destructor
    ~LivingIterator() {}

    //! Assignment operator
    LivingIterator& operator=(const LivingIterator& it);

    //! Advance the iterator
    void operator++(int i);

    //! Comparison operator
    bool operator==(const LivingIterator& it) const;

    //! Comparison operator
    bool operator!=(const LivingIterator& it) const {return !(*this == it);}

    //! Convenience overload to allow access to a feature via ->
    const Feature* operator->() const {return feature();}

    //! Convenience overload to allow access to a feature via *
    const Feature* operator*() const {return feature();}

    //! Determine whether this iterator is valid
    bool valid() const {return (mInitialized && (mIt != mEnd));}

    //! Return a pointer to the current feature
    const Feature* feature() const {return *mIt;}

  private:

    //! The parameter use to construct this iterator
    FunctionType mParameter;

    //! A bool that indicates whether the iterator has been initialized
    bool mInitialized;

    //! The iterator to the current feature
    std::vector<const Feature*>::const_iterator mIt;

    //! The iterator to the end of the list of potentially alive features
    std::vector<const Feature*>::const_iterator mEnd;

    //! A private constructor for the begin() functions
    LivingIterator(FunctionType parameter,
                   std::vector<const Feature*>::const_iterator start,
                   std::vector<const Feature*>::const_iterator stop);
  };

  //! An iterator that visits all active features at least once
  /*! The active iterator will start from a given list of living nodes and
   *  traverse each of their sub-trees in a breadth first manner. Furthermore,
   *  it stores a parity bit in each feature indicating whether a feature has
   *  been reached and odd (1) or even (0) number of times. 
   * 
   *  The iterator assumes that there are no shortcuts in the graph meaning if a
   *  feature can be reached from a representative multiple times the length of
   *  each path is the same. This is certainly correct in any tree. General
   *  graphs are created by constructing "new" features at each simplification
   *  operation. However, once the operation has been perform the original
   *  features no longer exist (only the aggregates). Thus, every time you have
   *  multiple representatives they must live on the same "level".
   * 
   *  The active iterator also assumes that the corresponding feature hierarchy
   *  is not adapted or otherwise modified during the traversal. If the feature
   *  hierarchy is changed during the traversal the behavior of this iterator is
   *  undefined.
   */
  class ActiveIterator 
  {
  public:
    
    //! Friend declaration to allow the hierarchy access to the private constructor
    friend class FeatureHierarchy;

    //! Default constructor
    ActiveIterator();

    //! Copy constructor
    ActiveIterator(const ActiveIterator& it);

    //! Destructor
    ~ActiveIterator();

    //! Assignment operator
    ActiveIterator& operator=(const ActiveIterator& it);

    //! Advance the iterator
    void operator++(int i);

    //! Comparison operator
    bool operator==(const ActiveIterator& it) const;

    //! Comparison operator
    bool operator!=(const ActiveIterator& it) const {return !(*this == it);}

    //! Convenience overload to allow access to a feature via ->
    const Feature* operator->() {return feature();}

    //! Convenience overload to allow access to a feature via ->
    const Feature* operator*() {return feature();}

    //! Return a pointer to the current feature
    const Feature* feature() const;

    //! Return a pointer to the representative of the current feature
    const Feature* rep() const;

    //! Return a pointer to the agent of the current feature
    const Feature* agent() const;

    
  private: 

    //! The buffer of the breadth-first traversal
    std::deque<const Feature*> mFront;

    //! Iterator to the root currently processed
    LivingIterator mRootIt;

    //! Private constructor for the begin() function
    ActiveIterator(LivingIterator it);

  };

  //! Create a feature hierarchy from a given file
  FeatureHierarchy();
  
  //! Destructor
  ~FeatureHierarchy();

  //! Clear a given hierarchy of all existsing information
  void clear();

  //! Read an initial hierarchy 
  int initialize(const SimplificationHandle& handle);
  
  //! Read an initial hierarchy and specify parameter
  int initialize(const SimplificationHandle& handle, bool useInputParameter, FunctionType param);

  //! Return the number of total features
  LocalIndexType featureCount() const {return mFeatures.size();}

  //! Return the feature corresponding to the given id
  Feature* feature(LocalIndexType id) {return &mFeatures[id];}

  //! Return the first feature from feature vector
  const std::vector<Feature>& allFeatures() const {return mFeatures;}

  //! Return the first feature from feature vector
  std::vector<Feature>& allFeatures() {return mFeatures;}

  //! Return the current parameter
  FunctionType parameter() const {return mParameter;}

  //! Return the minimal parameter
  FunctionType minParameter() const {return mMinPar;}
  
  //! Return the maximal parameter
  FunctionType maxParameter() const {return mMaxPar;}

  //! Set the parameter
  void parameter(FunctionType p) {mParameter = p;}

  //! Get the corresponding time index
  uint32_t timeIndex() const {return mTimeIndex;}

  //! Get the corresponding real time
  double time() const {return mTime;}
 
  //! Read an index map
  void initializeIndexMap(FILE* input, LocalIndexType feature_count, bool binary);

  //! Return the mapped index
  LocalIndexType mappedIndex(LocalIndexType i);

   /*******************************************************************************************
   ****************************  Iterators to access features  *******************************
   ******************************************************************************************/

  //! Return an iterator through all features that are alive
  LivingIterator beginLiving(FunctionType parameter) const;

  //! Return an iterator pointing after the last feature that is alive
  LivingIterator endLiving() const {return LivingIterator();}
  
  //! Return the number of living features (backward compatibility call for UI)
  LocalIndexType livingSize(FunctionType parameter) const;

  //! Return all leafs constituents.empty()
  std::vector<Feature*> leafs();

  //! Return all roots
  std::vector<Feature*> roots();

   //! Return the minimal parameter value such that there exist k or more features
  /*! Determine the minimal parameter value such that there exist count or more
   *  many features. If no such parameter exists the minimal parameter value is
   *  returned.
   *  @param count: number of features we are interested in
   *  @param resolution: The number of different parameter values between 
   *                     [min,max] that are tryed
   */
  //FunctionType minParameterWithCount(uint32_t count, uint32_t resolution = 200);

  //! Return the maximal parameter value such that there exist k or more features
  /*! Determine the maximal parameter value such that there exist count or more
   *  many features. If no such parameter exists the maximal parameter value is
   *  returned.
   *  @param count: number of features we are interested in
   *  @param resolution: The number of different parameter values between 
   *                     [min,max] that are tried */
  //FunctionType maxParameterWithCount(uint32_t count, uint32_t resolution = 200);

  //! Return an iterator through all features that are active
  ActiveIterator beginActive(FunctionType parameter);

  //! Return an iterator pointing after the last active feature
  ActiveIterator endActive() const {return ActiveIterator();}




  /*******************************************************************************************
   ****************************  Interface for hierarchy properties **************************
   ******************************************************************************************/

  //! Return the index of the desired hierarchy property
  /*!
   * Given a sting determine which hierarchy property is indicated
   * and return its internal index. If no property with the given
   * name is found return -1.
   * @param property: name of the property we are interested in
   * @return property index or -1 if no such property exists
   */
  static int32_t hierarchyPropertyIndex(const std::string& property);

  //! Return the property name for the given index
  static std::string propertyName(int32_t index) {return sPropertyNames[index];}

  //! Return the value of a hierarchy property
  /*!
   * Given a valid property index this function calls the correct
   * member function to compute the property value. The function
   * can depend on an arbitrary number of floating point parameters
   * (these have nothing to do with the *parameter* of the hierarchy).
   * If necessary the float values will be type-converted into
   * integer etc. if the underlying function requires it
   * @param index: The property index
   * @param parameters: The parameters for the property function
   * @return The value of the property or 0 if some error occured
   */
  //FunctionType hierarchyProperty(int32_t index, const std::vector<float>& parameters);


private:
  
  //! The current parameter value
  FunctionType mParameter;

  //! The minima parameter value which exists in any feature
  FunctionType mMinPar;
  
  //! The maximal parameter value which exists in any feature
  FunctionType mMaxPar;
  
  //! The time index of this hierarchy
  uint32_t mTimeIndex;

  //! The real time value of this hierarchy
  double mTime;

  //! The array of features including the id <-> local id map
  std::vector<Feature> mFeatures;

  //! The index map between feature id's and array index if the mapping is non trivial
  std::map<LocalIndexType,LocalIndexType> mIndexMap;
  
  //! An IntervalTree to quickly find living features
  const IntervalTree* mIntervalTree;

  //! Determine the agents for each feature
  void initializeAgent(Feature* f);

  //! Private rounding function since this seems to be non standard
  uint32_t round(float f) const {return (uint32_t)floor(f+0.5);}
};

} // namespace TopologicalFileFormat

#endif
