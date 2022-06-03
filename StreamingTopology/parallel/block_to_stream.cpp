#include <cstdio>
#include "AddressHandler.h"
#include "Block2Stream.h"

float* make_block(float* gdata, 
      int X, int Y, int Z, 
      int sX, int sY, int sZ, 
      int lX, int lY, int lZ) {
   int lsize = lX * lY * lZ;

   float* data = new float[lsize];
   for (int x = 0; x < lX; x++) {
      for (int y = 0; y < lY; y++) {
         for (int z = 0; z < lZ; z++) {
            data[x + y * lX + z * lX * lY] =
               gdata[ (x + sX) + (y + sY) * X + (z + sZ) * X * Y];
         }
      }
   }
   return data;
}

int main(int argc, char** argv) {

   if(argc < 8) {
      printf("USAGE: argv[0] inputfile threshold X Y Z nbx nby nbz\n");
      return 1;
   }

   char* filename = argv[1];
   float threshold;
   int nbx, nby, nbz;
   unsigned int global_base[3];
   unsigned int global_size[3];
   unsigned int local_size[3];
   unsigned int ghosts[6];

   sscanf(argv[2], "%f", &threshold);
   sscanf(argv[3], "%d", &global_size[0]);
   sscanf(argv[4], "%d", &global_size[1]);
   sscanf(argv[5], "%d", &global_size[2]);
   sscanf(argv[6], "%d", &nbx);
   sscanf(argv[7], "%d", &nby);
   sscanf(argv[8], "%d", &nbz);

   float* gdata = new float[global_size[0]*global_size[1]*global_size[2]];
   FILE* fin = fopen(filename, "rb");
   fread(gdata, sizeof(float), 
         global_size[0]*global_size[1]*global_size[2], fin);
   fclose(fin);


   //at this point only global_size is initialized, but this just 
   //sets pointers to arrays
   AddressHandler ah(global_base, global_size, local_size, ghosts);
  
   int xblock = global_size[0] / nbx + (global_size[0] % nbx ? 1:0);
   int yblock = global_size[1] / nby + (global_size[1] % nby ? 1:0);
   int zblock = global_size[2] / nbz + (global_size[2] % nbz ? 1:0);

   for (int x = 0; x < nbx; x++) {
      if (x == 0) {
         global_base[0] = 0;
         ghosts[0] = 0;
      } else {
         global_base[0] = x * xblock - 1;
         ghosts[0] = 1;
      }
      if (x == nbx-1) {
         local_size[0] = global_size[0] - global_base[0];
         ghosts[1] = 0;
      } else {
         local_size[0] = 1 + (x+1) * xblock - global_base[0];
         ghosts[1] = 1;
      }
   
/*     printf("gs=%d, gb=%d, ls=%d, gh=[%d, %d]\n",
           global_size[0], 
           global_base[0],
           local_size[0], ghosts[0], ghosts[1]);
  */    

      for (int y = 0; y < nby; y++) {
         if (y == 0) {
            global_base[1] = 0;
            ghosts[2] = 0;
         } else {
            global_base[1] = y * yblock - 1;
            ghosts[2] = 1;
         }
         if (y == nby-1) {
            local_size[1] = global_size[1] - global_base[1];
            ghosts[3] = 0;
         } else {
            local_size[1] = 1 + (y+1) * yblock - global_base[1];
            ghosts[3] = 1;
         }
         for (int z = 0; z < nbz; z++) {

            if (z == 0) {
               global_base[2] = 0;
               ghosts[4] = 0;
            } else {
               global_base[2] = z * zblock - 1;
               ghosts[4] = 1;
            }
            if (z == nbz-1) {
               local_size[2] = global_size[2] - global_base[2];
               ghosts[5] = 0;
            } else {
               local_size[2] = 1 + (z+1) * zblock - global_base[2];
               ghosts[5] = 1;
            }

            // local data block
            float* data = make_block(gdata,
                  global_size[0], global_size[1], global_size[2],
                  global_base[0], global_base[1], global_base[2],
                  local_size[0], local_size[1], local_size[2]);

            // now do the streaming
    //        do_block(ah, threshold, data, stdout);

         }
      }
   }

   return 1;

   //printf("hello world\n");
   // load a test dataset
   //float* testdata = new float[GSIZE*GSIZE*GSIZE];
   //for (int i = 0; i < GSIZE*GSIZE*GSIZE; i++)
   //   testdata[i] = ((float) i) / (GSIZE*GSIZE*GSIZE);
   //FILE* testfile = fopen("test.raw", "wb");
   //fwrite(testdata, sizeof(float), GSIZE*GSIZE*GSIZE, testfile);
   //fclose(testfile);

   //testfile = fopen("test.raw", "rb");
  // FILE* stream = fopen("out.bobj", "wb");

   //printf("hello world\n");
   //do_block(ah, -0.2f, testfile, stream);

   //printf("hello world\n");
   //fclose(testfile);
   //fclose(stream);

   //printf("hello world 1\n");
   //return 1;
};

