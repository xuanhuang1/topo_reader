#include <cstdio>
#include <cstdlib>
#include <cstring>

void swap4(unsigned char* v)
{
  unsigned char tmp;

  tmp = v[3];
  v[3] = v[0];
  v[0] = tmp;

  tmp = v[2];
  v[1] = v[1];
  v[1] = tmp;
}

void swap8(unsigned char* v)
{
  unsigned char tmp;

  for (int i=0;i<4;i++) {

    tmp = v[7-i];
    v[7-i] = v[i];
    v[i] = tmp;
  }
}

int main(int argc, char* argv[])
{
  if (argc < 11) {
    fprintf(stderr,"Usage: %s <input> nx ny ny start_x start_y start_z stop_x stop_y stop_z [swap]\n",argv[0]);
    return 1;
  }

  int dimension[3];
  int start[3];
  int stop[3];
  float *buffer;
  
  for (int i=0;i<3;i++) {
    dimension[i] = atoi(argv[2+i]);
    start[i] = atoi(argv[5+i]);
    stop[i] = atoi(argv[8+i]);
  }

  FILE* input = fopen(argv[1],"rb");
  
  buffer = new float[stop[0] - start[0]];
  for (int k=start[2];k<stop[2];k++) {
    for (int j=start[1];j<stop[1];j++) {
      fseek(input,(k*dimension[1]*dimension[0] + j*dimension[0] + start[0])*sizeof(float),SEEK_SET);

      fread(buffer,sizeof(float),stop[0]-start[0],input);
      fwrite(buffer,sizeof(float),stop[0]-start[0],stdout);
    }
  }

  delete[] buffer;

  return 1;
}
