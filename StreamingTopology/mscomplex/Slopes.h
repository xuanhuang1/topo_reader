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

#ifndef SLOPES_H
#define SLOPES_H

#include <math.h>
#include "EmbeddedVertex.h"

template <uint16_t dim=3, typename DataType=float>
class Slope
{
public:

  virtual ~Slope() {}
  
  virtual double operator()(const EmbeddedVertex<dim,DataType>& u, const EmbeddedVertex<dim,DataType>& v) const = 0;
  
};

template <uint16_t dim=3, typename DataType=float>
class EuclidianGradient : public Slope<dim,DataType>
{
public:

  double operator()(const EmbeddedVertex<dim,DataType>& u, const EmbeddedVertex<dim,DataType>& v) const
  {
    double mag = 0;
    uint16_t i;
    
    for (i=0;i<u.sFunctionDimension;i++) 
      mag += (v[i] - u[i]) * (v[i] - u[i]);

    i++;
    for(;i<u.sDimension;i++)
      mag += (v[i] - u[i]) * (v[i] - u[i]);
     
    mag = sqrt(mag);
    
    return (v.f() - u.f()) / mag;
  }
};


#endif

