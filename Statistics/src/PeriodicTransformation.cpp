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

#include "PeriodicTransformation.h"

namespace Statistics {

const double PeriodicTransformation::sTolerance = 1e-03;

const double PeriodicTransformation::sPresets[3] = {M_PI_2, M_PI, 2*M_PI};

FunctionType PeriodicTransformation::snapToPreset(FunctionType f)
{
  for (int i=0;i<sNrOfPresets;i++) {
    if (fabs(sPresets[i] - f) < sTolerance)
      return sPresets[i];
  }

  return f;
}

void PeriodicTransformation::operator()(FunctionType& f) const
{
  int8_t count;

  count = static_cast<int8_t>(f / mPeriod);
  if ((count < 0) || (f  < 0))
    count--;

  f -= count*mPeriod;
}

FunctionType PeriodicTransformation::transform(FunctionType reference, FunctionType f) const
{
  sterror(fabs(reference - f) > 1.5*mPeriod,"Problem with a periodic value reference %e and value %e are more than a period %e apart.",
          reference,f,mPeriod);

  if (fabs(reference - f) < mPeriod/2)
    return f;
  else if (reference < f)
    return f - mPeriod;
  else
    return f + mPeriod;
}

}
