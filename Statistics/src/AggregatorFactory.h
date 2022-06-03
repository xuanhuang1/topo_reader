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

#ifndef AGGREGATORFACTORY_H
#define AGGREGATORFACTORY_H

#include "string.h"
#include "PeriodicTransformation.h"
#include "Value.h"
#include "VertexCount.h"
#include "Mean.h"
#include "MaximalFunctionValue.h"
#include "MinimalFunctionValue.h"
#include "IntegratedFunctionValue.h"
#include "FunctionVariance.h"
#include "FunctionSkewness.h"
#include "FunctionKurtosis.h"
#include "Covariance.h"
#include "PeriodicMean.h"
#include "WeightedMean.h"
#include "Accessor.h"

namespace Statistics {


class Factory
{
public:

  //! Constructor
  Factory() {}

  ~Factory() {}


  Attribute* make_aggregator(const std::string& name) const { return make_aggregator_array(name, false); }
  Attribute* make_aggregator_array(const char* name, bool preAggregated) const;
  Attribute* make_aggregator_map(const char* name, bool preAggregated) const;
  Attribute* make_aggregator_array(const std::string& name, bool preAggregated) const {return make_aggregator_array(name.c_str(), preAggregated);}
  Attribute* make_aggregator_map(const std::string& name, bool preAggregated) const {return make_aggregator_map(name.c_str(), preAggregated);}

  Attribute* make_accessor_array(const Attribute* attribute, const char* base_name, const char* derived_name);
  Attribute* make_accessor_map(const Attribute* attribute, const char* base_name, const char* derived_name);

private:

  ValueArray mValueArray;
  VertexCountArray mVertexCountArray;
  MeanArray mMeanArray;
  MaximumArray mMaximalFunctionValueArray;
  MinimumArray mMinimalFunctionValueArray;
  SumArray mSumArray;
  VarianceArray mFunctionVarianceArray;
  SkewnessArray mFunctionSkewnessArray;
  KurtosisArray mFunctionKurtosisArray;
  CovarianceArray mCovarianceArray;
  WeightedMeanArray mWeightedMeanArray;
  PeriodicMeanArray mPeriodicMeanArray;

  //! Get the type from the name
  AggregatorType type(const char* name) const;
};
  


}


#endif
