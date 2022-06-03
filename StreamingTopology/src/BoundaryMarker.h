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


#ifndef BOUNDARYMARKER_H_
#define BOUNDARYMARKER_H_



enum BoundaryComponent {
  LX             = 1 << 0,
  HX             = 1 << 1,
  LY             = 1 << 2,
  HY             = 1 << 3,
  LZ             = 1 << 4,
  HZ             = 1 << 5,
  LX_LY          = 1 << 6,
  LX_HY          = 1 << 7,
  LX_LZ          = 1 << 8,
  LX_HZ          = 1 << 9,
  HX_LY          = 1 << 10,
  HX_HY          = 1 << 11,
  HX_LZ          = 1 << 12,
  HX_HZ          = 1 << 13,
  LY_LZ          = 1 << 14,
  LY_HZ          = 1 << 15,
  HY_LZ          = 1 << 16,
  HY_HZ          = 1 << 17,
  LX_LY_LZ       = 1 << 18,
  HX_LY_LZ       = 1 << 19,
  LX_HY_LZ       = 1 << 20,
  HX_HY_LZ       = 1 << 21,
  LX_LY_HZ       = 1 << 22,
  HX_LY_HZ       = 1 << 23,
  LX_HY_HZ       = 1 << 24,
  HX_HY_HZ       = 1 << 25,
};


//! A BoundaryMarker encodes which boundary component a vertex is part of
class BoundaryMarker
{
public:
  //! Typedef to define the number of available bits
  typedef uint8_t MarkerType;

  //! Default constructor
  BoundaryMarker() : mFlags(0) {}

  //! Type conversion constructor
  BoundaryMarker(MarkerType marker) : mFlags(marker) {}

  //! Copy constructor
  BoundaryMarker(const BoundaryMarker& marker) : mFlags(marker.mFlags) {}

  //! Destructor
  ~BoundaryMarker() {}

  //! Comparison operator
  bool operator==(const BoundaryMarker& marker) const {return (mFlags == marker.mFlags);}

  //! Comparison operator
  bool operator!=(const BoundaryMarker& marker) const {return (mFlags != marker.mFlags);}

  //! Determine whether all of my boundary markers are part of marker
  bool operator<(const BoundaryMarker& marker) const {return (mFlags == (mFlags & marker.mFlags));}

  //! Return the flags for this vertex
  MarkerType markers() const {return mFlags;}

  //! The the marker of the b's boundary component
  void set(MarkerType b);

  //! Clear all markers
  void clear() {mFlags = 0;}

private:

  //! The set of boundary markers for this vertex
  MarkerType mFlags;

  //! Determine whether the given flag(s) have been set
  bool marked(MarkerType b) {return b == (mFlags & b);}


};

#endif /* BOUNDARYMARKER_H_ */
