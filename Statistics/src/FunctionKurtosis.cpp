/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov and jcbenne@sandia.gov
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

#include "FunctionKurtosis.h"

namespace Statistics {


//template <typename FunctionType>
KurtosisAggregator::KurtosisAggregator(const KurtosisAggregator& kurtosis) : SkewnessAggregator(kurtosis)
{
  mMoment4 = kurtosis.mMoment4;
}

/*
template <typename FunctionType>
void FunctionKurtosisBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const FunctionKurtosisBase* p = static_cast<const FunctionKurtosisBase*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/

//template <typename FunctionType>
void KurtosisAggregator::reset()
{
   SkewnessAggregator::reset();
   mMoment4 = initialValue();
}

//template <typename FunctionType>
void KurtosisAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  FunctionType n = static_cast<FunctionType>(this->mCount)+1.;
  FunctionType inv_n = 1./n;
  FunctionType delta = f-this->mAvg;
  FunctionType A = delta*inv_n;

  mMoment4 += A * ( A * A * delta * this->mCount * ( n * ( n - 3. ) + 3. ) + 6. *A * this->mMoment2 - 4. * this->mMoment3 );

  SkewnessAggregator::addVertex(f, i);

}


//template <typename FunctionType>
double KurtosisAggregator::value() const
{

  if(this->mCount <= 1) return 0.;

  double n = this->mCount;
  double nm1 = n-1.;
  double inv_n = 1./n;

  double var = this->mMoment2/nm1;

  if(fabs(var) < 1e-12) return 0.;

  double var_inv = 1./var;
  double nvar_inv = var_inv*inv_n;

  double kurtosis = nvar_inv*var_inv*mMoment4-3;
  if(n == 2 || n == 3) return kurtosis;
  double nm2 = nm1-1.;
  return ( ( n + 1. ) * kurtosis + 6. ) + nm1 / (nm2 * (nm1 - 2.) );
}


//template <typename FunctionType>
void KurtosisAggregator::addSegment(const Aggregator* seg)
{
  const KurtosisAggregator* p = static_cast<const KurtosisAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of idenical type.");

  FunctionType delta = p->mAvg-this->mAvg;
  FunctionType n1 = static_cast<FunctionType>(this->mCount);
  FunctionType n2 = static_cast<FunctionType>(p->mCount);
  FunctionType N = n1 + n2;

  if(N < 1) {
    mMoment4 = 0.;
    SkewnessAggregator::addSegment(seg);
    return;
  }

  if(this->mCount < 1) {
    mMoment4 = p->mMoment4;
    SkewnessAggregator::addSegment(seg);
    return;
  }

  if(p->mCount < 1) {
    SkewnessAggregator::addSegment(seg);
    return;
  }

  FunctionType prod_n = n1*n2;
  FunctionType delta_sur_N = delta /  N ;
  FunctionType delta2_sur_N2 = delta_sur_N * delta_sur_N;
  FunctionType n1squared = n1*n1;
  FunctionType n2squared = n2*n2;

  mMoment4 += p->mMoment4
      + prod_n * (n1squared - prod_n + n2squared) * delta * delta_sur_N * delta2_sur_N2
      + 6. * (n1squared * p->mMoment2 + n2squared * this->mMoment2 ) * delta2_sur_N2
      + 4. * (n1* p->mMoment3 - n2* this->mMoment3) * delta_sur_N;


  SkewnessAggregator::addSegment(seg);

}

/*
template <typename FunctionType>
bool FunctionKurtosisBase<FunctionType>::compareToStreamingComputation(const std::vector<FunctionType> &values, FunctionType threshold)
{
  FunctionType nonIncAvg = 0.0;
  FunctionType nonIncMoment4 = 0.0;

  for(unsigned int i=0; i < values.size(); i++) {
    nonIncAvg += values[i];
  }

  nonIncAvg /= static_cast<FunctionType>(values.size());

  for(unsigned int i=0; i < values.size(); i++) {
    FunctionType diffFromMean = values[i]-nonIncAvg;
    nonIncMoment4 += (diffFromMean*diffFromMean*diffFromMean*diffFromMean);
  }

  FunctionType difference = fabs(nonIncMoment4-mMoment4);

  stmessage(difference >= threshold, "The streaming and non-streaming values do not match %1.32f, %1.32f, difference = %1.32f, threshold = %1.8f, numVals = %d, mCount = %d.", nonIncMoment4, mMoment4, difference, threshold, values.size(), this->mCount);

  return (difference < threshold);

}
*/

//template <typename FunctionType>
void KurtosisAggregator::writeASCII(std::ofstream & output) const
{
  SkewnessAggregator::writeASCII(output);
  output << " " << mMoment4;
}

//template <typename FunctionType>
void KurtosisAggregator::readASCII(std::ifstream& input)
{
  SkewnessAggregator::readASCII(input);
  input >> mMoment4;
}

/*
template <typename FunctionType>
void FunctionKurtosisBase<FunctionType>::writeBinary(std::ofstream & output) const
{
  FunctionSkewnessBase<FunctionType>::writeBinary(output);
  output.write((const char*)&mMoment4,sizeof(FunctionType));
}

template <typename FunctionType>
void FunctionKurtosisBase<FunctionType>::readBinary(std::ifstream& input)
{
  FunctionSkewnessBase<FunctionType>::readBinary(input);
  input.read((char*)&mMoment4,sizeof(FunctionType));
}

*/

}

