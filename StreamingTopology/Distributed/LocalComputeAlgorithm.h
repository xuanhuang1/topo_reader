/*
 * LocalComputeAlgorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef LOCALCOMPUTEALGORITHM_H_
#define LOCALCOMPUTEALGORITHM_H_

#include "PatchAlgorithm.h"
#include "TopoOutputStream.h"
#include "SegmentedMergeTree.h"
#include "Patch.h"
#include "Neighborhood.h"


#include <algorithm>
#include <vector>

using namespace std;

enum LocalAlgorithmType {
  LOCAL_SORTED_UF = 0,
};


//! The API to compute a local tree
class LocalComputeAlgorithm : public PatchAlgorithm
{
public:

  //! The factory function
  static LocalComputeAlgorithm* make(LocalAlgorithmType type, bool invert);

  //! Default constructor
  LocalComputeAlgorithm(bool invert=false) : PatchAlgorithm(invert) {}

  //! Destructor
  virtual ~LocalComputeAlgorithm() {}

  //! Apply this algorithm and store the resulting tree
  virtual int apply(const FunctionType* field, SegmentedMergeTree& tree,
                    TopoOutputStream* stream) const = 0;

};



//! The API to compute a local tree
class LocalSortedUF : public LocalComputeAlgorithm
{
protected:
	LocalIndexType* UFParents;
	vector<LocalIndexType> sorted_indices;


	LocalIndexType Find(LocalIndexType index) {
		if (UFParents[index] == index) return index;
		if (UFParents[index] == -1) {
			UFParents[index] = index;
			return index;
		}
		LocalIndexType result = Find(UFParents[index]);
		UFParents[index] = result;
		return result;
	}

	LocalIndexType Union(LocalIndexType a, LocalIndexType b) {
		LocalIndexType aroot = Find(a);
		LocalIndexType broot = Find(b);
		if (aroot == broot) return aroot;
		if (this->mInvert) {
			if (this->greater(aroot, broot)) {
				UFParents[aroot] = broot;
				return broot;
			} else {
				UFParents[broot] = aroot;
				return aroot;
			}

		} else {
			if (this->smaller(aroot, broot)) {
				UFParents[aroot] = broot;
				return broot;
			} else {
				UFParents[broot] = aroot;
				return aroot;
			}

		}
	}



public:

 

  //! Default constructor
  LocalSortedUF(bool invert=false) : LocalComputeAlgorithm(invert) {}

  //! Destructor
  virtual ~LocalSortedUF() {
	  delete[] UFParents;
  }

 


  //! Apply this algorithm and store the resulting tree
  virtual int apply( Patch* patch, const int field, SegmentedMergeTree& tree,
                    TopoOutputStream* stream) {

    const FunctionType* tField = patch->field(field);
		const Box& tDomain = patch->domain();
		
		Neighborhood neigh(tDomain);


		const LocalIndexType num_vertices = tDomain.localSize(); // field->num_vertices()?

		// initialize sorted vector and also uf parents
		UFParents = new LocalIndexType[num_vertices];
		sorted_indices.clear();
		for (int i = 0; i < num_vertices; i++) {
			sorted_indices.push_back(i);
			UFParents[i] = -1;
		}
		/*
		// sort vector
		if(this->mInvert) {
			std::sort(sorted_indices.begin(), sorted_indices.end(), smaller);
		} else {
			sort(sorted_indices.begin(), sorted_indices.end(), greater);
		}
		 */

		for (int i = 0; i < num_vertices; i++) {
			
			// add ith vertex to tree?
			int tBoundaryCount = tDomain.boundaryCount(i);

			int tNumRegions = 0;
			NeighborIterator it = neigh.begin(i);
			vector<LocalIndexType> tOthers;
			while (neigh.valid(it)) {
				// assume the iterator will only give us other vertices with the same boundary count
				
				LocalIndexType neighbor = neigh.value(it);
				neigh.advance(it);

				// SKIP OVER ANYTHING SMALLER - or "not seen"
				if (UFParents[neighbor] == -1) {
					// then it has not been seen
					continue;
				}

				if (Find(i) != Find(neighbor)) {
					tOthers.push_back(Find(neighbor));
					Union(i, neighbor);
					tNumRegions++;
				}
			}

			if (tNumRegions == 0) {
				//max, add to tree
				UFParents[i] = i;
				tree.addNode(i, tField[i], tBoundaryCount);

			} else if (tNumRegions == 1) {
				//regular, only add to segment
				tree.addToSegment(Find(i), i);

			} else {
				// saddle, so add to tree
				tree.addNode(i, tField[i], tBoundaryCount);
				for (int j = 0; j < tOthers.size(); j++) {
					tree.addEdge(tOthers[j], i);
				}
			}

			
		}

  }

};


#endif /* LOCALCOMPUTEALGORITHM_H_ */
