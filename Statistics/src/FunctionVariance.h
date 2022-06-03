/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov, jcbenne@sandia.gov
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

#ifndef FUNCTIONVARIANCE_H
#define FUNCTIONVARIANCE_H

#include "Mean.h"


/***************************************************************************
* This code makes use of both incremental and pairwise update formulas
* from "Numerically Stable, Single-Pass, Parallel Statisitcs Algorithms"
* available online at 
* http://www.janinebennett.org/index_files/ParallelStatisticsAlgorithms.pdf
* The paper has derivations for arbitrary-order centered moments as well as 
* for covariance.
****************************************************************************/

namespace Statistics {

//template<typename FunctionType>
class VarianceAggregator : public MeanAggregator
{
public:

  //! Make sure you overload the addVertex call
  using MeanAggregator::addVertex;

  //! Default constructor
  VarianceAggregator() : MeanAggregator(), mMoment2(initialValue()) {}
  
  //! Copy constructor
  VarianceAggregator(const VarianceAggregator& variance);

   ~VarianceAggregator() {}

  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/

  //! Return the type of this aggregator
   AggregatorType type() const {return ATT_VAR;}

  //! Return the type name 
   const char* typeName() const {return "variance";}

  //! Return the type of the this aggregators immediate base class
   const char* provides() const {return MeanAggregator::typeName();}



  //! Reinitialize the aggregator
   void reset();

  //! Return the initial value
   double initialValue() const { return 0.0; };

  //! Add another vertex to the segment
   void addVertex(FunctionType f, GlobalIndexType i);
  
  //! Add another segment
   void addSegment(const Aggregator* seg);

  //! Return the value as double
   double value() const;

  //! Return the value
  FunctionType variance() const {return value();}

  //! A test function that takes in a vector of values 
  //  computes a non-streaming version of the attribute 
  //  and compares it to the current aggregated attribute value.
  // bool compareToStreamingComputation(const std::vector &values, FunctionType threshold);

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
   uint32_t size() const {return sizeof(FunctionVariance);}
   */
public:

  FunctionType mMoment2;

protected:
  
  //! Write the ascii version of the element to the file
  // void writeASCIIInternal(std::ofstream& output) const;
};

class VarianceArray : public AttributeArray<VarianceAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  VarianceArray(GlobalIndexType size=1) : AttributeArray<VarianceAggregator>(1,size) {}

  //! Destructor
   ~VarianceArray() {}

  //! Create a clone of yourself
   Attribute* clone() {return new VarianceArray(*this);}
};

class VarianceMap : public AttributeMap<VarianceAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  VarianceMap(GlobalIndexType size=1) : AttributeMap<VarianceAggregator>(1,size) {}

  //! Destructor
   ~VarianceMap() {}

  //! Create a clone of yourself
   Attribute* clone() {return new VarianceMap(*this);}
};


} // Closing the namespace

  
#endif
