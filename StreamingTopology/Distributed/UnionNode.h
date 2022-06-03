/*
 * UnionNode.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef UNIONNODE_H_
#define UNIONNODE_H_

#include "FlexArray/BlockedArray.h"
#include "DistributedDefinitions.h"
#include "Token.h"

//! The node of a Merge- or SplitTree
class UnionNode
{
public:

  //! Friend declaration to allow array allocation
  friend class FlexArray::BlockedArray<UnionNode>;

  //! Default constructor
  UnionNode(FunctionType f, GlobalIndexType i);

  //! Destructor
  ~UnionNode() {}

  //! Convert a UnionNode into a VertexToken for I/O
  operator VertexToken() const;

  //! Return the function value
  FunctionType value() const {return mValue;}

  //! Return the index
  GlobalIndexType index() const {return mIndex;}

  //! return the down pointer
  UnionNode* down() {return mDown;}

  //! Return one of the up pointers
  UnionNode* up() {return mUp;}

  //! Bypass this node by linking all up pointers directly to the down pointers
  void bypass();

private:

  //! The function value of this node
  FunctionType mValue;

  //! The global index of this node
  GlobalIndexType mIndex;

  //! Pointer to one of the up pointer
  UnionNode* mUp;

  //! Pointer to the next up node in the list
  UnionNode* mNext;

  //! Pointer to the single down node
  UnionNode* mDown;

  //! Private constructor just for array allocation
  UnionNode() {}
};


#endif /* UNIONNODE_H_ */
