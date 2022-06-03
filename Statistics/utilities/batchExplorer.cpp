#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Engine.h"

using namespace std;
using namespace Statistics;

#define COMMANDLINE_OPTIONS 2
static const char* gCommandLineOptions[COMMANDLINE_OPTIONS] = {
  "--help",
  "--i",
};

#define COMMAND_OPTIONS 22
static const char* gCommandOptions[COMMAND_OPTIONS] = {
  "quit",
  "help",
  "printAttributeVariables",
  "printState",
  "loadAttributeVariable",
  "unloadAttributeVariable",
  "setActiveTimestep",
  "setThreshold",
  "setPlotResolution", 
  "setPlotType",
  "setPlotFeature",
  "setCrossFamilyAggregator",
  "setCrossClanAggregator",
  "clearSavedPlots",
  "savePlotToPLT",
  "clearRangeSelection",
  "filter",
  "saveActivePlot", 
  "setXLogScale", 
  "setYLogScale", 
  "updateState", 
  "setRestrictToActiveTimeStep",
};

// global variables
string gFilename;
Engine *gEngine;

void PrintCommandLineHelp(ostream &outfile, char *exec);
int ParseCommandLine(int argc, char** argv);
int ProcessCommand();
void PrintCommandHelp();
void PrintInstructions();
vector<string> Tokenize(const string &command);

int main(int argc, char **argv) {
  if (ParseCommandLine(argc,argv) == 0) return 0;

  gEngine = new Engine(gFilename);

  while(ProcessCommand()) {}

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
      fprintf(stderr,"\nError: Wrong input parameter \nTry %s --help\n\n",argv[0]);
      return 0;
    case 0:   // --help 
      PrintCommandLineHelp(cout, argv[0]);
      return 0;
    case 1:   // --i
      gFilename = string(argv[++i]);
      break;
    }
  }
   return 1;
}

void PrintCommandLineHelp(ostream &outfile, char *exec) {
  outfile << "Usage: " << exec << " [options]\nWhere options can be any of the following:\n\n" << endl;
  outfile << "--help:                   prints help message." << endl;
  outfile << "--i: <filename>           input filename." << endl;
} 

void PrintInstructions() { 
  cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  //cout << "INCLUDE PROGRAM DESCRIPTION BLURB HERE" << endl; 
  cout << "Enter a command (type help for command options, or quit to exit the program):" << endl; 
  cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  cout << "> ";
} 
 
void PrintCommandHelp() { 
  cout << "quit:                                          quit the program." << endl; 
  cout << "help:                                          print list of available commands." << endl; 
  cout << "printAttributeVariables:                        print the list of feature-attribute pairs available for exploration." << endl;
  cout << "printState:                                    print the active and global states." << endl;
  cout << "loadAttributeVariable <feature> <attribute>:    set a feature-attribute pair to active or inactive for exploration." << endl;
  cout << "unloadAttributeVariable <feature> <attribute>:  set a feature-attribute pair to active or inactive for exploration." << endl;
  cout << "setActiveTimestep <val>:                       set the active time step." << endl;
  cout << "setThreshold <value> <simpID>:                 set the threshold value." << endl;
  cout << "setPlotResolution <value>:                     set the plot resolution. " << endl;
  cout << "setPlotType <CDF | HIST | TIME | PARAM>:       set the plot type." << endl;
  cout << "setPlotFeature <feature> <attribute>:          set the axis of interest for the current plot." << endl;
  cout << "                                               to plot feature count type: setPlotFeature featureCount" << endl;
  cout << "                                               and make sure to setPlotType to TIME or PARAM." << endl;
  cout << "setCrossFamilyAggregator <stat-name> |" << endl;
  cout << "       <property-name> [<float> .. <float>]:   set the cross family aggregator." << endl;
  cout << "setCrossClanAggregator <min | max | mean>:     set the cross clan aggregator." << endl;
  cout << "clearSavedPlots:                               clear all saved plots." << endl;
  cout << "savePlotToPLT <filename>:                      write current saved plots to a gnuplot file." << endl;
  cout << "clearRangeSelection:                           clear range subselections." << endl;
  cout << "filter <feature> <attribute> <min> <max>:      set the filter for a feature-attribute pair." << endl;
  cout << "saveActivePlot:                                save the current state as a plot." << endl;
  cout << "setXLogScale <true | false>:                   display x in log scale." << endl;
  cout << "setYLogScale <true | false>:                   display y in log scale." << endl;
  cout << "updateState:                                   update active state." << endl;
  cout << "setRestrictToActiveTimeStep <true | false>:    restrict computations to active timestep." << endl;
  cout << "                                               note: when true plotType should be set to CDF, HIST, or PARAM." << endl;
}

vector<string> Tokenize(const string &command) {
  vector<string> tokenizedCommand;
  string token1, token2;
  istringstream iss1(command);
  while ( getline(iss1, token1, ' ') ) {
    string tokenStr1 = string(token1);
    if(tokenStr1 != string("") && tokenStr1 != string(" ")) {
      istringstream iss2(tokenStr1);
      while ( getline(iss2, token2, '\t') ) {
        string tokenStr2 = string(token2);
        if(tokenStr2 != string("") && tokenStr2 != string("\t")) {
          tokenizedCommand.push_back(token2);
          //cout << "pushing back '" << token2  << "'" << endl;
        }
      }
    }
  }
  return tokenizedCommand;
}

int ProcessCommand() {

  PrintInstructions();

  string command;
  getline(cin, command);
  vector<string> tokenizedCommand = Tokenize(command);
  //cout << "tokenized command contains " << tokenizedCommand.size() << " tokens." << endl;

  // find first word and compare to list
  int option = -1;
  if(tokenizedCommand.size() > 0) {
    for(int j=0; j < COMMAND_OPTIONS;j++) {
      if(string(gCommandOptions[j]) == tokenizedCommand[0]) { 
        option = j;
        break;
      }
    }
  }
    
  switch(option) {
   case -1:
     cout << "Error: command didn't match available commands.  Type help to see a list of available options." << endl;
     break;
   case 0:   // quit 
     cout << "Exiting the program!" << endl;;
     return 0;
   case 1:   // help 
     PrintCommandHelp();
     break;
   case 2:   // printAttributeVariables
     gEngine->printAvailableAttributeVariables();
     break;
   case 3:   // printActiveState
     gEngine->printState();
     break;
   case 4:   // loadAttributeVariable
     if(tokenizedCommand.size() != 3) {
      cout << "loadAttributeVariable syntax incorrect.  Type help to see proper syntax." << endl;
      return 1;
     }
     gEngine->loadAttributeVariable(tokenizedCommand[1], tokenizedCommand[2]);
     break;
   case 5:   // unloadAttributeVariable
     if(tokenizedCommand.size() != 3) {
      cout << "unloadAttributeVariable syntax incorrect.  Type help to see proper syntax." << endl;
      return 1;
     }
     gEngine->unloadAttributeVariable(tokenizedCommand[1], tokenizedCommand[2]);
     break;
   case 6:   // setActiveTimestep
     if(tokenizedCommand.size() != 2)  cout << "setActiveTimestep syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setActiveTimeStep(atof(tokenizedCommand[1].c_str()));
     break;
   case 7:   // setThreshold
     if(tokenizedCommand.size() != 3)  cout << "setThreshold syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setParameter(atof(tokenizedCommand[1].c_str()), atoi(tokenizedCommand[2].c_str()));
     break;
   case 8:   // setPlotResolution
     if(tokenizedCommand.size() != 2)  cout << "setPlotResolution syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setPlotResolution(atoi(tokenizedCommand[1].c_str()));
     break;
   case 9:   // setPlotType
     if(tokenizedCommand.size() != 2) cout << "setPlotType syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setPlotType(tokenizedCommand[1]);
     break;
   case 10:  // setPlotFeature
     // Changed structure to remove unecessary special case for feature count
     // This functionality has been replaced by the option to choose a
     // hierarchy property as a family aggregator
     // 09/21/10 ptb
     if(tokenizedCommand.size() != 3) cout << "setPlotFeature syntax incorrect.  Type help to see proper syntax." << endl;
     gEngine->setPlotFeature(tokenizedCommand[1], tokenizedCommand[2]);
     break;
   case 11: // setCrossFamilyAggregator
     // Added possible parameters to the aggregator
     if(tokenizedCommand.size() < 2) cout << "setCrossFamilyAggregator syntax incorrect.  Type help to see proper syntax." << endl;
     else {
       std::vector<float> params;
       for (uint32_t k=2;k<tokenizedCommand.size();k++)
         params.push_back(atof(tokenizedCommand[k].c_str()));

       gEngine->setCrossFamilyAggregator(FamilyAggregator(tokenizedCommand[1],params));
     }
     break;
   case 12:  // setCrossClanAggregator
     if(tokenizedCommand.size() != 2) cout << "setCrossClanAggregator syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setCrossClanAggregator(tokenizedCommand[1]);
     break;
   case 13:  // clearSavedPlots
     if(tokenizedCommand.size() != 1) cout << "clearSavedPlots syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->clearSavedPlots();
     break;
   case 14:  // savePlotToPLT
     if(tokenizedCommand.size() != 2) cout << "savePlotToPLT syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->savePlotToPLT(tokenizedCommand[1]);
     break;
   case 15:  // clearRangeSelection
     if(tokenizedCommand.size() != 1) cout << "clearRangeSelection syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->clearRangeSelection();
     break;
   case 16:  // filter
     if(tokenizedCommand.size() != 5) cout << "filter syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->setFilter(tokenizedCommand[1], tokenizedCommand[2], atof(tokenizedCommand[3].c_str()), atof(tokenizedCommand[4].c_str()));
     break;
   case 17:  // saveActivePlot
     if(tokenizedCommand.size() != 1) cout << "saveActivePlot syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->saveActivePlot();
     break;
   case 18:  // setXLogScale
     if(tokenizedCommand.size() != 2) cout << "setXLogScale syntax incorrect.  Type help to see proper syntax." << endl;
     else {
       if(tokenizedCommand[1] == std::string("true")) gEngine->setXLogScale(true);
       else if(tokenizedCommand[1] == std::string("false")) gEngine->setXLogScale(false);
       else cout << "setXLogScale syntax incorrect.  Type help to see proper syntax." << endl;
     }
     break;
   case 19:  // setYLogScale
     if(tokenizedCommand.size() != 2) cout << "setYLogScale syntax incorrect.  Type help to see proper syntax." << endl;
     else {
       if(tokenizedCommand[1] == std::string("true")) gEngine->setYLogScale(true);
       else if(tokenizedCommand[1] == std::string("false")) gEngine->setYLogScale(false);
       else cout << "setYLogScale syntax incorrect.  Type help to see proper syntax." << endl;
     }
     break;
   case 20: // updateState
     if(tokenizedCommand.size() != 1) cout << "updateState syntax incorrect.  Type help to see proper syntax." << endl;
     else gEngine->updateState();
     break;
   case 21: // setRestrictToActiveTimeStep
     if(tokenizedCommand.size() != 2) cout << "setRestrictToActiveTimeStep syntax incorrect.  Type help to see proper syntax." << endl;
     else {
       if(tokenizedCommand[1] == std::string("true")) gEngine->setRestrictToActiveTimeStep(true);
       else if(tokenizedCommand[1] == std::string("false")) gEngine->setRestrictToActiveTimeStep(false);
       else cout << "setRestrictToActiveTimeStep syntax incorrect.  Type help to see proper syntax." << endl;
     }
     break;

  }
  return 1;
} 

