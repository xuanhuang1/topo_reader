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

#include "Mean.h"

namespace Statistics {


//template <typename FunctionType>
MeanAggregator::MeanAggregator(const MeanAggregator& avg) : VertexCountAggregator(avg)
{
  mAvg = avg.mAvg;
}

/*
template <typename FunctionType>
void MeanBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const MeanBase* p = dynamic_cast<const MeanBase*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/

//template <typename FunctionType>
void MeanAggregator::reset()
{
  VertexCountAggregator::reset();
  mAvg = initialValue();
}

//template <typename FunctionType>
void MeanAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  int n = this->mCount+1;
  FunctionType inv_n = 1./static_cast<FunctionType>(n);
  FunctionType delta = f-mAvg;
  FunctionType A = delta*inv_n;
  mAvg += A;

  VertexCountAggregator::addVertex(f, i);
}

//template <typename FunctionType>
void MeanAggregator::addSegment(const Aggregator* seg)
{
  const MeanAggregator* p = static_cast<const MeanAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of identical type.");

  // This new variant of the MeanBase computation is necessary for the periodic
  // computation
  addRawValues(p->mCount,p->mAvg);
  return;


  FunctionType delta = p->mAvg - this->mAvg;
  int N = this->mCount + p->mCount;

  if(N < 1) {
    mAvg = 0.;
    return;
  }

  if(this->mCount < 1) {
    mAvg = p->mAvg;
    VertexCountAggregator::addSegment(seg);
    return;
  }

  if(p->mCount < 1) {
    VertexCountAggregator::addSegment(seg);
    return;
  }

  FunctionType delta_sur_N = delta / static_cast<FunctionType>( N );

  mAvg += p->mCount*delta_sur_N;

  VertexCountAggregator::addSegment(seg);

}

/*
template <typename FunctionType>
bool MeanBase<FunctionType>::compareToStreamingComputation(const std::vector<FunctionType>& values, FunctionType threshold)
{
  FunctionType nonIncAvg = 0.0;

  int numVals = values.size();
  for(int i=0; i < numVals; i++) {
    nonIncAvg += values[i];
  }
  nonIncAvg /= static_cast<FunctionType>(numVals);

  FunctionType difference = fabs(nonIncAvg-mAvg);
  stmessage(difference >= threshold, "The streaming and non-streaming values do not match %1.8f, %1.8f, difference = %1.8f, threshold = %1.8f, nVals = %d, mCount = %d ", nonIncAvg, mAvg, difference, threshold, numVals, this->mCount);

  return (difference < threshold);
}
*/

//template <typename FunctionType>
void MeanAggregator::addRawValues(uint64_t count, FunctionType MeanBase)
{
  FunctionType delta = MeanBase - this->mAvg;
  int N = this->mCount + count;

  if(N < 1) {
    mAvg = 0.;
    return;
  }

  if(count < 1)
     return;

  if(this->mCount < 1) {
    mAvg = MeanBase;
    this->mCount += count;
    return;
  }


  FunctionType delta_sur_N = delta / static_cast<FunctionType>( N );

  mAvg += count*delta_sur_N;
  this->mCount += count;
}


//template <typename FunctionType>
void MeanAggregator::writeASCII(std::ofstream& output) const
{
  VertexCountAggregator::writeASCII(output);
  output << " " << mAvg;
}

//template <typename FunctionType>
void MeanAggregator::readASCII(std::ifstream& input)
{
  VertexCountAggregator::readASCII(input);
  input >> mAvg;
}
/*
template <typename FunctionType>
void MeanBase<FunctionType>::writeBinary(std::ofstream& output) const
{
  VertexCount<FunctionType>::writeBinary(output);
  output.write((const char*)&mAvg,sizeof(FunctionType));
}

template <typename FunctionType>
void MeanBase<FunctionType>::readBinary(std::ifstream& input)
{
  VertexCount<FunctionType>::readBinary(input);
  input.read((char *)&mAvg,sizeof(FunctionType));
}
*/

}

