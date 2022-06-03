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


#ifndef BINARYPARSER_H
#define BINARYPARSER_H

#include "Parser.h"
#include "GenericData.h"

//! Class to parse a binary stream of tokens
template <class DataClass = GenericData<float> >
class BinaryParser : public Parser<DataClass>
{
public:

  typedef typename DataClass::FunctionType FunctionType;

  static const uint16_t sReadBufferSize = 1024;

  //! Default constructor
  BinaryParser(FILE* input, uint32_t edim, uint32_t fdim=0, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~BinaryParser();

  //! Read the next token
  virtual FileToken getToken();

  //! Return the grid size
  virtual void gridSize(GlobalIndexType size[3]) {size[0] = mGridSize[0];size[1] = mGridSize[1];size[2] = mGridSize[2];}

  //! Return a pointer to the last read vertex data
  const FunctionType* coord() const {return mAttributes;}

  //! Return the grid size
  const uint32_t* gridSize() const {return mGridSize;}

protected:

  //! The  dimensions of the (virtual) grid
  uint32_t mGridSize[3];

  //! The number of bytes we read as a chunck from the input buffer
  uint16_t mBufferSize;

  //! The input buffer
  unsigned char* mBuffer;

  //! The current position in the buffer
  uint16_t mPos; 

  //! Pointer to the last read vertex attributes
  FunctionType* mAttributes;

  //! Advance the buffer to have at least step many bytes
  void advance(uint32_t step);

  //! Return the token type of the next token and advance the buffer
  virtual char tokenType() {return mBuffer[mPos++];}

  //! Insert a vertex
  virtual GlobalIndexType insertID(GlobalIndexType id,FunctionType f);

  //! Map a vertex
  virtual GlobalIndexType mapID(GlobalIndexType id) {return id;}

  //! Map and erase a vertex
  virtual GlobalIndexType mapErase(GlobalIndexType id) {return id;}

  //! Indicate that we have read the last input
  virtual void finish() {}
};

template <class DataClass>
BinaryParser<DataClass>::BinaryParser(FILE* input, uint32_t edim, uint32_t fdim, 
                                      const std::vector<uint32_t>& adims) : 
  Parser<DataClass>(input,edim,fdim,adims), mBufferSize(sReadBufferSize), mPos(0)
                                                                            
{
  uint32_t index_bytes;
  uint32_t data_bytes;

  this->mPath.resize(2);

  fread(&index_bytes,sizeof(uint32_t),1,input);
  sterror(index_bytes != sizeof(GlobalIndexType),"Global index type %d-bytes of input does not match with current input of %d-bytes.",sizeof(GlobalIndexType),index_bytes);
    
  fread(&data_bytes,sizeof(uint32_t),1,input);
  sterror(data_bytes != sizeof(FunctionType),"Data size of executable %d-bytes does not match with current input of %d-bytes.",sizeof(FunctionType),data_bytes);
  
  fread(mGridSize,sizeof(uint32_t),3,input);

  fprintf(stderr,"Index bytes %d\nData bytes %d\nDimensions %d %d %d\n\n",
	  index_bytes,data_bytes,mGridSize[0],mGridSize[1],mGridSize[2]);

  mBuffer = new unsigned char[mBufferSize];

  mBufferSize = fread(mBuffer,1,mBufferSize,this->mInput);
}


template <class DataClass>
BinaryParser<DataClass>::~BinaryParser() 
{
  delete[] mBuffer;
}

template <class DataClass>
FileToken BinaryParser<DataClass>::getToken()
{
  // This while statement will take care of reading tokens until it finds a
  // valid one. Tokens can be invalid if vertices are not being processed or
  // edges contain such vertices
  while (true) {
    
    // Make sure that the buffer contains enough entries to at least parse one
    // more token
    advance(std::max(2*sizeof(GlobalIndexType),sizeof(GlobalIndexType)+this->mEDim*sizeof(FunctionType))+1);
    
    // If there is nothing left to read we will return the EMPTY token. Note
    // that this guarantees that the while (true) returns since once the file is
    // empty the loop will end
    if (mBufferSize == 0) {

      // We make sure to handle whatever information we have in various buffers
      finish();

      // End return the final token
      return EMPTY;
    }

    switch (tokenType()) {
    case 'v': {

      this->mId = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);

      mAttributes = (FunctionType*)(mBuffer + mPos);
    
      mPos += this->mEDim*sizeof(FunctionType);

      this->mId = insertID(this->mId,mAttributes[this->mFDim]);

      // If this vertex should be processed at all 
      if (this->mId != GNULL) {
      
        // For portability this assignment has been replaced with a copy constructor
        //call to allow the given DataClass to store more than just the function
        //value if necessary.
        //this->mData.f(attributes[this->mFDim]);
      
        // Read all the necessary attributes
        this->mData = DataClass(mAttributes,this->mFDim);
      
      
      
        if (this->mPersistentAttributes.size() > 0) {
          for (uint32_t i=0;i<this->mPersistentAttributes.size();i++)
            this->mAttributeCache[i]->add(this->mId,mAttributes[this->mPersistentAttributes[i]]);
        }
    

        return VERTEX;
      }
      break;
    }
    case 'e': {

      this->mPath[0] = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);
      this->mPath[1] = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);
 
      this->mPath[0] = mapID(this->mPath[0]);
      this->mPath[1] = mapID(this->mPath[1]);

      if ((this->mPath[0] != GNULL) && (this->mPath[1] != GNULL))
        return EDGE;
      break;
    }
    case 'f': {

      this->mFinal = *((GlobalIndexType*)(mBuffer + mPos));
      mPos += sizeof(GlobalIndexType);

      this->mFinal = mapErase(this->mFinal);
    
      if (this->mFinal != GNULL)
        return FINALIZE;
      break;
    }
    default: {
      sterror(true,"Unkown token: Could not parse file.");
      exit(1);
      return UNKNOWN;
      break;
    }
    }
  }
    
  return UNKNOWN;
}

template <class DataClass>
GlobalIndexType BinaryParser<DataClass>::insertID(GlobalIndexType id,FunctionType f) 
{
  if ((f < this->mFMin) || (f > this->mFMax))
    return GNULL;

  return id;
}


template <class DataClass>
void BinaryParser<DataClass>::advance(uint32_t step)
{
  if (mPos + step >= mBufferSize) {
    
    memcpy(mBuffer,mBuffer+mPos,std::max(mBufferSize-mPos,0));
    
    mBufferSize = fread(mBuffer+std::max(mBufferSize-mPos,0),1,mPos,this->mInput) + std::max(mBufferSize-mPos,0);
    mPos = 0;
  }
}

    

#endif
