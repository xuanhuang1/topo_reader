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

#ifndef VALUE_H
#define VALUE_H

#include "Aggregator.h"
#include "Attribute.h"

namespace Statistics {

//! An Attribute to hold a simple value
class ValueAggregator : public Aggregator
{
public:

  //! Make sure you overload the addVertex call
  using Aggregator::addVertex;

  //! Default constructor
  ValueAggregator() : Aggregator(), mData(initialValue()) {}

  //Copy constructor
  ValueAggregator(const ValueAggregator& v);

  //! Destructor
  ~ValueAggregator() {}

  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/

  //! Return the type of this aggregator
   AggregatorType type() const {return ATT_VALUE;}

  //! Return the type name 
   const char* typeName() const {return "Value";}
 
  //! Return the type of the this aggregators immediate base class
   const char* provides() const {return "";}

  //! Reinitialize the aggregator
  void reset() { mData = initialValue(); }

  //! Return the initial value
  double initialValue() const { return 0.0; }

  //! Add another vertex to the segment
  void addVertex(FunctionType f, GlobalIndexType i) {
    // set was removed from aggregator, so commenting out this warning
    //if(mData == initialValue()) {
      //stwarning("A value type should be set not computed.");
    //}
    mData = f;
  }
  
  //! Add another segment
  void addSegment(const Aggregator* seg) {stwarning("A value type should be set not computed.");}

  //! Return the value as double
  double value() const {return mData;}

  //! Return the value
  //FunctionType value() const {return mData;}


  //! A test function that takes in a vector of values 
  //  computes a non-streaming version of the attribute 
  //  and compares it to the current aggregated attribute value.
  //bool compareToStreamingComputation(const std::vector<FunctionType> &values, FunctionType threshold) {return false;}

  
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
  uint32_t size() const {return sizeof(Value<FunctionType>);}
   */

public:
  
  FunctionType mData;
};

class ValueArray : public AttributeArray<ValueAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  ValueArray(GlobalIndexType size=1) : AttributeArray<ValueAggregator>(1,size) {}

  //! Destructor
   ~ValueArray() {}

  //! Create a clone of yourself
   Attribute* clone() {return new ValueArray(*this);}
};

class ValueMap : public AttributeMap<ValueAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  ValueMap(GlobalIndexType size=1) : AttributeMap<ValueAggregator>(1,size) {}

  //! Destructor
   ~ValueMap() {}

  //! Create a clone of yourself
   Attribute* clone() {return new ValueMap(*this);}
};


} // Closing the namespace

#endif
