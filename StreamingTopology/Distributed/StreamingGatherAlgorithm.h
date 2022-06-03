/*
 * StreamingGatherAlgorithm.h
 *
 *  Created on: Feb 21, 2012
 *      Author: bremer5
 */

#ifndef STREAMINGGATHERALGORITHM_H_
#define STREAMINGGATHERALGORITHM_H_

#include "GatherAlgorithm.h"

class StreamingGatherAlgorithm : public GatherAlgorithm
{
public:

  //! Default constructor
  StreamingGatherAlgorithm(bool invert = false) : GatherAlgorithm(invert) {}

  //! Destructor
  virtual ~StreamingGatherAlgorithm() {}

  //! Apply the algorithm
  virtual int apply(MergeTree& tree, TopoInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow);

protected:

  //! The default undefined token
  static Token sUndefinedToken;

  //! The reference to the union tree
  MergeTree* mTree;

  //! The output stream
  TopoOutputStream* mDownstream;

  //! The vector of upstream trees for scattering
  TopoOutputStream* mUpstream;

  /********************************************************************
   ********************* Computation Interface *************************
   *******************************************************************/

  //! Return the child of v
  virtual TreeNodeIndex getChild(const TreeNodeIndex v) const {return (*mTree)[v].down;}

  //! Return the parent of v
  virtual TreeNodeIndex getParent(const TreeNodeIndex v) const {return (*mTree)[v].up;}

  //! Determine whether v is a regular node
  virtual bool isRegular(TreeNodeIndex v) const;

  //! Set the child of v to down
  virtual void setChild(TreeNodeIndex v, TreeNodeIndex down) const {sterror(v==down,"Illegal child pointer, no loops allowed.");(*mTree)[v].down = down;}

  //! Indicate whether v has been finalized
  /*! Determine whether the given tree node will be part of more edges.
   *  Note that each finalize call reduces the multiplicity and all input
   *  nodes are expected to be boundary nodes. As a result a node is
   *  finalizes if and only if its multiplicity has been reduce to 0.
   * @param v: The index of the node in question
   * @return true, if there are more edges coming; false otherwise
   */
  virtual bool isFinalized(TreeNodeIndex v) const {return ((*mTree)[v].multiplicity() == 0);}


  /****************************************************************
   **************   Internal Streaming API ************************
   ****************************************************************/

  //! Add an edge
  virtual TreeNodeIndex addEdge(TreeNodeIndex high, TreeNodeIndex low);

  //! Finalize a node
  virtual int finalizeVertex(TreeNodeIndex vertex);

  //! Finalize the remaining vertices
  virtual int finalize();

  /********************************************************************
   ********************* Subrountines          *************************
   *******************************************************************/

  //! Determine wether *u > *v
  bool greater(TreeNodeIndex u, TreeNodeIndex v) const {return Algorithm::greater((*mTree)[u],(*mTree)[v]);}

  //! Find the lowest child of high that is >= low
  virtual TreeNodeIndex findIntegrationVertex(TreeNodeIndex high, TreeNodeIndex low);

  //! Attach branch starting at head to the branch ending at tail
  /*! This call assumes that getChild(tail) == NULL and attaches the
   *  branch starting at head to the branch ending at tail. If tail
   *  was finalized the function will return tail to re-evaluate its
   *  type otherwise it return NULL.
   */
  virtual TreeNodeIndex attachBranch(TreeNodeIndex tail, TreeNodeIndex head);

  //! Merge the two branches starting at left and right respectively
  /*! This call merge-sorts the two paths starting at left and right
   *  respectively into a single branch. The initial assumption is
   *  that left > right. If the merging causes a finalized critial
   *  point to potentially change its type a pointer to this vertex is
   *  returned. Otherwise, the function returns NULL.
   */
  virtual TreeNodeIndex mergeBranches(TreeNodeIndex left, TreeNodeIndex right);

  //! Output a vertex
  virtual void outputVertex(const VertexToken& token) const;

  //! Output an edge
  virtual void outputEdge(GlobalIndexType v0, GlobalIndexType v1) const;

  //! Output tree
  virtual void outputTree(TreeNodeIndex root) const;
};


#endif /* STREAMINGGATHERALGORITHM_H_ */
