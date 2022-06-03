/*
 * TopoInputStream.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOINPUTSTREAM_H
#define TOPOINPUTSTREAM_H

#include <vector>
#include "Token.h"
#include "FiFoBuffer.h"

//! A TopoInputStream defines the streaming input API
class TopoInputStream : public FiFoBuffer
{

public:

  //! Default constructor
  TopoInputStream() {}

  //! Destructor
  virtual ~TopoInputStream() {}

  //! Read one token from the stream
  virtual TopoInputStream& operator>>(Token& t) = 0;

  //! Add the given elements at the "end" of the internal buffer
  /*! Add the given piece of memory to the "end" of the buffer. This
   *  means that if there is enough space it will be added to the front
   *  and otherwise to the back. However, the function guarantees
   *  that this block of memory will be contiguous. If necessary, the
   *  internal buffer will be grown.
   * @param tokens: pointer to the start of the memory buffer
   * @param size: size of the buffer in bytes
   * @return: 1 if successful; 0 otherwise.
   */
  virtual int push(char* tokens, uint32_t size) = 0;

  //! Remove the given number of element from the "beginning" of the buffer
  /*! Remove the given number of bytes from the internal buffer. The
   *  function will assume that this request does not wrap around the array
   *  and return an error if it does
   * @param size: the number of bytes to be removed
   * @return 1 if successfull; 0 otherwise.
   */
  virtual int pop(uint32_t size) = 0;
};



#endif /* TOPOINPUTSTREAM_H_ */
