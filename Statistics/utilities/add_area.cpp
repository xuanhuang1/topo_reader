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



float dist(const float* x, const float* y)
{
  return sqrt((x[0]-y[0])*(x[0]-y[0]) + (x[1]-y[1])*(x[1]-y[1]) + (x[2]-y[2])*(x[2]-y[2]));
}

float area(const float* x, const float* y, const float *z)
{
  float a[3],b[3];

  for (uint8_t i=0;i<3;i++) {
    a[i] = y[i] - x[i];
    b[i] = z[i] - x[i];
  }

  float cross[3];

  //fprintf(stderr,"<%f %f %f> x <%f %f %f>\n",a[0],a[1],a[2],b[0],b[1],b[2]);
  cross[0] = a[1]*b[2] - a[2]*b[1];
  cross[1] = a[2]*b[0] - a[0]*b[2];
  cross[2] = a[0]*b[1] - a[1]*b[0];

  return 0.5*fabs(sqrt(cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2]));
}


int main(int argc, char *argv[])
{
  if (argc < 8) {
    fprintf(stderr,"Usage: %s <family> <family_id> <simpl-id> <segmentation> <obj> <dimension> [ascii|binary]\n", argv[0]);
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

  clan.attach(argv[1]);
  family_id = atoi(argv[2]);
  simplification_id = atoi(argv[3]);

  family = clan.family(family_id);
  hierarchy.initialize(family.simplification(simplification_id));

  sclan.attach(argv[4]);
  seg_handle = sclan.family(0).segmentation();
  seg.initialize(seg_handle);

  dimension = atoi(argv[6]);

  if (strcmp(argv[7],"ascii") == 0)
    ascii = true;
  else if (strcmp(argv[7],"binary") == 0)
    ascii = false;
  else {
    fprintf(stderr,"File type \"%s\" not recognized\n",argv[7]);
    exit(0);
  }

  FILE* input = fopen(argv[5],"r");

  vector<float> vertices;
  vector<bool> vflags;
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

  vflags.resize(vcount,false);

  vector<Feature>::const_iterator it;
  vector<GlobalIndexType> feature;
  vector<GlobalIndexType>::iterator vIt;
  ValueArray derived(hierarchy.featureCount());
  uint32_t i;

  uint32_t feature_count = 0;
  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {
    // Reset the flags
    for (i=0;i<vcount;i++)
      vflags[i] = false;

    seg.segmentation(&(*it),feature);
    //fprintf(stderr,"Segment %d with %d elements ",it->id(),feature.size());

    // Mark all vertices
    for (vIt = feature.begin();vIt!=feature.end();vIt++) {
      //fprintf(stdout,"Mark %d \n",*vIt);
      vflags[*vIt] = true;
    }

    float a = 0;
    // Go through all faces
    for (i=0;i<fcount;i++) {

      // If this is an internal face
      if (vflags[faces[3*i]] && vflags[faces[3*i+1]] && vflags[faces[3*i+2]]) {
        a += area(&vertices[dimension*faces[3*i]],
                  &vertices[dimension*faces[3*i+1]],
                  &vertices[dimension*faces[3*i+2]]);

      }
    } // end-all-faces


    if (feature_count % 100 == 0)
      fprintf(stderr,"area %d of %d  = %f\n",feature_count,hierarchy.featureCount(),a);

    derived[feature_count++].mData = a;
    //break;
  }

  StatHandle derived_handle;

  derived_handle.aggregated(true);
  derived_handle.stat(derived[0].typeName());
  derived_handle.species("area");
  derived_handle.encoding(false);

  derived_handle.setData(&derived);

  clan.family(family_id).append(derived_handle);

  return 1;
}
