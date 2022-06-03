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


#include "Node.h"

bool nodeCmp(const Node* u, const Node* v)
{
  return (*u) < (*v);
}


bool Node::operator<(const Node& n) const
{
  if (this->f() < n.f())
    return true;
  else if ((this->f() == n.f()) && (this->id() < n.id()))
    return true;

  return false;
}

bool Node::operator>(const Node& n) const
{
  if (this->f() > n.f())
    return true;
  else if ((this->f() == n.f()) && (this->id() > n.id()))
    return true;

  return false;
}

TreeType Node::type() const
{
  if ((mUp.size() == 0) && (mDown.size() == 0))
    return ROOT;

  if ((mUp.size() + mDown.size()) == 1)
    return LEAF;

  if ((mUp.size() == 1) && (mDown.size() == 1))
    return INTERIOR;

  return BRANCH;
}

MorseType Node::morseType() const
{
  if ((mUp.size() == 0) && (mDown.size() == 0))
    return ISOLATED;

  if ((mUp.size() == 0) && (mDown.size() == 1))
    return MAXIMUM;

  if ((mUp.size() == 1) && (mDown.size() == 0))
    return MINIMUM;

  if ((mUp.size() == 1) && (mDown.size() > 1))
    return SPLIT_SADDLE;

  if ((mUp.size() > 1) && (mDown.size() == 1))
    return MERGE_SADDLE;

  if ((mUp.size() == 1) && (mDown.size() == 1))
    return REGULAR;


  return MULTI_SADDLE;
}


int Node::removeUp(Node* n)
{
  vector<Node* >::iterator it;

  for (it=mUp.begin();it!=mUp.end();it++) {
    if (*it == n) {
      iter_swap(it,mUp.end()-1);
      mUp.pop_back();
      return 1;
    }
  }

  stwarning("Could not find up pointer to remove.");
  return 0;
}

int Node::removeDown(Node* n)
{
  vector<Node*>::iterator it;

  for (it=mDown.begin();it!=mDown.end();it++) {
    if (*it == n) {
      iter_swap(it,mDown.end()-1);
      mDown.pop_back();
      return 1;
    }
  }

  stwarning("Could not find down pointer to remove.");
  return 0;
}

int Node::bypass()
{
  sterror(type() != INTERIOR,"Only regular nodes can be bypassed.");

  if (mUp[0]->removeDown(this) == 0) {
    stwarning("Arc structure inconsistent. Could not find pointer.");
    return 0;
  }
  mUp[0]->addDown(mDown[0]);

  if (mDown[0]->removeUp(this) == 0) {
    stwarning("Arc structure inconsistent. Could not find pointer.");
    return 0;
  }
  mDown[0]->addUp(mUp[0]);

  mUp.clear();
  mDown.clear();

  return 1;
}

void Node::toFile(FILE* output, const STMappedArray<Node >& graph) const
{
  uint16_t size;
  LocalIndexType index;
  std::vector<Node * >::const_iterator it;

  Vertex::saveBinary(output);

  size = mUp.size();
  fwrite(&size,sizeof(uint16_t),1,output);

  for (it=mUp.begin();it!=mUp.end();it++) {
    index = graph.findElementIndex((*it)->id());
    fwrite(&index,sizeof(LocalIndexType),1,output);
  }

  size = mDown.size();
  fwrite(&size,sizeof(uint16_t),1,output);

  for (it=mDown.begin();it!=mDown.end();it++) {
    index = graph.findElementIndex((*it)->id());
    fwrite(&index,sizeof(LocalIndexType),1,output);
  }

  fwrite(&mPersistence,sizeof(float),1,output);

  if (mParent == NULL)
    index = LNULL;
  else
    index = graph.findElementIndex(mParent->id());

  fwrite(&index,sizeof(LocalIndexType),1,output);
}

void Node::saveASCII(FILE* output, const std::map<GlobalIndexType,LocalIndexType>& index_map) const
{
  std::vector<Node*>::const_iterator it;
  std::map<GlobalIndexType,LocalIndexType>::const_iterator mIt;

  Vertex::saveASCII(output);

  fprintf(output,"%f\n",mPersistence);

  if (mParent == NULL)
    fprintf(output,"-1\n");
  else {
    mIt = index_map.find(mParent->id());
    sterror(mIt==index_map.end(),"Parent index %d not found in index map.",mParent->id());

    fprintf(output,"%d\n",mIt->second);
  }

  fprintf(output,"%d\n",(uint32_t)mUp.size());

  for (it=mUp.begin();it!=mUp.end();it++) {
    mIt = index_map.find((*it)->id());
    sterror(mIt==index_map.end(),"Up index not found in index map.");

    fprintf(output,"%d ",mIt->second);
  }
  fprintf(output,"\n");


  fprintf(output,"%d\n",(uint32_t)mDown.size());

  for (it=mDown.begin();it!=mDown.end();it++) {
    mIt = index_map.find((*it)->id());
    sterror(mIt==index_map.end(),"Down index not found in index map.");

    fprintf(output,"%d ",mIt->second);
  }
  fprintf(output,"\n");

}

void Node::fromFile(FILE* input, STMappedArray<Node >& graph)
{
  uint16_t size;
  LocalIndexType index;
  std::vector<Node*>::iterator it;

  Vertex::loadBinary(input);

  fread(&size,sizeof(uint16_t),1,input);
  mUp.resize(size);

  for (it=mUp.begin();it!=mUp.end();it++) {
    fread(&index,sizeof(LocalIndexType),1,input);
    *it = &graph[index];
  }

  fread(&size,sizeof(uint16_t),1,input);
  mDown.resize(size);

  for (it=mDown.begin();it!=mDown.end();it++) {
    fread(&index,sizeof(LocalIndexType),1,input);
    *it = &graph[index];
  }


  fread(&mPersistence,sizeof(float),1,input);

  fread(&index,sizeof(LocalIndexType),1,input);

  if (index == LNULL)
    mParent = NULL;
  else
    mParent = &graph[index];

}

