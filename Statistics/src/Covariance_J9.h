#ifndef COVARIANCEJ9_H
#define COVARIANCEJ9_H

#include "Mean.h"

namespace Statistics {

//template<typename FunctionType>
class Covariance_J9Base : public MeanAggregator
{
public:

  //! Default constructor
  Covariance_J9Base();

  //! Copy constructor
  Covariance_J9Base(const Covariance_J9Base& cov);

  //! Destructor
   ~Covariance_J9Base() {}

  /*
  //! Create a clone of yourself
   BaseAttribute<FunctionType>* clone() {return new Covariance_J9Base<FunctionType>(*this);}

  //! Allocate and array of the given size
   BaseAttribute<FunctionType>* allocate(LocalIndexType count=1) {return new Covariance_J9Base<FunctionType>[count];}

  //! Allocate and array of the given size
   void free(BaseAttribute<FunctionType>* p) {delete[] dynamic_cast<Covariance_J9Base<FunctionType>*>(p);}

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
   const char* provides() const {return MeanAggregator::typeName();}

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
   uint32_t size() const {return sizeof(Covariance_J9Base<FunctionType>);}
   */
public:

  //! The avg of the second function
  FunctionType mAvgG;

  //! The co-moment of the 2 functions
  FunctionType mMomentFG;

protected:

  //! Write the ascii version of the element to the file
  // void writeASCIIInternal(std::ofstream & output) const;
};

class Covariance_J9 : public AttributeArray<Covariance_J9Base>
{
public:

  //! Create a clone of yourself
   Attribute* clone() {return new Covariance_J9(*this);}

  //! Return the type of this attribute
  AggregatorType type() const {return ATT_COV;}

  //! Return a string identifying the attribute
  const char* typeName() const {return "covariance";}
  
};

//template <typename FunctionType>
Covariance_J9Base::Covariance_J9Base() : MeanAggregator(), mAvgG(0), mMomentFG(0)
{
}

//template <typename FunctionType>
Covariance_J9Base::Covariance_J9Base(const Covariance_J9Base& cov) : MeanAggregator(cov),
mAvgG(cov.mAvgG), mMomentFG(cov.mMomentFG)
{
}
/*
template <typename FunctionType>
void Covariance_J9Base<FunctionType>::set(const BaseAttribute<FunctionType>* seg)
{
  const Covariance_J9Base* p = dynamic_cast<const Covariance_J9Base*>(seg);

  sterror(p==NULL,"Can only set using segments of identical type.");

  *this = *p;
}
*/
//template <typename FunctionType>
void Covariance_J9Base::reset()
{
  MeanAggregator::reset();

  mAvgG = 0;
  mMomentFG = 0.0;
}

//template <typename FunctionType>
void Covariance_J9Base::addVertex(FunctionType f, FunctionType g, GlobalIndexType i)
{
  FunctionType deltaF= f-this->mAvg;
  FunctionType deltaG= g-this->mAvgG;


  FunctionType n = static_cast<FunctionType>(this->mCount+1);
  FunctionType inv_n = 1./n;
  mAvgG += deltaG*inv_n;
  
  MeanAggregator::addVertex(f, i);

  mMomentFG += ((n-1)/n)*deltaG*deltaF;
}

//template <typename FunctionType>
void Covariance_J9Base::addSegment(const Aggregator* seg)
{
  const Covariance_J9Base* p = static_cast<const Covariance_J9Base*>(seg);

  sterror(p==NULL,"Can only add segments of identical type.");

  FunctionType deltaF = p->mAvg - this->mAvg;
  FunctionType deltaG = p->mAvgG - this->mAvgG;
  int N = this->mCount + p->mCount;

  if(N < 1) {
    mMomentFG = 0.;
    return;
  }

  if(this->mCount < 1) {
    mAvgG = p->mAvgG;
    mMomentFG = p->mMomentFG;
    MeanAggregator::addSegment(seg);
    return;
  }

  if(p->mCount < 1) {
    MeanAggregator::addSegment(seg);
    return;
  }

  FunctionType delta_sur_N = deltaF*deltaG / static_cast<FunctionType>( N );

  mMomentFG = this->mMomentFG + p->mMomentFG + this->mCount*p->mCount*delta_sur_N;

  VertexCountAggregator::addSegment(seg);

}

//template <typename FunctionType>
double Covariance_J9Base::value() const
{
  if (this->mCount == 0 || this->mCount == 1)
    return 0;

  return mMomentFG/static_cast<FunctionType>(this->mCount);
}

//template <typename FunctionType>
void Covariance_J9Base::writeASCII(std::ofstream & output) const
{
  MeanAggregator::writeASCII(output);
  output << " " << mAvgG << " " << mMomentFG;// << " " << value();
}

//template <typename FunctionType>
void Covariance_J9Base::readASCII(std::ifstream& input)
{
  MeanAggregator::readASCII(input);
  input >> mAvgG;
  input >> mMomentFG;
}

/*
template <typename FunctionType>
void Covariance_J9Base<FunctionType>::writeBinary(std::ofstream & output) const
{
  Mean<FunctionType>::writeBinary(output);
  output.write((const char*)&mAvgG,sizeof(FunctionType));
  output.write((const char*)&mMomentFG,sizeof(FunctionType));
}

template <typename FunctionType>
void Covariance_J9Base<FunctionType>::readBinary(std::ifstream& input)
{
  Mean<FunctionType>::readBinary(input);
  input.read((char *)&mAvgG,sizeof(FunctionType));
  input.read((char *)&mMomentFG,sizeof(FunctionType));
}
*/
class Covariance_J9Array : public AttributeArray<Covariance_J9Base>
{
public:

  //! Default constructor creating an array of size at least 1
  Covariance_J9Array(GlobalIndexType size=1) : AttributeArray<Covariance_J9Base>(2,size) {}

  //! Destructor
   ~Covariance_J9Array() {}

  //! Create a clone of yourself
   Attribute* clone() {return new Covariance_J9Array(*this);}

  //! Return the type of this attribute
  AggregatorType type() const {return ATT_COV;}

  //! Return a string identifying the attribute
  const char* typeName() const {return "covariance";}



};

class Covariance_J9Map : public AttributeMap<Covariance_J9Base>
{
public:

  //! Default constructor creating an array of size at least 1
  Covariance_J9Map(GlobalIndexType size=1) : AttributeMap<Covariance_J9Base>(2,size) {}

  //! Destructor
   ~Covariance_J9Map() {}

  //! Create a clone of yourself
   Attribute* clone() {return new Covariance_J9Map(*this);}

  //! Return the type of this attribute
  AggregatorType type() const {return ATT_COV;}

  //! Return a string identifying the attribute
  const char* typeName() const {return "covariance";}
};





}


#endif /* COVARIANCEJ9_H */
