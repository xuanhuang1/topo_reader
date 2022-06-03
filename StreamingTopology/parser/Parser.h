/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
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


#ifndef PARSER_H
#define PARSER_H

#include <cstdio>
#include <vector>
#include "Definitions.h"
#include "GenericData.h"
#include "BoundaryMarker.h"

#ifndef ST_INCORE_ARRAYS
  //! Typedef to easily change between array representations
 #include "OOCArray.h" 
#else
 #include "BlockedArray.h" 
#endif


//! Set of possible file tokens
enum FileToken {
  
  EMPTY    = 0,
  VERTEX   = 1,
  EDGE     = 2,
  PATH     = 3,
  FINALIZE = 4,
  UNKNOWN  = 5,
};

//! Baseclass for all file parsers
/*! A Parser is forms the baseclass for all file readers and defines
 *  the common interface. The interface is designed for the streaming
 *  reading of data using three main calls: First, the creation of a
 *  new vertex; Second, the entry of a new path of vertices; and Third
 *  the finalization of a vertex.
 *
 *  Each time getToken is called the parser will advance in the file
 *  and return the type of the token read. It will store the
 *  corresponding data (the function data, index, path etc.) locally
 *  which can then be extracted using the given functions. 
 *
 *  A parser also support various number of input dimensions. As can be seen in
 *  the constructor, a parser must allow the user to pass in an "arbirtary"
 *  number of input dimensions (called the embedded dimension), let the user
 *  specify one of these as function dimension (fdim), and if necessary be
 *  capable of storing a subset of the input dimensions in an out-of-core array.
 */
template <class DataClass = GenericData<float> >
class Parser
{
public:

  typedef typename DataClass::FunctionType FunctionType;

#ifndef ST_INCORE_ARRAYS
  //! Typedef to easily change between array representations
  typedef FlexArray::OOCArray<FunctionType,LocalIndexType> CacheArray;
#else
  typedef FlexArray::BlockedArray<FunctionType,LocalIndexType> CacheArray;
#endif

  //! Default constructor
  /*! The default constructor describing the minimal functionality a parser
   *  should have. We are given a file stream to read from, the number of
   *  attributes each vertex has, which of these attributes is the function
   *  value, and a (potentially empty) list of dimensions we need to store for
   *  post-processing.
   *  @param input: The input file stream
   *  @param edim: The number of attributes provided for each vertex
   *  @param fdim: The index of the attribute that should be used as function value
   *  @param adims: A list of attributes that should be stored for processing
   */
  Parser(FILE* input, uint32_t edim, uint32_t fdim=0, const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor that automatically closes the file
  virtual ~Parser();

  //! Read the next token and return its type
  virtual FileToken getToken() = 0;

  //! Return the last id that was read
  virtual GlobalIndexType getId() const {return mId;}

  //! Return a reference to the last piece of data parsed
  virtual const DataClass& getData() const {return mData;}

  //! Return the (unsorted) list of vertices defining the last path !
  //! that was parsed
  virtual const std::vector<GlobalIndexType>& getPath() const {return mPath;}

  //! Return the id of the last vertex that was finalized
  virtual const GlobalIndexType getFinalized() const {return mFinal;}

  //! Determine whether the last finalized vertex is restricted
  virtual bool getRestricted() const {return mRestrictedFlag;}

  //! Return a pointer to all cached attributes
  virtual const std::vector<CacheArray*>& attributes() const {return mAttributeCache;}

  //! Return one of the cached attributes 
  virtual const CacheArray& attribute(uint32_t i) const;

  //! Return the minimal function value passed on 
  FunctionType fMin() {return mFMin;}

  //! Return the aximal function value passed on 
  FunctionType fMax() {return mFMax;}

  //! Set the minimal function value passed on 
  void fMin(FunctionType f) {mFMin = f;}

  //! Set the aximal function value passed on 
  void fMax(FunctionType f) {mFMax = f;}

protected:

  //! The input file stream
  FILE* mInput;
  
  //! Dimension of the ambient space / number of vertex attributes
  const uint32_t mEDim;

  //! Index of the attribute that should be used as function
  const uint32_t mFDim;

  //! Indices of the attributes that will be used for processing
  const std::vector<uint32_t> mPersistentAttributes;

  //! Array's storing all attribute values that will be used for processing
  std::vector<CacheArray*> mAttributeCache;

  //! The id of the last vertex read
  GlobalIndexType mId;

  //! The data corresponding to the last data type read
  DataClass mData;

  //! The last path that was parsed
  std::vector<GlobalIndexType> mPath;

  //! Index of the last vertex that was finalized
  GlobalIndexType mFinal;

  //! Flag indicating whether the last finalized vertex is restricted
  bool mRestrictedFlag;

  //! Minimum function value for which vertices are passed on
  FunctionType mFMin;
  
  //! Minimum function value for which vertices are passed on
  FunctionType mFMax;

  FILE* openFile(const char* filename, const char* mode);
  
  void closeFile();
  
};

template <class DataClass>
Parser<DataClass>::Parser(FILE* input, uint32_t edim, uint32_t fdim,const std::vector<uint32_t>& adims) 
  : mInput(input), mEDim(edim), mFDim(fdim), mPersistentAttributes(adims),
    mAttributeCache(adims.size(),NULL), mId(GNULL),
    mFinal(GNULL), mRestrictedFlag(false),mFMin(gMinValue), mFMax(gMaxValue)
{
  for (uint32_t i=0;i<adims.size();i++) 
    mAttributeCache[i] = new CacheArray(24);
  
}


template <class DataClass>
Parser<DataClass>::~Parser()
{
  for (uint32_t i=0;i<mAttributeCache.size();i++) {
    if (mAttributeCache[i] != NULL)
      delete mAttributeCache[i];
  }
  
}

template <class DataClass>
const typename Parser<DataClass>::CacheArray& Parser<DataClass>::attribute(uint32_t i) const
{
  sterror(i>=mAttributeCache.size(),"Attribute index %d out of range. This parser cached only %d many attributes.",i,mAttributeCache.size());

  return *mAttributeCache[i];
}


template <class DataClass>
FILE* Parser<DataClass>::openFile(const char* filename, const char* mode)
{
  FILE* f;

  f = fopen(filename,mode);
  
  if (f == NULL)
    stwarning("Could not open file \"%s\".",filename);
  
  return f;
}
  
template <class DataClass>
void Parser<DataClass>::closeFile()
{
  if (mInput != NULL) {
    fclose(mInput);
    mInput = NULL;
  }
}

#endif
