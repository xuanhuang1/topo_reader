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

#include <pthread.h>
#include "EdgeConstructor.h"

void* thread_start(void* data)
{
  EdgeConstructor::ThreadInfoType* info;

  //fprintf(stderr,"thread_start\n");

  info = static_cast<EdgeConstructor::ThreadInfoType*>(data);

  (info->sInstance->*(info->sCallback))(info);

  return NULL;
}

EdgeConstructor* EdgeConstructor::ThreadInfoType::sInstance = NULL;
EdgeConstructor::MemberFunc EdgeConstructor::ThreadInfoType::sCallback = NULL;

EdgeConstructor::ThreadInfoType::ThreadInfoType(AssemblyInterface* slave, 
                                                std::vector<uint32_t> first, 
                                                std::vector<uint32_t> last) :
  mSlave(slave), mFirst(first), mLast(last)
{
}

EdgeConstructor::ThreadInfoType::ThreadInfoType(const ThreadInfoType& info) :
   mSlave(info.mSlave), mFirst(info.mFirst), mLast(info.mLast)
{
}


EdgeConstructor::ThreadInfoType& EdgeConstructor::ThreadInfoType::operator=(const ThreadInfoType& info)
{
  mSlave = info.mSlave;
  mFirst = info.mFirst;
  mLast = info.mLast;

  return *this;
}


EdgeConstructor::EdgeConstructor() : mNeighborhood(EDGE_COMPLETE), mDegree(0)
{
}


void EdgeConstructor::createEdges(EdgeType neigh,uint8_t degree, 
                                  std::vector<uint32_t> samples,
                                  std::vector<AssemblyInterface*>& slaves)
{
  mThreadInfo.clear();
  std::vector<uint32_t> start(samples.size(),0);
  std::vector<pthread_t> threads;
  uint32_t i;

  for (i=0;i<samples.size();i++)
    samples[i]--;

  distributeDomain(slaves,start,samples,samples.size()-1);
  mDegree = degree;
  mSlabs.resize(samples.size());
  
  mSlabs[0] = 1;
  for (i=1;i<mSlabs.size();i++) 
    mSlabs[i] = mSlabs[i-1]*(samples[i]+1);

  threads.resize(mThreadInfo.size());

  ThreadInfoType::sInstance = this;
  switch (neigh) {

  case EDGE_COMPLETE:

    ThreadInfoType::sCallback = &EdgeConstructor::createCompleteEdges;
    break;
   case EDGE_DISJUNCT:

     ThreadInfoType::sCallback = &EdgeConstructor::createDisjunctEdges;
    break;

  };

  for (i=1;i<threads.size();i++) 
    pthread_create(&threads[i],NULL,thread_start,&mThreadInfo[i]);
  
  thread_start(&mThreadInfo[0]);

  for (i=1;i<threads.size();i++) 
    pthread_join(threads[i],NULL);

}

uint32_t EdgeConstructor::volume(std::vector<uint32_t>& start, std::vector<uint32_t>& stop)
{
  uint32_t vol = 1;

  for (uint32_t i=0;i<start.size();i++)
    vol *= stop[i] - start[i];

  return vol;
}


void EdgeConstructor::distributeDomain(std::vector<AssemblyInterface*>& slaves, 
                                       std::vector<uint32_t> start, 
                                       std::vector<uint32_t> stop,
                                       uint16_t split)
{
  // If we have refined enough to only have a single slave left or if we cannot
  // refine the domain any further
  if ((slaves.size() == 1) || (volume(start,stop) == 1)) {

    // Create a thread for the first slave with the given piece of the domain
    mThreadInfo.push_back(ThreadInfoType(slaves[0],start,stop));
    return;
  }

  // Otherwise, we can refine the domain. We first look for the next coordinate
  // along which we can split (there should be at least one
  while ((stop[split] - start[split]) < 2)
    split = (split + 1) % stop.size();

  uint32_t tmp;
  std::vector<AssemblyInterface*> subslaves;

  tmp = stop[split];

  stop[split] = (start[split] + stop[split]) / 2;
  subslaves.insert(subslaves.end(), slaves.begin(),slaves.begin() + (slaves.size() / 2));  
  distributeDomain(subslaves,start,stop,(split+1) % stop.size());

  start[split] = stop[split];
  stop[split] = tmp;
  subslaves.clear();
  subslaves.insert(subslaves.end(), slaves.begin() + (slaves.size() / 2), slaves.end());
  distributeDomain(subslaves,start,stop,(split+1) % stop.size());
  
}

void EdgeConstructor::createCompleteEdges(ThreadInfoType* info)
{
  fprintf(stderr,"Creating edges for [%d,%d,%d] x [%d,%d,%d]\n",info->mFirst[0],info->mFirst[1],info->mFirst[2],
          info->mLast[0],info->mLast[1],info->mLast[2]);

  std::vector<uint32_t> current = info->mFirst;
  uint32_t index = 1;
  uint32_t i,j,k;
  uint32_t count = 0;

  while (current.back() <= info->mLast.back()) {
    
    // Compute the current index 
    index = 0;
    for (i=0;i<current.size();i++) 
      index += mSlabs[i] * current[i];
    
    // Add all the 1-degree edges unless we are at the boundary
    for (i=0;i<current.size();i++) {
      
      if (current[i] < info->mLast[i]) {
        info->mSlave->add_edge(index,index+mSlabs[i]);
        count++;
      }
    }

    if (mDegree > 1) {
      // Add the degree 2 edges

      for (i=0;i<current.size();i++) {

        if (current[i] < info->mLast[i]) {
          
          for (j=i+1;j<current.size();j++) {
            
            if (current[j] < info->mLast[j]) {
              
              info->mSlave->add_edge(index,index+mSlabs[i]+mSlabs[j]);
              info->mSlave->add_edge(index+mSlabs[j],index+mSlabs[i]);
              count += 2;
            }
          }
        }
      }
    }
    
    if (mDegree > 2) {
      // Add the degree 3 edges

      for (i=0;i<current.size();i++) {

        if (current[i] < info->mLast[i]) {
          
          for (j=i+1;j<current.size();j++) {
            
            if (current[j] < info->mLast[j]) {
              
              for (k=j+1;k<current.size();k++) {
   
                if (current[k] < info->mLast[k]) {
              
                  info->mSlave->add_edge(index,index+mSlabs[i]+mSlabs[j]+mSlabs[k]);
                  info->mSlave->add_edge(index+mSlabs[j],index+mSlabs[i]+mSlabs[k]);
                  info->mSlave->add_edge(index+mSlabs[i],index+mSlabs[j]+mSlabs[k]);
                  info->mSlave->add_edge(index+mSlabs[k],index+mSlabs[i]+mSlabs[j]);

                  count += 4;
                }
              }
            }
          }
        }
      }
    }
    
    
    // Increase the index
    index++;
    current[0]++;
    
    // If necessary wrap the indices to a new line / slab / whatever
    for (i=0;i<current.size()-1;i++) {
      
      if (current[i] > info->mLast[i]) {
        
        current[i] = info->mFirst[i];
        current[i+1]++;
      }
    }
  }

  fprintf(stderr,"Created %d edges \n",count);
  //sleep(10);
}
        


void EdgeConstructor::createDisjunctEdges(ThreadInfoType* info)
{
}


