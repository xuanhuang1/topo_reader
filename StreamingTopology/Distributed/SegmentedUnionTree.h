/*
 * SegmentedUnionTree.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef SEGMENTEDUNIONTREE_H_
#define SEGMENTEDUNIONTREE_H_

#include "UnionTree.h"

//! A UnionTree that also contains segmentation and statistics information
class SegmentedUnionTree : public UnionTree
{
public:

  //! Constructor
  SegmentedUnionTree(GraphID id) : UnionTree(id) {}

  //! Destructor
  virtual ~SegmentedUnionTree();

  //! Function to remove a node and maintain the correction information
  virtual int removeNode();

  //! Some function to add a node and maintain the correction information
  virtual int addNode();

};


#endif /* SEGMENTEDUNIONTREE_H_ */
