#ifndef ATTRIBUTEVARIABLE_LIST
#define ATTRIBUTEVARIABLE_LIST
#include <string>
#include <iostream>
#include <vector>
#include "Attribute.h"
#include "AggregatorFactory.h"

namespace Statistics 
{

  class AttributeVariablePair 
  {
  public:
    AttributeVariablePair() { }
    ~AttributeVariablePair() {}

    AttributeVariablePair(const AttributeVariablePair &p) 
    {
      mAttribute = p.mAttribute;
      mVariable = p.mVariable;
      mDerivedAttribute = p.mDerivedAttribute;
      mVisible = p.mVisible;
      mLoaded = p.mLoaded;
    }

    AttributeVariablePair(std::pair<std::string, std::string> &pair) 
    {
     mAttribute = pair.first;
     mVariable = pair.second;
     mDerivedAttribute = pair.first;
     mVisible = mLoaded = false;
    }

    AttributeVariablePair(std::string &attr, std::string &variable, std::string &derived) 
    {
     mAttribute = attr;
     mVariable = variable;
     mDerivedAttribute = derived;
     mVisible = mLoaded = false;
    }


    std::string attribute() const { return mAttribute; }

    std::string variable() const { return mVariable; }

    std::string derivedAttribute() const { return mDerivedAttribute; }

    std::string attributeVariable() const { return (mAttribute + std::string("-") + mVariable); }

    bool isDerivedOff(const std::string &val) const { return(mDerivedAttribute == val && mAttribute != val); }

    bool isAccessor() const { return (mAttribute != mDerivedAttribute); }

    void makeVisible() { mVisible = true; }
    void makeInvisible() { mVisible = false; }
    bool isVisible() const { return mVisible; }
    bool isLoaded() const { return mLoaded; }
    void loaded(bool val) { mLoaded = val; }

    friend std::ostream & operator<<(std::ostream &out, const AttributeVariablePair &pair); 

    void ReplaceCharacterString(const std::string &findStr, const std::string &replaceStr) 
    {
      std::string newAttribute = mAttribute;
      std::string newVariable = mVariable;
      uint32_t pos=0;
      while((pos = newAttribute.find(findStr, 0)) != (uint32_t) std::string::npos) 
        newAttribute.replace(pos, 1, replaceStr);
      pos=0;
      while((pos = newVariable.find(findStr, 0)) != (uint32_t) std::string::npos)
        newVariable.replace(pos, 1, replaceStr);
      mAttribute = newAttribute;
      mVariable = newVariable;
    }
 
  public:
    std::string mAttribute;
    std::string mVariable;
    std::string mDerivedAttribute;
    bool mVisible;
    bool mLoaded;

  };

  class AttributeVariableList 
  {
  public:
    AttributeVariableList() {}
    ~AttributeVariableList() {}
    AttributeVariableList(const AttributeVariableList &p) 
    {
      mList = p.mList;
    }

  // build a attribute variable list from both accumulated and non-accumulated info 
    AttributeVariableList(std::vector<std::pair<std::string, std::string> > &attributeList, std::vector<std::pair<std::string, std::string> > &accumAttributeList) 
    {
      for(uint32_t i=0; i < accumAttributeList.size(); i++) {

        std::string curAttr = accumAttributeList[i].first;
        std::string firstAttr = curAttr;
        do {
          if(!contains(curAttr, accumAttributeList[i].second)) {
            mList.push_back(AttributeVariablePair(curAttr, accumAttributeList[i].second, firstAttr));
          }

          Factory factory;
          //std::cout << "curattr = " << curAttr << std::endl;
          Attribute* attr = factory.make_aggregator(curAttr);
          curAttr = std::string(attr->provides());
          //std::cout << "nextattr = " << curAttr << std::endl;
          delete attr;

        } while(curAttr != std::string(""));
      }

      for(uint32_t i=0; i < attributeList.size(); i++) {
        std::string curAttr = accumAttributeList[i].first;
        std::string firstAttr = curAttr;
        do {
          if(!contains(curAttr, attributeList[i].second)) {
            mList.push_back(AttributeVariablePair(curAttr, attributeList[i].second, firstAttr));
          }

          Factory factory;
          Attribute* attr =factory.make_aggregator(curAttr);
          curAttr = std::string(attr->provides());
          delete attr;

        } while(curAttr != std::string(""));
      }
    }

    bool operator==(const AttributeVariableList &other) const 
    {
      if(mList.size() != other.mList.size()) 
        return false;
      for(uint32_t i=0; i < mList.size(); i++) 
        if(mList[i].attributeVariable() != other.mList[i].attributeVariable()) return false;
      return true;
    }

    AttributeVariableList & operator=(const AttributeVariableList &p) 
    {
      mList = p.mList;
      return (*this);
    }

    AttributeVariablePair& operator[](int id) 
    { 
      return mList[id]; 
    }

    const AttributeVariablePair& operator[](int id) const 
    { 
      return mList[id]; 
    }

    uint32_t size() const 
    { 
      return mList.size(); 
    }

    friend std::ostream & operator<<(std::ostream &out, const AttributeVariableList &list);

    uint32_t getAttributeVariableIndex(const std::string &attributeName, const std::string &variableName) const 
    {
      for(uint32_t i=0; i < mList.size(); i++) 
        if(mList[i].attribute() == attributeName && mList[i].variable() == variableName) 
          return i;
      return -1;
    }

    std::string cleanedUpName(uint32_t i) 
    {
      AttributeVariablePair cleanedUpName = mList[i];
      cleanedUpName.ReplaceCharacterString(")", "-");
      cleanedUpName.ReplaceCharacterString("(", "-");
      cleanedUpName.ReplaceCharacterString("_", "-");
      return cleanedUpName.attributeVariable();
    }

    bool contains(const std::string &attribute, const std::string &variable) 
    {
      return (getAttributeVariableIndex(attribute, variable) != (uint32_t)-1);
    }

  public:
    std::vector<AttributeVariablePair> mList;
  };


};
#endif
