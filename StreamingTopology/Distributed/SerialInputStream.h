/*
 * SeriallInputStream.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef SERIALLINPUTSTREAM_H_
#define SERIALLINPUTSTREAM_H_

#include "TopoInputStream.h"
#include "FiFoBuffer.h"

//! Declaration of the communicator interface
class TopoCommunicator;

//! An input stream that pulls data from a communicator
class SerialInputStream : public TopoInputStream
{
public:

  //! The default undefined token
  static Token sUndefinedToken;

  //! Default constructor
  SerialInputStream(GraphID id, TopoCommunicator* comm);

  //! Destructor
  virtual ~SerialInputStream() {}

  //! Read one token from the stream
  virtual TopoInputStream& operator>>(Token& t);

  //! Add the given number of bytes at the end of the internal buffer
  int push(char* tokens, uint32_t size) {return mBuffer.push(tokens,size);}

  //! Remove the given number of bytes from the beginning of the buffer
  int pop(uint32_t size) {return mBuffer.pop(size);}


private:

  //! The last token we read
  Token& mToken;

  //! The element id we represent
  const GraphID mId;

  //! The communicator from which to pull data
  TopoCommunicator* const mCommunicator;

  //! The internal buffer of tokens
  FiFoBuffer mBuffer;

  //! Return the first token on the buffer
  Token& token() {return reinterpret_cast<Token&>(*mBuffer.top());}

};


#endif /* SERIALLINPUTSTREAM_H_ */
