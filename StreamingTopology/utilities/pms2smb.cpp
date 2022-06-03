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

#include <cstdio>
#include <cstdlib>
#include "TalassConfig.h"

int main(int argc, char* argv[])
{
  if (argc < 4) {
    fprintf(stderr,"Usage: %s <2D-bof> <dimx> <dimy> [<output.smb>]",argv[0]);
    exit(0);
  }

  FILE* input = fopen(argv[1],"rb");
  uint32_t dim_x = atoi(argv[2]);
  uint32_t dim_y = atoi(argv[3]);
  uint32_t i,j;
  char token;
  float v[3];
  float* buffer;

  FILE* output;
  if (argc > 4)
    output = fopen(argv[4],"wb");
  else
    output = stdout;

  // Read in the block of floats
  buffer = new float[dim_x*dim_y];
  fread(buffer,sizeof(float),dim_x*dim_y,input);
  fclose(input);

  // Write the first row of vertices
  token = 'v';
  for (i=0;i<dim_x;i++) {
    fwrite(&token,sizeof(char),1,output);

    v[0] = 2*i;
    v[1] = 0;
    v[2] = buffer[i];
    fwrite(v,sizeof(float),3,output);
  }

  // Write the other rows
  for (j=1;j<dim_y;j++) {

    // Write the mid-points
    token = 'v';
    for (i=1;i<dim_x;i++) {

      fwrite(&token,sizeof(char),1,output);
      v[0] = 2*i-1;
      v[1] = 2*j-1;
      v[2] = 0.25*(buffer[j*dim_x + i] + buffer[j*dim_x + i - 1] +
          buffer[(j-1)*dim_x + i] + buffer[(j-1)*dim_x + i - 1]);
      fwrite(v,sizeof(float),3,output);
    }

    // Write the next row of vertices
    for (i=0;i<dim_x;i++) {

      fwrite(&token,sizeof(char),1,output);
      v[0] = 2*i;
      v[1] = 2*j;
      v[2] = buffer[j*dim_x+i];
      fprintf(stderr,"v %f %f %f\n",v[0],v[1],v[2]);
      fwrite(v,sizeof(float),3,output);
    }

    token = 'e';
    // Write the edges
    for (i=1;i<dim_x;i++) {
      // For convenience we pre-compute the five indices involved
      uint32_t index[5]; //ll,rl,lu,ru,mid

      index[0] = (j-1)*dim_x + i - 1 + (j-1)*(dim_x-1) + 1;
      index[1] = (j-1)*dim_x + i     + (j-1)*(dim_x-1) + 1;
      index[2] =     j*dim_x + i - 1 +     j*(dim_x-1) + 1;
      index[3] =     j*dim_x + i     +     j*(dim_x-1) + 1;
      index[4] =     j*dim_x + i - 1 + (j-1)*(dim_x-1) + 1;

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+0,sizeof(uint32_t),1,output);
      fwrite(index+1,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+1,sizeof(uint32_t),1,output);
      fwrite(index+3,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+3,sizeof(uint32_t),1,output);
      fwrite(index+2,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+0,sizeof(uint32_t),1,output);
      fwrite(index+2,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+0,sizeof(uint32_t),1,output);
      fwrite(index+4,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+1,sizeof(uint32_t),1,output);
      fwrite(index+4,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+2,sizeof(uint32_t),1,output);
      fwrite(index+4,sizeof(uint32_t),1,output);

      fwrite(&token,sizeof(char),1,output);
      fwrite(index+3,sizeof(uint32_t),1,output);
      fwrite(index+4,sizeof(uint32_t),1,output);
    }
  }

  return 1;
}






