/*
 * PeriodicGridParser.h
 *
 *  Created on: Nov 7, 2013
 *      Author: bremer5
 */

#ifndef PERIODICGRIDPARSER_H
#define PERIODICGRIDPARSER_H

#include "GridParser.h"

//! Class to parse periodic non-interleaved grids
template <class DataClass = GenericData<float> >
class PeriodicGridParser : public virtual GridParser<DataClass>
{

public:

  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param function: file stream of the raw function to be read
   *  @param dim: index of samples in each dimension
   *  @param period: bitmask to determine which directions are periodic
   *  @param edim: number of coordinates per vertex (N/A)
   *  @param fdim: index of the coordinate that should be the function (N/A)
   *  @param adims: indices of the coordinates that should be preserved (N/A)
   *  @param attributes: file pointers to any additional attributes of interest
   *  @param map_file: optional file pointer to store the index map if compactifying
   */
  PeriodicGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                     uint8_t period, uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                     bool compact=true, FILE* map_file=NULL);

  //! Destructor
  virtual ~PeriodicGridParser();

  //! Is the grid periodic in X
  int periodicX() const {return ((mPeriod & 1) != 0) ? 1 : 0;}

  //! Is the grid periodic in Y
  int periodicY() const {return ((mPeriod & 2) != 0) ? 1 : 0;}

  //! Is the grid periodic in Z
  int periodicZ() const {return ((mPeriod & 4) != 0) ? 1 : 0;}

protected:

  //! The period in each dimension encoded a s bit flag where the
  //! lowest bit encodes the periodicity in X, the second lowest in y, and
  //! the third lowest in z;
  uint8_t mPeriod;

  //! The first plane of indices that store the index map to global indices
  GlobalIndexType* mFirstPlaneMap;

  //! Added the edges of the mesh to the stack
  virtual void addEdges();

  //! Added the edges of the mesh to the stack
  virtual void addEdges(int32_t i,int32_t j,int32_t k);

  //! Create an actual edge from the stack
  virtual void makeEdge();

  /*! According to the current local indices determine the global index of the
   * last vertex that may have an edge with mIndex / mId / mLocal
   * @return global index of the last vertex that may have an edge with this one
   */
  virtual GlobalIndexType lastUsed() const;

  //! Read the next plane of data
  virtual int readDataPlane();
};

template <class DataClass>
PeriodicGridParser<DataClass>::PeriodicGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                                                  uint8_t period, uint32_t fdim, const std::vector<uint32_t>& adims, bool compact, FILE* map_file)
                                                  : GridParser<DataClass>(attributes,dim_x,dim_y, dim_z, fdim,adims,compact,map_file),
                                                    mPeriod(period)
{
  mFirstPlaneMap = new GlobalIndexType[this->mDimX*this->mDimY];
}

template <class DataClass>
PeriodicGridParser<DataClass>::~PeriodicGridParser()
{
  delete[] mFirstPlaneMap;
}

template <class DataClass>
void PeriodicGridParser<DataClass>::addEdges()
{
  GlobalIndexType e;
  for (int i=0;i<this->sEdgeNr;i++) {
    if ((this->sEdgeTable[i][0]+this->mI < 0) || (this->sEdgeTable[i][0]+this->mI >= this->mDimX)
        || (this->sEdgeTable[i][1]+this->mJ < 0) || (this->sEdgeTable[i][1]+this->mJ >= this->mDimY)
        || (this->sEdgeTable[i][2]+this->mK < 0)) {
      continue;
    }

    //mEdges.push(mIndex + mEdgeOffset[i]);
    e = this->mIndexMap[this->sEdgeTable[i][2] + 1][(this->mJ+this->sEdgeTable[i][1])*this->mDimX + this->mI+this->sEdgeTable[i][0]];
    if (e != GNULL) {
      this->mEdges.push(this->mId);
      this->mEdges.push(e);
    }
  }

  // For the current vertex with global index mIndex and local index mLocal
  // figure out which is the last global vertex that may have an edge using
  // the current vertex
  this->mProcessed.push(typename GridParser<DataClass>::FinalizationInfo(lastUsed(),this->mId,&(this->mRestrictedFlag)));

  //return;
  // All the vertices on the first plane will be processed again later anyhow
  //if (this->mK == 0)
  //  return;

  // Now we need to add the extra periodic edges if necessary
  if (periodicX() && (this->mI == this->mDimX-1)) {
    if (periodicY() && (this->mJ == this->mDimY-1)) {
      if (periodicZ() && (this->mK == this->mDimZ-1)) {
        addEdges(0,0,0);
      }
      addEdges(0,0,this->mK);
    }
    else if (periodicZ() && (this->mK == this->mDimZ-1)) {
      addEdges(0,this->mJ,0);
    }

    addEdges(0,this->mJ,this->mK);
  }

  if(periodicY() && (this->mJ == this->mDimY-1)) {
    if (periodicZ() && (this->mK == this->mDimZ-1)) {
      addEdges(this->mI,0,0);
    }
    addEdges(this->mI,0,this->mK);
  }

  if (periodicZ() && (this->mK == this->mDimZ-1)) {
    addEdges(this->mI,this->mJ,0);
  }

}


template <class DataClass>
void PeriodicGridParser<DataClass>::addEdges(int32_t x,int32_t y, int32_t z)
{
  int32_t i,j,k;
  GlobalIndexType origin;
  if (z == this->mK)
    origin = this->mIndexMap[1][y*this->mDimX + x];
  else if (z == 0)
    origin = mFirstPlaneMap[y*this->mDimX + x];
  else
    origin = this->mIndexMap[0][y*this->mDimX + x];


  //fprintf(stderr,"Periodic edges of %d %d %d\n",x,y,z);
  GlobalIndexType e;
  for (int u=0;u<this->sEdgeNr;u++) {

    i = this->sEdgeTable[u][0] + x;
    if (periodicX())
      i = (i + this->mDimX) % this->mDimX;

    j = this->sEdgeTable[u][1] + y;
    if (periodicY())
      j = (j + this->mDimY) %this->mDimY;

    k = this->sEdgeTable[u][2] + z;
    if (periodicZ())
      k = (k + this->mDimZ) % this->mDimZ;

    // If these coordinates are outside the box
    if ((i < 0) || (i >= this->mDimX)
        || (j < 0) || (j >= this->mDimY)
        || (k < 0) || (k >= this->mDimZ))
      continue;


    // Given the modulo operations we might be inside the box but point to
    // vertices that do not yet exist
    if ((k > this->mK)
        || ((k == this->mK) && (j > this->mJ))
        || ((k == this->mK) && (j == this->mJ) && (i > this->mI)))
      continue;

    //fprintf(stderr,"\t periodic edge %d %d %d\n",i,j,k);

    if (k == this->mK)
      e = this->mIndexMap[1][j*this->mDimX + i];
    else if (k == 0)
      e = mFirstPlaneMap[j*this->mDimX + i];
    else
      e = this->mIndexMap[0][j*this->mDimX + i];

    if (e != GNULL) {
      this->mEdges.push(origin);
      this->mEdges.push(e);
    }
  }
}

template <class DataClass>
void PeriodicGridParser<DataClass>::makeEdge()
{
  this->mPath[1] = this->mEdges.top();
  this->mEdges.pop();

  this->mPath[0] = this->mEdges.top();
  this->mEdges.pop();
}


template <class DataClass>
GlobalIndexType PeriodicGridParser<DataClass>::lastUsed() const
{
  uint32_t lastI,lastJ,lastK;

  if ((!periodicX() || (this->mI > 0)) && (this->mI < this->mDimX-1))
    lastI = this->mI+1;
  else
    lastI = this->mDimX-1;


  if ((!periodicY() || (this->mJ > 0)) && (this->mJ < this->mDimY-1))
    lastJ = this->mJ+1;
  else
    lastJ = this->mDimY-1;


  if ((!periodicZ() || (this->mK > 0)) && (this->mK < this->mDimZ-1))
    lastK = this->mK+1;
  else
    lastK = this->mDimZ-1;

  return lastK*this->mDimX*this->mDimY + lastJ*this->mDimX + lastI;
}

template <class DataClass>
int PeriodicGridParser<DataClass>::readDataPlane()
{
  GridParser<DataClass>::readDataPlane();

  // This code is copied in ImplicitPeriodicGridParser
  if (this->mK == 1)
    memcpy(mFirstPlaneMap,this->mIndexMap[0],this->mDimX*this->mDimY*sizeof(GlobalIndexType));

  return 1;
}


#endif /* PERIODICGRIDPARSER_H_ */
