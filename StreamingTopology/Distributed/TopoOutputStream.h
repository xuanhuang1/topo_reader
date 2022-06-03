/*
 * TopoOutputStream.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOOUTPUTSTREAM_H_
#define TOPOOUTPUTSTREAM_H_

#include <vector>
#include "Token.h"

//! A TopoOutputStream defines the streaming output API
class TopoOutputStream {

public:

  //! The default message size
  static const uint32_t sDefaultMessageSize;

  //! Default constructor
  TopoOutputStream(const std::vector<GraphID>& destinations,uint32_t min_message = sDefaultMessageSize);

  //! Destructor
  virtual ~TopoOutputStream() {}

  //! Write a token to the stream
  TopoOutputStream& operator<<(const Token& t) {return write<Token>(t);}
  TopoOutputStream& operator<<(const VertexToken& t) {return write<VertexToken>(t);}
  TopoOutputStream& operator<<(const EdgeToken& t) {return write<EdgeToken>(t);}
  TopoOutputStream& operator<<(const FinalToken& t) {return write<FinalToken>(t);}

protected:

  //! The list of graphs which are the destinations of our messages
  const std::vector<GraphID> mDestinations;

private:

  //! The internal buffer to collect tokens before sending
  std::vector<char> mBuffer;

  //! The minimal size of a message in bytes
  uint32_t mMinimalMessageSize;

  //! The internal write function to be re-implemented
  /*! The write function used by the operator to actually pass on
   *  the latest buffer. The downstream function is responsible for
   *  copying the buffer is necessary since once this function
   *  returns the stream will reclaim the buffer.
   * @param buffer: Pointer to the start of the buffer
   * @param size: Number of bytes to write
   * @return 1 if successful; 0 otherwise.
   */
  virtual int write(char* buffer, uint32_t size) = 0;

  template <class TokenType>
  TopoOutputStream& write(const TokenType& token);
};

template <class TokenType>
TopoOutputStream& TopoOutputStream::write(const TokenType& token)
{
  uint32_t current = mBuffer.size();
  // Make sure that there is enough space in the buffer
  mBuffer.resize(current + token.size());

  memcpy(&mBuffer[current],&token,token.size());

  //! If we now have a message big enough to send
  if (mBuffer.size() > mMinimalMessageSize) {
    // Pass the buffer onward
    write(&mBuffer[0],mBuffer.size());

    // Clear the buffer. Note that this should not actually free
    // the memory of the buffer
    mBuffer.resize(0);
  }

  return *this;
}



#endif /* TOPOOUTPUTSTREAM_H_ */
