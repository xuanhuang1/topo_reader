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


#ifndef GRAPHIO_H
#define GRAPHIO_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include "TopoGraph.h"
#include "Node.h"


std::string default_node_attribute(const Node& node);

std::string default_arc_attribute(const Node& source, const Node& target);

template <class NodeData>
int write_dot(FILE* output, TopoGraph<NodeData>& graph, int resolution,
              double low,double high, const char* node_attributes = "",
              const char* arc_attributes = "",
              string (*nodeAttribute)(const Node&) = default_node_attribute,
              string (*arcAttribute)(const Node&, const Node&) = default_arc_attribute)
{
  int i;
  int quantized;
  string att;
  typename STMappedArray<typename TopoGraph<NodeData>::InternalNode>::const_iterator it;
  typename std::vector<Node*>::const_iterator vIt;

  fprintf(output,"digraph G {\n");
  fprintf(output,"\trankdir=TB;ranksep=0.2;\n");
  fprintf(output,"\tnode [shape=plaintext,fontsize=10];\n");
  fprintf(output,"\tedge [color=white];\n");

  high = MIN(high,graph.maxF());
  low = MAX(low,graph.minF());

  for (i=0;i<resolution;i++)
    fprintf(output,"f%d [label=\"%0.3f\"]",i,low + i*(high-low)/(resolution-1));


  fprintf(output,"f%d ",resolution-1);
  for (i=resolution-2;(int)i>=0;i--) {
    fprintf(output," -> f%d",i);
  }
  fprintf(output,"\n\n");

  fprintf(output,"\tnode [%s];\n",node_attributes);
  fprintf(output,"\tedge [color=black];\n");
  fprintf(output,"\tedge [%s];\n",arc_attributes);

  for (it=graph.nodes().begin();it!=graph.nodes().end();it++) {
    if (it->isActive() && (it->f() >= low) && (it->f() <= high)) {
      quantized = (int)(resolution*(it->f() - low) / (high-low));
      if (quantized == resolution)
        quantized = resolution-1;

      fprintf(output,"{rank = same; f%d; %Ld}\n",quantized,(uint64_t)it->id());

      //fprintf(output,"%Ld %s\n",(uint64_t)it->id(), nodeAttribute(*it));
      //nodeAttribute(*it);
      fprintf(output,"%d %s\n",it->id(), nodeAttribute(*it).c_str());

      for (vIt=it->down().begin();vIt!=it->down().end();vIt++)
        fprintf(output,"%d -> %d\n",it->id(),(*vIt)->id());
    }
  }


  fprintf(output,"\n}\n");

  return 1;
}


#endif
 
