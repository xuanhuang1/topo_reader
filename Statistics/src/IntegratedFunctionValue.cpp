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

#include "IntegratedFunctionValue.h"

namespace Statistics {

//template <typename FunctionType>
SumAggregator::SumAggregator(const SumAggregator& avg) : Aggregator(avg)
{
  mIntegral = avg.mIntegral;
}

/*
template <typename FunctionType>
void SumBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const SumBase* p = dynamic_cast<const SumBase*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void SumAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  mIntegral += f;
}

//template <typename FunctionType>
void SumAggregator::addSegment(const Aggregator* seg)
{
  const SumAggregator* p = static_cast<const SumAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of idenical type.");

  mIntegral += p->mIntegral;
}

/*
template <typename FunctionType>
bool SumBase<FunctionType>::compareToStreamingComputation(const std::vector<FunctionType>& values, FunctionType threshold)
{
  FunctionType nonIncIntegral = 0;

  for(unsigned int i=0; i < values.size(); i++) {
    nonIncIntegral += values[i];
  }

  FunctionType difference = fabs(nonIncIntegral-mIntegral);

  stmessage(difference >= threshold, "The streaming and non-streaming values do not match %1.8e, %1.8e, difference = %1.8e, threshold = %1.8e.", nonIncIntegral, mIntegral, difference, threshold);

  return (difference < threshold);
}
*/
//template <typename FunctionType>
void SumAggregator::writeASCII(std::ofstream& output) const
{
  output << mIntegral;
}

//template <typename FunctionType>
void SumAggregator::readASCII(std::ifstream& input)
{
  input >> mIntegral;
}
/*
template <typename FunctionType>
void SumBase<FunctionType>::writeBinary(std::ofstream& output) const
{
  output.write((const char*)&mIntegral,sizeof(FunctionType));
}

template <typename FunctionType>
void SumBase<FunctionType>::readBinary(std::ifstream& input)
{
  input.read((char*)&mIntegral,sizeof(FunctionType));
}
*/

}

