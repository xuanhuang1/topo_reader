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
#include "SMBParser.h"



int main(int argc, char **argv)
{

  if (argc < 5) {
    fprintf(stderr,"Usage: %s <input.smb> <EDim> <SDim> <output.smb>\n",argv[0]);
    return 0;
  }


  FILE* input;
  FILE* output;

  input = fopen(argv[1],"r");
  if (input == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[1]);
    return 0;
  }

  std::vector<uint32_t> count;

  int edim = atoi(argv[2]);
  int sdim = atoi(argv[3]);

  SMBParser<> parser(input,edim,sdim,edim-1);
  std::vector<GlobalIndexType>::const_iterator it;
  FileToken token;
  int NV = 0;
  int NF = 0;

  token = parser.getToken();
  while (token != EMPTY) {
    switch (token) {
    case VERTEX: {

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

  NV = NF = 0;
  
  SMBParser<> parser2(input,edim,sdim,edim-1);
  char c;
  token = parser2.getToken();
  while (token != EMPTY) {
    switch (token) {

      case VERTEX:
        c = 'v';
        fwrite(&c,sizeof(char),1,output);

        fwrite(parser2.coord(),sizeof(FunctionType),edim,output);

        NV++;
        break;

      case PATH: {

        if (sdim == 2)
          c = 'e';
        else
          c = 'f';
        fwrite(&c,sizeof(char),1,output);

        int32_t path[sdim];
        uint32_t i = 0;
        for (it=parser2.getPath().begin();it!=parser2.getPath().end();it++,i++) {
          count[*it] -= 1;

          if (count[*it] == 0)
            path[i] = -*it - 1;
          else
            path[i] = *it + 1;
        }
        fwrite(path,sizeof(GlobalIndexType),sdim,output);

        break;
      }
      default:
        break;
    }

    token = parser2.getToken();
  }

  fclose(output);
  fclose(input);

  return 1;
}
  
  
  
      
    
      
    
  

