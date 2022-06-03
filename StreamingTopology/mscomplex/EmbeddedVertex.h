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

#ifndef EMBEDDEDVERTEX_H
#define EMBEDDEDVERTEX_H

#include <string.h>
#include <cstdio>
#include "TalassConfig.h"

template <uint32_t dim=3, typename DataType=float>
class EmbeddedVertex {

public:
	static const uint32_t DATA_SIZE = dim * sizeof(DataType);
 
  static const uint32_t sDimension = dim;
	
  static uint32_t sFunctionDimension; 

  typedef DataType sDataType;
  
  typedef DataType FunctionType;

  static void functionDimension(uint32_t d) {sFunctionDimension = d;}

	EmbeddedVertex() {}
	EmbeddedVertex(const EmbeddedVertex &v) {memcpy(data,v.data,DATA_SIZE);}

	//! Constructor to only initialize the function but not the other coordinates
	EmbeddedVertex(const DataType *f, uint32_t f_dim) {memcpy(data,f,DATA_SIZE);}
  
  DataType operator[](int i) const {return data[i];}
  DataType &operator[](int i) {return data[i];}
	inline DataType getData(int i) const {return data[i];}

  DataType f() const {return data[sFunctionDimension];}
  void f(DataType v) {data[sFunctionDimension] = v;}

	inline void getData(DataType *f) {memcpy(f,data,DATA_SIZE);}
	inline float *getData() {return data;}	

	inline void setData(int i, DataType f) {data[i] = f;}
	inline void setData(DataType *f) {memcpy(data,f,DATA_SIZE);}

  //! Write the node in binary format to the given stream
  void toFile(FILE* output) const {fwrite(data,sizeof(DataType),dim,output);}

  //! Read the node in binary format from the given stream
  void fromFile(FILE* input) {fread(data,sizeof(DataType),dim,input);}

  

	DataType data[dim];
};

template <uint32_t dim, typename DataType>
uint32_t EmbeddedVertex<dim,DataType>::sFunctionDimension = dim -1;

#endif
