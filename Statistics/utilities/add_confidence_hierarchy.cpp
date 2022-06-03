
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "ClanHandle.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"
#include "FeatureData.h"
#include "IntegratedFunctionValue.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;

class Sample {
public:

  Sample(FunctionType qq, FunctionType vv) : q(qq), v(vv) {}
  FunctionType q;
  FunctionType v;
};

class Pair {
public:
  Pair(Feature* ff, FunctionType con) : f(ff), c(con) {}
  Feature* f;
  FunctionType c;
};

FunctionType coefficient_of_determination(vector<Sample>& graph)
{
  // A graph with 2 or less points always admits a perfect linear fit
  if (graph.size() < 3)
    return 1;

  FunctionType mean_q = 0;
  FunctionType mean_v = 0;
  FunctionType cov = 0;
  FunctionType stdev_q = 0;
  FunctionType stdev_v = 0;
  uint32_t i;

  for (i=0;i<graph.size();i++) {
    mean_q += graph[i].q;
    mean_v += graph[i].v;
  }

  mean_q /= graph.size();
  mean_v /= graph.size();

  for (i=0;i<graph.size();i++) {
    cov += (graph[i].q - mean_q) * (graph[i].v - mean_v);
    stdev_q += (graph[i].q - mean_q)*(graph[i].q - mean_q);
    stdev_v += (graph[i].v - mean_v)*(graph[i].v - mean_v);
  }

  if ((stdev_q == 0) || (stdev_v == 0))
    fprintf(stderr,"break\n");

  stdev_q = sqrt(stdev_q);
  stdev_v = sqrt(stdev_v);

  return std::min((FunctionType)1,pow(cov / (stdev_q*stdev_v),2));
}

int change_to_confidence(Feature* f, SumArray& volume)
{
  vector<Sample> graph;
  stack<Pair> path;
  Pair top(NULL,0);

  FunctionType confidence;
  FunctionType max_confidence = 1;

  while (true) {

    if (f->id() == 103194)
      fprintf(stderr,"break\n");

    // If this is the first sample we see or if this a sample with a new OW
    // threshold, we add it to the graph.
    if (graph.empty() || (graph.back().q < f->lifeTime()[1]))
      graph.push_back(Sample(f->lifeTime()[1],volume[f->id()].value()));
    else {
      // If this sample has the same OW threshold it will in fact have a larger volume.
      // This will be problematic for computing the coefficient of determination since
      // a graph in which all samples lie on the same (or very similar x-coordinates
      // cannot be fitted. Thus, we simply replace the last sample instead of adding a
      // new one.
      graph.back().q = f->lifeTime()[1];
      graph.back().v = volume[f->id()].value();
    }

    confidence = coefficient_of_determination(graph);

    sterror(isnan(confidence),"Coeffficient of determination is nan.");

    path.push(Pair(f,confidence));

    if ((f->repSize() > 0) && (f->agent() == f->rep(0)->agent()))
      f = f->rep(0);
    else
      break;

  }

  max_confidence = 0;
  while (!path.empty()) {
    top = path.top();

    max_confidence = std::max(max_confidence,top.c);
    path.pop();
    if (!path.empty()) {
      path.top().c = std::max(path.top().c,max_confidence);
      sterror(top.c > path.top().c,"Inverted confidence interval found %f %f.",top.c,path.top().c);
      top.f->lifeTime(0,top.c);
    }
    else {
      top.f->lifeTime(1,1);
    }
  }

  return 1;
}

void inflate_lifetime(Feature* f)
{
  FunctionType life[2];

  if (f->repSize() == 0) { // For a root
    life[0] = 0.0;
    life[1] = f->lifeTime()[1];
  }
  else {
    life[0] = f->rep(0)->lifeTime()[1];
    life[1] = std::max(f->lifeTime()[1],life[0]);
  }

  f->lifeTime(life[0], life[1]);
  for (uint32_t i=0;i<f->conSize();i++)
    inflate_lifetime(f->con(i));
}

int main(int argc, const char* argv[])
{
  if (argc < 2) {
    fprintf(stderr,"Usage: %s <family-file> [min-q]\n", argv[0]);
    return 0;
  }

  float min_q = -1e39;

  if (argc > 2)
    min_q = atof(argv[2]);


  ClanHandle clan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;
  StatHandle volume_handle;
  SumArray volume;

  clan.attach(argv[1]);
  family = clan.family(0);

  // For the moment we assume that we are given preaggregated volumes to work with
  if (!family.providesAggregate("sum","volume")) {
    fprintf(stderr,"Could not find the necessary aggregated volume attribute in family.\n");
    return 0;
  }

  // Get the handle to the volume information
  volume_handle = family.aggregate("sum","volume");

  // and read in the data
  volume.resize(volume_handle.elementCount());
  volume_handle.readData(&volume);


  // Read in the hierarchy
  hierarchy.initialize(family.simplification(0));

  int count = 0;
  int count2 = 0;
  // For all leafs of the tree
  for (uint32_t i=0;i<hierarchy.featureCount();i++) {
    if (hierarchy.feature(i)->conSize() == 0) {
      // change the life-times to confidence levels
      change_to_confidence(hierarchy.feature(i),volume);
      count++;
    }
    if (hierarchy.feature(i)->repSize() == 0)
      count2++;
  }

  fprintf(stderr,"Found %d leafs and %d roots\n",count,count2);

  // For all roots of the trees
  for (uint32_t i=0;i<hierarchy.featureCount();i++) {
    hierarchy.feature(i)->direction(false);

    if (hierarchy.feature(i)->repSize() == 0) {
      inflate_lifetime(hierarchy.feature(i));
    }

  }


  // Now create a new handle for the confidence hierarchy
  SimplificationHandle confidence;
  confidence.metric("Confidence");
  confidence.fileType(SINGLE_REPRESENTATIVE);
  confidence.setRange(0.5,1);

  // CHECK FIX
  // the simplification sequence is based off of filedata
  FeatureData data(&hierarchy.allFeatures());
  confidence.setData(&data);

  // and append it as the second simplification to the family.
  // Note that this call already writes the data to the file
  clan.family(0).append(confidence);

  return 1;
}

