/*
 * SegmentedMergeTree.h
 *
 *  Created on:
 *      Author: 
 */

#ifndef SEGMENTEDMERGETREE_H_
#define SEGMENTEDMERGETREE_H_

#include "MergeTree.h"
#include <vector>

class SegmentedMergeTree : public MergeTree
{

	std::vector< std::vector<LocalIndexType> > segmentations;

public:

 SegmentedMergeTree(GraphID id, uint8_t block_bits=FlexArray::BlockedArray<TreeNode>::sBlockBits) : MergeTree(id, block_bits) {}

 virtual ~SegmentedMergeTree() {}

	void addToSegment(LocalIndexType node_index, LocalIndexType curr_index) {
	  LocalIndexType v = findElementIndex(node_index);
		segmentations[v].push_back(curr_index);
	}

  virtual void addNode(LocalIndexType index, float value, MultiplicityType boundary_count) {
	  MergeTree::addNode(index, value, boundary_count);

	  // Add a new segmentation (do we need to check that the order matches ?)
	  segmentations.push_back(std::vector<LocalIndexType>());

	  // add a node to its own segmetnation
	  segmentations.back().push_back(index);
  }

};


#endif /* SEGMENTEDUNIONTREE_H_ */
