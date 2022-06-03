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


#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cstdio>
#include "TalassConfig.h"


static const uint8_t TOPOTREE_BLOCK_BITS = 19;

static const uint8_t TOPOGRAPH_BLOCK_BITS = 18;

static const uint8_t SEGMENTATION_BLOCK_BITS = 24;


static const FunctionType gMaxValue = 10e34;
static const FunctionType gMinValue = -10e34;

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))


#ifdef ST_COUNT_RETIRED

static int64_t gRetiredVertexCount = 0;

#define ST_RETIRED_COUNT_INC() gRetiredVertexCount++;
#define ST_RETIRED_COUNT_DEC() gRetiredVertexCount--;
#define printRetired() fprintf(stderr,"STATUS: There are %5d vertices finalized but not yet retired.\n");

#else

#define ST_RETIRED_COUNT_INC() ;
#define ST_RETIRED_COUNT_DEC() ;
#define printRetired() ;

#endif


#endif
