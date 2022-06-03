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
  FILE* input = fopen(argv[2],"rb");
  

  if (strcmp(argv[1],"float") == 0) {

    float f[10];

    fread(f,sizeof(float),10,input);
    fclose(input);

    fprintf(stderr,"Raw values\n");
    for (int i=0;i<10;i++) 
      fprintf(stderr,"Vertex %d value %e\n",i,f[i]); 


    fprintf(stderr,"\nSwapped values\n");
    
    for (int i=0;i<10;i++) {
      swap4((unsigned char*)(f+i));
      fprintf(stderr,"Vertex %d value %e\n",i,f[i]); 
    }
  }
  else if (strcmp(argv[1],"double") == 0) {

    double f[10];

    fread(f,sizeof(float),10,input);
    fclose(input);

    fprintf(stderr,"Raw values\n");
    for (int i=0;i<10;i++) 
      fprintf(stderr,"Vertex %d value %e\n",i,f[i]); 


    fprintf(stderr,"\nSwapped values\n");
    
    for (int i=0;i<10;i++) {
      swap8((unsigned char*)(f+i));
      fprintf(stderr,"Vertex %d value %e\n",i,f[i]); 
    }
  }
    

  return 1;
}
