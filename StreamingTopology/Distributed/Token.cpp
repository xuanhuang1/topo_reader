/*
 * Token.cpp
 *
 *  Created on: Jan 31, 2012
 *      Author: bremer5
 */

#include "Token.h"

GlobalIndexType token_mask(uint8_t token_size)
{
  //        setting the first k bits and shift them to the front
  return (((1 << token_size) - 1) << (8*sizeof(GlobalIndexType) - token_size));
}


std::vector<uint32_t> initialize_token_size()
{
  std::vector<uint32_t> s(NUM_TOKENS);
  for (uint32_t i=0;i<NUM_TOKENS;i++ ) {
    switch ((TokenType)i) {
      case UNDEFINED:
        s[i] = sizeof(Token);
        break;
      case VERTEX:
        s[i] = sizeof(VertexToken);
        break;
      case EDGE:
        s[i] = sizeof(EdgeToken);
        break;
      case FINAL:
        s[i] = sizeof(FinalToken);
        break;
      case EOF:
        s[i] = sizeof(Token);
        break;
    }
  }
  return s;
}

//! Initialize the token sizes
const std::vector<uint32_t> Token::sTokenSizes = initialize_token_size();

uint32_t Token::size() const
{
  return sTokenSizes[type()];
}


