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

#include "FunctionSkewness.h"

namespace Statistics {


//template <typename FunctionType>
SkewnessAggregator::SkewnessAggregator(const SkewnessAggregator& skewness) : VarianceAggregator(skewness)
{
  mMoment3 = skewness.mMoment3;
}

/*
template <typename FunctionType>
void FunctionSkewnessBase::set(const BaseAttribute* seg)
{
  const FunctionSkewnessBase* p = dynamic_cast<const FunctionSkewnessBase*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void SkewnessAggregator::reset()
{
   VarianceAggregator::reset();
   mMoment3 = initialValue();
}

//template <typename FunctionType>
void SkewnessAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  FunctionType n = static_cast<FunctionType>(this->mCount)+1;
  FunctionType inv_n = 1./n;
  FunctionType delta = f-this->mAvg;
  FunctionType A = delta*inv_n;
  FunctionType tempMean = this->mAvg + delta*inv_n;
  FunctionType B = f-tempMean;

  mMoment3 += A * ( B * delta * ( n - 2. ) - 3. * this->mMoment2 );

  VarianceAggregator::addVertex(f, i);

}

//template <typename FunctionType>
double SkewnessAggregator::value() const
{

  if(this->mCount <= 1) return 0.;

  double n = this->mCount;
  double nm1 = n-1.;
  double inv_n = 1./n;

  double var = this->mMoment2/nm1;

  if(fabs(var) < 1e-12) return 0.;

  double var_inv = 1./var;
  double nvar_inv = var_inv*inv_n;
  double skew = nvar_inv*sqrt(var_inv)*mMoment3;
  if(n == 2) return skew;
  double nm2 = nm1-1.;
  return ( n * n ) / (nm1-nm2) * skew;


}

//template <typename FunctionType>
void SkewnessAggregator::addSegment(const Aggregator* seg)
{
  const SkewnessAggregator* p = static_cast<const SkewnessAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of idenical type.");

  FunctionType n1 = static_cast<FunctionType>(this->mCount);
  FunctionType n2 = static_cast<FunctionType>(p->mCount);
  FunctionType N = n1 + n2;
  if(N < 1) {
    mMoment3 = 0.;
    VarianceAggregator::addSegment(seg);
    return;
  }
  if(this->mCount < 1) {
    mMoment3 = p->mMoment3;
    VarianceAggregator::addSegment(seg);
    return;
  }
  if(p->mCount < 1) {
    VarianceAggregator::addSegment(seg);
    return;
  }

  FunctionType prod_n = n1*n2;
  FunctionType delta = p->mAvg-this->mAvg;
  FunctionType delta_sur_N = delta /  N ;

  mMoment3 += p->mMoment3
      + prod_n * (n1-n2) * delta * delta_sur_N * delta_sur_N
      + 3. * (n1*p->mMoment2 - n2*this->mMoment2 ) * delta_sur_N;

  VarianceAggregator::addSegment(seg);
}

/*
template <typename FunctionType>
bool FunctionSkewnessBase::compareToStreamingComputation(const std::vector &values, FunctionType threshold)
{
  FunctionType nonIncAvg = 0.0;
  FunctionType nonIncMoment3 = 0.0;

  int n = 0;
  FunctionType inv_n;
  for(unsigned int i=0; i < values.size(); i++) {
    n = i + 1;
    inv_n = 1./static_cast(n);
    nonIncAvg += (values[i] - nonIncAvg) * inv_n;
  }


  for(unsigned int i=0; i < values.size(); i++) {
   FunctionType diffFromMean = values[i]-nonIncAvg;
   nonIncMoment3 += (diffFromMean*diffFromMean*diffFromMean);
  }

  FunctionType difference = fabs(nonIncMoment3-mMoment3);

  stmessage(difference >= threshold, "The streaming and non-streaming values do not match %1.8f, %1.8f, difference = %1.8f, threshold = %1.8f, numVals= %d, mCount = %d, nIncAvg = %1.8f, incAvg = %1.8f.", nonIncMoment3, mMoment3, difference, threshold, values.size(), this->mCount, nonIncAvg, this->mAvg);

  return (difference < threshold);
}
*/

//template <typename FunctionType>
void SkewnessAggregator::writeASCII(std::ofstream & output) const
{
  VarianceAggregator::writeASCII(output);
  output << " " << mMoment3;
}

//template <typename FunctionType>
void SkewnessAggregator::readASCII(std::ifstream& input)
{
  VarianceAggregator::readASCII(input);
  input >> mMoment3;
}
/*
template <typename FunctionType>
void FunctionSkewnessBase::writeBinary(std::ofstream & output) const
{
  FunctionVarianceBase::writeBinary(output);
  output.write((const char*)&mMoment3,sizeof(FunctionType));
}

template <typename FunctionType>
void FunctionSkewnessBase::readBinary(std::ifstream& input)
{
  FunctionVarianceBase::readBinary(input);
  input.read((char*)&mMoment3,sizeof(FunctionType));
}
*/

}

