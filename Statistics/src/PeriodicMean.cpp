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

#include "PeriodicMean.h"

namespace Statistics {


//template <typename FunctionType>
PeriodicMeanAggregator::PeriodicMeanAggregator(const PeriodicTransformation* transform) :
MeanAggregator(), mTransformation(transform)
{
}

//template <typename FunctionType>
void PeriodicMeanAggregator::addVertex(FunctionType f, GlobalIndexType i)
{
  MeanAggregator::addVertex((*mTransformation)(this->mean(),f),i);

  // Map the resulting mean to the core range
  (*mTransformation)(this->mAvg);
}

//template <typename FunctionType>
void PeriodicMeanAggregator::addSegment(const Aggregator* seg)
{
  const PeriodicMeanAggregator* p = static_cast<const PeriodicMeanAggregator*>(seg);

  sterror(p==NULL,"Can only add segments of identical type.");

  addRawValues(p->vertexCount(),(*mTransformation)(this->mean(),p->mean()));

  // Map the resulting mean to the core range
  (*mTransformation)(this->mAvg);
}

PeriodicTransformation PeriodicMeanArray::sTransformation;

//! Resize the array to contain size many elements
void PeriodicMeanArray::resize(GlobalIndexType size)
{
  PeriodicMeanAggregator agg(&sTransformation);
  mAttributes.resize(size,agg);
}


}
