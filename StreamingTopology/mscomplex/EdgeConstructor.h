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

#ifndef EDGECONSTRUCTOR_H
#define EDGECONSTRUCTOR_H

#include <vector>
#include "Definitions.h"
#include "MSCVertex.h"
#include "AssemblyInterface.h"

enum EdgeType {
  EDGE_COMPLETE = 0,
  EDGE_DISJUNCT = 1,
};


class EdgeConstructor
{
public:

  class ThreadInfoType;

  typedef void (EdgeConstructor::*MemberFunc)(ThreadInfoType *info);
  
  class ThreadInfoType {
  public:

    static MemberFunc sCallback;
    static EdgeConstructor* sInstance;

    ThreadInfoType(AssemblyInterface* slave, std::vector<uint32_t> first, 
                   std::vector<uint32_t> last);

    ThreadInfoType(const ThreadInfoType& info);
    
    ~ThreadInfoType() {}

    ThreadInfoType& operator=(const ThreadInfoType& info);

    AssemblyInterface* mSlave;

    std::vector<uint32_t> mFirst;
    std::vector<uint32_t> mLast;
  };

  EdgeConstructor();

  virtual ~EdgeConstructor() {}

  virtual void createEdges(EdgeType neigh,uint8_t degree, 
                           std::vector<uint32_t> samples,
                           std::vector<AssemblyInterface*>& slaves);

private:
  
  std::vector<uint32_t> mSlabs;
 
  std::vector<ThreadInfoType> mThreadInfo;

  EdgeType mNeighborhood;

  uint8_t mDegree;

  uint32_t volume(std::vector<uint32_t>& start, std::vector<uint32_t>& stop);

  virtual void distributeDomain(std::vector<AssemblyInterface*>& slaves, std::vector<uint32_t> start, std::vector<uint32_t> stop, uint16_t split); 

  virtual void createCompleteEdges(ThreadInfoType* info);

  virtual void createDisjunctEdges(ThreadInfoType* info);
};

#endif
