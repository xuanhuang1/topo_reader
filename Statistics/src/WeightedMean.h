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

#ifndef WEIGHTEDMEAN_H
#define WEIGHTEDMEAN_H

#include "VertexCount.h"
#include <iostream>
#include <cmath>

namespace Statistics {

//template<typename FunctionType>
class WeightedMeanAggregator : public Aggregator
{
public:

  //! Default constructor
  WeightedMeanAggregator() : Aggregator(), mAvg(initialValue()), mWeight(initialValue()) {}
  
  //! Copy constructor
  WeightedMeanAggregator(const WeightedMeanAggregator& avg);

   ~WeightedMeanAggregator() {}

  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/

  //! Return the type of this aggregator
   AggregatorType type() const {return ATT_WMEAN;}

  //! Return the type name 
   const char* typeName() const {return "weighted mean";}
 
  //! Return the type of the this aggregators immediate base class
   const char* provides() const {return "";}

  //! Reinitialize the aggregator
   void reset();

  //! Return the initial value
   double initialValue() const { return 0.0; }; 

  //! Add another vertex to the segment
   void addVertex(FunctionType f, GlobalIndexType i) {sterror(true,"A weighted mean aggregator needs two attributes.");}
  // void addVertex(FunctionType f, StatisticsGlobalIndexType i) {fprintf(stderr,"addVErtex %e %u\n",f,i);}

  //! Add another vertex to the segment with the weight w
   void addVertex(FunctionType f, FunctionType w, GlobalIndexType i);
  
  //! Add another segment
   void addSegment(const Aggregator* seg);

  //! Return the value as double
   double value() const {return mAvg;}

  //! Return the function values
  FunctionType weightedMean() const {return mAvg;}

  /*************************************************************************************
   **************************** File Interface *****************************************
   ************************************************************************************/
  
  //! Write the ascii version of the element to the file
  void writeASCII(std::ofstream& output) const;

  //! Read the ascii version of the element from the file
   void readASCII(std::ifstream& input);

  /*
  //! Write the binary version of the element to the file
   void writeBinary(std::ofstream& output) const;

  //! Read the binary version of the element from the file
   void readBinary(std::ifstream& input);

  //! Return the size in number of bytes
   uint32_t size() const {return sizeof(WeightedMeanBase<FunctionType>);}
*/

public:

  //! The current mean
  FunctionType mAvg;

  //! The current accumulated weight
  FunctionType mWeight;

protected:

  //! Add the given count and WeightedMean to the current WeightedMean
  void addRawValues(uint64_t count, FunctionType WeightedMean);
  
  //! Write the ascii version of the element to the file
  // void writeASCIIInternal(std::ofstream& output) const;

};

class WeightedMeanArray : public AttributeArray<WeightedMeanAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  WeightedMeanArray(GlobalIndexType size=1) : AttributeArray<WeightedMeanAggregator>(2,size) {}

  //! Destructor
   ~WeightedMeanArray() {}

  //! Create a clone of yourself
   Attribute* clone() {return new WeightedMeanArray(*this);}
};

class WeightedMeanMap : public AttributeMap<WeightedMeanAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  WeightedMeanMap(GlobalIndexType size=1) : AttributeMap<WeightedMeanAggregator>(1,size) {}

  //! Destructor
   ~WeightedMeanMap() {}

  //! Create a clone of yourself
   Attribute* clone() {return new WeightedMeanMap(*this);}
};


} // Closing the namespace

  
#endif
