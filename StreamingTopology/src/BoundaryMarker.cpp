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

#include "Definitions.h"
#include "BoundaryMarker.h"

void BoundaryMarker::set(MarkerType b)
{
  sterror(b > 5,"There can be at most six base boundaries.");

  // First set the raw flag for this boundary component
  mFlags |= (1 << b);

  return;
  // Now determine whether there are combinations that must be set
  switch (b) {

    case 0: // LX
      if (marked(LY))
        mFlags |= LX_LY;
      else if (marked(HY))
        mFlags |= LX_HY;

      if (marked(LZ))
        mFlags |= LX_LZ;
      else if (marked(HZ))
        mFlags |= LX_HZ;

      if (marked(LY) && marked(LZ))
        mFlags |= LX_LY_LZ;
      else if (marked(HY) && marked(LZ))
        mFlags |= LX_HY_LZ;
      else if (marked(LY) && marked(HZ))
        mFlags |= LX_LY_HZ;
      else if (marked(HY) && marked(HZ))
        mFlags |= LX_HY_HZ;

      break;

    case 1: // HX

      if (marked(LY))
        mFlags |= HX_LY;
      else if (marked(HY))
        mFlags |= HX_HY;

      if (marked(LZ))
        mFlags |= HX_LZ;
      else if (marked(HZ))
        mFlags |= HX_HZ;

      if (marked(LY) && marked(LZ))
        mFlags |= HX_LY_LZ;
      else if (marked(HY) && marked(LZ))
        mFlags |= HX_HY_LZ;
      else if (marked(LY) && marked(HZ))
        mFlags |= HX_LY_HZ;
      else if (marked(HY) && marked(HZ))
        mFlags |= HX_HY_HZ;

      break;

    case 2: // LY

      if (marked(LX))
        mFlags |= LX_LY;
      else if (marked(HX))
        mFlags |= HX_LY;

      if (marked(LZ))
        mFlags |= LY_LZ;
      else if (marked(HZ))
        mFlags |= LY_HZ;

      if (marked(LX) && marked(LZ))
        mFlags |= LX_LY_LZ;
      else if (marked(HX) && marked(LZ))
        mFlags |= HX_LY_LZ;
      else if (marked(LX) && marked(HZ))
        mFlags |= LX_LY_HZ;
      else if (marked(HX) && marked(HZ))
        mFlags |= HX_LY_HZ;

      break;

    case 3: // HY

      if (marked(LX))
        mFlags |= LX_HY;
      else if (marked(HX))
        mFlags |= HX_HY;

      if (marked(LZ))
        mFlags |= HY_LZ;
      else if (marked(HZ))
        mFlags |= HY_HZ;

      if (marked(LX) && marked(LZ))
        mFlags |= LX_HY_LZ;
      else if (marked(HX) && marked(LZ))
        mFlags |= HX_HY_LZ;
      else if (marked(LX) && marked(HZ))
        mFlags |= LX_HY_HZ;
      else if (marked(HX) && marked(HZ))
        mFlags |= HX_HY_HZ;

      break;

    case 4: // LZ

      if (marked(LX))
        mFlags |= LX_LZ;
      else if (marked(HX))
        mFlags |= HX_LZ;

      if (marked(LY))
        mFlags |= LY_LZ;
      else if (marked(HY))
        mFlags |= HY_LZ;

      if (marked(LX) && marked(LY))
        mFlags |= LX_LY_LZ;
      else if (marked(HX) && marked(LY))
        mFlags |= HX_LY_LZ;
      else if (marked(LX) && marked(HY))
        mFlags |= LX_HY_LZ;
      else if (marked(HX) && marked(HY))
        mFlags |= HX_HY_LZ;

      break;

    case 5: // HZ

      if (marked(LX))
        mFlags |= LX_HZ;
      else if (marked(HX))
        mFlags |= HX_HZ;

      if (marked(LY))
        mFlags |= LY_HZ;
      else if (marked(HY))
        mFlags |= HY_HZ;

      if (marked(LX) && marked(LY))
        mFlags |= LX_LY_HZ;
      else if (marked(HX) && marked(LY))
        mFlags |= HX_LY_HZ;
      else if (marked(LX) && marked(HY))
        mFlags |= LX_HY_HZ;
      else if (marked(HX) && marked(HY))
        mFlags |= HX_HY_HZ;

      break;
  }
}



