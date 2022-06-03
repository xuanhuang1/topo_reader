/*
 * bin2points.cpp
 *
 *  Created on: Nov 19, 2014
 *      Author: bremer5
 */

#include <cstdio>
#include <cstdlib>
#include <set>
#include <vector>
#include "Definitions.h"

using namespace std;
int main(int argc, const char *argv[])
{
  if (argc < 4) {
    fprintf(stderr,"Usage: %s <input.bin> <dimension> <output.pts>\n",argv[0]);
    return 0;
  }

  FILE* input = fopen(argv[1],"r");

  uint32_t dim = atoi(argv[2]);

  uint32_t index_bytes;
  uint32_t data_bytes;
  uint32_t grid_size[3];
  uint32_t edge[3];
  unsigned char token;

  fread(&index_bytes,sizeof(uint32_t),1,input);
  sterror(index_bytes != sizeof(GlobalIndexType),"Global index type %d-bytes of input does not match with current input of %d-bytes.",sizeof(GlobalIndexType),index_bytes);

  fread(&data_bytes,sizeof(uint32_t),1,input);
  sterror(data_bytes != sizeof(FunctionType),"Data size of executable %d-bytes does not match with current input of %d-bytes.",sizeof(FunctionType),data_bytes);

  fread(grid_size,sizeof(uint32_t),3,input);

  fprintf(stderr,"Index bytes %d\nData bytes %d\nDimensions %d %d %d\n\n",
          index_bytes,data_bytes,grid_size[0],grid_size[1],grid_size[2]);

  std::set<GlobalIndexType> index_set;
  GlobalIndexType index;
  vector<FunctionType> buffer(dim+3);
  vector<FunctionType> data;

  fread(&token,sizeof(char),1,input);
  while (!feof(input)) {

    if (token == 'e') {
      fread(edge,sizeof(GlobalIndexType),2,input);
      //fprintf(stderr,"Skipped edge <%d,%d>\n",edge[0],edge[1]);
    }
    else if (token == 'f')
      fread(edge,sizeof(GlobalIndexType),1,input);

    else if (token == 'v') {

      fread(&index,sizeof(GlobalIndexType),1,input);
      fread(&buffer[0],sizeof(FunctionType),dim+3,input);

      //fprintf(stderr,"Read Vertex %d\n",index);
      if (index_set.find(index) == index_set.end()) {
        index_set.insert(index);
        data.insert(data.end(),buffer.begin()+3,buffer.end());
      }
    }
    else {
      sterror(true,"Unexpected token \"%c\"",token);
    }

    fread(&token,sizeof(char),1,input);
  }
  fclose(input);

  FILE* output = fopen(argv[3],"w");

  uint32_t count = data.size() / dim;

  fprintf(stderr,"Found %d vertices with %d dimensions\n",count,dim);
  fwrite(&count,sizeof(uint32_t),1,output);
  fwrite(&dim,sizeof(uint32_t),1,output);

  fwrite(&data[0],sizeof(FunctionType),data.size(),output);
  fclose(output);

  return 1;
}


