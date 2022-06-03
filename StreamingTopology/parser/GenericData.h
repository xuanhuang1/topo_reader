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


#ifndef GENERICDATA_H
#define GENERICDATA_H

#include <cstdio>

//! Minimal data class
/*! Generic data implements the smallest possible vertex data type
 *  storing only a single function value. 
 */
template <typename DataTypeName>
class GenericData
{
public:
  
  typedef DataTypeName FunctionType;

  static GenericData interpolate(const GenericData& d0,const GenericData& d1, double t);

  GenericData(DataTypeName f=0) : mFunction(f) {}
  GenericData(const GenericData& data) {*this = data;}
  GenericData(const DataTypeName* coord, int f_dim) : mFunction(coord[f_dim]) {} 
  ~GenericData() {}

  GenericData& operator=(const GenericData& v) {mFunction = v.mFunction;return *this;}
  
  DataTypeName f() const {return mFunction;}
 
  void f(DataTypeName ff) {mFunction = ff;}

  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/
  
  //! Write the node in binary format to the given stream
  void toFile(FILE* output) const;

  void toASCII(FILE* output) const;

  //! Read the node in binary format from the given stream
  void fromFile(FILE* input);

private:

  DataTypeName mFunction;
};
 

typedef GenericData<float> FloatData;

template<typename DataTypeName>
GenericData<DataTypeName> GenericData<DataTypeName>::interpolate(const GenericData& d0,const GenericData& d1, double t)
{
  GenericData<DataTypeName> d;

  d.mFunction = (1-t)*d0.f() + t*d1.f();

  return d;
}

template<typename DataTypeName>
void GenericData<DataTypeName>::toFile(FILE* output) const
{
  fwrite(&mFunction,sizeof(DataTypeName),1,output);
};

template<typename DataTypeName>
void GenericData<DataTypeName>::toASCII(FILE* output) const
{
  fprintf(output,"%f",(double)mFunction);
}

template<typename DataTypeName>
void GenericData<DataTypeName>::fromFile(FILE* input)
{
  fwrite(&mFunction,sizeof(DataTypeName),1,input);
};





#endif
