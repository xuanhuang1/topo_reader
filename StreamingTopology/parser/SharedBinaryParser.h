/***********************************************************************
*
* copyright (c) 2008, Lawrence Livermore National Security, LLC.
* Produced at the Lawrence Livermore National Laboratory
* Written by bremer5@llnl.gov
* OCEC-08-107
* All rights reserved.
*
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/

#ifndef SHAREDBINARYPARSER_H_
#define SHAREDBINARYPARSER_H_

#include "BinaryParser.h"
#include "GenericData.h"

/*! Class to parse an enhanced binary stream of tokens containing
 * information about the codimension and number of clones that
 * exist for a vertex.
 */
template <class DataClass = GenericData<float> >
class SharedBinaryParser : BinaryParser<DataClass>
{
public:

  enum TokenType {
    VERTEX_TOKEN   = 0,
    EDGE_TOKEN     = 1,
    FINALIZE_TOKEN = 2,
  };

  //! Default constructor
  SharedBinaryParser(FILE* input, uint32_t edim, uint32_t fdim=0, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~SharedBinaryParser() {}

protected:

  //! Return the token type of the next token and advance the buffer
  virtual char tokenType();

};

template <class DataClass>
char SharedBinaryParser<DataClass>::tokenType()
{
  switch (this->mBuffer[this->mPos] & 3) {
    case VERTEX_TOKEN:
      return 'v';
    case EDGE_TOKEN:
      return 'e';
    case FINALIZE_TOKEN:
      return 'f';
    default:
      sterror(true,"Unknown token");
      break;
  }

  return 'v';
}



#endif /* SHAREDBINARYPARSER_H_ */
