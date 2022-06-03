/*
 * HandleKeys.h
 *
 *  Created on: Feb 26, 2012
 *      Author: bremer5
 */

#ifndef HANDLEKEYS_H_
#define HANDLEKEYS_H_

#include <string>
#include <iostream>
#include  "xmlParser.h"

namespace TopologyFileFormat {

typedef uint32_t TimeIndexType;

template <class BaseClass>
class BaseKey
{
public:

  //! Constructor
  BaseKey() {}

  //! Constructor
  BaseKey(const BaseClass& key) : mBaseKey(key) {}

  //! Copy constructor
  BaseKey(const BaseKey& key) : mBaseKey(key.base()) {}

  //! Destructor
  ~BaseKey() {}

  //! Less than operator
  bool operator<(const BaseKey& key) const {return (mBaseKey < key);}

  //! Return the base key
  const BaseClass& base() const {return mBaseKey;}

  /*
  //! Read in a key from xml
  int parseXML(const XMLNode& node, uint8_t count=0);

  //! Write the attributes to xml
  int attachXML(XMLNode& node, uint8_t count=0) const;
   */
private:

  //! The actual key
  BaseClass mBaseKey;
};

/*
template <class BaseClass>
int SingleKey<BaseClass>::parseXML(const XMLNode& node, uint8_t count)
{
  BaseClass::parseXML(node,count+1);

  char attr_name[10];
  sprintf(attr_name,"id_%d",count);

  if (node.getAttribute(attr_name,0) == NULL) {
     this->id("Unknown");
     return 0;
  }
  else {
    std::stringstream input(std::string(node.getAttribute(attr_name,0)));
    input >> this->mId;
  }

  return 1;
}

template <class BaseClass>
int SingleKey<BaseClass>::attachXML(XMLNode& node, uint8_t count) const
{
  BaseClass::attachXML(node,count+1);

  char attr_name[10];
  sprintf(attr_name,"id_%d",count);
  node.addAttribute(attr_name,mId.c_str());

  return 1;

}

*/


}

#endif /* HANDLECOLLECTION_H_ */
