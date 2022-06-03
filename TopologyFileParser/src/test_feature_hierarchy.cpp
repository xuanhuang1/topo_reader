#include <cstdio>
#include "FeatureHierarchy.h"
#include "ClanHandle.h"

using namespace TopologyFileFormat;

int main(int argc, char* argv[])
{
  //FILE* output = stdout;
    
  if (argc < 2) {
    fprintf(stderr,"Usage: %s <family-file> [parameter].\n",argv[0]);
    return 0;
  }

    
  
  ClanHandle clan;
  FeatureHierarchy hierarchy;
  FeatureHierarchy::ActiveIterator it;
  FeatureHierarchy::LivingIterator it2;

  clan.attach(argv[1]);
  
  hierarchy.initialize(clan.family(0).simplification(0));
    
  fprintf(stderr,"Read hierarchy within parameters [%f, %f]\n",hierarchy.minParameter(),hierarchy.maxParameter());

 
  float f;
  int count = 0;
  for (int i=2;i<argc;i++) {
    f = atof(argv[i]);

    fprintf(stderr,"\nParameter %f   \n\n",f);
    
    
     if (false) {
      for (it=hierarchy.beginActive(f);it!=hierarchy.endActive();it++) {
        fprintf(stderr,"[%u %u %u   %f %f]  ",it->id(),it.rep()->id(),it.agent()->id(),it->lifeTime()[0],it->lifeTime()[1]);
        count++;
      }
    }
    else {
      for (it2 =hierarchy.beginLiving(f);it2!=hierarchy.endLiving();it2++) {
        fprintf(stderr,"[%u %u  %f %f]  \n",it2->id(),it2->agent()->id(),it2->lifeTime()[0],it2->lifeTime()[1]);
        count++;
      }
    }
    
    
    fprintf(stderr,"\n%d features\n",count);
  }

  
  /*
  float low = 0;
  float high = 17;
  float delta = (high - low) / 20;
  int i;

  for (i=0;i<1;i++) {
    f = low + i*delta;
    hierarchy.parameter(f);
    fprintf(stderr,"Parameter %f    # of features %d\n",f,hierarchy.livingSize());
  }

  for (i=1;i>=0;i--) {
    f = low + i*delta;
    hierarchy.parameter(f);
    fprintf(stderr,"Parameter %f    # of features %d\n",f,hierarchy.livingSize());
  }
  */

  return 1;
}
