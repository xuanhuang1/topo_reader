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

#ifndef PERIODICTRANSFORMATION_H
#define PERIODICTRANSFORMATION_H

#include <cmath>
#include <vector>
#include "StatisticsDefinitions.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
# define M_PI_2 1.57079632679489661923
#endif

namespace Statistics {


//! A base class to enable value transformations for periodic values
/*! A PeriodicTransformation provides the ability to "guess" the correct periodic
 *  transformation for a given value based on an existing value. Each
 *  new value has three possible incarnations: Within the core range;
 *  Below the core range; and Above the core range. A PeriodicTransformation
 *  will pick the one closest to a reference value as the best guess.
 *  Note that, this does not guarantee correctness.
 */
class PeriodicTransformation
{
public:

  //! Default constructor setting the period
  PeriodicTransformation(FunctionType period = 0) : mPeriod(snapToPreset(period)) {}

  //! Copy constructor
  PeriodicTransformation(const PeriodicTransformation& transform) : mPeriod(transform.mPeriod) {}
  //! Destructor
  ~PeriodicTransformation() {}

  //! Syntactic sugar to call the transform function
  FunctionType operator()(FunctionType reference, FunctionType f) const {return transform(reference,f);}

  //! Syntactic sugar to map a single value into its core range
  void operator()(FunctionType& f) const;

  //! Return the current period
  FunctionType period() const {return mPeriod;}

  //! Transform the incoming value f to the best guess value
  /*!Given the reference value, transform f into its periodic
   * incarnation that is closest to the reference value.
   * @param reference The reference value
   * @param f The incoming value
   * @return The tansformed value
   */
  FunctionType transform(FunctionType reference, FunctionType f) const;

private:

  //! The tolerance for periods close to M_PI etc. values
  static const double sTolerance;

  //! The number of preset values
  static const int sNrOfPresets = 3;

  //! An array of preset values for periods and ankers to snap to
  static const double sPresets[sNrOfPresets];

  //! Function to snap given values to one of the presets
  static FunctionType snapToPreset(FunctionType f);

  //! The period value such that f = f+mPeriod
  FunctionType mPeriod;

};

}

#endif /* PeriodicTransformation_H */
