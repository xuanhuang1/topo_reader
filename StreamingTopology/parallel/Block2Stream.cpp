#include "Block2Stream.h"
#include "AddressHandler.h"

#define VERBOSE 1

int do_block(AddressHandler ah, float threshold, float* data, FILE* fout) {

   unsigned int coords[3];
         for (coords[2] = ah.ghosts[4]; coords[2] < ah.local_size[2] - ah.ghosts[5]; coords[2]++) {
      for (coords[1] = ah.ghosts[2]; coords[1] < ah.local_size[1] - ah.ghosts[3]; coords[1]++) {
   for (coords[0] = ah.ghosts[0]; coords[0] < ah.local_size[0] - ah.ghosts[1]; coords[0]++) {

            // local index
            GlobalIndexType id = ah.l_coords_to_l_index(coords);
            // omit low valued vertices
            if (data[id] < threshold) continue;
            //printf("doing %d %d %d\n", coords[0], coords[1], coords[2]);

            // count number of existing neighbors
            int ncoords[3];
            char count = 0;
            for (ncoords[0] = -1; ncoords[0] <= 1; ncoords[0]++) {
               for (ncoords[1] = -1; ncoords[1] <= 1; ncoords[1]++) {
                  for (ncoords[2] = -1; ncoords[2] <= 1; ncoords[2]++) {
                     unsigned int nc[3];
                     nc[0] = coords[0] + ncoords[0];
                     if (nc[0] < 0 || nc[0] >= ah.local_size[0]) continue;
                     nc[1] = coords[1] + ncoords[1];
                     if (nc[1] < 0 || nc[1] >= ah.local_size[1]) continue;
                     nc[2] = coords[2] + ncoords[2];
                     if (nc[2] < 0 || nc[2] >= ah.local_size[2]) continue;

                     GlobalIndexType nid = ah.l_coords_to_l_index(nc);
                     if (nid == id) continue;
                     if (data[nid] < threshold) continue;
                     count++;
                  }
               }
            }

            GlobalIndexType gid = ah.translate_l2g(id);

            // now count has the number of edges this vertex will see, write
            // vertex
            char kind = 'v';
            fwrite(&kind, sizeof(char), 1, fout);
            fwrite(&gid, sizeof(GlobalIndexType), 1, fout);
            fwrite(&count, sizeof(char), 1, fout);
            fwrite(&data[id], sizeof(float), 1, fout);

#if VERBOSE
            fprintf(stderr,"\nk=%c gid=%u count=%d val=%f\n",kind,gid,count,data[id]);
#endif


            // now output lower edges
            for (ncoords[0] = -1; ncoords[0] <= 1; ncoords[0]++) {
               for (ncoords[1] = -1; ncoords[1] <= 1; ncoords[1]++) {
                  for (ncoords[2] = -1; ncoords[2] <= 1; ncoords[2]++) {
                     unsigned int nc[3];
                     nc[0] = coords[0] + ncoords[0];
                     if (nc[0] < 0 || nc[0] >= ah.local_size[0]) continue;
                     nc[1] = coords[1] + ncoords[1];
                     if (nc[1] < 0 || nc[1] >= ah.local_size[1]) continue;
                     nc[2] = coords[2] + ncoords[2];
                     if (nc[2] < 0 || nc[2] >= ah.local_size[2]) continue;

                     GlobalIndexType nid = ah.l_coords_to_l_index(nc);
                     if (nid >= id) continue;
                     if (data[nid] < threshold) continue;
                     GlobalIndexType ngid = ah.translate_l2g(nid);
                     // if the edge goes to a ghost cell, we write that out
                     if (ah.l_is_ghost(nc)) {
                        char tcount = -1;
                        kind = 'v';
                        fwrite(&kind, sizeof(char), 1, fout);
                        fwrite(&ngid, sizeof(GlobalIndexType), 1, fout);
                        fwrite(&tcount, sizeof(char), 1, fout);
                        fwrite(&data[nid], sizeof(float), 1, fout);
#if VERBOSE
                        fprintf(stderr,"\nk=%c gid=%u tcount=%d val=%f\n",kind,ngid,tcount,data[nid]);
#endif

                     }

                     // write edge
                     kind = 'e';
                     fwrite(&kind, sizeof(char), 1, fout);
                     fwrite(&gid, sizeof(GlobalIndexType), 1, fout);
                     fwrite(&ngid, sizeof(GlobalIndexType), 1, fout);

#if VERBOSE
                     fprintf(stderr,"\nk=%c gid=%u ngid=%u\n",kind,gid,ngid);
#endif

                  }
               }
            }
         }
      }
   }

   return 1;
}


int do_block(AddressHandler ah, float threshold, FILE* fin, FILE* fout) {

   float* data = new float[ah.l_block_size()];
   fread(data, sizeof(float), ah.l_block_size(), fin);
   return do_block(ah, threshold, data, fout);
}

