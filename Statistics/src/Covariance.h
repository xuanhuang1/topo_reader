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

#ifndef COVARIANCE_H
#define COVARIANCE_H

#include "VertexCount.h"

namespace Statistics {

//template<typename FunctionType>
class CovarianceAggregator : public VertexCountAggregator
{
public:

  //! Default constructor
  CovarianceAggregator();

  //! Copy constructor
  CovarianceAggregator(const CovarianceAggregator& cov);

  //! Destructor
   ~CovarianceAggregator() {}

  /*
  //! Create a clone of yourself
   BaseAttribute<FunctionType>* clone() {return new CovarianceBase<FunctionType>(*this);}

  //! Allocate and array of the given size
   BaseAttribute<FunctionType>* allocate(LocalIndexType count=1) {return new CovarianceBase<FunctionType>[count];}

  //! Allocate and array of the given size
   void free(BaseAttribute<FunctionType>* p) {delete[] dynamic_cast<CovarianceBase<FunctionType>*>(p);}

  //! Offset operator to provide access to arrays represented only as BaseAttribute*
   BaseAttribute<FunctionType>& operator[](unsigned int i) {return *(this + i);}
   */
  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/
  //! Return the type of this aggregator
   AggregatorType type() const {return ATT_COV;}

  //! Return the type name
   const char* typeName() const {return "covariance";}

   //! Return the type of the this aggregators immediate base class
   const char* provides() const {return VertexCountAggregator::typeName();}


  //! Return the number of attributes necessary for this aggregator
   uint8_t numAttributes() const {return 2;}

  /*
  //! Assuming the same type copy the data of seg
   void set(const BaseAttribute<FunctionType>* seg);
   */
  //! Reinitialize the aggregator
   void reset();

  //! Add another vertex to the segment
   void addVertex(FunctionType f, GlobalIndexType i) {sterror(true,"A covariance aggregator needs two attributes.");}

  //! Add another vertex to the segment
   void addVertex(FunctionType f, FunctionType g, GlobalIndexType i);

 //! Add another segment
   void addSegment(const Aggregator* seg);

  //! Return the value as double
   double value() const;

  /*************************************************************************************
   **************************** File Interface *****************************************
   ************************************************************************************/

  //! Write the ascii version of the element to the file
  void writeASCII(std::ofstream& output) const;

  //! Read the ascii version of the element from the file
   void readASCII(std::ifstream& input);

  /*
  //! Write the binary version of the element to the file
   void writeBinary(std::ofstream & output) const;

  //! Read the binary version of the element from the file
   void readBinary(std::ifstream& input);

  //! Return the size in number of bytes
   uint32_t size() const {return sizeof(CovarianceBase<FunctionType>);}
*/
public:

  //! The sum of the first function
  FunctionType mSumF;

  //! The sum of the second Function
  FunctionType mSumG;

  //! The sum of the product
  FunctionType mSumProduct;

protected:

  //! Write the ascii version of the element to the file
  // void writeASCIIInternal(std::ofstream & output) const;
};

class CovarianceArray : public AttributeArray<CovarianceAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  CovarianceArray(GlobalIndexType size=1) : AttributeArray<CovarianceAggregator>(2,size) {}

  //! Destructor
   ~CovarianceArray() {}

  //! Create a clone of yourself
   Attribute* clone() {return new CovarianceArray(*this);}

  //! Return the type of this attribute
  AggregatorType type() const {return ATT_COV;}

  //! Return a string identifying the attribute
  const char* typeName() const {return "covariance";}



};

class CovarianceMap : public AttributeMap<CovarianceAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  CovarianceMap(GlobalIndexType size=1) : AttributeMap<CovarianceAggregator>(2,size) {}

  //! Destructor
   ~CovarianceMap() {}

  //! Create a clone of yourself
   Attribute* clone() {return new CovarianceMap(*this);}

  //! Return the type of this attribute
  AggregatorType type() const {return ATT_COV;}

  //! Return a string identifying the attribute
  const char* typeName() const {return "covariance";}
};


}


#endif /* COVARIANCE_H_ */
