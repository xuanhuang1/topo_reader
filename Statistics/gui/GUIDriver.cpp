#include "GUIDriver.h"
#include <time.h>

void portableSleep(uint32_t secs) {
  for (time_t t = time(0) + secs; time(0) < t; ) {}
}



GUIDriver::GUIDriver(Engine* eg, bool useExistingOptFile, const char *optFileName) {
  mUpdateStateManually = false;
  mStateUpdated = false;

  if(useExistingOptFile) mGuiOptFilename = std::string(optFileName);
  else mGuiOptFilename = std::string("flexgui_opt.txt");
  mGuiInFilename = std::string("flexgui_in.txt");
  mGuiOutFilename = std::string("flexgui_out.txt");
  mEngineFilename = std::string("enginefile.txt");
  mSelectedSegmentFilename = std::string("selectedSegment.txt");


  // clear engine file
  FILE *enginefile = fopen(mEngineFilename.c_str(), "w");
  // force all ids active
  fprintf(enginefile, "%d\n", -1);
  fclose(enginefile);

  FILE *selectedSegmentFile = fopen(mSelectedSegmentFilename.c_str(), "w");
  fclose(selectedSegmentFile);
  mSelectedSegment = -1;


  mEngine = eg;


  if(!useExistingOptFile)  {
    AttributeVariableList features = mEngine->getGlobalAttributeVariables();
    FILE *guiOptionsFile = fopen(mGuiOptFilename.c_str(), "w");
    for(uint32_t i=0; i < features.size(); i++) {
      fprintf(guiOptionsFile, "%s %s 0 \n", features[i].attribute().c_str(), features[i].variable().c_str());
    }
    fclose(guiOptionsFile);
  } else {
    std::set<LocalIndexType> attributesToLoad = identifyAttributeVariablesToLoad();
    mEngine->initializeAttributeVariablesFromList(attributesToLoad);
    if(attributesToLoad.size() > 0) {
      AttributeVariableList features = mEngine->getGlobalAttributeVariables();
      LocalIndexType id = *(attributesToLoad.begin());
      mEngine->setPlotFeature(features[id].attribute(), features[id].variable());
    }
  }


  writeGuiInFile();
  portableSleep(3);


//  std::cout << "Please edit file: " << mGuiOptFilename << " to add sliders  & checkbuttons for features/attributes of interest" << std::endl;


#ifdef WIN32
  system("start python flexgui.py");
#else
  system("python flexgui.py &");
#endif
  portableSleep(3);

  struct stat fileInfo;
  while (stat(mGuiOptFilename.c_str(), &fileInfo) != 0) {
    fprintf(stderr,"Waiting for \"%s\" file to appear\n",mGuiOptFilename.c_str());
    portableSleep(3);
  }
  sterror(stat(mGuiOptFilename.c_str(), &fileInfo)!= 0, "couldn't get status of %s", mGuiOptFilename.c_str());
  //mLastGuiOptTime = fileInfo.st_mtime;

  while (stat(mGuiOutFilename.c_str(), &fileInfo) != 0) {
    fprintf(stderr,"Waiting for \"%s\" file to appear\n",mGuiOutFilename.c_str());
    portableSleep(3);
  }
  sterror(stat(mGuiOutFilename.c_str(), &fileInfo)!= 0, "couldn't get status of %s", mGuiOutFilename.c_str());
  //mLastGuiOutTime = fileInfo.st_mtime;
  sterror(stat(mSelectedSegmentFilename.c_str(), &fileInfo)!= 0, "couldn't get status of %s", mSelectedSegmentFilename.c_str());
  //mLastSelectedSegmentTime = fileInfo.st_mtime;
  mLastSelectedSegmentTime = 0;
  mLastGuiOutTime = 0;
  mLastGuiOptTime = 0;

  mEngine->printState();

}


GUIDriver::~GUIDriver() { }



bool GUIDriver::guiOptUpdated() {
   struct stat fileInfo;
   // Use stat() to get the info
   // in while because it can fail if writing at the time
   while(stat(mGuiOptFilename.c_str(), &fileInfo)!= 0); //std::cout << "p";

   if(difftime(fileInfo.st_mtime, mLastGuiOptTime) > 0) {
     mLastGuiOptTime = fileInfo.st_mtime;
     std::cout << mGuiOptFilename << " was modified : " << std::ctime(&mLastGuiOptTime);
     return true;
   }
   return false;
}


bool GUIDriver::guiOutUpdated() {

   struct stat fileInfo;
   // Use stat() to get the info
   // in while because it can fail if writing at the time
   while(stat(mGuiOutFilename.c_str(), &fileInfo)!= 0); //std::cout << "u";

   if(difftime(fileInfo.st_mtime, mLastGuiOutTime) > 0) {
     mLastGuiOutTime = fileInfo.st_mtime;
     std::cout << mGuiOutFilename << " was modified : " << std::ctime(&mLastGuiOutTime);
     mStateUpdated = false;
     //std::cout << "state updated = false -- gui out modified" << std::endl;
     return true;
   }
   return false;
}


bool GUIDriver::selectedSegmentUpdated() {

   struct stat fileInfo;
   // Use stat() to get the info
   // in while because it can fail if writing at the time
   while(stat(mSelectedSegmentFilename.c_str(), &fileInfo)!= 0) std::cout << "v";

   if(difftime(fileInfo.st_mtime, mLastSelectedSegmentTime) > 0) {
     mLastSelectedSegmentTime = fileInfo.st_mtime;
     std::cout << mSelectedSegmentFilename << " was modified : " << std::ctime(&mLastSelectedSegmentTime);
     return true;
   }

   return false;
}



uint32_t GUIDriver::aggregatorToInt(const std::string &agg) {
 std::cout << "agg == featureCount" << (agg == std::string("featureCount")) << std::endl;

  if(agg == std::string("min")) return 0;
  if(agg == std::string("max")) return 1;
  if(agg == std::string("mean")) return 2;
  if(agg == std::string("featureCount")) return 3;
  if(agg == std::string("minParamWithCount")) return 4;
  if(agg == std::string("maxParamWithCount")) return 5;
  return (uint32_t) -1;
}


std::string GUIDriver::intToAggregator(uint32_t id) {
  if(id == 0) return std::string("min");
  if(id == 1) return std::string("max");
  if(id == 2) return std::string("mean");
  if(id == 3) return std::string("featureCount");
  if(id == 4) return std::string("minParamWithCount");
  if(id == 5) return std::string("maxParamWithCount");
  return std::string("");
}



void GUIDriver::writeGuiInFile() {
  FILE *guiFile = fopen(mGuiInFilename.c_str(), "w");
  std::cout << "writing gui file " << std::endl;
  // write threshold slider
  fprintf(guiFile, "#START\n");
  FunctionType increment;
  if(mEngine->getParameterRange().Increment(50) < 0.1) increment = mEngine->getParameterRange().Increment(50);
  else increment = 0.1;
  fprintf(guiFile, "{SLIDER (Parameter, %e:%e, s=%e, i=%e)}\n", mEngine->getParameterRange().Min(), mEngine->getParameterRange().Max(), increment, mEngine->getParameter());
  if(mEngine->getNumSimplificationSequences() > 1) {
    fprintf(guiFile, "{SLIDER (simpSequence, 0:%e, s=1, i=%e)}\n", static_cast<double>(mEngine->getNumSimplificationSequences()-1), static_cast<double>(mEngine->getSimplificationID()));
  }
  fprintf(guiFile, "{FLAG, Restrict Computations To Active Time Step, i=%d}\n", (int)mEngine->getRestrictToActiveTimeStep());
  // write active timestep slider
  fprintf(guiFile, "{SLIDER (Active Timestep, %e:%e, s=%e, i=%e)}\n", mEngine->getTimeWindow().Min(), mEngine->getTimeWindow().Max(), 1.0, (float) mEngine->getActiveTimeStep());
  // write graph resolution slider
  fprintf(guiFile, "{SLIDER (Graph Resolution, 2:400, s=1, i=%e)}\n", (float) mEngine->getPlotResolution());
  // write CDF/Histogram/Timeseries radio buttons
  fprintf(guiFile, "{MULTI, PlotType, CDF, WCDF, HISTOGRAM, TIMESERIES, THRESHOLD, i=%d}\n", (int)(mEngine->getPlotType()-1));
  // write display flags
  fprintf(guiFile, "{FLAG, Draw Log Scale X, i=%d}\n", (int)mEngine->getXLogScale());
  fprintf(guiFile, "{FLAG, Draw Log Scale Y, i=%d}\n", (int)mEngine->getYLogScale());
  // write Cross family aggregator radio buttons
  fprintf(guiFile, "{MULTI, Cross-family Aggregator, min, max, mean, featureCount, minParamWithCount, maxParamWithCount, i=%d}\n", aggregatorToInt(mEngine->getCrossFamilyAggregatorName()));

  // write Cross clan aggregator radio buttons
  fprintf(guiFile, "{MULTI, Cross-clan Aggregator, min, max, mean, i=%d}\n", aggregatorToInt(mEngine->getCrossClanAggregatorType()));

  std::string names = std::string("");

  std::map<LocalIndexType, Range > activeFilters = mEngine->getActiveFilters();

  for(std::map<LocalIndexType, Range >::iterator itr=activeFilters.begin(); itr != activeFilters.end(); itr++) {
    Range curRange = itr->second;
    LocalIndexType id = itr->first;
    std::string cleanName = mEngine->getGlobalAttributeVariables().cleanedUpName(id);
    // write write feature/BaseAttribute min & max slider
    FunctionType increment;
    if(curRange.Increment(500) < 0.1) increment = curRange.Increment(500);
    else increment = 0.1;
    fprintf(guiFile, "{SLIDER (%s MIN, %e:%e, s=%e, i=%e)}\n", cleanName.c_str(), curRange.Min(), curRange.Max(), increment, curRange.Min());
    fprintf(guiFile, "{SLIDER (%s MAX, %e:%e, s=%e, i=%e)}\n", cleanName.c_str(), curRange.Min(), curRange.Max(), increment, curRange.Max());
    if(names != std::string("")) names += ", ";
    names += cleanName;
  }

  LocalIndexType index = 0;
  for(std::map<LocalIndexType, Range >::iterator itr=activeFilters.begin(); itr != activeFilters.end(); itr++, index++) {
    std::cout << itr->first << " and plot id = " << mEngine->getPlotAttributeID() << std::endl;
    if(itr->first == mEngine->getPlotAttributeID()) break;
  }
  if(names != std::string("")) fprintf(guiFile, "{MULTI, Plot Feature, %s, i=%d}\n", names.c_str(), index);

  fprintf(guiFile, "#END");
  fclose(guiFile);
}



void GUIDriver::loadStateFromFile() {
  std::cout << "Loading state from file for Statistics Browser" << std::endl;

  FILE *guiFile = fopen(mGuiOutFilename.c_str(), "r");

  char label[256];
  fscanf(guiFile, "%s", label);
  sterror(strcmp(label, "#START") != 0, "Gui output file appears to be corrupt");

  std::map<LocalIndexType, Range > activeFilters = mEngine->getActiveFilters();
  AttributeVariableList globalAttributeVariables = mEngine->getGlobalAttributeVariables();


  // threshold : gui object slider
  FunctionType threshold;
  fscanf(guiFile, "%f", &threshold);


  FunctionType simpID = 0;
  if(mEngine->getNumSimplificationSequences() > 1) {
    fscanf(guiFile, "%f", &simpID);
  }
  // restrictToActive: flag
  int restrictToActive;
  fscanf(guiFile, "%d", &restrictToActive);

  //activeTimestep : gui object slider
  FunctionType activeTimestep;
  fscanf(guiFile, "%f", &activeTimestep);

  // graphResolution : gui object slider
  FunctionType plotResolution;
  fscanf(guiFile, "%f", &plotResolution);

  // plotType : gui object radio button indicating CDF, HISTOGRAM, TIMESERIES, or PARAMTER
  int plotTypeID;
  fscanf(guiFile, "%d", &plotTypeID);
  std::string plotType = plotTypeToString((PlotType) (plotTypeID+1));

  // dispXLogScale, dipYLogScale:  bool flags: gui flags (both can be on/off)
  int xLogScale, yLogScale;
  fscanf(guiFile, "%d", &xLogScale);
  fscanf(guiFile, "%d", &yLogScale);

  int crossClanAggregatorID, crossFamilyAggregatorID;
  fscanf(guiFile, "%d", &crossFamilyAggregatorID);
  fscanf(guiFile, "%d", &crossClanAggregatorID);
  std::string crossFamilyAggregator = intToAggregator(crossFamilyAggregatorID);
  std::string crossClanAggregator = intToAggregator(crossClanAggregatorID);

  FunctionType minR, maxR;
  std::map<LocalIndexType, Range > newActiveFilters;
  std::map<LocalIndexType, std::string> filterFeature, filterAttribute;
  for(std::map<LocalIndexType, Range >::iterator itr=activeFilters.begin(); itr != activeFilters.end(); itr++) {
    fscanf(guiFile, "%f", &minR);
    fscanf(guiFile, "%f", &maxR);
    newActiveFilters[itr->first] = Range(minR, maxR);
    filterFeature[itr->first] = globalAttributeVariables[itr->first].attribute();
    filterAttribute[itr->first] = globalAttributeVariables[itr->first].variable();
  }

  int plotFeatureID;
  std::string plotFeature, plotAttribute;
  if(activeFilters.size() > 0) {
    // read in axisID
    fscanf(guiFile, "%d", &plotFeatureID);

    std::map<LocalIndexType, Range >::iterator itr=activeFilters.begin();
    for(int id=0; id < plotFeatureID; id++, itr++) {
      if(itr == activeFilters.end()) break;
    }
    if(itr != activeFilters.end()) {
      plotFeature = globalAttributeVariables[itr->first].attribute();
      plotAttribute = globalAttributeVariables[itr->first].variable();
    } else {
      plotFeature = gHierarchyDefined;
      plotAttribute = std::string("");
    }
  } else {
      plotFeature = gHierarchyDefined;
      plotAttribute = std::string("");
  }


  fscanf(guiFile, "%s", label);

  fclose(guiFile);
  sterror(strcmp(label, "#END") != 0, "Gui output file appears to be corrupt");

  fprintf(stderr,"Loaded plottype %s\n",plotType.c_str());
  // initialize variables
  mEngine->setPlotType(plotType);
  mEngine->setPlotResolution(plotResolution);
  mEngine->setActiveTimeStep(activeTimestep);
  mEngine->setRestrictToActiveTimeStep(restrictToActive);
  mEngine->setXLogScale( (bool) xLogScale);
  mEngine->setYLogScale( (bool) yLogScale);

  mEngine->setPlotFeature(plotFeature, plotAttribute);
  FamilyAggregator agg(crossFamilyAggregator);
  mEngine->setCrossFamilyAggregator(agg);
  mEngine->setCrossClanAggregator(crossClanAggregator);

  bool rewriteGui = mEngine->setParameter(threshold, static_cast<LocalIndexType>(simpID));

  for(std::map<LocalIndexType, Range >::iterator itr=newActiveFilters.begin(); itr != newActiveFilters.end(); itr++) {
    mEngine->setFilter(filterFeature[itr->first], filterAttribute[itr->first], itr->second.Min(), itr->second.Max());
  }



  if(!mUpdateStateManually) {
    mEngine->updateState();
    mEngine->writeSubSelectedIDsToFile();
  }

  if(rewriteGui) writeGuiInFile();

}


std::set<LocalIndexType> GUIDriver::identifyAttributeVariablesToLoad() const {
  FILE *guiFile = fopen(mGuiOptFilename.c_str(), "r");

  AttributeVariableList globalAttributeVariables = mEngine->getGlobalAttributeVariables();
  std::set<LocalIndexType> attributesToLoad;
  std::cout << "feature attribute list = " << globalAttributeVariables << std::endl;

  for(LocalIndexType i = 0; i < globalAttributeVariables.size(); i++) {
    char line[256]; // The next line in the file
    char attribute[256]; // BaseAttribute name
    uint8_t length = globalAttributeVariables.mList[i].attribute().size(); // Length of the value type name
    int val; // Flag

    // Read the next line
    fgets(line,256,guiFile);

    // Make sure the value type is the one we are expecting
    sterror(globalAttributeVariables.mList[i].attribute().compare(0, length, line, 0, length) != 0,
                "Option File contents not in order. Expected value type \"%s\" but got value type \"%s\"",
                globalAttributeVariables.mList[i].attribute().c_str(),line);

    // Starting after he value type string parse the attribute name and the flag
    sscanf(line+length,"%s %d",attribute,&val);

    if(atoi(attribute) == 1) attributesToLoad.insert(i);
    if(val == 1) attributesToLoad.insert(i);
  }

  fclose(guiFile);
  return attributesToLoad;

}


std::string GUIDriver::getStatus() const {
  if(mUpdateStateManually && !mStateUpdated) {
    std::cout << "getting status -- mStateUpdated = " << mStateUpdated << std::endl;
    return std::string("Press the update state button when you are ready to update the plot.");
  }

  else return gValidState;
}


std::string GUIDriver::loadSelectedSegmentFromFile() {
  std::string info;
  FILE* selectedSegmentFile = fopen(mSelectedSegmentFilename.c_str(), "r");
  if(!feof(selectedSegmentFile))
  {
    fscanf(selectedSegmentFile, "%d", &mSelectedSegment);
    if(mSelectedSegment != -1) info = mEngine->getSelectedSegmentInformation(mSelectedSegment);
    std::cout << info << std::endl;
  }
  fclose(selectedSegmentFile);
  return info;
}


