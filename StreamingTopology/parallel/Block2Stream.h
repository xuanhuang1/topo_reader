#ifndef BLOCK2STREAM_H
#define BLOCK2STREAM_H

#include <cstdio>

class AddressHandler;


int do_block(AddressHandler ah, float threshold, float* fin, FILE* fout);
int do_block(AddressHandler ah, float threshold, FILE* fin, FILE* fout);



#endif
