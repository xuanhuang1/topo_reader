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

#ifndef BASEATTRIBUTE_H
#define BASEATTRIBUTE_H

#include <cstdio>
#include <string>
#include <vector>
#include <math.h>
#include "StatisticsDefinitions.h"

namespace Statistics {

enum AggregatorType {
  ATT_BASE  = 0,
  ATT_VERTEXCOUNT  = 1,
  ATT_MEAN = 2,
  ATT_MAX = 3,
  ATT_MIN = 4,
  ATT_SUM = 5,
  ATT_VAR = 6,
  ATT_SKEW = 7,
  ATT_KURT = 8,
  ATT_VALUE = 9,
  ATT_COV = 10,
  ATT_PERMEAN = 11,
  ATT_WMEAN = 12,
};

//! Interface class for all aggregators
//template<typename FunctionType>
class Aggregator
{
public:

  //! Standard constructor
  Aggregator() {}

   //! Destructor
   ~Aggregator() {}

  /*
  //! Create a clone of yourself
   BaseAttribute* clone() = 0;

  //! Allocate and array of the given size 
   BaseAttribute* allocate(LocalIndexType count=1) = 0;

  //! Allocate and array of the given size 
   void free(BaseAttribute* p) = 0;

  //! Offset operator to provide access to arrays represented only as BaseAttribute*
   BaseAttribute& operator[](unsigned int i) = 0; 
   */

  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/

  //! Reset the aggregator
   void reset() {sterror(true,"Necessary function not overloaded.");}
  
  //! Return the initial value as a double
   double initialValue() const {sterror(true,"Necessary function not overloaded.");return 0;}



  //! Assuming the same type copy the data of seg
  //void set(const BaseAttribute* seg) {sterror(true,"Necessary function not overloaded.");}
  
  //! Add another vertex to the segment
   void addVertex(FunctionType f, GlobalIndexType i) {sterror(true,"Necessary function not overloaded.");}

  //! Add another vertex to the segment
   void addVertex(FunctionType f, FunctionType g,
                 GlobalIndexType i) {sterror(true,"Necessary function not overloaded.");}

  //! Add another segment
   void addSegment(const Aggregator* seg) {sterror(true,"Necessary function not overloaded.");}

  //! Return the value as double
   double value() const {sterror(true,"Necessary function not overloaded.");return 0;}

  //! Return the type of this aggregator 
   AggregatorType type() const { sterror(true,"Necessary function not overloaded."); return ATT_BASE;}

  //! Return the type name
   const char* typeName() const { sterror(true,"Necessary function not overloaded.");return 0;}
  
  //! Return the type of the this aggregators immediate base class
   const char* provides() const { return "";}

  //! A test function that takes in a vector of values 
  //  computes a non-streaming version of the attribute 
  //  and compares it to the current aggregated attribute value.
  //bool compareToStreamingComputation(const std::vector<FunctionType> &values, FunctionType threshold) {sterror(true,"Not implemented.");return false;}
};
}

#endif
