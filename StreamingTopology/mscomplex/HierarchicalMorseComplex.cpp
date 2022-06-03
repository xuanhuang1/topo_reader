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

#include "HierarchicalMorseComplex.h"


int HierarchicalMorseComplex::constructHierarchy(float persistence, HierarchyMode mode, int direction)
{
  std::priority_queue<Cancellation,std::vector<Cancellation>,CancellationCmp> queue;
  HierarchicalMorseComplex::iterator nIt;
  Cancellation top;
  Substitution sub;
  NodeType* tmp;
  uint16_t i;

  // First we iterate through all nodes and collect the saddle extremum arcs as
  // potential cancellations
  for (nIt=this->begin();nIt!=this->end();nIt++) {

    //if (nIt->id() == 32887)
    //  fprintf(stderr,"break\n");

    if (nIt->morseType() == MERGE_SADDLE) {
      top.a.u = nIt;

      // For each of u's neighbors add an arc
      for (i=0;i<nIt->neighbors().size();i++) {
        top.a.v = nIt->neighbors()[i];
        top.p = direction*(top.a.v->f()-top.a.u->f()) / (this->maxF()-this->minF());
        queue.push(top);
      }
    }
  }

  while (!queue.empty()) {

    top = queue.top();
    queue.pop();

    // Note that top can potentially contain pointers to nodes that
    // have been removed. However, potentially the location has been
    // removed so it is not straight forward to determine whether or
    // not a pointer is still valid. Here we do it by checking whether
    // the global id currently stored matches the one the index map
    // has stored.
    tmp = this->findElement(top.a.u->id());
    if (tmp != top.a.u)
      continue;

    tmp = this->findElement(top.a.v->id());
    if (tmp != top.a.v)
      continue;

    //If one of the critical points has already been cancelled
    if ((top.a.u->persistence() < gMaxValue) || (top.a.v->persistence() < gMaxValue))
      continue;

    // If the persistence of the next smallest cancellation is larger
    // than the given threshold
    if (top.p > persistence)
      break;


    // Otherwise top represents the cancellation with smallest
    // "persistence"
    sub = cancelBranch(top,mode);

    //if (sub.p > -2)
    //  fprintf(stderr,"Cancelled %d %d   %f\n",top.a.u->id(),top.a.v->id(),top.p);


    // We need to add all incoming arcs to the queue
    for (i=0;i<sub.incoming.size();i++) {

      top.a.u = sub.incoming[i].u;
      top.a.v = sub.incoming[i].v;
      top.p = direction*(top.a.v->f()-top.a.u->f()) / (this->maxF()-this->minF());
      queue.push(top);
    }
  }

  if (mode != RECOVERABLE) {
    mLevel = 0;
    mPersistence = -1;
  }
  else {
    mLevel = mHierarchy.size();
    mPersistence = gMaxValue;
  }

  updatePersistenceLevel(0);

  //return 1;
  // Finally, we remove all the additional copies of the multi-saddles
  // that we may have hanging around. We do this my first marking all
  // such copies as ISOLATED, then go through all cancellations and
  // remove all arcs that mentions these nodes

  std::map<GlobalIndexType,std::set<NodeType*> >::iterator mIt;
  std::set<NodeType*>::iterator sIt;

  for (mIt=mMultiSaddles.begin();mIt!=mMultiSaddles.end();mIt++) {
    // For each multi saddle find the guy with the largest extended persistence
    // that has not been cancelled

    float p,max_p = -1;
    NodeType* extra = NULL;

    for (sIt=mIt->second.begin();sIt!=mIt->second.end();sIt++) {
      if ((*sIt)->parent() == NULL) {
        p = getExtendedPersistence(*sIt);
        if (p > max_p) {
          extra = *sIt;
          max_p = p;
        }
      }
    }

    if (extra != NULL)
      extra->morseType(ISOLATED);
  }

  std::vector<Arc>::iterator vIt;
  // Go through the hierarchy and remove all arcs to newly isolated nodes
  for (uint32_t i=0;i<mHierarchy.size();i++) {

    vIt = mHierarchy[i].incoming.begin();
    while (vIt != mHierarchy[i].incoming.end()) {
      if ((vIt->u->morseType() == ISOLATED) || (vIt->v->morseType() == ISOLATED)) {
        *vIt = mHierarchy[i].incoming.back();
        mHierarchy[i].incoming.pop_back();
      }
      else
        vIt++;
    }

    vIt = mHierarchy[i].outgoing.begin();
    while (vIt != mHierarchy[i].outgoing.end()) {
      if ((vIt->u->morseType() == ISOLATED) || (vIt->v->morseType() == ISOLATED)) {
        *vIt = mHierarchy[i].outgoing.back();
        mHierarchy[i].outgoing.pop_back();
      }
      else
        vIt++;
    }
  }

  std::vector<NodeType*>::iterator it;
  // Finally, we remove the currently active arcs and the nodes themselfs
  for (nIt=this->begin();nIt!=this->end();nIt++) {
    if (nIt->morseType() == ISOLATED) {
      for (it=nIt->neighbors().begin();it!=nIt->neighbors().end();it++)
        (*it)->removeNeighbor(nIt);

      nIt->removeNeighbors();
      removeNode((NodeType*)nIt);
    }
  }



  return 1;
}


void HierarchicalMorseComplex::addMultiSaddle(NodeType* node)
{
  if (mMultiSaddles.find(node->data().meshIndex()) == mMultiSaddles.end())
    mMultiSaddles[node->data().meshIndex()] = std::set<NodeType*>();

  mMultiSaddles[node->data().meshIndex()].insert(node);
}


HierarchicalMorseComplex::Substitution HierarchicalMorseComplex::cancelBranch(const Cancellation& can,
                                                                              HierarchyMode mode)
{
  Substitution sub;
  std::vector<NodeType*> neighbors;
  std::vector<NodeType*>::iterator it;
  NodeType *parent = NULL;

  sub.extremum = can.a.v;
  sub.saddle = can.a.u;
  sub.p = can.p;

  if (sub.saddle->valence() == 2)  {//Standard case valence 2 saddle
    parent = sub.saddle->neighbors()[0];
    if (parent == sub.extremum)
      parent = sub.saddle->neighbors()[1];
  }
  else
    sterror(true,"Ambiguous configuration saddle %d with valence %d.",sub.saddle->id(),sub.saddle->valence());

  if (sub.extremum == parent) {// If we have found a loop
    sub.p = -2; // Indicate that this cancellation is invalid
    return sub; // We return an empty substitution
  }
  this->removeArc(sub.saddle,sub.extremum);
  sub.outgoing.push_back(Arc(sub.saddle,sub.extremum));

  if (sub.saddle->valence() == 1) { //Standard case valence 2 saddle

    parent = sub.saddle->neighbors()[0];
    sub.outgoing.push_back(Arc(sub.saddle,sub.saddle->neighbors()[0]));
    this->removeArc(sub.saddle,sub.saddle->neighbors()[0]);

    if (mode == DESTRUCTIVE)
      this->removeNode(sub.saddle);
    else {
      sub.saddle->parent(parent);
      sub.saddle->persistence(can.p);
      sub.saddle->active(false);
    }
  }
  else { // Multi-saddle
    //sterror(sub.saddle->valence() > 2,"Ambiguous configuration saddle %d with valence %d.",sub.saddle->id(),sub.saddle->valence());

    parent = sub.saddle->neighbors()[0];
  }

  sub.extremum->parent(parent);
  sub.extremum->persistence(can.p);
  sub.extremum->active(false);

  sterror(sub.extremum == parent,"Node %d cannot be its own parent.",sub.extremum->id());

  neighbors = sub.extremum->neighbors();
  for (it=neighbors.begin();it!=neighbors.end();it++) {
    this->removeArc(sub.extremum,*it);
    sub.outgoing.push_back(Arc(sub.extremum,*it));

    this->addArc(*it,parent);
    sub.incoming.push_back(Arc(*it,parent));
  }

  if (mode == DESTRUCTIVE)
    this->removeNode(sub.extremum);


  if (mode == RECOVERABLE)
    mHierarchy.push_back(sub);



  return sub;
}


void HierarchicalMorseComplex::updatePersistenceLevel(int level)
{
  uint32_t effective;

  effective = MAX(MIN(level,(int)mHierarchy.size()),0);

  while ((mLevel > 0) && (mLevel > effective)) {
    mLevel--;
    refineGraph(mLevel);

    if (mLevel == 0)
      mPersistence = MIN(mHierarchy[mLevel].p-1,0);
    else
      mPersistence = mHierarchy[mLevel-1].p;
  }

  while ((mLevel < mHierarchy.size()) && (mLevel < effective)) {
    coarsenGraph(mLevel);
    mPersistence = mHierarchy[mLevel].p;

    mLevel++;
  }
}


void HierarchicalMorseComplex::updatePersistence(float p)
{
  uint32_t level = mLevel;


  while ((level > 0) && (mHierarchy[level].p > p))
    level--;

  while ((level < mHierarchy.size()) && (mHierarchy[level].p <= p))
    level++;

  //fprintf(stderr,"MultiResGraph::updatePersistence   %f  to level %d\n",p,level);

  updatePersistenceLevel(level);
}


HierarchicalMorseComplex::NodeType* HierarchicalMorseComplex::findActiveNode(GlobalIndexType index)
{
  NodeType* n;

  n = this->findElement(index);

  sterror(n==NULL,"Node index %d not found MultiResGraph inconsistent.",index);

  while ((n != NULL) && (!n->isActive()))
    n = n->parent();

  return n;
}



void HierarchicalMorseComplex::outputExtremaVsPersistence(FILE* output, const char* label)
{
  uint16_t i;

  fprintf(output,"0.0 %llu\n",mHierarchy.size() + 1);

  for (i=0;i<mHierarchy.size();i++)
    fprintf(output,"%0.5f %llu\n",mHierarchy[i].p,mHierarchy.size() - i);

  if (mHierarchy.size() > 0) {
    if (mHierarchy.back().p < 0.5)
      fprintf(output,"0.5 %d\n",1);
  }
  else
    fprintf(output,"0.5 1\n");


  if (label != NULL)
    fprintf(output,"@ s_ legend \"%s\"\n",label);
  fprintf(output,"@ s_ line type 3\n");

}

void HierarchicalMorseComplex::outputCancellationTree(FILE* output,bool ascii)
{
  uint32_t count = 0;
  Graph::iterator nIt;
  std::map<LocalIndexType,uint32_t> index_map;
  std::map<LocalIndexType,uint32_t>::iterator mIt,mIt2;
  NodeType* saddle;
  uint32_t cycle_count = 0;

  //fprintf(stderr,"Running HierarchicalMorseComplex::outputCancellationTree\n");

  // Find and count all critical points
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    index_map[nIt->id()] = count++;

    if ((nIt->morseType() == MERGE_SADDLE) && (nIt->parent() == NULL))
      cycle_count++;

  }

  // Output all nodes their parents
  fprintf(output,"# nodes %d\n",count);
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->parent() != NULL) {
      mIt = index_map.find(nIt->parent()->id());
      sterror(mIt==index_map.end(),"Parent index %d not found in index map",nIt->parent()->id());
      fprintf(output,"%u %f %u\n",nIt->data().meshIndex(),nIt->persistence(),mIt->second);
    }
    else
      fprintf(output,"%u %f %u\n",nIt->data().meshIndex(),nIt->persistence(),GNULL);
  }


  // Write out the number of arcs which is two for each saddle
  fprintf(output,"# arcs %d\n",(int)(mHierarchy.size()));

  for (uint32_t i=0;i<mHierarchy.size();i++) {
    saddle = mHierarchy[i].saddle;

    mIt2 = index_map.find(saddle->id());
    sterror(mIt2==index_map.end(),"Saddle index %u not found in index map",saddle->id());

    sterror(saddle->valence()!=2,"A saddle in a Morse complex must have valence two");

    mIt = index_map.find(saddle->neighbors()[0]->id());
    sterror(mIt==index_map.end(),"Neighbor index %d not found in index map",saddle->neighbors()[0]->id());
    fprintf(output,"%u %u",mIt2->second,mIt->second);

    mIt = index_map.find(saddle->neighbors()[1]->id());
    sterror(mIt==index_map.end(),"Neighbor index %d not found in index map",saddle->neighbors()[1]->id());
    fprintf(output," %u\n",mIt->second);
  }

  // There should be geometry for all arcs
  fprintf(output,"# paths %d\n",(int)(2*mHierarchy.size()));

  // Finally, we output the path geometry in form of <start-node> <stop-node> <v0> .... <vn>
  for (uint32_t i=0;i<mGeometry.size();i++) {

    NodeType* saddle = this->findElement(mGeometry[i][0]);

    // Initially we only output the paths that belong to saddles that were cancelled
    if ((saddle != NULL) && (saddle->parent() != NULL)) {


      // The first index in the path is the saddle id
      mIt = index_map.find(mGeometry[i][0]);

      // Only the saddles that were part of cancellations should be output
      if (mIt != index_map.end()) {

        mIt2 = index_map.find(mGeometry[i].back());
        sterror(mIt==index_map.end(),"End of path %d is \"%u\" which was not found in the index map",i,mGeometry[i].back());

        // Output the node indices of the start and end node
        fprintf(output,"%u %u",mIt->second,mIt2->second);

        // Ignore the first vertex of the path since it represented the saddle
        // id rather than geometry
        for (std::vector<LocalIndexType>::iterator it=mGeometry[i].begin()+1;it!=mGeometry[i].end();it++) {
          fprintf(output," %d",*it);
        }
        fprintf(output,"\n");
      }
    }
  }

  // Finally, we write out the remaining cycles
  fprintf(output,"# cycles %d\n",cycle_count);
  for (nIt = this->begin(); nIt != this->end(); nIt++) {

    if ((nIt->morseType() == MERGE_SADDLE) && (nIt->parent() == NULL)) {
      mIt2 = index_map.find(nIt->id());
      sterror(mIt2==index_map.end(),"Saddle index %u not found in index map",nIt->id());

      sterror(nIt->valence()!=2,"A saddle in a Morse complex must have valence two");

      mIt = index_map.find(nIt->neighbors()[0]->id());
      sterror(mIt==index_map.end(),"Neighbor index %d not found in index map",nIt->neighbors()[0]->id());
      fprintf(output,"%u %u",mIt2->second,mIt->second);

      mIt = index_map.find(nIt->neighbors()[1]->id());
      sterror(mIt==index_map.end(),"Neighbor index %d not found in index map",nIt->neighbors()[1]->id());
      fprintf(output," %u \n",mIt->second);
    }
  }

  fprintf(output,"# cycle_path %d\n",2*cycle_count);
  for (uint32_t i=0;i<mGeometry.size();i++) {

    NodeType* saddle = this->findElement(mGeometry[i][0]);

    // Now output the paths we previously ignored
    if ((saddle != NULL) && (saddle->parent() == NULL)) {

      // The first index in the path is the saddle id
      mIt = index_map.find(mGeometry[i][0]);

      // Only the saddles that were part of cancellations should be output
      if (mIt != index_map.end()) {

        mIt2 = index_map.find(mGeometry[i].back());
        sterror(mIt==index_map.end(),"End of path %d is \"%u\" which was not found in the index map",i,mGeometry[i].back());

        // Output the node indices of the start and end node
        fprintf(output,"%u %u",mIt->second,mIt2->second);

        // Ignore the first vertex of the path since it represented the saddle
        // id rather than geometry
        for (std::vector<LocalIndexType>::iterator it=mGeometry[i].begin()+1;it!=mGeometry[i].end();it++) {
          fprintf(output," %d",*it);
        }
        fprintf(output,"\n");
      }
    }
  }
}


void HierarchicalMorseComplex::outputComplex(FILE* output)
{
  uint32_t count = 0;
  uint32_t max_count;
  Graph::iterator nIt;
  std::map<LocalIndexType,uint32_t> index_map;
  std::map<LocalIndexType,uint32_t>::iterator mIt,mIt2;

  //fprintf(stderr,"Running HierarchicalMorseComplex::outputCancellationTree\n");

  // Find and enter all maxima into the index map
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() != MERGE_SADDLE)
      index_map[nIt->id()] = count++;
  }
  fprintf(output,"%u\n",count);
  max_count = count;

  // Find and enter all saddles
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() == MERGE_SADDLE)
      index_map[nIt->id()] = count++;
  }
  fprintf(output,"%u\n",count-max_count);

  // Output the maxima
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() != MERGE_SADDLE) {
      //fprintf(output,"%u %e\n",nIt->id(),nIt->f());
      fprintf(output,"%u %e\n",nIt->data().meshIndex(),nIt->f());
    }
  }

  // Output the saddles
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() == MERGE_SADDLE) {
      //fprintf(output,"%u %e\n",nIt->id(),nIt->f());
      fprintf(output,"%u %e\n",nIt->data().meshIndex(),nIt->f());
    }
  }

  // For all the saddles
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() == MERGE_SADDLE) {

      sterror(nIt->valence()!=2,"A saddle in a Morse complex must have valence two");

      mIt = index_map.find(nIt->neighbors()[0]->id());
      sterror(mIt==index_map.end(),"Neighbor index %d not found in index map",nIt->neighbors()[0]->id());

      mIt2 = index_map.find(nIt->neighbors()[1]->id());
      sterror(mIt2==index_map.end(),"Neighbor index %d not found in index map",nIt->neighbors()[1]->id());

      fprintf(output,"%u %u\n",mIt->second,mIt2->second);
    }
  }

  // For all arcs
  for (uint32_t i=0;i<mGeometry.size();i++) {
    fprintf(output,"%d",mGeometry[i][1]);
    for (uint32_t k=2;k<mGeometry[i].size();k++)
      fprintf(output," %d",mGeometry[i][k]);
    fprintf(output,"\n");
  }

}

void HierarchicalMorseComplex::outputLabelHierarchy(FILE* output)
{
  uint32_t count = 0;
  Graph::iterator nIt;
  std::map<LocalIndexType,uint32_t> index_map;
  std::map<LocalIndexType,uint32_t>::iterator mIt;

  //fprintf(stderr,"Running HierarchicalMorseComplex::outputCancellationTree\n");

  // Find and enter all maxima into the index map
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() != MERGE_SADDLE)
      index_map[nIt->id()] = count++;
  }
  // Go through all maxima again and output their mesh index,
  // persistence and parent
  for (nIt = this->begin(); nIt != this->end(); nIt++) {
    if (nIt->morseType() != MERGE_SADDLE) {

      fprintf(output,"%d %e",nIt->id(),nIt->persistence());

      if (nIt->parent() == NULL) {
        mIt = index_map.find(nIt->id());
        sterror(mIt==index_map.end(),"Parent node must be in the index map");
        fprintf(output," %d\n",mIt->second);
      }
      else {
        mIt = index_map.find(nIt->parent()->id());
        sterror(mIt==index_map.end(),"Parent node must be in the index map");
        fprintf(output," %d\n",mIt->second);
      }

    }
  }

}



void HierarchicalMorseComplex::refineGraph(int level)
{
  Substitution& sub = mHierarchy[level];
  std::vector<Arc>::iterator it;

  sterror(this->findElement(sub.saddle->id())!=sub.saddle,"Id mismatch multi-resolution graph inconsistent.");
  sterror(this->findElement(sub.extremum->id())!=sub.extremum,"Id mismatch multi-resolution graph inconsistent.");
  // sterror(sub.extremum->type()!=ROOT,"Higher resolution nodes should be trivial.");

  sub.saddle->active(true);
  sub.extremum->active(true);

  for (it=sub.incoming.begin();it!=sub.incoming.end();it++)
    this->removeArc(it->u,it->v);

  for (it=sub.outgoing.begin();it!=sub.outgoing.end();it++)
    this->addArc(it->u,it->v);
}


void HierarchicalMorseComplex::coarsenGraph(int level)
{
  Substitution& sub = mHierarchy[level];
  std::vector<Arc>::iterator it;

  sterror(this->findElement(sub.saddle->id())!=sub.saddle,"Id mismatch multi-resolution graph inconsistent.");
  sterror(this->findElement(sub.extremum->id())!=sub.extremum,"Id mismatch multi-resolution graph inconsistent.");

  sub.extremum->active(false);

  for (it=sub.outgoing.begin();it!=sub.outgoing.end();it++)
    this->removeArc(it->u,it->v);

  for (it=sub.incoming.begin();it!=sub.incoming.end();it++)
    this->addArc(it->u,it->v);

  if (sub.saddle->valence() == 0)
    sub.saddle->active(false);
}


float HierarchicalMorseComplex::getExtendedPersistence(const NodeType* saddle)
{
  return 0;
}


