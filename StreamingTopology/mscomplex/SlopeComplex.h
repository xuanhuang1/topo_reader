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

#ifndef SLOPECOMPLEX_H
#define SLOPECOMPLEX_H

#include <map>
#include <vector>
#include <algorithm>
#include "GradientComplex.h"
#include "MSCVertex.h"
#include "Slopes.h"

//! A slope complex stores a number of vertices along with their steepest
//! neighbor in a- and descending direction
template <class VertexClass = MSCVertex<3,float>, class ArrayClass=FlexArray::BlockedArray<VertexClass,LocalIndexType> >
class SlopeComplex : public GradientComplex<VertexClass,ArrayClass>
{
public:
  //! Typedef to define the operator that computes slopes
  typedef Slope<VertexClass::sDimension,typename VertexClass::sDataType> SlopeType;

  //! Typedef for the function type
  typedef typename VertexClass::FunctionType FunctionType;

  //! Default constructor
  SlopeComplex(const SlopeType& s) : GradientComplex<VertexClass,ArrayClass>(s) {}

  //! Default destructor
  virtual ~SlopeComplex() {}

  //! Compute unstable segmentation by doing a union-find of the steepest neighbors
  virtual void computeUnstableSegmentation() {this->computeGradientSegmentation(&VertexClass::unstableInfo);}

  virtual void outputUnstableSegmentation(FILE *output,bool ascii=true) {this->outputMorseSegmentation(output,ascii,mUnstableComplex,&VertexClass::unstableInfo);}

  //! Output the pairs of stable and unstable indices defining the slopes
  virtual void outputSlopeSegmentation(FILE *output,bool ascii=true);
  
  void createUnstableComplex(ComplexAssembly<VertexClass>* assembly);
  
  void simplifyUnstableComplex(float p) {mUnstableComplex.updatePersistence(p);}

  //! Output an xmgrace-stype file describing the # of extrema vs persistence
  void outputExtremaVsPersistence(FILE* output);

  //! Output an xmgrace-stype file describing the # of maxima vs persistence
  void outputMaximaVsPersistence(FILE* output);

  //! Output the cancellation tree corresponding to the unstable complex
  void outputUnstableCancellationTree(FILE* output, bool ascii=true) {mUnstableComplex.outputCancellationTree(output,ascii);}

  //! Output the unstable complex
  void outputUnstableComplex(FILE* output) {mUnstableComplex.outputComplex(output);}

  //! Output the label hierarchy of the unstable complex
  void outputUnstableLabelHierarchy(FILE* output) {mUnstableComplex.outputLabelHierarchy(output);}

protected:

  //! The Morse complex of the unstable manifolds
  HierarchicalMorseComplex mUnstableComplex;
};


template <class VertexClass, class ArrayClass>
void SlopeComplex<VertexClass,ArrayClass>::createUnstableComplex(ComplexAssembly<VertexClass>* assembly) 
{
  GlobalIndexType index;

  this->createComplex(mUnstableComplex,assembly,&VertexClass::unstableInfo,-1);

  // If we have found no arc at all this means we have a lonley maximum
  if (mUnstableComplex.size() == 0) {
    index = this->mVertices[0].unstableInfo().steepest();
    mUnstableComplex.addNode(index,this->mVertices[index].f(),MAXIMUM);
  } 

  // Now we need to add any loney maxima that had no edge attached
  for (uint32_t i=0;i<this->mVertices.size();i++) {
    
    if (i == this->mVertices[i].unstableInfo().steepest()) {
      if (mUnstableComplex.findElement(i) == NULL)
        mUnstableComplex.addNode(i,this->mVertices[i].f(),MAXIMUM);
    }
  }
}


template <class VertexClass, class ArrayClass>
void SlopeComplex<VertexClass,ArrayClass>::outputSlopeSegmentation(FILE *output,bool ascii)
{
  LocalIndexType count = 0;
  LocalIndexType i;
  std::map<LocalIndexType,uint16_t> indexMap;
  std::map<LocalIndexType,uint16_t>::iterator mIt;
  std::map<uint32_t,uint32_t> cellMap;
  std::map<uint32_t,uint32_t>::iterator cIt;
  std::vector<uint32_t> reorder;
  uint32_t stable,unstable,index;
  typename HierarchicalMorseComplex::iterator nIt;

  sterror(!ascii,"Sorry, binary slop output not implemented yet.");

  for (nIt=this->mStableComplex.begin();nIt!=this->mStableComplex.end();nIt++) {
    if (nIt->isActive() && (nIt->morseType() != MERGE_SADDLE)) {
      //fprintf(stderr,"Found Max %d  %e\n",nIt->id(),nIt->persistence());
      indexMap[nIt->id()] = count++;
    }
      
  }
  
  for (nIt=this->mUnstableComplex.begin();nIt!=this->mUnstableComplex.end();nIt++) {
    if (nIt->isActive() && (nIt->morseType() != MERGE_SADDLE))
      indexMap[nIt->id()] = count++;
  }
  
  fprintf(output,"# extrema %u\n",count);

  for (nIt=this->mStableComplex.begin();nIt!=this->mStableComplex.end();nIt++) {
    if (nIt->isActive() && (nIt->morseType() != MERGE_SADDLE)) {
      if (nIt->parent() != NULL) {
        mIt = indexMap.find(nIt->parent()->id());
        sterror(mIt==indexMap.end(),"Parent index %d not found in index map",nIt->parent()->id());
        fprintf(output,"max %u %e %d\n",nIt->id(),nIt->persistence(),mIt->second);
      }
      else
        fprintf(output,"max %u %e %d\n",nIt->id(),nIt->persistence(),GNULL);
    }
  }
  
  for (nIt=this->mUnstableComplex.begin();nIt!=this->mUnstableComplex.end();nIt++) {
    if (nIt->isActive() && (nIt->morseType() != MERGE_SADDLE)) {
      if (nIt->parent() != NULL) {
        mIt = indexMap.find(nIt->parent()->id());
        sterror(mIt==indexMap.end(),"Parent index %d not found in index map",nIt->parent()->id());
        fprintf(output,"min %u %e %d\n",nIt->id(),nIt->persistence(),mIt->second);
      }
      else
        fprintf(output,"min %u %e %d\n",nIt->id(),nIt->persistence(),GNULL);
    }
  }
  
  
  count = 0;
  for (i=0;i<this->mVertices.size();i++) {
    
    //fprintf(stderr,"Processing vertex %d  stable: %d  unstable %d\n",i,this->mVertices[i].stableInfo().steepest(),
    //        this->mVertices[i].unstableInfo().steepest());

    stable = this->mVertices[i].stableInfo().extremum();
    if (this->mStableComplex.size() > 0)
      stable = this->mStableComplex.findActiveNode(stable)->id();

    mIt = indexMap.find(stable);
    sterror(mIt==indexMap.end(),"Stable manifold index %d not found",this->mVertices[i].stableInfo().extremum());

    index = mIt->second;

    unstable = this->mVertices[i].unstableInfo().extremum();
    if (this->mUnstableComplex.size() > 0)
      unstable = this->mUnstableComplex.findActiveNode(unstable)->id();
   
    mIt = indexMap.find(unstable);
    sterror(mIt==indexMap.end(),"Unstable manifold index %d not found",this->mVertices[i].unstableInfo().extremum());

    index |= mIt->second << 16;

    cIt = cellMap.find(index);
    if (cIt == cellMap.end()) 
      cellMap[index] = count++;
   
  }
 
  fprintf(output,"# cells %u\n",count);
  

  reorder.resize(count);

  for (cIt=cellMap.begin();cIt!=cellMap.end();cIt++) 
    reorder[cIt->second] = cIt->first;

  for (i=0;i<count;i++) 
    fprintf(output,"cell %d %d\n",((reorder[i] << 16) >> 16),reorder[i] >> 16);

  
  fprintf(output,"# vertices %u\n",(uint32_t)this->mVertices.size());
  for (i=0;i<this->mVertices.size();i++) {
    
    stable = this->mVertices[i].stableInfo().extremum();
    if (this->mStableComplex.size() > 0)
      stable = this->mStableComplex.findActiveNode(stable)->id();

    mIt = indexMap.find(stable);
    sterror(mIt==indexMap.end(),"Stable manifold index %u not found",this->mVertices[i].stableInfo().extremum());

    index = mIt->second;

    unstable = this->mVertices[i].unstableInfo().extremum();
    if (this->mUnstableComplex.size() > 0)
      unstable = this->mUnstableComplex.findActiveNode(unstable)->id();
   
    mIt = indexMap.find(unstable);
    sterror(mIt==indexMap.end(),"Unstable manifold index %d not found",this->mVertices[i].unstableInfo().extremum());
    
    index |= mIt->second << 16;

    cIt = cellMap.find(index);
    sterror(cIt==cellMap.end(),"Cell index %u not found.",index);

    fprintf(output,"v %u %u %u\n",cIt->second,(index << 16) >> 16,index >> 16);
  }
    

}


template <class VertexClass, class ArrayClass>
void SlopeComplex<VertexClass,ArrayClass>::outputExtremaVsPersistence(FILE* output)
{
  std::vector<typename HierarchicalMorseComplex::Substitution> combined;
  uint32_t i,k;

  combined.resize(this->mStableComplex.hierarchy().size()+mUnstableComplex.hierarchy().size());

  for (i=0;i<this->mStableComplex.hierarchy().size();i++)
    combined[i] = this->mStableComplex.hierarchy()[i];
  
  k = this->mStableComplex.hierarchy().size();

  for (i=0;i<mUnstableComplex.hierarchy().size();i++)
    combined[i+k] = this->mUnstableComplex.hierarchy()[i];

  sort(combined.begin(),combined.end());


  fprintf(output,"0.0 %lu\n",combined.size()+2);
  for (i=0;i<combined.size();i++) 
    fprintf(output,"%0.5e %lu\n",combined[i].p,combined.size()+1 - i);
  
  if (!combined.empty() && (combined.back().p < 0.5))
    fprintf(output,"0.5 2\n");
    
  if (combined.empty())
    fprintf(output,"0.5 2\n");

  fprintf(output,"@ s_ legend \"# of extrema\"\n");
  fprintf(output,"@ s_ line type 3\n");

  
  this->mStableComplex.outputExtremaVsPersistence(output,"# of maxima");
  mUnstableComplex.outputExtremaVsPersistence(output,"# of minima");

}

template <class VertexClass, class ArrayClass>
void SlopeComplex<VertexClass,ArrayClass>::outputMaximaVsPersistence(FILE* output)
{
  this->mStableComplex.outputExtremaVsPersistence(output,"# of maxima");
}



#endif
