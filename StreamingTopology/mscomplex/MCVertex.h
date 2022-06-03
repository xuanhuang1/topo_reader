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

#ifndef MCVERTEX_H
#define MCVERTEX_H

#include "EmbeddedVertex.h"
#include "GradientInfo.h"

template <uint16_t dim=3, typename DataType=float>
class MCVertex : public EmbeddedVertex<dim,DataType>
{
public:

  //! Default constructor
  MCVertex() : EmbeddedVertex<dim,DataType>() {}

  //! Upcast constructor
  MCVertex(const EmbeddedVertex<dim,DataType>& v) : EmbeddedVertex<dim,DataType>(v) {}

  //! Copy Constructor
  MCVertex(const MCVertex& v) : EmbeddedVertex<dim,DataType>(v), mStableInfo(v.mStableInfo) {}

  //! Constructor to initialize all coordinates
  //MCVertex(const DataType* data) : EmbeddedVertex<dim,DataType>(data) {}
  
  //! Destructor
  ~MCVertex() {}

  GradientInfo& stableInfo() {return mStableInfo;}
  
  const GradientInfo& stableInfo() const {return mStableInfo;}
  
private:

  //! The current stable manifold information
  GradientInfo mStableInfo;
};

#endif

