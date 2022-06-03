/*
 * IntervalTree.h
 *
 *  Created on: Sep 23, 2016
 *      Author: bremer5
 */

#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include <vector>
#include <algorithm>

#include "Feature.h"

namespace TopologyFileFormat
{

struct MergeTreeInterval
{
  //! Return the primary key of the feature
  static FunctionType primaryKey(const Feature* f) {return f->lifeTime()[1];}

  //! Return the secondary key of the feature
  static FunctionType secondaryKey(const Feature* f) {return f->lifeTime()[0];}

  //! Accumulation operator
  static FunctionType accumulate(FunctionType x, FunctionType y) {return std::min(x,y);}

  //! Comparison operator
  static bool cmpf(const Feature* f, const Feature* g) {return primaryKey(f) < primaryKey(g);}

  //! Comparison operator
  static bool cmp(FunctionType f, const Feature* g) {return f < primaryKey(g);}

  //! Comparison operator
  static bool cmp(FunctionType f, FunctionType g) {return f < g;}
};

struct SplitTreeInterval
{
  //! Return the primary key of the feature
  static FunctionType primaryKey(const Feature* f) {return f->lifeTime()[0];}

  //! Return the secondary key of the feature
  static FunctionType secondaryKey(const Feature* f) {return f->lifeTime()[1];}

  //! Accumulation operator
  static FunctionType accumulate(FunctionType x, FunctionType y) {return std::max(x,y);}

  //! Comparison operator
  static bool cmpf(const Feature* f, const Feature* g) {return primaryKey(f) > primaryKey(g);}

  //! Comparison operator
  static bool cmp(FunctionType f, const Feature* g) {return f > primaryKey(g);}

  //! Comparison operator
  static bool cmp(FunctionType f, FunctionType g) {return f > g;}
};

class IntervalTree
{
public:

  //! Construct an IntervalTree from the given set of features
  IntervalTree() {}

  virtual ~IntervalTree() {}

  //! Get the first possible feature that can be active/living for threshold t
  virtual std::vector<const Feature*>::const_iterator begin(FunctionType t) const = 0;

  //! Get the feature after the last feature that might be living
  virtual std::vector<const Feature*>::const_iterator end(FunctionType t) const = 0;
};


template <typename IntervalType>
class IntervalTreeSpecific : public IntervalTree
{
public:

  //! Typedef to make the code less ugly
  typedef std::vector<const Feature*>::const_iterator IteratorType;

  //! Construct an IntervalTree from the given set of features
  IntervalTreeSpecific(const std::vector<Feature>& features);

  virtual ~IntervalTreeSpecific() {}

  //! Get the first possible feature that can be active/living for threshold t
  //! This is equivalent to the first f with cmp(primaryKey(f), t) == false
  virtual std::vector<const Feature*>::const_iterator begin(FunctionType t) const;

  //! Get the feature after the last feature that might be living
  //! This is equivalent to the first f with cmp(t, accumulate(f)) == true
  virtual std::vector<const Feature*>::const_iterator end(FunctionType t) const;

private:

  //! The (sorted) array of pointers to features
  std::vector<const Feature*> mFeatures;

  //! The corresponding array of accumulated keys
  std::vector<FunctionType> mAccumulate;

  //! Accumulate the secondary key
  FunctionType accumulateSecondaryKey(uint64_t start, uint64_t stop);

  //! The recursive call to find end()
  std::vector<const Feature*>::const_iterator end(FunctionType t, uint64_t start, uint64_t stop) const;

};

template <typename IntervalType>
IntervalTreeSpecific<IntervalType>::IntervalTreeSpecific(const std::vector<Feature>& features) : IntervalTree()
{
  //! Initialize the feature and accumulation vector
  mFeatures.resize(features.size());
  mAccumulate.resize(features.size());


  //! Initialize the pointers
  for (int64_t i=0;i<features.size();i++)
    mFeatures[i] = &features[i];

  //! Sort the features by the primary key
  std::sort(mFeatures.begin(),mFeatures.end(),IntervalType::cmpf);

  accumulateSecondaryKey(0,mFeatures.size()-1);
}

template <typename IntervalType>
FunctionType IntervalTreeSpecific<IntervalType>::accumulateSecondaryKey(uint64_t start, uint64_t stop)
{
  // The convention will be that we do not store accumulations for the leafs
  // This allows us to treat all the features as leafs of the tree rather
  // then dealing with features that are internal nodes
  if (stop == start) {
    return IntervalType::secondaryKey(mFeatures[start]);
  }

  uint64_t middle = (start + stop) >> 1;

  // We simply use the middle to store the accumulated value for the range which
  // should produce the correct values. Effectively, you make the root of a
  // subtree part of the "left" side traversal
  mAccumulate[middle] = IntervalType::accumulate(accumulateSecondaryKey(start,middle),accumulateSecondaryKey(middle+1,stop));

  return mAccumulate[middle];
}

template <typename IntervalType>
std::vector<const Feature*>::const_iterator IntervalTreeSpecific<IntervalType>::begin(FunctionType t) const
{
  // This is a little trick to allow us to use std::lower_bound to look for a threshold.
  // We simply create a feature with both primary and secondary keys == t and search
  // for that feature
  Feature f;
  f.lifeTime(t,t);

  return std::lower_bound(mFeatures.begin(),mFeatures.end(),&f,IntervalType::cmpf);
}

template <typename IntervalType>
std::vector<const Feature*>::const_iterator IntervalTreeSpecific<IntervalType>::end(FunctionType t) const
{
  uint64_t middle = (mFeatures.size()-1) >> 1;

  // If there exists no element with cmp(t,feature) == false
  // I.e. the current threshold below the minimal range in the case
  // of a merge tree
  if (IntervalType::cmp(t,mAccumulate[middle]) == true)
    return mFeatures.begin(); // Then no feature is living and end == begin

  // Otherwise, there must be at least one element with cmp(t,feature) == false
  return end(t,0,mFeatures.size()-1);
}

template <typename IntervalType>
std::vector<const Feature*>::const_iterator IntervalTreeSpecific<IntervalType>::end(FunctionType t,
                                                                                    uint64_t start, uint64_t stop) const
{
  // We are looking for the first f with cmp(t, accumulate(f)) == true.
  // To make things simpler we will search for the last element with
  // cmp(t,accumulate(f)) == false and the return the next element. The
  // invariant we will maintain is that the "subtree" [start,stop] must
  // contain at least one element with cmp(t,accumulate(f)) == false

  // If we are at a leaf of the traversal
  if (start == stop) {

    // We should have found a matching element
    assert (IntervalType::cmp(t,IntervalType::secondaryKey(mFeatures[start])) == false);

    // Return an iterator to the next element
    return mFeatures.begin()+start+1;
  }

  // Middle encodes the accumulated second key
  uint64_t middle = (start + stop) >> 1;

  // First we check whether the right subtree contains a matching
  // element
  uint64_t right_middle = (middle + 1 + stop) >> 1;

  FunctionType accumulated;

  // First we compute the accumulated value of the right subtree
  if (middle + 1 == stop)
    accumulated = IntervalType::secondaryKey(mFeatures[stop]);
  else
    accumulated = mAccumulate[right_middle];

  // If cmp() == false then the right subtree contains at least
  // one element with cmp() == false and we recurse there.
  if (IntervalType::cmp(t,accumulated) == false)
    return end(t,middle+1,stop);
  else // Otherwise, the left subtree must contain what we are looking for
    return end(t,start,middle);
}



}



#endif /* INTERVALTREE_H_ */
