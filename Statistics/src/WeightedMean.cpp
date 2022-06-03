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

#include <cmath>
#include "WeightedMean.h"


namespace Statistics {

//template <typename FunctionType>
WeightedMeanAggregator::WeightedMeanAggregator(const WeightedMeanAggregator& avg) : Aggregator(avg)
{
  mAvg = avg.mAvg;
  mWeight = avg.mWeight;
}

/*
template <typename FunctionType>
void WeightedMeanBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const WeightedMeanBase* p = static_cast<const WeightedMeanBase*>(seg);

  sterror(p==NULL,"Can only set using segments of identical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void WeightedMeanAggregator::reset()
{
  mAvg = initialValue();
  mWeight = initialValue();
}

//template <typename FunctionType>
void WeightedMeanAggregator::addVertex(FunctionType f, FunctionType w, GlobalIndexType i)
{
  // Stable update formulae
  // new_avg = (weight*avg + w*f) / (weight + w)
  // new_avg = (weight+w)*avg / (weight+w)  - w*avg / (weight+w) + w*f/(weight + w)

  mWeight += w;

  if (mWeight > 1e-15)
    mAvg += (w*(f - mAvg)) / mWeight;

  /*
  if (std::isnan(mAvg) || std::isinf(mAvg)) {
    fprintf(stderr,"WeightedMean not a number after added %e %e \n\t\t%e    %e",f,w,mAvg,mWeight);
    assert(false);
  }
  */
}

//template <typename FunctionType>
void WeightedMeanAggregator::addSegment(const Aggregator* seg)
{
  const WeightedMeanAggregator* p = static_cast<const WeightedMeanAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of identical type.");

  mWeight += p->mWeight;

  if (mWeight > 1e-15)
    mAvg += p->mWeight*((p->mAvg - mAvg) / mWeight);
}

//template <typename FunctionType>
void WeightedMeanAggregator::writeASCII(std::ofstream& output) const
{
  //if (std::isnan(mAvg) || std::isinf(mAvg))
  //  fprintf(stderr,"WeightedMean not a number after at write  %e %e\n",mAvg, mWeight);

  output << " " << mAvg << " " << mWeight;
}

//template <typename FunctionType>
void WeightedMeanAggregator::readASCII(std::ifstream& input)
{
  input >> mAvg;
  input >> mWeight;
}
/*
template <typename FunctionType>
void WeightedMeanBase<FunctionType>::writeBinary(std::ofstream& output) const
{
  output.write((const char*)&mAvg,sizeof(FunctionType));
  output.write((const char*)&mWeight,sizeof(FunctionType));
}

template <typename FunctionType>
void WeightedMeanBase<FunctionType>::readBinary(std::ifstream& input)
{
  input.read((char *)&mAvg,sizeof(FunctionType));
  input.read((char *)&mWeight,sizeof(FunctionType));
}
*/

}

