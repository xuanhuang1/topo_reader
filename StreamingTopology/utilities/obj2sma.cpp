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


#include <stdio.h>
#include <vector>

#include "Definitions.h"
#include "OOCArray.h"
#include "SMAParser.h"

void splitLine(char* line, char** token)
{
  int i = 0;
  int n = 0;

  if ((line[i] != '\0') && (line[0] != ' ')) {
    token[0] = line;
    n++;
  }

  if (line[0] == '\0')
    return;


  while (true) {

    while ((line[i] != '\0') && (line[i] != ' '))
      i++;

    if (line[i] == '\0')
      return;

    while ((line[i] == ' ') && (line[i] != '\0'))
      i++;

    if (line[i] == '\0')
      return;

    token[n++] = line+i;
  }
}




int main(int argc, char **argv)
{

  if (argc < 5) {
    fprintf(stderr,"Usage: %s <input.obj> <EDim> <SDim> <output.sma>\n",argv[0]);
    return 0;
  }


  FILE* input;
  FILE* output;

  input = fopen(argv[1],"r");
  if (input == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[1]);
    return 0;
  }

  FlexArray::OOCArray<float,int> vertices;
  FlexArray::OOCArray<int,int> index_map;
  FlexArray::OOCArray<int,int> count;

  int edim = atoi(argv[2]);
  int sdim = atoi(argv[3]);

  SMAParser<> parser(input,edim,sdim,edim-1);
  std::vector<float>::const_iterator vIt;
  std::vector<GlobalIndexType>::const_iterator it;
  FileToken token;
  int NV = 0;
  int NF = 0;

  token = parser.getToken();
  while (token != EMPTY) {
    switch (token) {
    case VERTEX: {

      for (vIt=parser.vertex().begin();vIt!=parser.vertex().end();vIt++)
        vertices.push_back(*vIt);

      index_map.push_back(-1);
      count.push_back(0);
      NV++;
      break;
    }
    case PATH: {
      
      for (it=parser.getPath().begin();it!=parser.getPath().end();it++) {
        count[*it] += 1;
      }      

      NF++;
      break;
    }
    default:
      break;
    }

    token = parser.getToken();
  }

  fclose(input);
  input = fopen(argv[1],"r");
  output = fopen(argv[4],"w");
  if (output == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[4]);
    return 0;
  }

  fprintf(output,"# nverts %d\n",NV);
  fprintf(output,"# nfaces %d\n",NF);

  NV = NF = 0;
  
  token = parser.getToken();
  while (token != EMPTY) {
    switch (token) {
    case PATH: {
      
      for (it=parser.getPath().begin();it!=parser.getPath().end();it++) {
        
        if (index_map[*it] == -1) {
          index_map[*it] = NV++;
          fprintf(output,"v");
          for (int i=0;i<edim;i++) {
	    fprintf(output," %f",vertices[(*it)*edim + i]);
	  }
          fprintf(output,"\n");
        }
      }

      fprintf(output,"f");
      for (it=parser.getPath().begin();it!=parser.getPath().end();it++) {
        count[*it] -= 1;
        
        if (count[*it] == 0)
          fprintf(output," %d",index_map[*it] - NV);
        else
          fprintf(output," %d",index_map[*it]+1);
      }
      fprintf(output,"\n");
      break;
    }
    default:
      break;
    }

    token = parser.getToken();
  }

  fclose(output);
  fclose(input);

  return 1;
}
  
  
  
      
    
      
    
  

