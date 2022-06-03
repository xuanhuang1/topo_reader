/*
 * Token.h
 *
 *  Created on: Jan 31, 2012
 *      Author: bremer5
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include <vector>
#include "DistributedDefinitions.h"

//! The number of recognized tokens
#define NUM_TOKENS 5

//! All recognized token types
enum TokenType {
  UNDEFINED = 0,
  VERTEX    = 1,
  EDGE      = 2,
  FINAL     = 3,
  EMPTY     = 4,
};


//! The number of bits necessary to encode tokens
static const uint8_t sTokenSize = 3;

//! Function to compute derived masked to avoid static initialization fiasco
GlobalIndexType token_mask(uint8_t token_size);

//! The mask extracting the index values for a combined index + type
static const GlobalIndexType sIndexMask = ~token_mask(sTokenSize);

//! The mask extracting the token bits from a combined index + type
static const GlobalIndexType sTokenMask = token_mask(sTokenSize);


class FinalToken
{
public:

  //! The full token type mask
  static const GlobalIndexType sTokenType = FINAL << (8*sizeof(GlobalIndexType) - sTokenSize);

  //! Return the raw index without the type bits
  GlobalIndexType index() const {return mIndex & sIndexMask;}

  //! Set the raw index which sets the type bits
  void index(GlobalIndexType i) {sterror(i&sTokenType != 0,"Index out of range");mIndex = i | sTokenType;}

  //! Return the size in bytes of this token
  uint32_t size() const {return sizeof(FinalToken);}

public:

  //! The index with the type encoded in the top k bits
  GlobalIndexType mIndex;

};

//! Token that encodes a base vertex
template <class DataClass, TokenType type>
class BaseVertexToken
{
public:

  //! The full token type mask
  static const GlobalIndexType sTokenType = type << (8*sizeof(GlobalIndexType) - sTokenSize);

  //! Return the raw index without the type bits
  GlobalIndexType index() const {return mIndex & sIndexMask;}

  //! Set the raw index which sets the type bits
  void index(GlobalIndexType i) {sterror(i&sTokenType != 0,"Index out of range");mIndex = i | sTokenType;}

  //! Return the multiplcity
  uint8_t multiplicity() const {return mMultiplicity;}

  //! Set the multiplicity
  void multiplicity(uint8_t mult) {mMultiplicity = mult;}

  //! Return a reference to the data object
  const DataClass& value() const {return mValue;}

  //! Return the size in bytes of this token
  uint32_t size() const {return sizeof(BaseVertexToken<DataClass,type>);}

public:

  //! The index with the type encoded in the top k bits
  GlobalIndexType mIndex;

  //! The data
  DataClass mValue;

  //! The multiplicity
  uint8_t mMultiplicity;
};


//! The default vertex token uses the FunctionType
typedef BaseVertexToken<FunctionType,VERTEX> VertexToken;


//! Token to encode an edge between to vertices
class EdgeToken
{
public:

  //! The full token type mask
  static const GlobalIndexType sTokenType = EDGE << (8*sizeof(GlobalIndexType) - sTokenSize);

  //! Access operator (note that it does not return a reference)
  GlobalIndexType operator[](int i) const {return mIndex[i] & sIndexMask;}

  //! Set the source
  void source(GlobalIndexType index) {mIndex[0] = index | sTokenType;}

  //! Set the destination
  void destination(GlobalIndexType index) {mIndex[1] = index;}

  //! Determine the size
  uint32_t size() const {return sizeof(EdgeToken);}

public:

  //! The two edge indices
  GlobalIndexType mIndex[2];
};

//! Class to identify and convert tokens used by TopoStreams
class Token {
public:

  //! The array of token sizes to avoid the run-time switch statement
  static const std::vector<uint32_t> sTokenSizes;

  //! Convert a token into a finalization token reference
  operator const FinalToken&() const {return reinterpret_cast<const FinalToken&>(*this);}

  //! Convert a token into a vertex reference
  operator const VertexToken&() const {return reinterpret_cast<const VertexToken&>(*this);}

  //! Convert a token into a edge reference
  operator const EdgeToken&() const {return reinterpret_cast<const EdgeToken&>(*this);}

  //! The size in bytes of this token
  uint32_t size() const;

  //! The type of this token
  //TokenType type() const {return (TokenType)(mToken >> (8*sizeof(mToken) - sTokenSize);}
  TokenType type() const {return VERTEX;}

  //! Set the token type
  void type(TokenType t) {mToken = t << (8*sizeof(mToken) - sTokenSize);}

  uint32_t mToken;
};



#endif /* TOKEN_H_ */
