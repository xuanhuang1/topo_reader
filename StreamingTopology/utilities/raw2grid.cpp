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

#include <cstdlib>
#include <cstdio>

int main(int argc, char* argv[])
{
  if (argc < 6) {
    fprintf(stderr,"Usage: %s <input.raw> <dimx> <dimy> <dimz> <outpu.grid>\n",argv[0]);
    return 0;
  }

  FILE* input = fopen(argv[1],"rb");
  FILE* output = fopen(argv[5],"wb");
  int dimx,dimy,dimz;
  float *buffer; 
  float v[4];

  dimx = atoi(argv[2]);
  dimy = atoi(argv[3]);
  dimz = atoi(argv[4]);


  buffer = new float[dimx*dimy*dimz];
  fread(buffer,sizeof(float),dimx*dimy*dimz,input);
  fclose(input);

  for (int i=0;i<dimz;i++) {
    for (int j=0;j<dimy;j++) {
      for (int k=0;k<dimx;k++) {

        v[0] = k;
        v[1] = j;
        v[2] = i;
        v[3] = buffer[i*dimx*dimy + j*dimx + k];
          
        fwrite(v,sizeof(float),4,output);
      }
    }
  }

  fclose(output);

  return 1;
}
