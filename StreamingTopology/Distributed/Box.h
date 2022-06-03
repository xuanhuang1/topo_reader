/*
 * Box.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef BOX_H_
#define BOX_H_

#include "DistributedDefinitions.h"
#include "PointIndex.h"

//! A box encapsulates the notion of a discrete region of index space
class Box
{
public:

  //! Default constructor creatign an invalid box
  Box();

  //! Construct a sub-box
  /*! Create a local sub-box covering the index interval [low,high]
   *  and assume the global array has the given dimensions
   * @param low: Left most corner of the local index space
   * @param high: Right most corner of the local index space
   * @param dim: Size of the global index space
   */
  Box(const PointIndex& low, const PointIndex& high, const PointIndex& dim) : mLow(low), mHigh(high), mDimensions(dim) {
	  for (int i = 0; i < 3; i++) mLocalDimensions[i] = mHigh[i] - mLow[i];
  };

  //! returns the total size of the global index space
  GlobalIndexType globalSize() const { return mDimensions.size(); };

  LocalIndexType localSize() const {
	  return mLocalDimensions.size();
  }

  PointIndex coordinates(LocalIndexType index) const {
	  //++ return the PointIndex of my localIndexType
	  // in global? or local?
	  PointIndex result;
	  return result;
  } 

  int boundaryCount(LocalIndexType index) const {
	return 0; //++ NEEDS BOUNDARY COUNT
  }
  //! Destructor
  ~Box() {}

private:

	PointIndex mLocalDimensions;

  //! The left most corner of the index box
  PointIndex mLow;

  //! The right most corner of the index box
  PointIndex mHigh;

  //! The dimensions of the global index space
  PointIndex mDimensions;
};

#endif /* BOX_H_ */
