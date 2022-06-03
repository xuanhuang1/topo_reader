/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov 
* OCEC-08-107
* All rights reserved.  
*   
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*   
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*   
*  
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/


#ifndef PERIODICGRIDEDGEPARSER_H
#define PERIODICGRIDEDGEPARSER_H

#include <stack>

#include "GridEdgeParser.h"
#include "GenericData.h"

//! Class to parse interleaved grids
template <class DataClass = GenericData<FunctionType> >
class PeriodicGridEdgeParser : public GridEdgeParser<DataClass>
{

public:

  //! Repeat of the typedef since most compilers are not smart enough
  typedef typename DataClass::FunctionType FunctionType;

  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param input: file stream of the file to be read
   *  @param dim_x: number of samples on the x-axis
   *  @param dim_y: number of samples on the y-axis
   *  @param dim_z: number of samples on the z-axis
   *  @param edim: number of coordinates per vertex
   *  @param fdim: index of the coordinate that should be the function
   *  @param adims: indices of the coordinates that should be preserved
   *  @param period: bitmask to determine which directions are periodic
   */
  PeriodicGridEdgeParser(FILE* input,  int dim_x=1, int dim_y=1, int dim_z=1, uint32_t edim=3,  
                 uint32_t fdim=0,const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                 uint8_t period=0);

  //! Destructor
  virtual ~PeriodicGridEdgeParser();
  
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
  const uint8_t mPeriod;

  virtual bool isPeriodic() const {return (mPeriod != 0);}

  //! Add the necessary edges for the current vertex
  virtual void addEdges();

  //! Add the necessary processed vertices
  virtual void addProcessed();
};


template <class DataClass>
PeriodicGridEdgeParser<DataClass>::PeriodicGridEdgeParser(FILE* input, int dim_x, int dim_y, int dim_z, uint32_t edim,
                                          uint32_t fdim, const std::vector<uint32_t>& adims, uint8_t period)
  : GridEdgeParser<DataClass>(input, dim_x, dim_y, dim_z, edim, fdim, adims), mPeriod(period)
{
  if (periodicZ()) {
    fprintf(stderr,"Not implemented yet");
    exit(0);
  }

}

template <class DataClass>
PeriodicGridEdgeParser<DataClass>::~PeriodicGridEdgeParser()
{
}

template <class DataClass>
void PeriodicGridEdgeParser<DataClass>::addEdges()
{
  // These are all the "normal" edges that point "behind the current front
  for (int i=0;i<this->sEdgeNr;i++) {
    if ((this->sEdgeTable[i][0]+this->mI < 0) || (!periodicX() && (this->sEdgeTable[i][0]+this->mI >= this->mDimX))
        || (this->sEdgeTable[i][1]+this->mJ < 0) || (!periodicY() && (this->sEdgeTable[i][1]+this->mJ >= this->mDimY))
        || (this->sEdgeTable[i][2]+this->mK < 0)) {
      continue;
    }

    // We have to account for potential periodic edges
    this->mEdges.push((this->mI + this->sEdgeTable[i][0]) % this->mDimX +
                      (this->mJ + this->sEdgeTable[i][1]) % this->mDimY * this->mDimX +
                      (this->mK + this->sEdgeTable[i][2])*this->mDimX*this->mDimY);
  }


  int8_t offset[3] = {0,0,0};

  if (periodicX() && (this->mI == this->mDimX-1))
    offset[0] = 1;

  if (periodicY() && (this->mJ == this->mDimY-1))
    offset[1] = 1;

  if (periodicZ() && (this->mK == this->mDimZ-1))
    offset[2] = 1;


  if (offset[0] + offset[1] + offset[2] > 0) {

    for (int z=0;z<=offset[2];z++) {
      for (int y=0;y<=offset[1];y++) {
         for (int x=0;x<=offset[0];x++) {

           if (x+y+z == 0)
             continue;

           for (int i=0;i<this->sEdgeNr;i++) {
             if ((this->sEdgeTable[i][0]+this->mI < 0) || (this->sEdgeTable[i][0]+this->mI >= this->mDimX+x) // Notice the = instead of the >=
                 || (this->sEdgeTable[i][1]+this->mJ < 0) || (this->sEdgeTable[i][1]+this->mJ >= this->mDimY+y)
                 || (this->sEdgeTable[i][2]+this->mK < 0)) {
               continue;
             }

             // The id of this vertex in periodic space
             this->mPeriodicEdges.push(((this->mI + x)%this->mDimX + ((this->mJ + y) % this->mDimY) * this->mDimX +this->mK*(this->mDimX*this->mDimY)));
             this->mPeriodicEdges.push((this->mI + x + this->sEdgeTable[i][0]) % this->mDimX +
                                (this->mJ + y + this->sEdgeTable[i][1]) % this->mDimY * this->mDimX +
                                (this->mK + z + this->sEdgeTable[i][2])*this->mDimX*this->mDimY);
           }
         }
      }
    }
  }

}

template <class DataClass>
void PeriodicGridEdgeParser<DataClass>::addProcessed()
{
  if (this->mK > periodicZ()) {

    if ((this->mI > periodicX()) && (this->mJ > periodicY()))
      this-> mProcessed.push(this->mIndex + this->mEdgeOffset[0]);

    if ((this->mI == this->mDimX-1) && (this->mJ > periodicY())) {
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[1]);

      if (periodicX())
        this->mProcessed.push((this->mJ-1)*this->mDimX +this->mK*this->mDimX*this->mDimY);
    }

    if ((this->mJ == this->mDimY-1) && (this->mI > periodicX())) {
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[3]);

      if (periodicY())
        this->mProcessed.push(this->mI-1 + this->mK*this->mDimX*this->mDimY);
    }

    if ((this->mJ == this->mDimY-1) && (this->mI == this->mDimX-1)) {
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[4]);

      if (periodicX())
        this->mProcessed.push(this->mJ*this->mDimY + this->mK*this->mDimX*this->mDimY);

      if (periodicY())
        this->mProcessed.push(this->mI + this->mK*this->mDimX*this->mDimY);

      if (periodicX() && periodicY())
        this->mProcessed.push(this->mK*this->mDimX*this->mDimY);
    }
  }

  if (this->mK == this->mDimZ-1) {

    if ((this->mI > periodicX()) && (this->mJ > periodicY()))
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[9]);

    if ((this->mI == this->mDimX-1) && (this->mJ > periodicY())) {
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[10]);

      if (periodicX())
        this->mProcessed.push((this->mJ-1)*this->mDimX +this->mK*this->mDimX*this->mDimY);
    }

    if ((this->mJ == this->mDimY-1) && (this->mI > periodicX())) {
      this->mProcessed.push(this->mIndex + this->mEdgeOffset[12]);

      if (periodicY())
        this->mProcessed.push(this->mI-1 + this->mK*this->mDimX*this->mDimY);
    }

    if ((this->mJ == this->mDimY-1) && (this->mI == this->mDimX-1)) {
      this->mProcessed.push(this->mIndex);

      if (periodicX())
        this->mProcessed.push(this->mJ*this->mDimY + this->mK*this->mDimX*this->mDimY);

      if (periodicY())
        this->mProcessed.push(this->mI + this->mK*this->mDimX*this->mDimY);

      if (periodicX() && periodicY())
        this->mProcessed.push(this->mK*this->mDimX*this->mDimY);
    }
  }


}


#endif
