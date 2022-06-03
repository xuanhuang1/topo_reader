#include "Attribute.h"
#include "AggregatorFactory.h"

namespace Statistics {

AttributeRef::AttributeRef(Attribute* att, GlobalIndexType i) : mAttribute(att), mIndex(i)
{
}

void AttributeRef::addVertex(FunctionType f, GlobalIndexType i)
{
  mAttribute->addVertex(mIndex,f,i);
}

void AttributeRef::addVertex(FunctionType f, FunctionType g,
                             GlobalIndexType i)
{
  mAttribute->addVertex(mIndex,f,g,i);
}

void AttributeRef::addSegment(const Aggregator* seg)
{
  mAttribute->addSegment(mIndex,seg);
}

//! Return the value as double
double AttributeRef::value() const
{
  return mAttribute->value(mIndex);
}

//! One and only constructor
AttributeConstRef::AttributeConstRef(const Attribute* att, GlobalIndexType i) :
    mAttribute(att), mIndex(i)
{
}

//! Reset the attribute
void AttributeConstRef::reset() 
{
  sterror(true, "cannot reset a const ref");
  //mAttribute->reset(mIndex);
}

//! Return the value as double
double AttributeConstRef::value() const
{
  return mAttribute->value(mIndex);
}

Attribute::Attribute(uint8_t num_variables) :
    mVariableNames(num_variables,defaultName()),
    mPreAggregated(false)
{
}


Attribute* Attribute::accessor_array(std::string& name) const
{
  Factory factory;

  return factory.make_accessor_array(this,typeName(),name.c_str());
}

Attribute* Attribute::accessor_map(std::string& name) const
{
  Factory factory;

  return factory.make_accessor_map(this,typeName(),name.c_str());
}
};
