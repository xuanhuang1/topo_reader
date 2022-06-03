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

#ifndef ANALYTICFUNCTIONS_H
#define ANALYTICFUNCTIONS_H

#include <math.h>
#include "EmbeddedVertex.h"

enum FunctionID {
  FRIEDMAN1 = 0,
};


template<uint32_t dim,typename DataType>
class AnalyticFunction 
{
public:
  
  AnalyticFunction() {}

  virtual ~AnalyticFunction() {}

  virtual DataType eval(const EmbeddedVertex<dim,DataType>& v) const = 0;
};


template<uint32_t dim,typename DataType>
class Friedman1 : public AnalyticFunction<dim,DataType>
{
public:
  DataType eval(const EmbeddedVertex<dim,DataType>& v) const;
};


template<uint32_t dim,typename DataType>
AnalyticFunction<dim,DataType>* analyticFunction(FunctionID t)
{
  switch (t) {

  case FRIEDMAN1:
    
    return new Friedman1<dim,DataType>();
    break;
  }

  return NULL;
} 


template<uint32_t dim,typename DataType>
DataType Friedman1<dim,DataType>::eval(const EmbeddedVertex<dim,DataType>& v) const
{
  DataType f = 0;

  if (dim > 1)
    f += 0.1*exp(4*v[0]);

  if (dim > 2)
    f += 4 / (1 + exp(-20*(v[1] - 0.5)));

  if (dim > 3)
    f += 3*v[2];

  if (dim > 4)
    f += 2*v[3];

  if (dim > 5)
    f += v[4];

  return f;
}

#endif


