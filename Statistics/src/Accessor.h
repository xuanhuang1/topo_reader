
#ifndef ACCESSOR_H
#define ACCESSOR_H

#include "Attribute.h"

namespace Statistics {


template <class BaseClass,class DerivedClass>
class Accessor : public Attribute
{
public:

  //! Friend declaration to allow calling the protected access functionsq
  friend class AttributeRef;

  //! Default constructor
  Accessor(const Attribute* base);

  //! Copy constructor
  Accessor(const Accessor& acc);

  //! Destructor
  virtual ~Accessor() {}

  //! Create a clone of yourself
   virtual Attribute* clone()=0;

  //! Return the current size
  virtual GlobalIndexType size() const=0; 

  //! Resize the array to contain size many elements
  virtual void resize(GlobalIndexType size) {sterror(true,"Accessors cannot modify attributes.");}

  //! clear the array 
  virtual void clear() {sterror(true,"Accessors cannot modify attributes.");}

  //! Return the type of this attribute
  virtual AggregatorType type() const {BaseClass b; return b.type();}

  //! Return a string identifying the attribute
  virtual const char* typeName() const {BaseClass b; return b.typeName();}

  //! Return the type of the this aggregators immediate base class
  virtual const char* provides() const  {BaseClass b; return b.provides(); };



  /*************************************************************************************
   **************************** File Interface *****************************************
   ************************************************************************************/

  //! Write the ascii version of the element to the file
  virtual void writeASCII(std::ofstream& output) const {sterror(true,"Accessors cannot be written to file.");}

  //! Read the ascii version of the element from the file
  virtual void readASCII(std::ifstream& input) {sterror(true,"Accessors cannot be read from file.");}

  //! Write the binary version of the element to the file
  virtual void writeBinary(std::ofstream& output) const {sterror(true,"Accessors cannot be written to file.");}

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input) {sterror(true,"Accessors cannot be read from file.");}


protected:

  /*************************************************************************************
   **************************** Single Value Compute Interface *************************
   ************************************************************************************/

  //! Reset the attribute
  virtual void reset(GlobalIndexType i) {sterror(true,"Accessors cannot modify attributes.");}

  //! Return the initial value as a double
  virtual double initialValue(GlobalIndexType i) const =0;

  //! Add another vertex to the segment
  virtual void addVertex(GlobalIndexType i,FunctionType f, GlobalIndexType v) {sterror(true,"Accessors cannot modify attributes.");}

  virtual void addVertex(GlobalIndexType i,FunctionType f, FunctionType g, GlobalIndexType v) {sterror(true,"Accessors cannot modify attributes.");}

  //! Add another segment
  virtual void addSegment(GlobalIndexType i,const Aggregator* seg) {sterror(true,"Accessors cannot modify attributes.");}

  //! Add another segment
  virtual void addSegment(GlobalIndexType i, GlobalIndexType j) {sterror(true,"Accessors cannot modify attributes.");}

  //! Return the value as double
  virtual double value(GlobalIndexType i) const=0; 

};

template <class BaseClass, class DerivedClass>
Accessor<BaseClass,DerivedClass>::Accessor(const Attribute* base) : Attribute(*base) { }

template <class BaseClass, class DerivedClass>
Accessor<BaseClass,DerivedClass>::Accessor(const Accessor& acc) : Attribute(acc) {}


template <class BaseClass,class DerivedClass>
class AccessorArray : public Accessor<BaseClass, DerivedClass>
{
public:

  //! Typedef for the attribute array
  typedef AttributeArray<DerivedClass> ArrayClass;

  //! Default constructor
  //AccessorArray(const Accessor<BaseClass, DerivedClass>* base);
  AccessorArray(const Attribute* base);

  //! Copy constructor
  AccessorArray(const AccessorArray& acc);

  //! Destructor
  virtual ~AccessorArray() {}

  //! Create a clone of yourself
  virtual Attribute* clone() {return new AccessorArray(*this);}

  //! Return the current size
  virtual GlobalIndexType size() const {return mBasePointer->size();}


protected:

  /*************************************************************************************
   **************************** Single Value Compute Interface *************************
   ************************************************************************************/

  //! Return the initial value as a double
  //virtual double initialValue(GlobalIndexType i) const {return mBasePointer->getContainedClass(i).BaseClass::initialValue();}
  virtual double initialValue(GlobalIndexType i) const {return (*mBasePointer)[i].BaseClass::initialValue();}
  //virtual double initialValue(GlobalIndexType i) const {return (*mBasePointer).indexElement(i).BaseClass::initialValue();}

  //! Return the value as double
  //virtual double value(GlobalIndexType i) const {return mBasePointer->getContainedClass(i).BaseClass::value();}
  virtual double value(GlobalIndexType i) const {return (*mBasePointer)[i].BaseClass::value();}
  //virtual double value(GlobalIndexType i) const {return (*mBasePointer).indexElement(i).BaseClass::value();}

private:

    const ArrayClass* mBasePointer;
};

template <class BaseClass, class DerivedClass>
//AccessorArray<BaseClass,DerivedClass>::AccessorArray(const Accessor<BaseClass, DerivedClass>* base) : Accessor<BaseClass, DerivedClass>(base), mBasePointer(dynamic_cast<const ArrayClass*>(base))
AccessorArray<BaseClass,DerivedClass>::AccessorArray(const Attribute* base) : Accessor<BaseClass, DerivedClass>(base), mBasePointer(dynamic_cast<const ArrayClass*>(base))
{
  sterror(mBasePointer==NULL,"Could not up-cast attribute of type %s",base->typeName());
}

template <class BaseClass, class DerivedClass>
AccessorArray<BaseClass,DerivedClass>::AccessorArray(const AccessorArray& acc) : Accessor<BaseClass, DerivedClass>(acc), mBasePointer(acc.mBasePointer)
{
  sterror(mBasePointer==NULL,"Could not up-cast attribute of type %s",acc.typeName());
}


template <class BaseClass,class DerivedClass>
class AccessorMap : public Accessor<BaseClass, DerivedClass>
{
public:

  //! Typedef for the attribute array
  typedef AttributeMap<DerivedClass> MapClass;

  //! Default constructor
  //AccessorMap(const Accessor<BaseClass, DerivedClass>* base);
  AccessorMap(const Attribute* base);

  //! Copy constructor
  AccessorMap(const AccessorMap& acc);

  //! Destructor
  virtual ~AccessorMap() {}

  //! Create a clone of yourself
  virtual Attribute* clone() {return new AccessorMap(*this);}

  //! Return the current size
  virtual GlobalIndexType size() const {return mBasePointer->size();}


protected:

  /*************************************************************************************
   **************************** Single Value Compute Interface *************************
   ************************************************************************************/

  //! Return the initial value as a double
  //virtual double initialValue(GlobalIndexType i) const {return mBasePointer->getContainedClass(i).BaseClass::initialValue();}
  //virtual double initialValue(GlobalIndexType i) const {return (*mBasePointer).indexElement(i).BaseClass::initialValue();}
  virtual double initialValue(GlobalIndexType i) const {return (*mBasePointer)[i].BaseClass::initialValue();}

  //! Return the value as double
  //virtual double value(GlobalIndexType i) const {return mBasePointer->getContainedClass(i).BaseClass::value();}
  virtual double value(GlobalIndexType i) const {return (*mBasePointer)[i].BaseClass::value();}
  //virtual double value(GlobalIndexType i) const {return (*mBasePointer).indexElement(i).BaseClass::value();}

private:

    const MapClass* mBasePointer;
};

template <class BaseClass, class DerivedClass>
//AccessorMap<BaseClass,DerivedClass>::AccessorMap(const Accessor<BaseClass, DerivedClass>* base) : Accessor<BaseClass, DerivedClass>(base), mBasePointer(dynamic_cast<const MapClass*>(base))
AccessorMap<BaseClass,DerivedClass>::AccessorMap(const Attribute* base) : Accessor<BaseClass, DerivedClass>(base), mBasePointer(dynamic_cast<const MapClass*>(base))
{
  sterror(mBasePointer==NULL,"Could not up-cast attribute of type %s",base->typeName());
}

template <class BaseClass, class DerivedClass>
AccessorMap<BaseClass,DerivedClass>::AccessorMap(const AccessorMap& acc) : Accessor<BaseClass, DerivedClass>(acc), mBasePointer(acc.mBasePointer)
{
  sterror(mBasePointer==NULL,"Could not up-cast attribute of type %s",acc.typeName());
}
}
#endif /* ACCESSOR_H_ */
