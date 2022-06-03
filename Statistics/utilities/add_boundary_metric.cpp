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
#include <algorithm>

#include "TalassConfig.h"
#include "ValueElement.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;

//! A sorted pair of indices marking an edge that compares as un-sorted
class Edge {

public:

  Edge(uint32_t u, uint32_t v) {mIndex[0] = u;mIndex[1] = v;}

  ~Edge() {}

  const uint32_t& operator[](int i) const {return mIndex[i];}

  bool operator<(const Edge& e) const {
    if ((min() < e.min()) || ((min() == e.min()) && (max() < e.max())))
      return true;
    return false;
  }

  uint32_t min() const {return std::min(mIndex[0],mIndex[1]);}
  uint32_t max() const {return std::max(mIndex[0],mIndex[1]);}
//private:

  uint32_t mIndex[2];
};


float dist(const float* x, const float* y)
{
  //fprintf(stderr,"%f %f %f     %f %f %f\n",x[0],x[1],x[2],y[0],y[1],y[2]);
  return sqrt((x[0]-y[0])*(x[0]-y[0]) + (x[1]-y[1])*(x[1]-y[1]) + (x[2]-y[2])*(x[2]-y[2]));
}

int main(int argc, char *argv[])
{
  if (argc < 11) {
    fprintf(stderr,"Usage: %s <family> <family_id> <simpl-id> <segmentation> <obj> <dimension> [binary | ascii] <attribute-name> <attribute-id> <statistic>\n", argv[0]);
    exit(0);
  }

  ClanHandle clan,sclan;
  uint32_t family_id;
  uint32_t simplification_id;
  FamilyHandle family;
  SegmentationHandle seg_handle;
  FeatureHierarchy hierarchy;
  FeatureSegmentation seg;
  uint32_t dimension;
  bool ascii;
  uint32_t attribute;
  string attribute_name;
  string statistic;
  FunctionType parameter;

  clan.attach(argv[1]);
  family_id = atoi(argv[2]);
  simplification_id = atoi(argv[3]);

  family = clan.family(family_id);
  hierarchy.initialize(family.simplification(simplification_id));

  dimension = atoi(argv[6]);

  if (strcmp(argv[7],"ascii") == 0)
    ascii = true;
  else if (strcmp(argv[7],"binary") == 0)
    ascii = false;
  else {
    fprintf(stderr,"File type \"%s\" not recognized\n",argv[7]);
    exit(0);
  }

  attribute_name = string(argv[8]);
  attribute = atoi(argv[9]);
  statistic = string(argv[10]);

  if (argc > 11)
    parameter = atof(argv[11]);

  sclan.attach(argv[4]);
  seg_handle = sclan.family(0).segmentation();
  seg.initialize(seg_handle);


  FILE* input = fopen(argv[5],"r");

  vector<float> vertices;
  vector<bool> vflags;
  vector<uint32_t> faces;
  vector<bool> eflags;
  map<Edge,uint32_t> edge_map;
  map<Edge,uint32_t>::iterator mIt;
  char* line;
  size_t linecap = 0;
  uint32_t vcount = 0;
  uint32_t fcount = 0;
  uint32_t ecount = 0;


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
        f[0]   = abs((int)f[0]) - 1;
        f[1] = abs((int)f[1]) - 1;
        f[2] = abs((int)f[2]) - 1;

        faces[3*fcount]   = f[0];
        faces[3*fcount+1] = f[1];
        faces[3*fcount+2] = f[2];

        //fprintf(stderr,"f %d %d %d\n",faces[3*fcount],faces[3*fcount+1],faces[3*fcount+2]);

        fcount++;
        for (uint8_t i=0;i<3;i++) {
          if (edge_map.find(Edge(f[i],f[(i+1)%3])) == edge_map.end())
            edge_map[Edge(f[i],f[(i+1)%3])] = ecount++;
        }
      }
      else
        assert (false);

    }

  }

  vflags.resize(vcount,false);
  eflags.resize(ecount,false);

  Factory fac;
  Attribute* agg = fac.make_aggregator(statistic);


  vector<Feature>::const_iterator it;
  vector<GlobalIndexType> feature;
  vector<GlobalIndexType>::iterator vIt;
  ValueArray derived(hierarchy.featureCount());
  vector<FunctionType> cdf;
  uint32_t i,k;

  uint32_t feature_count = 0;
  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    if ((argc <= 11) || (!it->living(parameter)))
      continue;

    //if (it->id() != 6650)
   //   continue;

    // Reset the flags
    for (i=0;i<vcount;i++)
      vflags[i] = false;

    for (i=0;i<ecount;i++)
      eflags[i] = false;

    seg.segmentation(&(*it),feature);
    //if (it->id() == 126)
    fprintf(stderr,"Segment %d with %d elements \n",it->id(),feature.size());

    // Mark all vertices
    for (vIt = feature.begin();vIt!=feature.end();vIt++) {
      //fprintf(stderr,"Mark %d %f\n",*vIt,vertices[dimension*(*vIt)+3]);
      vflags[*vIt] = true;
    }

    // Go through all faces
    for (i=0;i<fcount;i++) {

      // If this is an internal face
      if (vflags[faces[3*i]] && vflags[faces[3*i+1]] && vflags[faces[3*i+2]]) {
        // Go through its edges and mark them
        for (k=0;k<3;k++) {
            // invert its edge flag
          Edge e(faces[3*i+k],faces[3*i+(k+1)%3]);
          uint32_t eindex = edge_map.find(e)->second;

          eflags[eindex] = !eflags[eindex];
          //if ((e[0] == 9) || (e[1] == 9))
          //fprintf(stderr,"\teflags[]  %d %d  %d\n",e[0],e[1],(bool)eflags[eindex]);
        }
      }
    } // end-all-faces


    set<uint32_t> boundary;
    set<uint32_t>::iterator sIt;
    for (mIt=edge_map.begin();mIt!=edge_map.end();mIt++) {
      if (eflags[mIt->second]) {
        //fprintf(stderr,"e: %d %d \n",mIt->first.mIndex[0],mIt->first.mIndex[1]);

        boundary.insert(mIt->first[0]);
        boundary.insert(mIt->first[1]);
      }
    }

    agg->reset();
    for (sIt=boundary.begin();sIt!=boundary.end();sIt++) {
      agg->addVertex(vertices[dimension*(*sIt)+attribute],0);
    }

    fprintf(stderr,"metric = %f  computed from %d vertices\n",agg->value(),boundary.size());

    if (argc <= 11)
      derived[feature_count++].mData = agg->value();
    else if (!boundary.empty())
      cdf.push_back(agg->value());
    //break;
  }

  if (argc <= 11) {

    StatHandle derived_handle;

    attribute_name.insert(0,"Boundary_");
    derived_handle.aggregated(true);
    derived_handle.stat(agg->typeName());
    derived_handle.species(attribute_name);
    derived_handle.encoding(false);

    derived_handle.setData(&derived);

    clan.family(family_id).append(derived_handle);
  }
  else {
    sort(cdf.begin(),cdf.end());

    for (int i=0;i<cdf.size();i++) {
      fprintf(stdout,"%f %f\n",cdf[i],(i+1)/(float)cdf.size());
    }

  }

  return 1;
}
