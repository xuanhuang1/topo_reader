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
#include <ctime>
#include <vector>
#include <algorithm>

#include "Definitions.h"
#include "Node.h"
#include "Statistics/AggregatorFactory.h"

using namespace std;

typedef SegmentAggregator<float> Agg;

enum FamilyType {
  MAXIMA_FAMILY = 0,
  MINIMA_FAMILY = 1
};

FamilyType gFamilyType = MAXIMA_FAMILY;

//! Class to encode a single feature 
class Feature {
  
public:

  //! Number of attributes stored for each feature
  static const int sNumberOfAttributes = 0;

  //! Default constructor
  Feature(const vector<Agg*>& global_attributes);

  //! Default destructor which frees the memory
  ~Feature();

  float mLifeSpan[2];
  uint32_t mParent;
  vector<Agg*> mAttributes;
};

Feature::Feature(const vector<Agg*>& global_attributes) 
{
  mLifeSpan[0] = 0;
  mLifeSpan[1] = 1;
  mParent = 0;
  mAttributes.resize(global_attributes.size());

  for (uint32_t i=0;i<mAttributes.size();i++) 
    mAttributes[i] = global_attributes[i]->clone();
}

Feature::~Feature()
{
  for (vector<Agg*>::iterator it=mAttributes.begin();it!=mAttributes.end();it++) 
    delete *it;
}

struct orderCmp {
  static std::vector<std::vector<float> >* ref;

  bool operator()(int i,int j) {return ((*ref)[i][0] < (*ref)[j][0]);}
};
std::vector<std::vector<float> >* orderCmp::ref = NULL;

//! Wrapper to throw reasonable exception at file read-in
FILE* openFile(const char* filename,const char* options)
{
  FILE* f = fopen(filename,options);
  if (f == NULL) {
    fprintf(stderr,"Could not open file \"%s\"\n",filename);
    exit(0);
  }

  return f;
}

int extractTimeFromFilename(const char *filename)
{
  const char* searchString = "_t_";
  char const* head = filename;
  char num[50];
  int len = strlen(filename);
  int i;

  while ((head - filename < len) && (strncmp(head,searchString,3) != 0)) 
    head++;

  if (head == filename+len) {
    fprintf(stderr,"Could not determine time step of file \"%s\"\n",filename);
    return 0;
  }

  head += 3;

  i = 0;
  while ((head - filename < len) && isdigit(head[0])) {
    num[i++] = *head;
    head++;
  }

  num[i] = '\0';

  return atoi(num);
}

void aggregateFamily(FILE* input, FILE* output, const vector<Agg*>& attributes)
{
  uint32_t n;
  char line[2000];
  uint32_t indices[3];
  float life_span[2];
  std::vector<std::vector<float> > data;
  std::vector<uint32_t> order;

  fscanf(input,"NodeCount %d\n",&n);

  fwrite(&n,sizeof(uint32_t),1,output);
  
  // Ignore the Range info
  fscanf(input,"Range %f %f\n",life_span,life_span+1);
  
  // Ignore the Attribute info
  fgets(line,200,input);

  data.resize(n);
  order.resize(n);
  for (uint32_t i=0;i<n;i++) {
    data[i].resize(2+attributes.size());

    fscanf(input,"%d %d %d",indices,indices+1,indices+2);
    fscanf(input,"%f %f",&data[i][0],&data[i][1]);

    order[i] = n-1-i;

    for (uint32_t j=0;j<attributes.size();j++) {
      attributes[j]->infoFromASCII(input);
      data[i][j+2] = attributes[j]->value();
    }
  }

  
  // Create on order by inceasing birth value
  orderCmp::ref = &data;
  std::sort(order.begin(),order.end(),orderCmp());

  // Write the features in increasing birth order
  for (uint32_t i=0;i<n;i++) 
    fwrite(&data[order[i]][0],sizeof(float),attributes.size()+2,output);
 
}  

int main(int argc, const char** argv)
{
  FILE* input;
  FILE* output;
  char line[200];
  int nr_of_attributes;
  vector<Agg*> attributes;
  Aggregator::AggregatorFactory<FunctionType> factory;
  uint32_t t;
  vector<Feature> family;
  time_t rawtime;
  struct tm* timeinfo;
  float range[2] = {gMaxValue,gMinValue};
  float r[2];
  
  if (argc < 3)  {
    fprintf(stderr,"No input families specified.\n");
    return 1;
  }

  input = openFile(argv[1],"r");
  
  // Ignore the NodeCount info
  fgets(line,200,input);

  // Ignore the Range infor
  fgets(line,200,input);
 
  fscanf(input,"%s %d",line,&nr_of_attributes);
  
  attributes.resize(nr_of_attributes);
  for (int i=0;i<nr_of_attributes;i++) {
    fscanf(input,"%s",line);
    attributes[i] = factory.make(line);
    sterror(attributes[i]==NULL,"Could not identify attribute \"%s\"\n",line);

    fscanf(input,"%s",line);
    attributes[i]->attributeName(std::string(line));
  }
  fclose(input);

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  output = openFile(argv[argc-1],"w");
  fprintf(output,"#\n");
  fprintf(output,"# File created by \"%s\" with arguments\n",argv[0]);
  fprintf(output,"# \"%s\" ... \"%s\"\n",argv[1],argv[argc-2]);
  fprintf(output,"# %s",asctime (timeinfo) );
  fprintf(output,"#\n");
  
  fprintf(output,"Attributes %d ",nr_of_attributes);
  for (int i=0;i<nr_of_attributes;i++) {
    attributes[i]->typeToASCII(output);
    fprintf(output," ");
  }
  fprintf(output,"\n");
  
  // First we accumulated the range
  for (int i=1;i<argc-1;i++) {
    fprintf(stderr,"Scanning family \"%s\"\n",argv[i]);
    input = openFile(argv[i],"r");
    
    fscanf(input,"NodeCount %d\n",&t);
    
    fscanf(input,"Range %f %f\n",r,r+1);

    range[0] = std::min(r[0],range[0]);
    range[1] = std::max(r[1],range[1]);
    
    fclose(input);
  }
  fprintf(output,"Range %f %f\n",range[0],range[1]);

  fprintf(output,"FamilyCount %d\n",argc-2);
  for (int i=1;i<argc-1;i++) {
    fprintf(stderr,"Aggregating on family \"%s\"\n",argv[i]);
    t = extractTimeFromFilename(argv[i]);
    fwrite(&t,sizeof(uint32_t),1,output);

    input = openFile(argv[i],"r");
    aggregateFamily(input,output,attributes);
    fclose(input);
  }
  
  fclose(output);

  for (int i=0;i<nr_of_attributes;i++) 
    delete attributes[i];
  

  return 1;
}


