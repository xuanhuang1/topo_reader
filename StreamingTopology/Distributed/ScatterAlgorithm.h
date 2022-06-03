/*
 * ScatterAlgorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef SCATTERALGORITHM_H_
#define SCATTERALGORITHM_H_

#include <vector>
#include "TopoInputStream.h"
#include "TopoOutputStream.h"
#include "MergeTree.h"
#include "Algorithm.h"

enum ScatterAlgorithmType {
  SCATTER_ITERATIVE_TRAVERSAL = 0,
};

//! The API for an algorithm to correct a tree from a single input
class ScatterAlgorithm : public Algorithm
{
public:

  //! The factory function
  static ScatterAlgorithm* make(ScatterAlgorithmType type, bool invert);

  //! Default constructor
  ScatterAlgorithm(bool invert=false) : Algorithm(invert) {}

  //! Destructor
  ~ScatterAlgorithm() {}

  virtual int apply(MergeTree& tree, TopoInputStream* input,
                    TopoOutputStream* outputs) const = 0;

};

#endif /* SCATTERALGORITHM_H_ */
