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

#ifndef PERIODICMEAN_H_
#define PERIODICMEAN_H_

#include "Mean.h"
#include "PeriodicTransformation.h"

namespace Statistics {


//template <typename FunctionType = float>
class PeriodicMeanAggregator : public MeanAggregator
{
public:

  //! Make sure you overload the addVertex call
  using MeanAggregator::addVertex;

  //! Default constructor
  PeriodicMeanAggregator() : MeanAggregator(), mTransformation(NULL) {}

  //! Alternate constructor
  PeriodicMeanAggregator(const PeriodicTransformation* transform);

  //! Copy constructor
  PeriodicMeanAggregator(const PeriodicMeanAggregator& mean) : MeanAggregator(mean), mTransformation(mean.mTransformation) {}

   ~PeriodicMeanAggregator() {}

  /*************************************************************************************
   **************************** Computation Interface **********************************
   ************************************************************************************/

  //! Return the type of this aggregator
   AggregatorType type() const {return ATT_PERMEAN;}

  //! Return a string identifying the attribute
   const char* typeName() const {return "periodic mean";}
  
  //! Return the type of the this aggregators immediate base class
   const char* provides() const {return MeanAggregator::typeName();}

  //! Add another vertex to the segment
   void addVertex(FunctionType f, GlobalIndexType i);

  //! Add another segment
   void addSegment(const Aggregator* seg);

private:

   //! Pointer to the external transformation
   const PeriodicTransformation* mTransformation;
};


class PeriodicMeanArray : public AttributeArray<PeriodicMeanAggregator>
{
public:

  //! The static periodic transformation
  static PeriodicTransformation sTransformation;

  //! Default constructor creating an array of size at least 1
  PeriodicMeanArray(GlobalIndexType size=1) : AttributeArray<PeriodicMeanAggregator>(1,0) {resize(size);}

  //! Destructor
   ~PeriodicMeanArray() {}

  //! Create a clone of yourself
   Attribute* clone() {return new PeriodicMeanArray(*this);}

   //! Set the local transformation to use
   void transformation(const PeriodicTransformation& transform) {sTransformation = transform;}

   //! Resize the array to contain size many elements
   virtual void resize(GlobalIndexType size);

};

class PeriodicMeanMap : public AttributeMap<PeriodicMeanAggregator>
{
public:

  //! Default constructor creating an array of size at least 1
  PeriodicMeanMap(GlobalIndexType size=1) : AttributeMap<PeriodicMeanAggregator>(1,size) {}

  //! Destructor
   ~PeriodicMeanMap() {}

  //! Create a clone of yourself
   Attribute* clone() {return new PeriodicMeanMap(*this);}
};



} // Closing the namespace

#endif /* PERIODICMEAN_H_ */
