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

void parseHeader(FILE* input, int edim, int &nv, int &nf)
{
  int count = 0;
  char c;
  char line[200];
  float f;
  int dummy;
  
  fscanf(input,"%f",&f); // read over the tag
  for (int i=0;i<edim;i++) {
    fscanf(input,"%s",line); //read over the x,y,z,f tags
    fprintf(stderr,"Tag: %s\n",line);
  }

  fscanf(input,"%d %d",&nf,&dummy); // read the number of faces

  // Find the eight's opening bracket
  while (count < 8) {
    c = getc(input);
    if (c == '(')
      count++;
  }

  // The number after is the number of vertices
  fscanf(input,"%d,",&nv);

  // The mef file provides the last used index rather than the number
  // of vertices used
  nv++;

  // Skip the rest of the lines
  fgets(line,200,input);

  fprintf(stderr,"Last header line: \"%s\"",line);
  fprintf(stderr,"%d vertices    %d faces\n",nv,nf);
}



int main(int argc, char **argv)
{

  if (argc < 4) {
    fprintf(stderr,"Usage: %s <input.obj> <EDim> <SDim> [<output.sma>]\n",argv[0]);
    return 0;
  }


  FILE* input;
  FILE* output = stdout;

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

  FunctionType *coord = new FunctionType[edim];
  int32_t *s = new int32_t[sdim];

  int nv = 0;
  int nf = 0;

  // Parse the header information and extract the number of vertices and faces
  parseHeader(input,edim,nv,nf);


  vertices.resize(nv*edim);
  index_map.resize(nv);
  count.resize(nv);

  for (int i=0;i<nv;i++) {
    
    fread(coord,sizeof(FunctionType),edim,input);
    
    for (int k=0;k<edim;k++)
      vertices[i*edim + k] = coord[k];

    index_map[i] = -1;
    count[i] = 0;
  }

  for (int i=0;i<nf;i++) {
    fread(s,sizeof(int32_t),sdim,input);

    for (int k=0;k<sdim;k++)
      count[s[k]-1] += 1;
  }
    
  fclose(input);


  input = fopen(argv[1],"r");
  parseHeader(input,edim,nv,nf);

  if (argc > 4)
    output = fopen(argv[4],"w");
  if (output == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[4]);
    return 0;
  }

  fprintf(output,"# nverts %d\n",nv);
  fprintf(output,"# nfaces %d\n",nf);

  // Now skip the vertex info
  fseek(input,sizeof(FunctionType)*nv*edim,SEEK_CUR);

  int vcount = 0;
  for (int i=0;i<nf;i++) {
    fread(s,sizeof(int32_t),sdim,input);
    
    for (int k=0;k<sdim;k++) {
      if (index_map[s[k]-1] == -1) {
        index_map[s[k]-1] = vcount++;
        fprintf(output,"v");
        for (int j=0;j<edim;j++)
          fprintf(output," %f",vertices[(s[k]-1)*edim + j]);
        fprintf(output,"\n");
      }
    }

    fprintf(output,"f");

    for (int k=0;k<sdim;k++) {
      count[s[k]-1] -= 1;
      
      if (count[s[k]-1] == 0)
        fprintf(output," %d",index_map[s[k]-1] - vcount);
      else
        fprintf(output," %d",index_map[s[k]-1]+1);
    }
    fprintf(output,"\n");
  }


  fclose(output);
  fclose(input);

  return 1;
}
  
  
  
      
    
      
    
  

