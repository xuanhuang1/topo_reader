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
#include <stdlib.h>
#include <vector>
#include <map>

#include "Definitions.h"
#include "BinaryParser.h"



int main(int argc, char **argv)
{

  if (argc < 4) {
    fprintf(stderr,"Usage: %s <input.bin> <EDim> <output.bin>\n",argv[0]);
    return EXIT_FAILURE;
  }


  FILE* input;
  FILE* output;

  input = fopen(argv[1],"r");
  if (input == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[1]);
    return EXIT_FAILURE;
  }

  std::map<GlobalIndexType,uint16_t> count;
  std::map<GlobalIndexType,uint16_t>::iterator mIt,mIt2;

  int edim = atoi(argv[2]);

  BinaryParser<> parser(input,edim);
  std::vector<GlobalIndexType>::const_iterator it;
  FileToken token;
  int NV = 0;

  token = parser.getToken();
  while (token != EMPTY) {
    switch (token) {
    case VERTEX: {

      //fprintf(stderr,"Vertex %lld\n",parser.getId());
      count[parser.getId()] = 0;
      NV++;
      break;
    }
    case PATH: {
      
      for (it=parser.getPath().begin();it!=parser.getPath().end();it++) {
        count.find(*it)->second += 1;
      }      
      break;
    }
    case EDGE: {

      count.find(parser.getPath()[0])->second += 1;
      count.find(parser.getPath()[1])->second += 1;

      break;
    }
    default:
      break;
    }

    token = parser.getToken();
  }

  fclose(input);
  input = fopen(argv[1],"r");
  output = fopen(argv[3],"w");
  if (output == NULL) {
    stwarning("Could not open file %s. Exiting\n",argv[4]);
    return EXIT_FAILURE;
  }

  NV = 0;
  
  BinaryParser<> parser2(input,edim);
  char c;
  GlobalIndexType index;
  uint32_t header[5];

  header[0] = sizeof(GlobalIndexType);
  header[1] = sizeof(FunctionType);
  header[2] = parser2.gridSize()[0];
  header[3] = parser2.gridSize()[1];
  header[4] = parser2.gridSize()[2];
  fwrite(header,sizeof(uint32_t),5,output);


  token = parser2.getToken();
  while (token != EMPTY) {
    switch (token) {

      case VERTEX:
        c = 'v';
        fwrite(&c,sizeof(char),1,output);

        index = parser2.getId();
        fwrite(&index,sizeof(GlobalIndexType),1,output);
        fwrite(parser2.coord(),sizeof(FunctionType),edim,output);

        NV++;
        break;

      case EDGE:
        c = 'e';
        fwrite(&c,sizeof(char),1,output);
        GlobalIndexType path[2];

        mIt = count.find(parser2.getPath()[0]);
        mIt->second -= 1;
        mIt2 = count.find(parser2.getPath()[1]);
        mIt2->second -= 1;

        path[0] = mIt->first;
        path[1] = mIt2->first;
        fwrite(path,sizeof(GlobalIndexType),2,output);


        if (mIt->second == 0) {
          c = 'f';
          index = mIt->first;
          fwrite(&c,sizeof(char),1,output);
          fwrite(&index,sizeof(GlobalIndexType),1,output);

 		  count.erase(mIt);
        }

        if (mIt2->second == 0) {
          c = 'f';
          index = mIt2->first;
          fwrite(&c,sizeof(char),1,output);
          fwrite(&index,sizeof(GlobalIndexType),1,output);

          count.erase(mIt2);
        }
        break;

      default:
        break;
    }

    token = parser2.getToken();
  }

  fclose(output);
  fclose(input);

  for (mIt=count.begin();mIt!=count.end();mIt++) {
    fprintf(stderr,"Vertex %lld not finalized .... %d instances left\n",mIt->first,mIt->second);
  }
}
