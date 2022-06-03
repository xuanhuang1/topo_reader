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


#include "BlockDecomposition.h"

BlockDecomposition::BlockDecomposition(GlobalIndexType domain[], uint32_t sub_domains[]) 
{
  mGlobalDomain[0] = domain[0];
  mGlobalDomain[1] = domain[1];
  mGlobalDomain[2] = domain[2];
  
  mSizeX = computeSize(mGlobalDomain[0],sub_domains[0]);
  mSizeY = computeSize(mGlobalDomain[1],sub_domains[1]);
  mSizeZ = computeSize(mGlobalDomain[2],sub_domains[2]);

  mSubDomains[0] = computeSize(mGlobalDomain[0],mSizeX);
  mSubDomains[1] = computeSize(mGlobalDomain[1],mSizeY);
  mSubDomains[2] = computeSize(mGlobalDomain[2],mSizeZ);
}

BlockDecomposition::BlockDecomposition(const BlockDecomposition& decomposition)
{
  for (int i=0;i<3;i++) {
    mGlobalDomain[i] = decomposition.mGlobalDomain[i];
    mSubDomains[i] = decomposition.mSubDomains[i];
  }

  mSizeX = decomposition.mSizeX;
  mSizeY = decomposition.mSizeY;
  mSizeZ = decomposition.mSizeZ;
}

BlockDecomposition::~BlockDecomposition()
{
}


bool BlockDecomposition::isShared(GlobalIndexType index) const
{
  GlobalIndexType i,j,k;

  splitIndex(index,i,j,k);

  if ((i % mSizeX == 0) && (i > 0) && (i < mGlobalDomain[0]-1))
    return true;

  if ((j % mSizeY == 0) && (j > 0) && (j < mGlobalDomain[1]-1))
    return true;

  if ((k % mSizeZ == 0) && (k > 0) && (k < mGlobalDomain[2]-1))
    return true;
  
  return false;
}

bool BlockDecomposition::isShared(GlobalIndexType index0, GlobalIndexType index1) const
{
  sterror(index0==index1,"A BlockDecomposition cannot handle degenerate edges.");

  GlobalIndexType i0,i1,j0,j1,k0,k1;

  splitIndex(index0,i0,j0,k0);
  splitIndex(index1,i1,j1,k1);


  if ((i0 == i1) && (i0 % mSizeX == 0) && (i0 > 0) && (i0 < mGlobalDomain[0]-1))
    return true;

  if ((j0 == j1) && (j0 % mSizeY == 0) && (j0 > 0) && (j0 < mGlobalDomain[1]-1))
    return true;

  if ((k0 == k1) && (k0 % mSizeZ == 0) && (k0 > 0) && (k0 < mGlobalDomain[2]-1))
    return true;
  
  return false;
}

bool BlockDecomposition::commonBoundary(GlobalIndexType index, GlobalIndexType boundary) const
{
  GlobalIndexType i,j,k;
  GlobalIndexType bi,bj,bk;

  splitIndex(index,i,j,k);
  splitIndex(boundary,bi,bj,bk);

  uint8_t codim_index;
  uint8_t codim_boundary;

  // First we compute the codimensions of the boundary components each
  // vertex is on. Note that this computation is not correct for edges
  // on the global boundary. However, this function assumes that the
  // given edge lies on a boundary between domains and thus this
  // situation should not occur
  codim_index = (i % mSizeX == 0) + (j % mSizeY == 0) + (k % mSizeZ == 0);
  codim_boundary = (bi % mSizeX == 0) + (bj % mSizeY == 0) + (bk % mSizeZ == 0);
  
  // If the boundary component of index has a lower dimension (higher
  // codimension) that that of boundary it cannot be part of
  // boundary's component
  if (codim_index > codim_boundary)
    return false;

  // If the boundary component of index has a higher dimension than
  // that of boundary it is always part of boundary's component
  // (assuming both are part of a sub-domain boundary in the first
  // place)
  if (codim_index < codim_boundary)
    return true;

  // If the dimensionalities match we are almost done. However, there
  // can exist edges that connect different boundary components of
  // equal dimension (edge across corners of quads for example). These
  // must be filtered out.

  // The equivalent condition is that for each coordinate axis that is
  // a boundary the indices must coincide

  // If index lies on an x-boundary but the x-index of the two
  // vertices do not agree
  if ((i % mSizeX == 0) && (i > 0) && (i < mGlobalDomain[0]-1) && (i != bi)) 
    return false; // Then we have a cross-boundary connection

  // Ditto for j
  if ((j % mSizeY == 0) && (j > 0) && (j < mGlobalDomain[1]-1) && (j != bj)) 
    return false;

  // Ditto for k
  if ((k % mSizeZ == 0) && (k > 0) && (k < mGlobalDomain[2]-1) && (k != bk)) 
    return false;

  return true;
}      


const std::vector<DomainID>& BlockDecomposition::getDomains(GlobalIndexType index)
{
  GlobalIndexType i,j,k;
  DomainID dx,dy,dz;

  splitIndex(index,i,j,k);

  dx = MIN(i / mSizeX,mSubDomains[0]-1);
  dy = MIN(j / mSizeY,mSubDomains[1]-1);
  dz = MIN(k / mSizeZ,mSubDomains[2]-1);
  

  // If the vertex lies on an x boundary
  if ((i % mSizeX == 0) && (i > 0) && (i < mGlobalDomain[0]-1)) {
    
    // And a y boundary
    if ((j % mSizeY == 0) && (j > 0) && (j < mGlobalDomain[1]-1)) {
    
      // And a z boundary
      if ((k % mSizeZ == 0) && (k > 0) && (k < mGlobalDomain[2]-1)) {
        
        this->mDomains.resize(8);
        int count = 0;
        for (int kk=-1;kk<1;kk++) {
          for (int jj=-1;jj<1;jj++) {
            for (int ii=-1;ii<1;ii++) {
              
              this->mDomains[count++] = ((dz+kk)*(mSubDomains[0]*mSubDomains[1]) 
                                         + (dy+jj)*mSubDomains[0] + dx + ii);
            }
          }
        }
      }
      else { // An x an y boundary 
        this->mDomains.resize(4);
        int count = 0;
        for (int jj=-1;jj<1;jj++) {
          for (int ii=-1;ii<1;ii++) {
              
            this->mDomains[count++] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                                       + (dy+jj)*mSubDomains[0] + dx + ii);
          }
        }
      }
    }
    else { // An x boundary only 
      this->mDomains.resize(2);
      this->mDomains[0] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                           + dy*mSubDomains[0] + dx -1);
      this->mDomains[1] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                           + dy*mSubDomains[0] + dx);
    }
  }
  else if ((j % mSizeY == 0) && (j > 0) && (j < mGlobalDomain[1]-1)) { // Not an x but a y boundary
    
      // And a z boundary
      if ((k % mSizeZ == 0) && (k > 0) && (k < mGlobalDomain[2]-1)) {
        this->mDomains.resize(4);
        int count = 0;
        for (int kk=-1;kk<1;kk++) {
          for (int jj=-1;jj<1;jj++) {
              
            this->mDomains[count++] = ((dz+kk)*(mSubDomains[0]*mSubDomains[1]) 
                                       + (dy+jj)*mSubDomains[0] + dx);
          }
        }
      }
      else { // Only a y boundary

        this->mDomains.resize(2);
        this->mDomains[0] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                             + (dy-1)*mSubDomains[0] + dx);
        this->mDomains[1] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                             + dy*mSubDomains[0] + dx);
      }
  }
  else if ((k % mSizeZ == 0) && (k > 0) && (k < mGlobalDomain[2]-1)) { // Only a z boudary
    
    this->mDomains.resize(2);
    this->mDomains[0] = ((dz-1)*(mSubDomains[0]*mSubDomains[1]) 
                         + dy*mSubDomains[0] + dx);
    this->mDomains[1] = (dz*(mSubDomains[0]*mSubDomains[1]) 
                         + dy*mSubDomains[0] + dx);
  }
  else { // No Boundary at all
    this->mDomains.resize(1);
    this->mDomains[0] = dz*(mSubDomains[0]*mSubDomains[1]) + dy*mSubDomains[0] + dx;
  }
   
  return this->mDomains;
}
        

const std::vector<DomainID>& BlockDecomposition::getDomains(GlobalIndexType index0,
                                                            GlobalIndexType index1)
{
  sterror(index0==index1,"A BlockDecomposition cannot handle degenerate edges.");

  GlobalIndexType i0,i1,j0,j1,k0,k1;
  DomainID d0x,d0y,d0z;

  splitIndex(index0,i0,j0,k0);
  splitIndex(index1,i1,j1,k1);
  
  d0x = MIN(MIN(i0 / mSizeX,i1 / mSizeX),mSubDomains[0]-1);
  d0y = MIN(MIN(j0 / mSizeY,j1 / mSizeY),mSubDomains[1]-1);
  d0z = MIN(MIN(k0 / mSizeZ,k1 / mSizeZ),mSubDomains[2]-1);
  
  
  //sterror(d0x!=d1x,"No edge should span different domains.");
  //sterror(d0y!=d1y,"No edge should span different domains.");
  //sterror(d0z!=d1z,"No edge should span different domains.");
  
  // If the vertex lies on an x boundary
  if ((i0 == i1) && (i0 % mSizeX == 0) && (i0 > 0) && (i0 < mGlobalDomain[0]-1)) {
    
    // And a y boundary
    if ((j0 == j1) && (j0 % mSizeY == 0) && (j0 > 0) && (j0 < mGlobalDomain[1]-1)) {
    
      this->mDomains.resize(4);
      int count = 0;
      for (int jj=-1;jj<1;jj++) {
        for (int ii=-1;ii<1;ii++) {
          
          this->mDomains[count++] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                                     + (d0y+jj)*mSubDomains[0] + d0x + ii);
        }
      }
    }
    else if ((k0 == k1) && (k0 % mSizeZ == 0) && (k0 > 0) && (k0 < mGlobalDomain[2]-1)) { // x and z boundary

      this->mDomains.resize(4);
      int count = 0;
      for (int kk=-1;kk<1;kk++) {
        for (int ii=-1;ii<1;ii++) {
          
          this->mDomains[count++] = ((d0z+kk)*(mSubDomains[0]*mSubDomains[1]) 
                                     + d0y*mSubDomains[0] + d0x + ii);
        }
      }
    }
    else { // An x boundary only 
      this->mDomains.resize(2);
      this->mDomains[0] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                           + d0y*mSubDomains[0] + d0x -1);
      this->mDomains[1] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                           + d0y*mSubDomains[0] + d0x);
    }
  }
  else if ((j0==j1) && (j0 % mSizeY == 0) && (j0 > 0) && (j0 < mGlobalDomain[1]-1)) { // Not an x but a y boundary
    
      // And a z boundary
    if ((k0==k1) && (k0 % mSizeZ == 0) && (k0 > 0) && (k0 < mGlobalDomain[2]-1)) {
      this->mDomains.resize(4);
      int count = 0;
      for (int kk=-1;kk<1;kk++) {
        for (int jj=-1;jj<1;jj++) {
          
          this->mDomains[count++] = ((d0z+kk)*(mSubDomains[0]*mSubDomains[1]) 
                                     + (d0y+jj)*mSubDomains[0] + d0x);
        }
      }
    }
    else { // Only a y boundary
      
      this->mDomains.resize(2);
      this->mDomains[0] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                           + (d0y-1)*mSubDomains[0] + d0x);
      this->mDomains[1] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                           + d0y*mSubDomains[0] + d0x);
    }
  }
  else if ((k0==k1) && (k0 % mSizeZ == 0) && (k0 > 0) && (k0 < mGlobalDomain[2]-1)) { // Only a z boudary
    
    this->mDomains.resize(2);
    this->mDomains[0] = ((d0z-1)*(mSubDomains[0]*mSubDomains[1]) 
                         + d0y*mSubDomains[0] + d0x);
    this->mDomains[1] = (d0z*(mSubDomains[0]*mSubDomains[1]) 
                         + d0y*mSubDomains[0] + d0x);
  }
  else { // No Boundary at all
    this->mDomains.resize(1);
    this->mDomains[0] = d0z*(mSubDomains[0]*mSubDomains[1]) + d0y*mSubDomains[0] + d0x;
  }
  
    
  return this->mDomains;
}


GlobalIndexType BlockDecomposition::computeSize(GlobalIndexType size, uint32_t n)
{
  GlobalIndexType tmp;

  tmp = size / n;

  if (tmp*n < size) 
    tmp++;

  return MAX(1,tmp);
}

void BlockDecomposition::splitIndex(GlobalIndexType index, GlobalIndexType& i, GlobalIndexType& j, 
                                    GlobalIndexType& k) const
{
  k = index / (mGlobalDomain[0]*mGlobalDomain[1]);
  j = (index / mGlobalDomain[0]) % mGlobalDomain[1];
  i = index % mGlobalDomain[0];
}
