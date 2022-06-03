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


#ifndef SMAPARSER_H
#define SMAPARSER_H

#include <stack>

#include "Parser.h"
#include "GenericData.h"

//! Parser interface for streaming meshes in ascii format
/*! This class implements the parser interface for simplicial meshes
 *  of any dimension stored in streaming format. The accepted format
 *  is as follows.
 * 
 *  v <float> <float> ... <float>  : a vertex with with n coordinates
 *  f <int> <int> .... <int>       : an m-dimensional simplex
 *  x <int>                        : explicit finalization of a vertex
 *
 *  All indices are 1-based following standard obj
 *  conventions. Vertices can be finalized in two ways. Either by
 *  explicit finalization using "x <index>" to finalize the vertex
 *  with the given index; Or by negating the index in a
 *  simplex. Meaning a triangle "f -2 3 -4" indicates that this
 *  triangle is the last triangle that uses vertices 2 and 4. 
 *
 *  The class uses a function called constructData to convert the n
 *  coordinates read for each vertex into an instance of
 *  DataClass. This is a virtual function that can be overwritten by
 *  dervied classes that want to create more sophisticated data
 *  classes.
 */
template <class DataClass = GenericData<float> >
class SMAParser : public Parser<DataClass>
{
public:

  //! Repeat of the typedef since most compilers are not smart enough
  typedef typename DataClass::FunctionType FunctionType;


  //! Default constructor
  /*! The default constructor which set the necessary dimensions.
   *  @param input: Pointer to the input stream
   *  @param emb_dim: Dimension of the embedding space / number of 
   *                  vertex coordinates
   *  @param sim_dim: Dimension+1 of the mesh / number of vertices
   *                  per simplex
   *  @param func_dim: Dimension that should be used as function
   */
  SMAParser(FILE* input, uint32_t edim, uint32_t sdim, uint32_t fdim=0, 
            const std::vector<uint32_t>& adims = std::vector<uint32_t>());

  //! Destructor
  virtual ~SMAParser();

  //! Read the next token
  FileToken getToken();

  const std::vector<float>& vertex() const {return mVertex;}

protected:

  //! Mesh dimension+1 / number of vertices per simplex
  const uint32_t mSDim;

  //! Buffer of vertices that must be finalized
  std::stack<GlobalIndexType> mFinalize;

  //! Vertex coordinates
  std::vector<FunctionType> mVertex;

  //! Maximal number of charcters allowed on a line
  const int32_t mBufferSize;

  //! Buffer to hold each line
  char* mBuffer; 

  //! Split the given line at all spaces and return the start of all
  //tokens
  void splitLine(char* line, char** token);

  //! Convert the n-dimensional mVertex into a DataClass object
  virtual DataClass constructData();
};
  

template <class DataClass>
SMAParser<DataClass>::SMAParser(FILE* input, uint32_t edim,uint32_t sdim, uint32_t fdim, 
                                const std::vector<uint32_t>& adims) :
  Parser<DataClass>(input,edim,fdim,adims), mSDim(sdim), mBufferSize(this->mEDim * 30 + 5)
{
  this->mPath.resize(mSDim);
  mVertex.resize(this->mEDim);

  this->mId = static_cast<GlobalIndexType>(-1);

  mBuffer = new char[mBufferSize];
}

template <class DataClass>
SMAParser<DataClass>::~SMAParser()
{
  delete mBuffer;
}

template <class DataClass>
FileToken SMAParser<DataClass>::getToken()
{
  char *tmp;

  // If we have stored finalization information that has not been read
  // output this first
  if (!mFinalize.empty()) {
    this->mFinal = mFinalize.top();
    mFinalize.pop();
    
    return FINALIZE;
  }
  

  if (feof(this->mInput)) 
    return EMPTY;

  tmp = fgets(mBuffer,mBufferSize,this->mInput);

  if (tmp == NULL) {
    //stwarning("Problems parsing input file no tokens can be read.");
    return UNKNOWN;
  }

  switch (mBuffer[0]) {
  case 'v': {
    char** numbers = new char*[this->mEDim+1];
    float tmp;

    splitLine(mBuffer,numbers);
    for (uint32_t i=0;i<this->mEDim;i++) {
      sscanf(numbers[i+1],"%f",&tmp);
      mVertex[i] = tmp;
    }
    
    
    // For protability this assignment has been replaced with a copy constructor
    // call to allow the given DataClass to store more than just the function
    // value if necessary. (ptb Jan 2010)
    // this->mData.f(mVertex[this->mFDim]);
    this->mData = DataClass(&mVertex[0],this->mFDim);

    this->mId++;

    if (this->mPersistentAttributes.size() > 0) {
      
      for (uint32_t k=0;k<this->mPersistentAttributes.size();k++) {
        this->mAttributeCache[k]->add(this->mId,mVertex[this->mPersistentAttributes[k]]);
      }
    }


	delete[] numbers;
    return VERTEX;
  }
  case 'f': {
    char** numbers = new char*[mSDim+1];
    int32_t index;

    splitLine(mBuffer,numbers);
    for (uint32_t i=0;i<mSDim;i++) {
      sscanf(numbers[i+1],"%d",&index);
      if (index < 0) {
        //index += this->mId + 1;
        index = -index -1;
        mFinalize.push(static_cast<GlobalIndexType>(index));
      }
      else
        index--;
      
      this->mPath[i] = index;
    }
    delete[] numbers;
    return PATH;
  }
  case 'e':
    int32_t path[2];
    sscanf(mBuffer,"e %d %d",path,path+1);
    if (path[0] < 0) {
      this->mPath[0] = path[0] +this->mId + 1;
      mFinalize.push(static_cast<GlobalIndexType>(this->mPath[0]));
    }
    else
      this->mPath[0] = path[0] - 1;

      
    if (path[1] < 0) {
      this->mPath[1] = path[1] + this->mId + 1;
      mFinalize.push(static_cast<GlobalIndexType>(this->mPath[1]));
    }
    else
      this->mPath[1] = path[1] - 1;

    return EDGE;

  case 'x': {
    uint64_t tmp;
    sscanf(mBuffer,"%llx",&tmp);
    
    this->mFinal = static_cast<GlobalIndexType>(tmp);

    return FINALIZE;
  }
  default:
    return UNKNOWN;
  }

  return UNKNOWN;
}
  

  
template <class DataClass>
void SMAParser<DataClass>::splitLine(char* line, char** token)
{
  int i = 0;
  int n = 0;

  if ((line[i] != '\0') && (line[0] != ' ')) {
    token[0] = line;
    n++;
  }

  if (line[0] == '\0')
    return;
  

  while (true) {
    
    while ((line[i] != '\0') && (line[i] != ' '))
      i++;
    
    if (line[i] == '\0')
      return;

    while ((line[i] == ' ') && (line[i] != '\0'))
      i++;

    if (line[i] == '\0')
      return;
 
    token[n++] = line+i;
  }
}
  

template <class DataClass>
DataClass SMAParser<DataClass>::constructData()
{
  return DataClass(&mVertex[0],this->mFDim);
}




#endif
