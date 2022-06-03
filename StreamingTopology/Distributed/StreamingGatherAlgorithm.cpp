/*
 * StreamingGatherAlgorithm.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: bremer5
 */

#include "StreamingGatherAlgorithm.h"
#include "Token.h"

Token StreamingGatherAlgorithm::sUndefinedToken;

int StreamingGatherAlgorithm::apply(MergeTree& tree, TopoInputStream* inputs,
                                    TopoOutputStream* downstream,
                                    TopoOutputStream* upstream,
                                    const ControlFlow* gather_flow)
{
  // First we initialize some internal pointers for convinience
  mTree = &tree;
  sterror(mTree == NULL,"No union tree given");

  // Determine how many trees total we are merging
  uint32_t input_count = gather_flow->sources(mTree->id()).size();

  mDownstream = downstream;
  sterror((mDownstream==NULL)
          && (gather_flow->sinks(mTree->id()).size() > 0),
          "No downstream output given but we expect sinks");

  mUpstream = upstream;

  // Now we implement the main loop
  Token& token = sUndefinedToken;

  // We read tokens until we have seen the last EMPTY token
  while (true) {

    //! Read the next token
    (*inputs) >> token;

    switch (token.type()) {

      // If one of the trees is done
      case EMPTY: {
        input_count--;

        // If that was the last tree we needed
        if (input_count == 0) {
          finalize(); // We clean-up
          return 1; // and return
        }
        break;
      }

      case VERTEX: {
        const VertexToken& v = token;

        // If this node is not yet part of the tree
        if (mTree->findElement(v.index()) == NULL) {

          // Add it to the tree
          mTree->addNode(v);
        }
        break;
      }

      case EDGE: {
        const EdgeToken& e = token;

        TreeNodeIndex high = mTree->findElementIndex(e[0]);
        TreeNodeIndex low = mTree->findElementIndex(e[1]);

        sterror((high == TNULL) || (low == TNULL),"Inconsistent stream, could not find vertex.");

        high = addEdge(high,low);

        //! If we found a previously finalized vertex
        if (high != TNULL)
          finalizeVertex(high); // We must re-finalize it

        break;
      }

      case FINAL: {
        const FinalToken& t = token;

        TreeNodeIndex v = mTree->findElementIndex(t.index());

        (*mTree)[v].reduceMultiplicity();
        if ((*mTree)[v].multiplicity() == 0)
          finalizeVertex(v);

        break;
      }
    }
  }
}

bool StreamingGatherAlgorithm::isRegular(TreeNodeIndex v) const
{
  if ((*mTree)[v].down == TNULL)
    return false;

  if (getParent(v) == TNULL)
    return false;

  if ((*mTree)[getParent(v)].next != getParent(v))
    return false;

  return true;
}


TreeNodeIndex StreamingGatherAlgorithm::addEdge(TreeNodeIndex head, TreeNodeIndex tail)
{
  // Search for the lowest child of head that is still above tail
  head = findIntegrationVertex(head,tail);


  if (head == tail) { // If this edge already exists
    return TNULL; // There is nothing to do
  }
  else if (getChild(head) == TNULL) {// If even the root of the tree is higher than tail
    // Attach the tail branch to the head branch
    return attachBranch(head,tail);
  }
  else {
    // Otherwise, we need to merge two path
    return mergeBranches(head,tail);
  }
}

int StreamingGatherAlgorithm::finalizeVertex(TreeNodeIndex v)
{
  // If the node is now regular
  if (isRegular(v)) {

    // Unlink it from the tree
    mTree->bypass(v);

    // And remove it from memory
    mTree->deleteElement(v);
  }

  return 1;
}

int StreamingGatherAlgorithm::finalize()
{
  MergeTree::iterator it;

  // For all still active node
  for (it=mTree->begin();it!=mTree->end();it++) {

    // For all roots
    if (it->down == TNULL)
      outputTree(it);
  }

  Token empty;
  empty.type(EMPTY);

  (*mDownstream) << empty;
  (*mUpstream) << empty;

  return 1;
}



TreeNodeIndex StreamingGatherAlgorithm::findIntegrationVertex(TreeNodeIndex high, TreeNodeIndex low)
{
  // Search for the lowest child of high that is still above low
  while ((getChild(high) != TNULL) && !greater(low,getChild(high))) {
    high = getChild(high);
  }

  return high;

}

TreeNodeIndex StreamingGatherAlgorithm::attachBranch(TreeNodeIndex tail, TreeNodeIndex head)
{
  sterror(getChild(tail)!=TNULL,"You can only attach to disjoined branches.");

  setChild(tail,head); // tail becomes the new root

  if (isFinalized(tail)) // If head was finalized earlier
    return tail;// We must re-evaluate its type
  else
    return TNULL;
}

TreeNodeIndex StreamingGatherAlgorithm::mergeBranches(TreeNodeIndex left, TreeNodeIndex right)
{
  sterror(!greater(left,right),"Assumption violated. Left should be > right.");

  TreeNodeIndex next_right;
  TreeNodeIndex next_left;

  // While the two paths have not merged
  while ((left != right) && (getChild(left) != TNULL)){

    // We need to splice right between left and child(left)

    // Store the next vertex on the right path
    next_right = getChild(right);

    // To make things slightly faster we use a tmp variable
    next_left = getChild(left);
    setChild(left,right);
    setChild(right,next_left);

    // Now we determine whether we may have changed a finalized type

    // If we have reached the end of the right path
    if (next_right == TNULL) {
      if (isFinalized(right)) // And it previous root was finalized
        return right; // It just changed to become a regular vertex
      else
        return TNULL; // Otherwise we simply stop here
    }

    // If left used to be the root of its tree
    if (next_left == TNULL) {
      if (isFinalized(left)) // and it was finalized
        return left; // It just changed to become a regular vertex
      else
        return TNULL; // We are done anyway
    }

    // If both paths continue we reset the pointers

    left = right;
    right = next_right;

    // Advance the left path until you find the next integration
    // point. In case a derived class wants to use this version of
    // mergeBranches we make sure we stay within our class hierarchy
    left = StreamingGatherAlgorithm::findIntegrationVertex(left,right);
  }

  // If we get here this means that left == right != NULL
  sterror(left==TNULL,"Two paths should not merge at NULL.");

  // If we broke the traversal because there only remains an attaching
  // to do
  if (left != right) {

    setChild(left,right); // tail becomes the new root

    if (isFinalized(left)) // If head was finalized earlier it was a root
      return left;// We must re-evaluate its type (it became regular)
    else
      return TNULL;
  }

  // If the saddle at which we merge has been finalized it may have
  // changed type
  if ((left == right) && isFinalized(left))
    return left; // The saddle must be fixed

  return TNULL;
}

void StreamingGatherAlgorithm::outputVertex(const VertexToken& token) const
{
  std::vector<TopoOutputStream*>::const_iterator it;

  (*mDownstream) << token;
  (*mUpstream) << token;
}

void StreamingGatherAlgorithm::outputEdge(GlobalIndexType v0, GlobalIndexType v1) const
{
  std::vector<TopoOutputStream*>::const_iterator it;
  EdgeToken token;

  token.source(v0);
  token.destination(v1);

  (*mDownstream) << token;
  (*mUpstream) << token;
}

void StreamingGatherAlgorithm::outputTree(TreeNodeIndex root) const
{
  if ((*mTree)[root].up == TNULL) {
    (*mDownstream) << (*mTree)[root];
    (*mUpstream) << (*mTree)[root];
  }
  else {
    TreeNodeIndex up = (*mTree)[root].up;
    EdgeToken token;

    do {
      outputTree(up);

      if (up == (*mTree)[root].up)
        (*mUpstream) << (*mTree)[root]; // UP or down ?

      token.source(up);
      token.destination(root);

      (*mDownstream) << token;
      (*mUpstream) << token;


      up = (*mTree)[up].next;
    } while (up != (*mTree)[root].up);
  }

  FinalToken final;

  final.index(root);

  (*mDownstream) << final;
  (*mUpstream) << final;
}

