#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Engine.h"

using namespace std;
using namespace Statistics;

#define COMMANDLINE_OPTIONS 16
static const char* gCommandLineOptions[COMMANDLINE_OPTIONS] = {
  "--help",
  "--i",
  "--printAttributeVariables",
  "--threshold",
  "--plotResolution",
  "--plotType",
  "--plotFeature",
  "--crossFamilyAggregator",
  "--crossClanAggregator",
  "--activeTimestep",
  "--filter", 
  "--XLogScale", 
  "--YLogScale", 
  "--o",
  "--restrictToActiveTimestep",
  "--simplificationID",
};

// global variables
const string UNINIT = string("");
string gFilename = UNINIT;
string gOutputFilename = UNINIT;
string gPlotType = string("CDF"); 
string gPlotFeature = UNINIT;
string gPlotAttribute = UNINIT;
FamilyAggregator gCrossFamilyAggregator;
string gCrossClanAggregator = string("min");
bool gRestrictToActiveTimeStep = false;
vector<string> gFilterFeature;
vector<string> gFilterAttribute;
vector<float> gFilterMin;
vector<float> gFilterMax;
bool gPrintAttributeVariables = false;
float gActiveTimestep = 0.0;
float gActiveThreshold = -1.0;
int gPlotResolution = 20;
int gSimplificationID = 0;
bool gXLogScale = false;
bool gYLogScale = false;
Engine *gEngine;

void PrintCommandLineHelp(ostream &outfile, char *exec);
int ParseCommandLine(int argc, char** argv);

int main(int argc, char **argv) {
  if (ParseCommandLine(argc,argv) == 0) return 0;

  if(gFilename == UNINIT) {
    std::cout << "Error: must provide an input filename" << std::endl;
    return 0;
  }

  gEngine = new Engine(gFilename);

  if(gPrintAttributeVariables) {
    gEngine->printAvailableAttributeVariables();
    return 0;
  }

  // Unless we use a hierarchy defined feature load plot feature/attributes
  if (gPlotFeature != UNINIT)
    gEngine->loadAttributeVariable(gPlotFeature, gPlotAttribute);

  
  // load all filter feature/attributes
  for(uint32_t i=0; i < gFilterFeature.size(); i++) {
    gEngine->loadAttributeVariable(gFilterFeature[i], gFilterAttribute[i]); 
  }


  // initialize variables
  gEngine->setPlotType(gPlotType);

  if (gPlotFeature != UNINIT)
    gEngine->setPlotFeature(gPlotFeature, gPlotAttribute);

  gEngine->setPlotResolution(gPlotResolution);
  gEngine->setCrossFamilyAggregator(gCrossFamilyAggregator);
  gEngine->setCrossClanAggregator(gCrossClanAggregator);
  gEngine->setRestrictToActiveTimeStep(gRestrictToActiveTimeStep);
  gEngine->setActiveTimeStep(gActiveTimestep);
  gEngine->setXLogScale(gXLogScale);
  gEngine->setYLogScale(gYLogScale);

 
  if(gActiveThreshold != -1.0) gEngine->setParameter(gActiveThreshold, gSimplificationID);
  for(uint32_t i=0; i < gFilterFeature.size(); i++) {
    gEngine->setFilter(gFilterFeature[i], gFilterAttribute[i], gFilterMin[i], gFilterMax[i]);
  }

  gEngine->updateState();

  gEngine->saveActivePlot();
  gEngine->savePlotToPLT(gOutputFilename);

  return 0;
}

int ParseCommandLine(int argc, char** argv) {
   int i, j, option;
   if(argc == 1) {
      fprintf(stderr,"\nError: No input parameters specified! \nTry %s --help\n\n",argv[0]);
      return 0;
   }

  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < COMMANDLINE_OPTIONS;j++) {
      if(strcmp(gCommandLineOptions[j],argv[i])==0)
      option= j;
    }
    
    switch (option) {
    case -1:  // Wrong input parameter
      fprintf(stderr,"\nError: Wrong input parameter \"%s\" \nTry %s --help\n\n",argv[i],argv[0]);
      return 0;
    case 0:   // --help 
      PrintCommandLineHelp(cout, argv[0]);
      return 0;
    case 1:   // --i
      gFilename = string(argv[++i]);
      break;
    case 2:   // --printAttributeVariables
      gPrintAttributeVariables = true;
      break;
    case 3:   // --threshold
      gActiveThreshold = atof(argv[++i]);
      break;
    case 4:   // --plotResolution
      gPlotResolution = atoi(argv[++i]);
      break;
    case 5:   // --plotType
      gPlotType = string(argv[++i]);
      break;
    case 6:   // --plotFeature
      gPlotFeature = string(argv[++i]);
      gPlotAttribute = string(argv[++i]);
      break;
    case 7: {   // --crossFamilyAggregator
      std::string name;
      std::vector<float> parameters;

      name = string(argv[++i]);

      // Potentially the aggregator is a hierarchy property.
      // In this case we try to read some additional parameters
      while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0))
        parameters.push_back(atof(argv[++i]));

      // Now we can create the actual family aggregator
      gCrossFamilyAggregator = FamilyAggregator(name,parameters);

      break;
    }
    case 8:   // --crossClanAggregator
      gCrossClanAggregator = string(argv[++i]);
      break;
    case 9:   // --activeTimestep
      gActiveTimestep = atof(argv[++i]);
      break;
    case 10:   // --filter
      gFilterFeature.push_back(string(argv[++i]));
      gFilterAttribute.push_back(string(argv[++i]));
      gFilterMin.push_back(atof(argv[++i]));
      gFilterMax.push_back(atof(argv[++i]));
      break;
    case 11:   // --plotLogX
      gXLogScale = true;
      break;
    case 12:   // --plotLogY
      gYLogScale = true;
      break;
    case 13:   // --o
      gOutputFilename = string(argv[++i]);
      break;
    case 14:   // --restrictToActiveTimestep
      gRestrictToActiveTimeStep = true;
      break;
    case 15:   // --simplificationID
      gSimplificationID = atoi(argv[++i]);
      break;
    }
  }
   return 1;
}

void PrintCommandLineHelp(ostream &outfile, char *exec) {
  outfile << "Usage: " << exec << " [options]\nWhere options can be any of the following:\n\n" << endl;
  outfile << "--help:                                          prints list of available commands." << endl;
  outfile << "--i: <filename>                                  input filename." << endl;
  outfile << "--o: <filename>                                  output filename." << endl;
  outfile << "--printAttributeVariables:                        print the list of feature-attribute pairs available for exploration." << endl;
  outfile << "--threshold <value>:                             set the threshold value." << endl;
  outfile << "--plotResolution <value>:                        set the plot resolution. " << endl;
  outfile << "--plotType <CDF | HIST| TIME | PARAM>:           set the plot type." << endl;
  outfile << "--plotFeature <feature> <attribute>:             set the axis of interest for the current plot." << endl;
  outfile << "                                                 to plot feature count type: --plotFeature featureCount" << endl;
  outfile << "                                                 and make sure to set plotType to TIME or PARAM." << endl;
  outfile << "--crossFamilyAggregator <min | max | mean>:      set the cross family aggregator." << endl;
  outfile << "--crossClanAggregator <min | max | mean>:        set the cross clan aggregator." << endl;
  outfile << "--activeTimestep <val>:                          set the active time step." << endl;
  outfile << "--filter <feature> <attribute> <min> <max>:      set the filter for a feature-attribute pair." << endl;
  outfile << "--XLogScale:                                     plot the log of x." << endl;
  outfile << "--YLogScale:                                     plot the log of y." << endl;
  outfile << "--restrictToActiveTimestep:                      restrict computations and plots to active timestep." << endl;
  outfile << "                                                 note: when using this flag plotType should be set to CDF, HIST, or PARAM." << endl;
  outfile << "--simplificationID <value>:                      set the simplification sequence id (default value is 0)" << endl;
} 


