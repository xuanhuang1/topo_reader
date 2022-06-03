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

#ifndef GRADIENTCOMPLEX_H
#define GRADIENTCOMPLEX_H

#include <cstdlib>
#include "BlockedArray.h"
#include "Slopes.h"
#include "MCVertex.h"
#include "GradientComplexInterface.h"
#include "HierarchicalMorseComplex.h"
#include "ComplexAssembly.h"

//! A gradient complex stores a number of vertices along with their steepest neighbor
template <class VertexClass = MCVertex<3,float>, class ArrayClass=FlexArray::BlockedArray<VertexClass,LocalIndexType> >
class GradientComplex : public GradientComplexInterface<VertexClass>
{
public:

  //! Typedef to define the array class
  typedef ArrayClass ArrayType;

  //! Typedef to define the operator that computes slopes
  typedef Slope<VertexClass::sDimension,typename VertexClass::sDataType> SlopeType;

  //! Typedef to allow easy access to only the vertex information
  typedef EmbeddedVertex<VertexClass::sDimension,typename VertexClass::sDataType> BaseVertex;

  //! Typedef for the function type
  typedef typename VertexClass::FunctionType FunctionType;

  //! Typedef for stable/unstableInfo() member functions
  typedef GradientInfo& (VertexClass::*GradInfoMemberFunc)();

  //! Default constructor
  GradientComplex(const SlopeType& s);

  //! Destructor
  virtual ~GradientComplex() {}
  
  //! Access the array of all vertices
  virtual const ArrayClass& vertices() const {return mVertices;}

  //! Access to a single vertex
  virtual VertexClass& vertex(LocalIndexType index) {return mVertices[index];}

  //! Access to a single vertex
  virtual const VertexClass& vertex(LocalIndexType index) const {return mVertices[index];}

  //! Return the current number of vertices
  virtual LocalIndexType size() {return mVertices.size();}

  //! Add a vertex
  virtual void addVertex(GlobalIndexType index,const BaseVertex& data);

  //! Compute the slope between two vertices
  virtual double slope(LocalIndexType u, LocalIndexType v) {return mSlope(mVertices[u],mVertices[v]);}

  //! SoS compare the vertices corresponding to the indices
  virtual bool smaller(LocalIndexType u, LocalIndexType v) const;

  //! Compute stable segmentation by doing a union-find of the steepest neighbors
  virtual void computeStableSegmentation() {computeGradientSegmentation(&VertexClass::stableInfo);}

  //! Output the stable manifold indices of all vertices in order
  virtual void outputStableSegmentation(FILE *output,bool ascii=true) {outputMorseSegmentation(output,ascii,mStableComplex,&VertexClass::stableInfo);}

  void createStableComplex(ComplexAssembly<VertexClass>* assembly);
  
  void simplifyStableComplex(float p) {mStableComplex.updatePersistence(p);}

  //! Output an xmgrace-stype file describing the # of extrema vs persistence
  void outputExtremaVsPersistence(FILE* output) {mStableComplex.outputExtremaVsPersistence(output,"# of maxima");}

  //! Output the cancellation tree corresponding to this Morse complex
  void outputStableCancellationTree(FILE* output, bool ascii=true) {mStableComplex.outputCancellationTree(output,ascii);}

  //! Output the full complex
  void outputStableComplex(FILE* output) {mStableComplex.outputComplex(output);}

  //! Output the label hierarchy of the stable complex
  void outputStableLabelHierarchy(FILE* output) {mStableComplex.outputLabelHierarchy(output);}

protected:

  //! The operator to compute the slope between two vertices
  const SlopeType& mSlope;

  //! The array storing the vertex data and gradient information
  ArrayClass mVertices;

  //! The Morse complex of the stable manifolds
  HierarchicalMorseComplex mStableComplex;

  //! Minimal function value seen so far
  FunctionType mMinF;

  //! Maxmima function value seen so far
  FunctionType mMaxF;

  //! Compute the gradient segmentation based on the gradient info return by the
  //! given function
  void computeGradientSegmentation(GradInfoMemberFunc gradientInfo);

  //! Traverse and shortcut the union find information relative to the given
  //! gradient info
  /*! This function will follow the steepest gradient "upwards" until a maximum
   *  is found. The index of the maximum is used as index for its associate
   *  stable manifold and transfered in form of the steepest index to all
   *  vertices of this integral line
   *  @param index: The index of the vertex we currently work on
   *  @param gradientInfo: Pointer to a member function of VertexClass that will 
   *                       return the appropriate gradient information (stable or 
   *                       unstable)
   *  @return: The index of the extremum that lies at the end of this integral line
   */
  LocalIndexType unionFind(LocalIndexType index, GradInfoMemberFunc gradientInfo);

  //! Create a complex from the given assembly
  void createComplex(HierarchicalMorseComplex& complex, ComplexAssembly<VertexClass>* assembly,
                     GradInfoMemberFunc gradientInfo, int direction);

  //! Create the path from the saddle to the extremum
  void createPath(LocalIndexType saddle_id, LocalIndexType extremum,
                  LocalIndexType saddle_vertex, LocalIndexType start, std::vector<LocalIndexType>& path,
                  GradInfoMemberFunc gradientInfo);

  void outputMorseSegmentation(FILE *output,bool ascii,HierarchicalMorseComplex& complex,
                               GradInfoMemberFunc gradientInfo);


};
  
template <class VertexClass, class ArrayClass>
GradientComplex<VertexClass,ArrayClass>::GradientComplex(const SlopeType& s) : 
  GradientComplexInterface<VertexClass>(), mSlope(s), mStableComplex()
{
}


template <class VertexClass, class ArrayClass>
bool GradientComplex<VertexClass,ArrayClass>::smaller(LocalIndexType u, LocalIndexType v) const
{
  if (mVertices[u].f() < mVertices[v].f())
    return true;

  if ((mVertices[u].f() == mVertices[v].f()) && (u < v))
    return true;

  return false;
}

template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::addVertex(GlobalIndexType index,const BaseVertex& data)  
{
  mMinF = MIN(mMinF,data.f());
  mMaxF = MAX(mMaxF,data.f());
  mVertices.add(index,VertexClass(data));
}


template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::computeGradientSegmentation(GradInfoMemberFunc gradientInfo)
{
  // For each of our vertices
  for (LocalIndexType i=0;i<mVertices.size();i++) {
    if (i == 108)
      fprintf(stderr,"break  %d %d\n",(mVertices[i].*gradientInfo)().steepest(),(mVertices[i].*gradientInfo)().extremum());
    // Trace the steepest neighbors until you find a maximum and shortcut all
    // intermediate vertices.
    unionFind(i,gradientInfo);
  }
}

template <class VertexClass, class ArrayClass>
LocalIndexType GradientComplex<VertexClass,ArrayClass>::unionFind(LocalIndexType index, GradInfoMemberFunc gradientInfo)
{
  LocalIndexType next;

  // Get the index of the next steepest neighbor
  next = (mVertices[index].*gradientInfo)().steepest();

  // If the vertex had no "higher" neighbors than it is a "maximum" and the
  // root of its union-find tree. Thus its extremum index, which will become
  // the stable manifold index is index.
  if (next == LNULL) {
    (mVertices[index].*gradientInfo)().extremum(index);
    return index;
  }
  
  // Otherwise, we first fix the extremum index of our steepest neighbor
  next = unionFind(next,gradientInfo);

  // And use it as our own index
  (mVertices[index].*gradientInfo)().extremum(next);
  
  // Return the index of this component
  return next;
}

template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::createStableComplex(ComplexAssembly<VertexClass>* assembly) 
{
  GlobalIndexType index;

  createComplex(mStableComplex,assembly,&VertexClass::stableInfo,1);

  // If we have found no arc at all this means we have a lonley maximum
  if (mStableComplex.size() == 0) {
    index = mVertices[0].stableInfo().extremum();
    mStableComplex.addNode(index,mVertices[index].f(),MAXIMUM);
  }

  // Now we need to add any loney maxima that had no edge attached
  for (uint32_t i=0;i<mVertices.size();i++) {
    
    if (i == mVertices[i].stableInfo().extremum()) {
      if (mStableComplex.findElement(i) == NULL)
        mStableComplex.addNode(i,mVertices[i].f(),MAXIMUM);
    }
  }
}


template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::createComplex(HierarchicalMorseComplex& complex, ComplexAssembly<VertexClass>* assembly,
                                                            GradInfoMemberFunc gradientInfo, int direction)
{
  typename ComplexAssembly<VertexClass>::MapIterator mIt;
  LocalIndexType m1,m2,a1,a2,saddle_id,saddle_vertex;
  LocalIndexType count = mVertices.size();
  std::vector<LocalIndexType> path;

  //fprintf(stderr,"Neighborhood size  %u\n",assembly->mNeighborMap.size());

  for (mIt=assembly->mNeighborMap.begin();mIt!=assembly->mNeighborMap.end();mIt++) {

    m1 = mIt->first.i;
    m2 = mIt->first.j;
    saddle_vertex  = mIt->second.saddle;
    a1 = mIt->second.left;
    a2 = mIt->second.right;

    sterror(m1==m2,"There should not exist a loop.");

    //if (s == 108)
    //fprintf(stderr,"GradientComplex processing  triple %d %d %d\n",m1,m2,saddle_vertex);

    complex.addNode(m1,mVertices[m1].f(),MAXIMUM);
    complex.addNode(m2,mVertices[m2].f(),MAXIMUM);

    // If for whatever reason the saddle already exists then we found
    // a multi-saddle (the index pair must be different). We add another
    // node to the complex with the same coordinates
    if (complex.findElement(saddle_vertex) != NULL) {
      complex.addNode(count,mVertices[saddle_vertex].f(),MERGE_SADDLE);
      saddle_id = count++;

      fprintf(stderr,"Found multi-saddle at %d\n",saddle_vertex);
      // Add both nodes into the list of multi-saddles
      complex.addMultiSaddle(complex.findElement(saddle_vertex));
      complex.addMultiSaddle(complex.findElement(saddle_id));
    }
    else {
      complex.addNode(saddle_vertex,mVertices[saddle_vertex].f(),MERGE_SADDLE);
      saddle_id = saddle_vertex;
    }

    complex.addArc(m1,saddle_id);
    createPath(saddle_id,m1,saddle_vertex,a1,path,gradientInfo);
    complex.addPath(path);

    complex.addArc(m2,saddle_id);
    createPath(saddle_id,m2,saddle_vertex,a2,path,gradientInfo);
    complex.addPath(path);
  }



  complex.minF(mMinF);
  complex.maxF(mMaxF);
  complex.constructHierarchy(gMaxValue/2,RECOVERABLE,direction);
}

template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::createPath(LocalIndexType saddle_id, LocalIndexType extremum,
                                                         LocalIndexType saddle_vertex, LocalIndexType start,
                                                         std::vector<LocalIndexType>& path,
                                                         GradInfoMemberFunc gradientInfo)
{
  // Clean the path
  path.clear();

  // Push the saddle id (which for multi-saddles is different from the saddle
  // vertex into the path. This allows us to uniquely identify the saddle-extremum
  // arc from the first and last index
  path.push_back(saddle_id);

  // If the saddle vertex  is not actually the starting point we force
  // it to be to create two paths starting at the same saddle
  if (saddle_vertex != start)
    path.push_back(saddle_vertex);

  // Now at the starting vertex to the path
  path.push_back(start);

  // Until we have reach an extremum follow the steepest edges
  while ((mVertices[start].*gradientInfo)().steepest() != LNULL) {
    start = (mVertices[start].*gradientInfo)().steepest();

    path.push_back(start);
  }

  sterror(path.back()!=extremum,"The path geometry ends at %u but should end at %u",path.back(),extremum);
}

template <class VertexClass, class ArrayClass>
void GradientComplex<VertexClass,ArrayClass>::outputMorseSegmentation(FILE *output,bool ascii, HierarchicalMorseComplex& complex,
                                                                      GradInfoMemberFunc gradientInfo)
{
  LocalIndexType index;

  if (ascii) {
    for (uint32_t i=0;i<mVertices.size();i++) {
      index = (mVertices[i].*gradientInfo)().extremum();

      if (complex.size() > 0)
        index = complex.findActiveNode(index)->id();

      if (i == index) 
        fprintf(output,"vt %d\n",index);
      else
        fprintf(output,"vt %d\n",index);
    }
  }
  else {
    for (uint32_t i=0;i<mVertices.size();i++) {
      index = (mVertices[i].*gradientInfo)().extremum();

      if (complex.size() > 0)
        index = complex.findActiveNode(index)->id();

      fwrite(&index,sizeof(LocalIndexType),1,output);
    }
  }      
}



#endif
