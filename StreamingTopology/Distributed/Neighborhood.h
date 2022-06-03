#ifndef NEIGHBORHOOD
#define NEIGHBORHOOD




struct NeighborIterator{
	int position;
	LocalIndexType baseID;
	int boundary;
};

const int neighborList[27][3] = {
	{ 0,0,0 },
	{ -1,-1,-1 },{ 0,-1,-1 },{ 1,-1,-1 },
	{ -1, 0,-1 },{ 0, 0,-1 },{ 1, 0,-1 },
	{ -1, 1,-1 },{ 0, 1,-1 },{ 1, 1,-1 },
	
	{ -1,-1, 0 },{ 0,-1, 0 },{ 1,-1, 0 },
	{ -1, 0, 0 },            { 1, 0, 0 },
	{ -1, 1, 0 },{ 0, 1, 0 },{ 1, 1, 0 },

	{ -1,-1, 1 },{ 0,-1, 1 },{ 1,-1, 1 },
	{ -1, 0, 1 },{ 0, 0, 1 },{ 1, 0, 1 },
	{ -1, 1, 1 },{ 0, 1, 1 },{ 1, 1, 1 }
};

class Neighborhood {




	LocalIndexType mOffsetList[27];

	const Box& mBox;

	bool insideBox(NeighborIterator& it) {
		return true;
		//++ this is where the index to point test, then test coordinates goes
	}

public:

	Neighborhood(const Box& box) : mBox(box) { 
		//for (int i = 0; i < 27; i++) {
		//	offsetList[i] = neighborList[i][0] + 
		//		neighborList[i][1] * mBox.
		//}
		//++ NEED LOCAL DIMENSIONS HERE AVAILABLE
	}

	virtual void initialize(){}
	virtual NeighborIterator begin(LocalIndexType index) {
		NeighborIterator it;
		it.position = 0;
		it.baseID = index;
		it.boundary = mBox.boundaryCount(index);
		
		// now get first valid position
		advance(it);
		return it;
	}
	virtual void advance(NeighborIterator& it) {
		
		if (! valid(it)) return;
		if (it.boundary > 0) {
			it.position++;

			while (valid(it) &&
				! insideBox(it) &&
				mBox.boundaryCount(value(it)) < it.boundary) {
					it.position++;
			}
		} else {
			it.position++;
		}
	
	}
	virtual bool valid(NeighborIterator& it) { 
		return it.position < 27; // hardcoded neighborhood
	}
	virtual LocalIndexType value(NeighborIterator& it) {
		return it.baseID + mOffsetList[it.position];
	}



};


#endif
