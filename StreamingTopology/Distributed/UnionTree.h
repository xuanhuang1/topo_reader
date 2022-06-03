/*
 * UnionTree.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef UNIONTREE_H_
#define UNIONTREE_H_

#include "FlexArray/BlockedArray.h"
#include "UnionNode.h"

//! The basic merge tree class implementing the data structure
class UnionTree
{
public:

  //! Default constructor
  UnionTree(GraphID id) : mId(id) {}

  //! Destructor
  ~UnionTree() {}

  //! Return the id
  GraphID id() const {return mId;}

  //! Some function to remove a node (overloaded in SegmentedUnionTree)
  virtual int removeNode();

  //! Some function to add a node (overloaded in SegmentedUnionTree)
  virtual int addNode();

public:

  //! The global id of this tree
  GraphID mId;

  //! The array of nodes
  FlexArray::BlockedArray<UnionNode> mNodes;

};

#endif /* UNIONTREE_H_ */
