#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include "plotViewerUI.h"
#include "FL/Fl_File_Chooser.H"

GLuint paletteID;
const char* gInputFileName = NULL;
const char* gOptFileName = NULL;
bool gUseExistingOptFile = false;
bool gUseThreshold = false;
float gThreshold = -1.0;

#define NUM_OPTIONS 4
static const char* gOptions[NUM_OPTIONS] = {
  "--help",
  "--i",
  "--option_file",
  "--threshold",
};


void print_help(FILE* output,char* exec)
{
  fprintf(output,"Usage: %s [options]\nWhere options can be any of the following:\n\n",exec);

  fprintf(output,"--i <filename>\n\
  \tRequired: Name of file containing the input data.\n");

  fprintf(output,"--option_file <filename>\n\
  \tOptional: Name of file with initial gui options set.\n");

  fprintf(output,"--threshold <value>\n\
  \tOptional: Initial threshold parameter value.\n");
};

int parse_command_line(int argc, char** argv)
{
  int i,j,option;

  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < NUM_OPTIONS;j++) {
      if(strcmp(gOptions[j],argv[i])==0)
        option= j;
    }
    
    switch (option) {
      
    case -1:  // Wrong input parameter
      fprintf(stderr,"\nError: Wrong input parameter \"%s\"\nTry %s --help\n\n",argv[i],argv[0]);
      return 0;
    case 0:   // --help 
      print_help(stdout,argv[0]);
      return 0;
    case 1:   // --i
      gInputFileName = argv[++i];
      break;
    case 2:   // --option_file
      gUseExistingOptFile = true;
      gOptFileName = argv[++i];
      break;
    case 3:   // --threshold
      gUseThreshold = true;
      gThreshold = atof(argv[++i]);
      break;
  default:
      break;
    }
  }
  return 1;
};

int main(int argc, char **argv) {
  std::cout << "Got here!!!!" << std::endl;
  if(argc == 1) {
    fprintf(stderr,"\nError: Try %s --help\n\n",argv[0]);
    return 0;
  }

  if(!parse_command_line(argc, argv)) {
     return 0;
  }

  plotViewerUI *pvui=new plotViewerUI(gInputFileName, gUseExistingOptFile, gOptFileName, gUseThreshold, gThreshold);	
  Fl::visual(FL_DOUBLE|FL_INDEX);
  pvui->show(argc, argv);

  int retVal = Fl::run();
  return retVal; 
}
