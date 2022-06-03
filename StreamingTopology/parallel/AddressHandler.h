#ifndef ADDRESSHANDLER_H
#define ADDRESSHANDLER_H

#include <cstdio>
#include "TalassConfig.h"

using namespace std;

class AddressHandler {

public:
   unsigned int* global_base; // the start coords in global space of the local block
   unsigned int* global_size; // the global sizes
   unsigned int* local_size; // the local sizes
   unsigned int* ghosts; // to be used to determine what is in and out of a block

   AddressHandler(unsigned int* gb, unsigned int* gs, unsigned int* ls, unsigned int* gh) :
      global_base(gb), global_size(gs), local_size(ls), ghosts(gh) {}

   GlobalIndexType translate_l2g(GlobalIndexType id) {
      GlobalIndexType d[3];
      GlobalIndexType accum = 1;
      // get global coordinates
      for (int i = 0; i < 3; i++) {
         d[i] = (id % (local_size[i] * accum)) / accum; // local coord
         d[i] += global_base[i]; // convert to global coord
         accum *= local_size[i];
      }

      // get global index
      accum = 1;
      GlobalIndexType res = 0;
      for (int i = 0; i < 3; i++) {
         res += d[i] * accum;
         accum *= global_size[i];
      }
      return res;
   }

   GlobalIndexType l_block_size() {
      return local_size[0] *
         local_size[1] *
         local_size[2];
   }

   bool l_is_ghost(unsigned int* coords) {
      if (coords[0] < ghosts[0] ||
            coords[0] >= local_size[0] - ghosts[1] ||
            coords[1] < ghosts[2] ||
            coords[1] >= local_size[1] - ghosts[3] ||
            coords[2] < ghosts[4] ||
            coords[2] >= local_size[4] - ghosts[5])
         return true;
      return false;
   }

   GlobalIndexType l_coords_to_l_index(unsigned int* coords) {
      GlobalIndexType accum = 1;
      GlobalIndexType res = 0;
      for (int i = 0; i < 3; i++) {
         res += coords[i] * accum;
         accum *= local_size[i];
      }
      return res;
   }

   
};

#endif
