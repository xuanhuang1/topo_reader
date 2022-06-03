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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "TalassConfig.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include "FileData.h"
#include "Aggregator.h"

namespace Statistics {

class Attribute;
class AttributeRef
{
public:

  //! One and only constructor
  AttributeRef(Attribute* att, GlobalIndexType i);

  //! Add another vertex to the segment
  void addVertex(FunctionType f, GlobalIndexType i);

  //! Add another vertex to the segment
  void addVertex(FunctionType f, FunctionType g, GlobalIndexType i);

  //! Add another segment
  void addSegment(const Aggregator* seg);

  //! Reset the attribute
  void reset() ;

  //! Return the value as double
  double value() const;

  //! Pointer to the source attribute
  Attribute* mAttribute;  // This must remain public for double SpecializedAttributeRef<AttributeClass>::value() to be able to access it.

private:

private:
  
  const GlobalIndexType mIndex;
};

template <class AttributeClass>
class SpecializedAttributeRef : public AttributeRef
{
public:

  //! Default Constructor
  SpecializedAttributeRef(Attribute* att, GlobalIndexType i);

  //! Return the value as double
  double value() const;

};

template <class AttributeClass>
double SpecializedAttributeRef<AttributeClass>::value() const
{
  return static_cast<AttributeClass*>(mAttribute)->AttributeClass::value();

  //mAttribute->AttributeClass::value();
}

class AttributeConstRef
{
public:

  //! One and only constructor
  AttributeConstRef(const Attribute* att, GlobalIndexType i);

  ////! Reset the attribute
  void reset();

  //! Return the value as double
  double value() const;

private:

  //! Pointer to the source attribute
  const Attribute* mAttribute;

  const GlobalIndexType mIndex;
};

class Attribute : public TopologyFileFormat::FileData
{
public:

  //friend class AttributeRef;

  //! Default constructor creating space for n variable names
  Attribute(uint8_t num_variables);

  //! Copy constructor
  Attribute(const Attribute& copy) : mVariableNames(copy.mVariableNames), mPreAggregated(copy.mPreAggregated) {}

  //! Destructor
  virtual ~Attribute() {}

  //! Create a clone of yourself
  virtual Attribute* clone() = 0;

  //! Create an accessor array of the given type
  Attribute* accessor_array(std::string& name) const;

  //! Create an accessor map of the given type
  Attribute* accessor_map(std::string& name) const;

  //! Set preaggregation flag
  virtual void preAggregated(bool v) { mPreAggregated = v;};

  //! Return preaggregation flag
  virtual bool preAggregated() const  { return mPreAggregated;};

  /*************************************************************************************
   **************************** Array Interface ****************************************
   ************************************************************************************/

  //! Return the current size
  virtual GlobalIndexType size() const = 0;

  //! Resize the array to contain size many elements
  virtual void resize(GlobalIndexType size) = 0;

  //! Return the i'th attribute
  AttributeRef operator[](GlobalIndexType i) {
    //std::cout << "returning Attribute::attributeRef " << i << std::endl;
    return AttributeRef(this,i);
  }

  //! Return the i'th attribute
  const AttributeConstRef operator[](GlobalIndexType i) const {
    //std::cout << "returning Attribute::attributeRef " << i << std::endl;
    return AttributeConstRef(this,i);
  }

  //! clear the array
  virtual void clear() = 0;



  /*************************************************************************************
   **************************** Attribute Interface ************************************
   ************************************************************************************/

  //! Return the type of this attribute
  virtual AggregatorType type() const = 0;

  //! Return a string identifying the attribute
  virtual const char* typeName() const = 0;

  //! Return the type of the this aggregators immediate base class
  virtual const char* provides() const = 0;

  //! The default variable name
  std::string defaultName() const {return "Unknown";}

//! Return the name of the variables we aggregate
  const std::string& variableName(uint8_t i=0) const {return mVariableNames[i];}

  //! Set the attribute name
  void variableName(const std::string& name, uint8_t i=0) {mVariableNames[i] = name;}

  //! Return the true if attribute name has been specified as something other than the default
  bool variableNameSet(uint8_t i=0) const {return (mVariableNames[i] == defaultName());}

  //! Return the number of variables necessary for this attribute
  uint8_t numVariables() const {return mVariableNames.size();}

  /*************************************************************************************
   **************************** Single Value Interface *********************************
   ************************************************************************************/

   //! Reset the attribute
   virtual void reset() {reset(0);}

   //! Return the initial value as a double
   virtual double initialValue() const {return initialValue(0);}

   //! Add another vertex to the segment
   virtual void addVertex(FunctionType f, GlobalIndexType v) {addVertex((GlobalIndexType)0,f,v);}

   //! Add another vertex to the segment
   virtual void addVertex(FunctionType f, FunctionType g,
                          GlobalIndexType v) {addVertex(0,f,g,v);}

   //! Add another segment
   virtual void addSegment(const Aggregator* seg) {addSegment(0,seg);}

   //! Return the value as double
   virtual double value() const {return value(0);}


   /*************************************************************************************
    **************************** Computation Interface **********************************
    ************************************************************************************/

    //! Reset the attribute
    virtual void reset(GlobalIndexType i) = 0;

    //! Return the initial value as a double
    virtual double initialValue(GlobalIndexType i) const = 0;

    //! Add another vertex to the segment
    virtual void addVertex(GlobalIndexType i, FunctionType f, GlobalIndexType v) = 0;

    //! Add another vertex to the segment
    virtual void addVertex(GlobalIndexType i, FunctionType f, FunctionType g,
                           GlobalIndexType v) = 0;

    //! Add another segment
    virtual void addSegment(GlobalIndexType i, const Aggregator* seg) = 0;

    //! Add another segment
    virtual void addSegment(GlobalIndexType i, GlobalIndexType j) = 0;

    //! Return the value as double
    virtual double value(GlobalIndexType i) const = 0;


private:

  //! A vector of variable names
  std::vector<std::string> mVariableNames;

  bool mPreAggregated;
};


//const std::string Attribute::sDefaultName = "Unkown";

template <class AttributeClass>
class AttributeArray : public Attribute
{
public:

  //! Default constructor
  AttributeArray(uint8_t num_variables, GlobalIndexType size=1) : Attribute(num_variables) {resize(size);}

  //! Destructor
  ~AttributeArray() {}

  /*************************************************************************************
   **************************** Array Interface ****************************************
   ************************************************************************************/
  
  //! Return a reference to the i'th element
  //virtual AttributeRef operator[](GlobalIndexType i) {return AttributeRef(this, i);}
  AttributeClass& operator[](GlobalIndexType i) {
    //std::cout << "returning attributeArray::AttributeClass " << i << std::endl;
    return mAttributes[i];
  }

  //! Return a const reference to the i'th element
  //virtual const AttributeConstRef operator[](GlobalIndexType i) const {return AttributeConstRef(this,i);}
  const AttributeClass& operator[](GlobalIndexType i) const {
  //  std::cout << "returning attributeArray::AttributeClass " << i << std::endl;
    return mAttributes[i];
  }

  //! Return the current size
  GlobalIndexType size() const {return mAttributes.size();}

  //! Resize the array to contain size many elements
  virtual void resize(GlobalIndexType size) {mAttributes.resize(size);}
  
  //! clear the array 
  virtual void clear() {mAttributes.clear();}

  /*************************************************************************************
   **************************** Attribute Interface ************************************
   ************************************************************************************/

  //! Return the type of this attribute
   virtual AggregatorType type() const {return mAttributes[0].type();}

   //! Return a string identifying the attribute
   virtual const char* typeName() const {return mAttributes[0].typeName();}

  //! Return the type of the this aggregators immediate base class
   virtual const char* provides() const {return mAttributes[0].provides();}



  /*************************************************************************************
   **************************** File Interface *****************************************
   ************************************************************************************/

  //! Write the ascii version of the element to the file
  virtual void writeASCII(std::ofstream& output) const;

  //! Read the ascii version of the element from the file
  virtual void readASCII(std::ifstream& input);

  //! Write the binary version of the element to the file
  virtual void writeBinary(std::ofstream& output) const;

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input);

   /*************************************************************************************
    **************************** Computation Interface **********************************
    ************************************************************************************/

  //! Reset all attributes
  virtual void reset();

  //! Reset the attribute
  virtual void reset(GlobalIndexType i) {return mAttributes[i].reset();}

  //! Return the initial value as a double
  virtual double initialValue(GlobalIndexType i) const {return mAttributes[i].initialValue();}

  //! Add another vertex to the segment
  virtual void addVertex(GlobalIndexType i, FunctionType f, GlobalIndexType v) {mAttributes[i].addVertex(f,v);}

  //! Add another vertex to the segment
  virtual void addVertex(GlobalIndexType i, FunctionType f, FunctionType g,
                         GlobalIndexType v) {(&mAttributes[i])->addVertex(f,g,v);}

  //! Add another segment
  virtual void addSegment(GlobalIndexType i, const Aggregator* seg) {mAttributes[i].addSegment(seg);}

  //! Add another segment
  virtual void addSegment(GlobalIndexType i, GlobalIndexType j) {mAttributes[i].addSegment(&mAttributes[j]);}

  //! Return the value as double
  virtual double value(GlobalIndexType i) const {
    //std::cout << "size = " << mAttributes.size() << std::endl;
    ////std::cout << "i = " << i << std::endl;
    //mAttributes[i];
    return (mAttributes[i]).value();
  }

protected:

  //! The vector of attributes
  std::vector<AttributeClass> mAttributes;

};

template <class AttributeClass>
void AttributeArray<AttributeClass>::writeASCII(std::ofstream& output) const
{
  typename std::vector<AttributeClass>::const_iterator it;

  for (it=mAttributes.begin();it!=mAttributes.end();it++) {
    it->writeASCII(output);
    output << std::endl;
  }

}

template <class AttributeClass>
void AttributeArray<AttributeClass>::readASCII(std::ifstream& input)
{
  typename std::vector<AttributeClass>::iterator it;

  for (it=mAttributes.begin();it!=mAttributes.end();it++)
    it->readASCII(input);
}

template <class AttributeClass>
void AttributeArray<AttributeClass>::writeBinary(std::ofstream& output) const
{
  output.write((const char*)&(mAttributes)[0],sizeof(AttributeClass)*mAttributes.size());
}

template <class AttributeClass>
void AttributeArray<AttributeClass>::readBinary(std::ifstream& input)
{
  input.read((char*)&(mAttributes)[0],sizeof(AttributeClass)*mAttributes.size());
  //std::cout << "just read " << sizeof(AttributeClass) << " * " << mAttributes.size() << " = " << sizeof(AttributeClass)*mAttributes.size() << " bytes" << std::endl;

}

template <class AttributeClass>
void AttributeArray<AttributeClass>::reset()
{
  typename std::vector<AttributeClass>::iterator it;

  for (it=mAttributes.begin();it!=mAttributes.end();it++)
    it->reset();

}

template <class AttributeClass>
class AttributeMap : public Attribute
{
public:

  //! Default constructor
  AttributeMap(uint8_t num_variables, GlobalIndexType size=1) : Attribute(num_variables) {}

  //! Destructor
  ~AttributeMap() {}

  /*************************************************************************************
   **************************** Array Interface ****************************************
   ************************************************************************************/
 
  //! Return a reference to the i'th element
  //virtual AttributeRef operator[](GlobalIndexType i) {return AttributeRef(this, i);}
  AttributeClass & operator[](GlobalIndexType i) {
    //std::cout << "returning attributeMap::AttributeClass " << i << std::endl;
    return mAttributes[i];
  }

  //! Return a const reference to the i'th element
  //virtual const AttributeConstRef operator[](GlobalIndexType i) const {return AttributeConstRef(this,i);}
  const AttributeClass & operator[](GlobalIndexType i) const {
    //std::cout << "returning attributeMap::AttributeClass " << i << std::endl;
    typename std::map<GlobalIndexType, AttributeClass>::const_iterator itr = mAttributes.find(i); 
    return itr->second;
  }

  //! Return the current size
  GlobalIndexType size() const {return mAttributes.size();}

  //! Resize the array to contain size many elements
  virtual void resize(GlobalIndexType size) {sterror(true,"AttributeMaps cannot be resized.");}
  
  //! clear the array 
  virtual void clear() {mAttributes.clear();}

  /*************************************************************************************
   **************************** Attribute Interface ************************************
   ************************************************************************************/

  //! Return the type of this attribute
   virtual AggregatorType type() const {return (mAttributes.begin())->second.type();}

   //! Return a string identifying the attribute
   virtual const char* typeName() const {return mAttributes.begin()->second.typeName();}

   //! Return the type of the this aggregators immediate base class
   virtual const char* provides() const {return mAttributes.begin()->second.provides();}


  /*************************************************************************************
   **************************** File Interface *****************************************
   ************************************************************************************/

  //! Write the ascii version of the element to the file
  virtual void writeASCII(std::ofstream& output) const {sterror(true,"AttributeMaps cannot be written to file.");}

  //! Read the ascii version of the element from the file
  virtual void readASCII(std::ifstream& input) {sterror(true,"AttributeMaps cannot be read from file.");}

  //! Write the binary version of the element to the file
  virtual void writeBinary(std::ofstream& output) const {sterror(true,"AttributeMaps cannot be written to file.");}

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input) {sterror(true,"AttributeMaps cannot be read from file.");}

   /*************************************************************************************
    **************************** Computation Interface **********************************
    ************************************************************************************/

    //! Reset the attribute
    virtual void reset(GlobalIndexType i) {return mAttributes[i].reset();}

    //! Return the initial value as a double
    virtual double initialValue(GlobalIndexType i) const {
      typename std::map<GlobalIndexType, AttributeClass>::const_iterator itr = mAttributes.find(i); 
      return (itr->second).initialValue();
    }

    //! Add another vertex to the segment
    virtual void addVertex(GlobalIndexType i, FunctionType f, GlobalIndexType v) {mAttributes[i].addVertex(f,v);}

    //! Add another vertex to the segment
    virtual void addVertex(GlobalIndexType i, FunctionType f, FunctionType g,
                           GlobalIndexType v) {(&mAttributes[i])->addVertex(f,g,v);}

    //! Add another segment
    virtual void addSegment(GlobalIndexType i, const Aggregator* seg) {mAttributes[i].addSegment(seg);}

    //! Add another segment
    virtual void addSegment(GlobalIndexType i, GlobalIndexType j) {mAttributes[i].addSegment(&mAttributes[j]);}

    //! Return the value as double
    virtual double value(GlobalIndexType i) const {
      typename std::map<GlobalIndexType, AttributeClass>::const_iterator itr = mAttributes.find(i); 
      return (itr->second).value();
    }

 protected:

  //! The map of attributes
  std::map<GlobalIndexType, AttributeClass> mAttributes;

};

}

#endif /* ATTRIBUTEARRAY_H_ */
