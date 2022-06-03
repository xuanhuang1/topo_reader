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

#include "FunctionVariance.h"

namespace Statistics {



//template <typename FunctionType>
VarianceAggregator::VarianceAggregator(const VarianceAggregator& variance) : MeanAggregator(variance)
{
  mMoment2 = variance.mMoment2;
}

/*
template <typename FunctionType>
void FunctionVariance::set(const BaseAttribute* seg)
{
  const FunctionVariance* p = dynamic_cast<const FunctionVariance*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void VarianceAggregator::reset()
{
   MeanAggregator::reset();
   mMoment2 = initialValue();
}

//template <typename FunctionType>
void VarianceAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  FunctionType delta= f-this->mAvg;

  MeanAggregator::addVertex(f, i);

  mMoment2 +=  delta*(f-this->mAvg);

}

//template <typename FunctionType>
double VarianceAggregator::value() const
{
  if (this->mCount <= 1) return 0.;
  double inv_nm1 = 1./static_cast<double>(this->mCount);
  return (mMoment2*inv_nm1);

}


//template <typename FunctionType>
void VarianceAggregator::addSegment(const Aggregator* seg)
{
  const VarianceAggregator* p = static_cast<const VarianceAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of idenical type.");

  FunctionType delta = p->mAvg-this->mAvg;
  int N = this->mCount + p->mCount;
  if(N < 1) {
    mMoment2 = 0.;
    MeanAggregator::addSegment(seg);
    return;
  }

  if(this->mCount < 1) {
    mMoment2 = p->mMoment2;
    MeanAggregator::addSegment(seg);
    return;
  }
  if(p->mCount < 1) {
    MeanAggregator::addSegment(seg);
    return;
  }

  int prod_n = this->mCount*p->mCount;
  FunctionType delta_sur_N = delta / static_cast<FunctionType>( N );

  mMoment2 += p->mMoment2 + prod_n*delta*delta_sur_N;

  MeanAggregator::addSegment(seg);

}

/*
template <typename FunctionType>
bool FunctionVarianceBase::compareToStreamingComputation(const std::vector &values, FunctionType threshold)
{
  FunctionType nonIncAvg = 0.0;
  FunctionType nonIncMoment2 = 0.0;

  for(unsigned int i=0; i < values.size(); i++) {
    nonIncAvg += values[i];
  }
  nonIncAvg /= static_cast(values.size());


  for(unsigned int i=0; i < values.size(); i++) {
    FunctionType diffFromMeanBase = values[i]-nonIncAvg;
    nonIncMoment2 += (diffFromMeanBase*diffFromMeanBase);
  }

  FunctionType difference = fabs(nonIncMoment2 - mMoment2);

  stmessage(difference >= threshold, "The streaming and non-streaming values do not match %1.8f, %1.8f, difference = %1.8f, threshold = %1.8f.", nonIncMoment2, mMoment2, difference, threshold);

  return (difference < threshold);

}
*/
//template <typename FunctionType>
void VarianceAggregator::writeASCII(std::ofstream& output) const
{
  MeanAggregator::writeASCII(output);
  output << " " << mMoment2;// << " " << value();
}

//template <typename FunctionType>
void VarianceAggregator::readASCII(std::ifstream& input)
{
  MeanAggregator::readASCII(input);
  input >> mMoment2;
}

/*
template <typename FunctionType>
void FunctionVariance::writeBinary(std::ofstream& output) const
{
  MeanBase::writeBinary(output);
  output.write((const char*)&mMoment2,sizeof(FunctionType));
}

template <typename FunctionType>
void FunctionVariance::readBinary(std::ifstream& input)
{
  MeanBase::readBinary(input);
  input.read((char *)&mMoment2,sizeof(FunctionType));
}
*/
}

