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

#ifndef GRIDS_H
#define GRIDS_H

#include <vector>
#include "AnalyticFunctions.h"
#include "EmbeddedVertex.h"
#include "GradientComplexInterface.h"

template<class VertexClass>
void createUniformGrid(GradientComplexInterface<VertexClass>* complex, 
                       AnalyticFunction<VertexClass::sDimension, typename VertexClass::sDataType>* f,
                       const std::vector<uint32_t>& samples,
                       const std::vector<typename VertexClass::sDataType>& low,const std::vector<typename VertexClass::sDataType>& high)
{
  std::vector<typename VertexClass::sDataType> delta(low.size());
  uint32_t count = 0;
  uint32_t size = 1;
  uint16_t i;
  std::vector<uint32_t> slabs;
  VertexClass v;

  for (i=0;i<low.size();i++) {
    delta[i] = (high[i] - low[i]) / (samples[i]-1);
    v[i] = low[i];
  }

  for (i=0;i<samples.size();i++) 
    size *= samples[i];

  slabs.resize(samples.size());
  slabs[0] = 1;

  for (i=1;i<slabs.size();i++) 
    slabs[i] = slabs[i-1] * samples[i-1];

   while (count < size) {
     
     v[VertexClass::sDimension-1] = f->eval(v);
     complex->addVertex(count,v);
     
     //fprintf(stderr,"v %f %f %f %f\n",v[0],v[1],v[2],v[3]);

     count++;
     v[0] += delta[0];
     
     for (i=1;i<slabs.size();i++) {
       
       if (count % slabs[i] == 0) {
         v[i-1] = low[i-1];
         v[i] += delta[i];
       }
     }
  }
   
}


#endif

