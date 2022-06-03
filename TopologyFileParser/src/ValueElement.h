#ifndef VALUEELEMENT_H
#define VALUEELEMENT_H

#include "TalassConfig.h"
#include "FileElement.h"

namespace TopologyFileFormat {

template <typename ValueType>
class ValueElement : public FileElement
{
public:

  friend std::ofstream& operator<<(std::ofstream& output, const ValueElement<ValueType>& elem) {
    output << elem.mValue << std::endl;
    return output;
  }

  friend std::ifstream& operator>>(std::ifstream& input, ValueElement<ValueType>& elem) {
    input >> elem.mValue;
    return input;
  }

  //! Default constructor
  ValueElement(ValueType f=0) : FileElement(), mValue(f) {}

  // Destructor
  ~ValueElement() {}

  //! Write the ascii version of the element to the file
  virtual void writeASCII(std::ofstream& output) const {output << mValue << std::endl;}

  //! Read the ascii version of the element from the file
  virtual void readASCII(std::ifstream& input) {input >> mValue;}

  //! Write the binary version of the element to the file
  virtual void writeBinary(std::fstream& output) const {output.write((const char*)(&mValue),sizeof(ValueType));}

  //! Read the binary version of the element from the file
  virtual void readBinary(std::ifstream& input) {input.read((char *)(&mValue),sizeof(ValueType));}

  //! Return the size in number of bytes
  virtual unsigned long size() const {return sizeof(ValueElement);}

  //! Offset operator to provide access to arrays represented only as FileElement*
  virtual FileElement& operator[](unsigned int i) {return *(this + i);}

  ValueType value() const {return mValue;}

  void value(const ValueType& v) {mValue = v;}

private:

  //! The actual value
  ValueType mValue;

};

/*

template <typename ValueType>
std::ofstream& operator<<(std::ofstream& output, const ValueElement<ValueType>& elem)
{
  elem.writeASCII(output);
  return output;
}

template <typename ValueType>
std::ifstream& operator>>(std::ifstream& input, ValueElement<ValueType>& elem)
{
  elem.readASCII(input);
  return input;
}

std::ofstream& operator<<(std::ofstream& output, const ValueElement<FunctionType>& elem)
{
  elem.writeASCII(output);
  return output;
}


std::ifstream& operator>>(std::ifstream& input, ValueElement<FunctionType>& elem)
{
  elem.readASCII(input);
  return input;
}
*/

} // end namespace



#endif
