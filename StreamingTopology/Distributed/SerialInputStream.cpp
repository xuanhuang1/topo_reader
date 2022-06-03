/*
 * SerialInputStream.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#include "TopoCommunicator.h"
#include "SerialInputStream.h"

Token SerialInputStream::sUndefinedToken = {UNDEFINED};

SerialInputStream::SerialInputStream(GraphID id, TopoCommunicator* comm)
: mToken(sUndefinedToken), mId(id), mCommunicator(comm)
{
  comm->connect(mId,this);
}


TopoInputStream& SerialInputStream::operator>>(Token& t)
{
  // Remove the last token read from the buffer
  mBuffer.pop(this->mToken.size());

  sterror(mBuffer.empty(),"Found empty serial buffer.");

  // If the internal buffer is not empty we assume that there is
  // at least one more *complete* token that we can read
  this->mToken = token();

  t = mToken;

  return *this;
}
