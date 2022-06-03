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

#include "VertexCount.h"

namespace Statistics {


//template <typename FunctionType>
VertexCountAggregator::VertexCountAggregator(const VertexCountAggregator& count) : Aggregator(count)
{
  mCount = count.mCount;
}

/*
template <typename FunctionType>
void VertexCountBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const VertexCountBase* p = dynamic_cast<const VertexCountBase*>(seg);

  sterror(p==NULL,"Can only set using segments of idenical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void VertexCountAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  mCount++;
}

//template <typename FunctionType>
void VertexCountAggregator::addSegment(const Aggregator* seg)
{
  const VertexCountAggregator* p = static_cast<const VertexCountAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of idenical type.");

  mCount += p->mCount;
}

/*
template <typename FunctionType>
bool VertexCountBase<FunctionType>::compareToStreamingComputation(const std::vector<FunctionType>& values, FunctionType threshold)
{

  uint64_t nonIncCount = values.size();

  stmessage(nonIncCount != mCount,"The streaming and non-streaming values do not match.");

  return (nonIncCount == mCount);
}
*/
//template <typename FunctionType>
void VertexCountAggregator::writeASCII(std::ofstream& output) const
{
  output << mCount;
}

//template <typename FunctionType>
void VertexCountAggregator::readASCII(std::ifstream& input)
{
  input >> mCount;
}

/*
template <typename FunctionType>
void VertexCountBase<FunctionType>::writeBinary(std::ofstream& output) const
{
  output.write((const char*)&mCount,sizeof(uint64_t));
}

template <typename FunctionType>
void VertexCountBase<FunctionType>::readBinary(std::ifstream& input)
{
  input.read((char *)&mCount,sizeof(uint64_t));
}
*/

}
