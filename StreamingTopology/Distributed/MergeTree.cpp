/*
 * MergeTree.cpp
 *
 *  Created on: Feb 28, 2012
 *      Author: bremer5
 */

#include "MergeTree.h"

using namespace FlexArray;

TreeNode::TreeNode(GlobalIndexType index, FunctionType f, MultiplicityType multiplicity) :
    MappedElement<GlobalIndexType,TreeNodeIndex>(index), mValue(f), mMultiplicity(multiplicity), mFlag(false)
{
  up = TNULL;
  next = TNULL;
  down = TNULL;
}

TreeNode::TreeNode(const TreeNode& node) : MappedElement<GlobalIndexType,TreeNodeIndex>(node), mValue(node.mValue),
    mMultiplicity(node.mMultiplicity), mFlag(node.mFlag)
{
  up = node.up;
  next = node.next;
  down = node.down;
}

TreeNode& TreeNode::operator=(const TreeNode& node)
{
  MappedElement<GlobalIndexType,TreeNodeIndex>::operator=(node);

  up = node.up;
  next = node.next;
  down = node.down;

  mValue = node.mValue;
  mMultiplicity = node.mMultiplicity;
  mFlag = node.mFlag;

  return *this;
}

TreeNode::operator const VertexToken () const
{
  VertexToken v = {id(), mValue, mMultiplicity};

  return v;
}


void MergeTree::addNode(GlobalIndexType index, FunctionType value, MultiplicityType boundary_count)
{
  TreeNode node(index,value,boundary_count);
  TreeNodeIndex n;

  // At the node to the array and the internal map
  n = insertElement(node);

  // Initialize the circular list of next "pointers" indicating that this node
  // is now part of a tree
  at(n).next = n;
}

void MergeTree::addNode(const VertexToken& token)
{
  TreeNodeIndex n;

  // At the node to the array and the internal map
  n = insertElement(TreeNode(token.index(),token.value(),token.multiplicity()));

  // Initialize the circular list of next "pointers" indicating that this node
  // is now part of a tree
  at(n).next = n;
}


void MergeTree::addEdge(GlobalIndexType upper, GlobalIndexType lower)
{
  TreeNodeIndex up;
  TreeNodeIndex down;

  up = findElementIndex(upper);
  down = findElementIndex(lower);

  sterror((up==TNULL)||(down==TNULL),"Could not find both nodes of an edge.");

  TreeNode& u = at(up);
  TreeNode& l = at(down);

  // Need to put in correct logic ... what happens if upper->down != TNULL
  /*
  if (l.up == -1) {
     l.up = up;
     u.down = down;
     return;
   } else {
     TreeNodeIndex next = at(l.up).next;
     u.next = next;
     at(l.up).next = upper_id;
     u.down = down;
   }
*/
}

