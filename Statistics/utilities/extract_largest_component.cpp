/*
 * add_circumference.cpp
 *
 *
 *  Created on: Nov 12, 2013
 *      Author: bremer5
 */


#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <string.h>

#include "TalassConfig.h"

using namespace std;

class CountCmp {
public:

  CountCmp(const vector<uint32_t>& count) : mCounts(count) {}

  bool operator()(uint32_t i, uint32_t j) {
    if (mCounts[i] > mCounts[j])
      return true;
    else
      return false;
  }

  const vector<uint32_t>& mCounts;
};

uint32_t rep(vector<uint32_t>& parents,uint32_t i)
{
  if (parents[i] == i)
    return i;

  uint32_t r = rep(parents,parents[i]);

  parents[i] = r;

  return r;
}

void join(vector<uint32_t>& parents,uint32_t i,uint32_t j)
{
  i = rep(parents,i);
  j = rep(parents,j);

  if (i < j)
    parents[i] = j;
  else
    parents[j] = i;
}


int main(int argc, char *argv[])
{
  if (argc < 5) {
    fprintf(stderr,"Usage: %s <obj> <dimension> [ascii | binary] <nr-of-components>\n", argv[0]);
    exit(0);
  }

  uint32_t dimension;
  bool ascii;
  uint32_t components;


  dimension = atoi(argv[2]);

  if (strcmp(argv[3],"ascii") == 0)
    ascii = true;
  else if (strcmp(argv[3],"binary") == 0)
    ascii = false;
  else {
    fprintf(stderr,"File type \"%s\" not recognized\n",argv[7]);
    exit(0);
  }

  components = atoi(argv[4]);

  FILE* input = fopen(argv[1],"r");

  vector<float> vertices;
  vector<uint32_t> parent;
  vector<uint32_t> count;
  vector<uint32_t> faces;
  char* line;
  size_t linecap = 0;
  uint32_t vcount = 0;
  uint32_t fcount = 0;

  if (ascii) {

    while (getline(&line,&linecap,input) > 0) {
      //fprintf(stderr,"%s",line);
      if (line[0] == 'v') {
        float v[3];
        vertices.resize(3*vcount+3);

        sscanf(line,"v %f %f %f",v,v+1,v+2);
        vertices[3*vcount]   = v[0];
        vertices[3*vcount+1] = v[1];
        vertices[3*vcount+2] = v[2];

        vcount++;
      }
      else if (line[0] == 'f') {

        int32_t f[3];

        faces.resize(3*fcount + 3);

        sscanf(line,"f %d %d %d",f,f+1,f+2);

        f[0] = abs(f[0]) - 1;
        f[1] = abs(f[1]) - 1;
        f[2] = abs(f[2]) - 1;

        faces[3*fcount]   = f[0];
        faces[3*fcount+1] = f[1];
        faces[3*fcount+2] = f[2];

        fcount++;
      }
      else {
        assert (false);

      }
    }
  }
  else { // binary input
    char token;

    while (!feof(input)) {
      fread(&token,sizeof(char),1,input);

      if (token == 'v') {
        vertices.resize(dimension*(vcount+1));

        fread(&vertices[dimension*vcount],sizeof(float),dimension,input);

        //fprintf(stderr,"v %f %f %f\n",vertices[dimension*vcount],vertices[dimension*vcount+1],vertices[dimension*vcount+2]);
        vcount++;
      }
      else if (token == 'f') {
        faces.resize(3*fcount + 3);

        int32_t f[3];
        fread(f,sizeof(uint32_t),3,input);
        faces[3*fcount]   = abs((int)f[0]) - 1;
        faces[3*fcount+1] = abs((int)f[1]) - 1;
        faces[3*fcount+2] = abs((int)f[2]) - 1;

        //fprintf(stderr,"f %d %d %d\n",faces[3*fcount],faces[3*fcount+1],faces[3*fcount+2]);

        fcount++;
      }
      else
        assert (false);

    }

  }

  parent.resize(vcount); // flags for the union-find
  count.resize(vcount,0); // counts of vertices / faces

  // initialize all parents
  uint32_t i,j;
  for (i=0;i<vcount;i++)
    parent[i] = i;


  // Do the union-find for all edges
  for (i=0;i<fcount;i++) {

    join(parent,faces[3*i+0],faces[3*i+1]);
    join(parent,faces[3*i+1],faces[3*i+2]);
    join(parent,faces[3*i+2],faces[3*i+0]);

  }

  // Count the vertices in each component
  for (i=0;i<vcount;i++) {
    count[rep(parent,i)]++;
  }

  vector<uint32_t> features;
  CountCmp cmp(count);

  // Get all the final components
  for (i=0;i<vcount;i++) {
    if (parent[i] == i)
      features.push_back(i);
  }

  // Sort them by count
  std::sort(features.begin(),features.end(),cmp);


  for (i=1;i<features.size();i++) {
    if (i == components)
      break;

    fprintf(stderr,"Joining component %d with %d vertices to %d with %d vertices\n",features[i],count[features[i]],
            features[0],count[features[0]]);
    parent[features[i]] = features[0];
  }

  fprintf(stderr,"Largest component has %d vertices of %d\n",count[features[0]], vcount);


  // Reset the counts to indicate the number of triangles
  for (i=0;i<vcount;i++)
    count[i] = 0;

  map<uint32_t,uint32_t> index_map;
  map<uint32_t,uint32_t>::iterator mIt;
  uint32_t id=0;

  // for all faces that are part of the largest component (the union find should be shortcutted
  for (i=0;i<fcount;i++) {
    if ((rep(parent,faces[3*i+0]) == features[0])
        && (rep(parent,faces[3*i+1]) == features[0])
        && (rep(parent,faces[3*i+2]) == features[0])) {

      // Increase the incident count
      for (j=0;j<3;j++)
        count[faces[3*i+j]]++;

    }
  }

  // For the same faces
  char token;
  for (i=0;i<fcount;i++) {
    if ((rep(parent,faces[3*i+0]) == features[0])
        && (rep(parent,faces[3*i+1]) == features[0])
        && (rep(parent,faces[3*i+2]) == features[0])) {

      int32_t f[3];
      for (j=0;j<3;j++) { // FOr all three vertices
        mIt = index_map.find(faces[3*i+j]);
        if (mIt == index_map.end()) { // If the vertex hasn't been outputed
          index_map[faces[3*i+j]] = id++; // Give it a new index
          mIt = index_map.find(faces[3*i+j]);

          // and write it out
          token = 'v';
          fwrite(&token,sizeof(char),1,stdout);
          fwrite(&vertices[dimension*mIt->first],sizeof(float),dimension,stdout);
        }

        // Decrease the incident count
        count[faces[3*i+j]]--;

        // And compute the new index
        if (count[faces[3*i+j]] == 0) // If this was the last triangle make the index negative
          f[j] = -mIt->second-1;
        else
          f[j] = mIt->second+1;
      }

      // write the triangle
      token = 'f';
      fwrite(&token,sizeof(char),1,stdout);
      fwrite(f,sizeof(int32_t),3,stdout);

    }

  }

  fprintf(stderr,"Wrote file with %d vertices\n",id);

  return 1;
}
