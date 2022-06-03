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


#include "Vertex.h"

char Vertex::sStr[50];

bool Vertex::smaller(const Vertex* v0, const Vertex* v1)
{
  if (v0->f() < v1->f())
    return true;

  if ((v0->f() == v1->f()) && (v0->id() < v1->id()))
    return true;

  return false;
}

bool Vertex::greater(const Vertex* v0, const Vertex* v1)
{
  if (v0->f() > v1->f())
    return true;

  if ((v0->f() == v1->f()) && (v0->id() > v1->id()))
    return true;

  return false;
}

Vertex::Vertex(GlobalIndexType id, const FunctionType f)
  : FlexArray::MappedElement<GlobalIndexType,LocalIndexType>(id), mFlags(LEAF), mFunc(f)
{
}

Vertex& Vertex::operator=(const Vertex& v)
{
  mFlags = v.mFlags;
  mFunc = v.mFunc;

  *static_cast<FlexArray::MappedElement<GlobalIndexType,LocalIndexType>*>(this) = static_cast<FlexArray::MappedElement<GlobalIndexType,LocalIndexType> >(v);

  return *this;
}


void Vertex::saveBinary(FILE* output) const
{
  fwrite(&mFlags,sizeof(uint8_t),1,output);
  fwrite(&mFunc,sizeof(FunctionType),1,output);
}

void  Vertex::saveASCII(FILE* output) const
{
  fprintf(output,"%d %e %d\n",this->id(),mFunc,mFlags);
}

void Vertex::loadBinary(FILE* input)
{
  fread(&mFlags,sizeof(uint8_t),1,input);
  fread(&mFunc,sizeof(FunctionType),1,input);
}

