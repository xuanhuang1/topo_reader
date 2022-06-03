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

#include "Covariance.h"

namespace Statistics {

  //template class CovarianceBase<StatisticsDataType>;
  
//template <typename FunctionType>
CovarianceAggregator::CovarianceAggregator() : VertexCountAggregator(), mSumF(0), mSumG(0), mSumProduct(0)
{
}

//template <typename FunctionType>
CovarianceAggregator::CovarianceAggregator(const CovarianceAggregator& cov) : VertexCountAggregator(cov),
mSumF(cov.mSumF), mSumG(cov.mSumG), mSumProduct(cov.mSumProduct)
{
}

/*
template <typename FunctionType>
void CovarianceBase<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const CovarianceBase* p = dynamic_cast<const CovarianceBase*>(seg);

  sterror(p==NULL,"Can only set using segments of identical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void CovarianceAggregator::reset()
{
  VertexCountAggregator::reset();

  mSumF = 0.0;
  mSumG = 0.0;
  mSumProduct = 0.0;
}

//template <typename FunctionType>
void CovarianceAggregator::addVertex(FunctionType f, FunctionType g, GlobalIndexType i)
{
  VertexCountAggregator::addVertex(f, i);

  mSumF += f;
  mSumG += g;
  mSumProduct += f*g;
}

//template <typename FunctionType>
void CovarianceAggregator::addSegment(const Aggregator* seg)
{
  const CovarianceAggregator* p = static_cast<const CovarianceAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of identical type.");

  VertexCountAggregator::addSegment(seg);

  mSumF += p->mSumF;
  mSumG += p->mSumG;
  mSumProduct += p->mSumProduct;
}

//template <typename FunctionType>
double CovarianceAggregator::value() const
{
  if (this->mCount == 0)
    return 0;

  return (mSumProduct - mSumF*mSumG / (this->mCount)) / (this->mCount);
}

//template <typename FunctionType>
void CovarianceAggregator::writeASCII(std::ofstream & output) const
{
  VertexCountAggregator::writeASCII(output);
  output << " " << mSumF << " " << mSumG << " " << mSumProduct;// << " " << value();
}

//template <typename FunctionType>
void CovarianceAggregator::readASCII(std::ifstream& input)
{
  VertexCountAggregator::readASCII(input);
  input >> mSumF;
  input >> mSumG;
  input >> mSumProduct;
}

/*
template <typename FunctionType>
void CovarianceBase<FunctionType>::writeBinary(std::ofstream & output) const
{
  VertexCount<FunctionType>::writeBinary(output);
  output.write((const char*)&mSumF,sizeof(FunctionType));
  output.write((const char*)&mSumG,sizeof(FunctionType));
  output.write((const char*)&mSumProduct,sizeof(FunctionType));
}

template <typename FunctionType>
void CovarianceBase<FunctionType>::readBinary(std::ifstream& input)
{
  VertexCount<FunctionType>::readBinary(input);
  input.read((char *)&mSumF,sizeof(FunctionType));
  input.read((char *)&mSumG,sizeof(FunctionType));
  input.read((char *)&mSumProduct,sizeof(FunctionType));
}
*/


}


