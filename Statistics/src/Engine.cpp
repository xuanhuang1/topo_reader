#include "Engine.h"
namespace Statistics {


void Engine::initFamily(uint32_t timestep, bool useThreshold, float threshold) {

  // For now we assume there exists only one family in the file so we always
  // process the first family
  mFamilies[timestep].initializeFamily(mReaders[timestep],useThreshold, threshold, 0);

  std::vector<std::pair<std::string, std::string> > attributeList = mReaders[timestep].attributes();
  std::vector<std::pair<std::string, std::string> > accumulatedFeatureList = mReaders[timestep].aggregates();

  AttributeVariableList localAttributeVariableList = AttributeVariableList(attributeList, accumulatedFeatureList);
  if(timestep == 0) {
    mGlobalAttributeVariables = localAttributeVariableList;
    mParameterRange.resize(mReaders[timestep].numSimplifications());
    mParameter.resize(mReaders[timestep].numSimplifications());
  }

  else {
    // make sure all files contain same attributes - if not quit!!!
    sterror(!(mGlobalAttributeVariables == localAttributeVariableList), "Error files have different attribute variable lists - quitting!");
  }

  for(uint32_t j=0; j < mReaders[timestep].numSimplifications(); j++) {
    FunctionType range[2];
    mReaders[timestep].simplification(j).range(range);
    Range parameterRange(range[0],range[1]);
    mParameterRange[j].UpdateRange(parameterRange);
    std::cout << "updating prameterRange " << j << " with " << range[0] << ", " << range[1] << std::endl;
  }

  mGlobalRanges.resize(mGlobalAttributeVariables.size());

  // set active state variables
  mActiveTimeStep = 0;
  for(uint32_t j=0; j < mParameter.size(); j++) {
    mParameter[j] = mFamilies[mActiveTimeStep].getParameter(j);
  }
  //std::cout << "parameter = " << mParameter << std::endl;
  mPlotAttributeID = mGlobalAttributeVariables.size();


}


Engine::Engine(const std::string &dataFileName, bool useThreshold, float threshold) {
  // open data file and read in num files + all filenames
  // initialize each of the readers accoridngly
  mSimplificationID = 0;
  std::string line;

  int numFiles;
  {
    FILE *fp = fopen(dataFileName.c_str(), "r");
    if(!fp)
    {
      std::cout<<std::endl<<"!!!!!"<<std::endl<<\
        "Can't open file - please recheck parameters/data and start again."<<\
        std::endl<<dataFileName.c_str()<<std::endl<<"!!!!!"<<std::endl;
      std::cin.get();
      exit(0);
    }
    else
      fclose(fp);
  }

  FILE *datafile = fopen(dataFileName.c_str(), "r");

  fscanf(datafile, "%d", &numFiles);
  char filename[512];

  std:: cout << "read in data file that there are " << numFiles  << " timesteps. " << std::endl;

  mReaders.resize(numFiles);
  mFamilies.resize(numFiles);
  // set global state variables
  mTimeWindow.UpdateRange(Range(0, numFiles-1));
  for(int i=0; i < numFiles; i++) {
    fscanf(datafile, "%s", filename);
    std::cout << "initializing data file " << filename << std::endl;
    {
      FILE *fp = fopen(filename, "r");
      if(!fp)
      {
        std::cout<<std::endl<<"!!!!!"<<std::endl<<\
          "Can't open file - please recheck parameters/data and start again."<<\
          std::endl<<datafile<<std::endl<<"!!!!!"<<std::endl;
        std::cin.get();
        exit(0);
      }
      else
        fclose(fp);
    }

    TopologyFileFormat::ClanHandle cHandle;
    cHandle.attach(filename);
    mReaders[i] = cHandle.family(0);
    initFamily(i, useThreshold, threshold);
  }

};


Engine::Engine(const HandleCollection &hc, bool useThreshold, float threshold) {
  mSimplificationID = 0;
  std::vector<ClanKey> clanKeys;
  hc.getClanKeys(clanKeys);
  if(clanKeys.size() != 1) {
    std::cout << "we can only process a single clan" << std::endl;
    exit(0);
  }

  hc.getFamilyHandles(clanKeys[0], mReaders);

  mFamilies.resize(hc.numFamilies(clanKeys[0]));
  mTimeWindow.UpdateRange(Range(0, mFamilies.size()-1));

  for(uint32_t i=0; i < mReaders.size(); i++) initFamily(i, useThreshold, threshold);
};



void Engine::setActiveTimeStep(LocalIndexType timestep) {
  if(!mTimeWindow.Contains(timestep)) {
    std::cout << "active time step not set - the timestep must lie within the allowable range: ";
    //mTimeWindow.Print();
    std::cout << std::endl;
    return;
  }
  if(timestep != mActiveTimeStep) {
    mActiveTimeStep = timestep;
    mPlot.setRegenerateData(true);
  }
}


void Engine::setRestrictToActiveTimeStep(bool val) {
  if(val != mRestrictToActiveTimestep) {
    mRestrictToActiveTimestep = val;
    if(!mRestrictToActiveTimestep) updateParameterForActiveFamilies();
    mPlot.clearSavedPlots();
    mPlot.setRegenerateData(true);
  }
}


bool Engine::setParameter(FunctionType parameter, LocalIndexType simpID) {
  if(simpID < 0 || simpID > mParameter.size()-1) {
    //std::cout << "simplification id " << simpID << "not set - it must lie within the allowable range: 0 - " << mParameter.size() << std::endl;
    return false;
  }
  if(simpID != mSimplificationID) {
    mSimplificationID = simpID;
    mPlot.setRegenerateData(true);
    mPlot.clearSavedPlots();
    updateParameterForActiveFamilies();
    return true;
  }
  if(!mParameterRange[mSimplificationID].Contains(parameter)) {
    //std::cout << "parameter " << mSimplificationID << "not set to " << parameter << " - it must lie within the allowable range:" ;
    //mParameterRange[mSimplificationID].Print();
    std::cout << std::endl;
    return false;
  }
  if(mParameter[mSimplificationID] != parameter) {
    //std::cout << "parameter " << mSimplificationID << " set to " << parameter  << std::endl;
    mParameter[mSimplificationID] = parameter;
    mPlot.setRegenerateData(true);
    //mPlot.clearSavedPlots();

    updateParameterForActiveFamilies();
  }
  return false;
}


void Engine::updateParameterForActiveFamilies() {
  if(mRestrictToActiveTimestep) mFamilies[mActiveTimeStep].updateParameter(mParameter[mSimplificationID], mSimplificationID);
  else {
    for(LocalIndexType i=0;  i < mFamilies.size(); i++) {
      std::cout << "updating parameter " << mSimplificationID << " for family " << i << " to " << mParameter[mSimplificationID] << std::endl;
      mFamilies[i].updateParameter(mParameter[mSimplificationID], mSimplificationID);
    }
  }
}


void Engine::setPlotResolution(LocalIndexType resolution) {
  mPlot.setResolution(resolution);
}


void Engine::setPlotType(const std::string &plotType) {
  bool resetParam = false;
  if(mPlot.getPlotType() == PARAM) resetParam = true;

  mPlot.setPlotType(plotType);
  clearPlotSelection();

  if(resetParam) updateParameterForActiveFamilies();
}


void Engine::loadAttributeVariable(const std::string &attribute, const std::string &variable) {

  // get index from global state
  LocalIndexType attributeID = mGlobalAttributeVariables.getAttributeVariableIndex(attribute, variable);
  // see if index is already loaded
  if(attributeID == (LocalIndexType)-1) {
    if(attribute != gHierarchyDefined) {
      std::cout << "could not load " << attribute << " " << variable << std::endl;
      std::cout << "it was not found in the global attribute variable list." << std::endl;
    }
    return;
  }

  // if is accessor
  if(mGlobalAttributeVariables[attributeID].isAccessor()) {
    std::string derivedAttribute = mGlobalAttributeVariables[attributeID].derivedAttribute();
    LocalIndexType derivedAttributeID = mGlobalAttributeVariables.getAttributeVariableIndex(derivedAttribute, variable);
    // if derivedAttribute is not loaded then load it and determine the range
    if(!mGlobalAttributeVariables[derivedAttributeID].isLoaded()) {
      Range attributeVariableRange;
      for(LocalIndexType i=0; i < mFamilies.size(); i++) {
        Range currentRange;
        // check for aggregate first and then look for attribute
        if(mReaders[i].providesAggregate(derivedAttribute, variable)) {
          std::cout << "loading derived attribute " << derivedAttribute << " " << variable << " from file for timestep " << i << "." << std::endl;
          mFamilies[i].loadAttributeVariables(mReaders[i].aggregate(derivedAttribute,variable), derivedAttributeID, currentRange, mSimplificationID);
        } else if(mReaders[i].providesAttribute(derivedAttribute, variable)) {
          std::cout << "loading derived attribute " << derivedAttribute << " " << variable << " from file for timestep " << i << "." << std::endl;
          mFamilies[i].loadAttributeVariables(mReaders[i].attribute(derivedAttribute,variable), derivedAttributeID, currentRange, mSimplificationID);
        } else {
          sterror(true, "Error, this file does not provide %s - %s.",derivedAttribute.c_str(),variable.c_str());
        }
        attributeVariableRange.UpdateRange(currentRange);
      }
      mGlobalAttributeVariables[derivedAttributeID].loaded(true);
      if(!mGlobalRanges[derivedAttributeID].Initialized()) mGlobalRanges[derivedAttributeID] = attributeVariableRange;
    }

    // make accessors
    std::cout << "loading accessor!" << std::endl;
    Range attributeVariableRange;
    for(LocalIndexType i=0; i < mFamilies.size(); i++) {
      Range currentRange;
      mFamilies[i].makeAccessors(attributeID, derivedAttributeID, attribute, derivedAttribute, currentRange, mSimplificationID);
      attributeVariableRange.UpdateRange(currentRange);
    }

    mActiveFilters[attributeID] = attributeVariableRange;
    if(!mGlobalRanges[attributeID].Initialized()) mGlobalRanges[attributeID] = attributeVariableRange;

  } else if(!mGlobalAttributeVariables[attributeID].isVisible()) {
    if(!mGlobalAttributeVariables[attributeID].isLoaded()) {
      // if is not accessor & not already loaded, then load attribute
      Range attributeVariableRange;
      for(LocalIndexType i=0; i < mFamilies.size(); i++) {
        Range currentRange;
        // check for aggregate first and then look for attribute
        if(mReaders[i].providesAggregate(attribute, variable)) {
          std::cout << "loading attribute " << attribute << " " << variable << " from file for timestep " << i << "." << std::endl;
          mFamilies[i].loadAttributeVariables(mReaders[i].aggregate(attribute,variable), attributeID, currentRange, mSimplificationID);
        } else if(mReaders[i].providesAttribute(attribute, variable)) {
          std::cout << "loading attribute " << attribute << " " << variable << " from file for timestep " << i << "." << std::endl;
          mFamilies[i].loadAttributeVariables(mReaders[i].attribute(attribute,variable), attributeID, currentRange, mSimplificationID);
        } else {
          sterror(true, "Error, this file does not provide %s - %s.",attribute.c_str(),variable.c_str());
        }
        attributeVariableRange.UpdateRange(currentRange);
      }
      mActiveFilters[attributeID] = attributeVariableRange;
      if(!mGlobalRanges[attributeID].Initialized()) mGlobalRanges[attributeID] = attributeVariableRange;
    } else {
      mActiveFilters[attributeID] = mGlobalRanges[attributeID];
    }
  }

  mGlobalAttributeVariables[attributeID].loaded(true);
  mGlobalAttributeVariables[attributeID].makeVisible();
  mPlot.setRegenerateData(true);
}


void Engine::unloadAttributeVariable(const std::string &attribute, const std::string &variable) {
  // get index from global state
  LocalIndexType attributeID = mGlobalAttributeVariables.getAttributeVariableIndex(attribute, variable);
  // see if index is already loaded
  if(attributeID == (LocalIndexType)-1) {
    std::cout << "could not unload " << attribute << " " << variable << std::endl;
    std::cout << "it was not found in the global attribute variable list." << std::endl;
    return;
  }

  if(!mGlobalAttributeVariables[attributeID].isLoaded()) return;

  std::cout << "unloading: " << attribute << " " << variable << std::endl;
  // if is accessor
  if(mGlobalAttributeVariables[attributeID].isAccessor()) {
    std::cout << "is accessor" << std::endl;
    // if associated attribute is not visible then unload
    std::string derivedAttribute = mGlobalAttributeVariables[attributeID].derivedAttribute();
    LocalIndexType derivedAttributeID = mGlobalAttributeVariables.getAttributeVariableIndex(derivedAttribute, variable);
    if(!mGlobalAttributeVariables[derivedAttributeID].isVisible()) {
      for(LocalIndexType i=0; i < mFamilies.size(); i++) {
        std::cout << "on family " << i << " with attributeID " << attributeID << std::endl;
        mFamilies[i].unloadAttributeVariables(derivedAttributeID);
      }
      mActiveFilters.erase(derivedAttributeID);
      mGlobalAttributeVariables[derivedAttributeID].loaded(false);
    }

  } else {
    std::cout << "is NOT accessor" << std::endl;
    //  unload all accessors that derive off attribute & make them invisible
    for(LocalIndexType j=0; j < mGlobalAttributeVariables.size(); j++) {
      if(mGlobalAttributeVariables[j].isLoaded() && mGlobalAttributeVariables[j].isDerivedOff(attribute)) {
        std::cout << "on attribute" << j << std::endl;
        for(LocalIndexType i=0; i < mFamilies.size(); i++) {
          std::cout << "on family " << i << " with attributeID " << j << std::endl;
          mFamilies[i].unloadAttributeVariables(j);
        }
        mActiveFilters.erase(j);
        mGlobalAttributeVariables[j].loaded(false);
        mGlobalAttributeVariables[j].makeInvisible();
      }
    }
  }

  // unload attribute
  std::cout << "unloading attribute" << std::endl;
  for(LocalIndexType i=0; i < mFamilies.size(); i++) {
    std::cout << "on family " << i << " with attributeID " << attributeID << std::endl;
    mFamilies[i].unloadAttributeVariables(attributeID);
  }
  mActiveFilters.erase(attributeID);

  if(attributeID == mPlotAttributeID) setPlotFeature(gHierarchyDefined, std::string(""));

  mGlobalAttributeVariables[attributeID].loaded(false);
  mGlobalAttributeVariables[attributeID].makeInvisible();

  mPlot.setRegenerateData(true);
}


void Engine::setPlotFeature(const std::string &attribute, const std::string &variable) {

  LocalIndexType attributeID;

  if(attribute != gHierarchyDefined) {
    loadAttributeVariable(attribute, variable);
    attributeID = mGlobalAttributeVariables.getAttributeVariableIndex(attribute, variable);
    if(attributeID == (LocalIndexType)-1) {
     std::cout << "could not load " << attribute << " " << variable << std::endl;
     std::cout << "it was not found in the global attribute variable list." << std::endl;
     return;
    }
  } else {
    attributeID = mGlobalAttributeVariables.size();
  }

  if(mPlotAttributeID != attributeID) {
    mPlotAttributeID = attributeID;
    mPlot.clearSavedPlots();
    mPlot.setRegenerateData(true);
  }
}


void Engine::setCrossClanAggregator(const std::string &aggregator) {
  mPlot.setCrossClanAggregator(aggregator);
}


void Engine::setCrossFamilyAggregator(const FamilyAggregator& agg) {
  mPlot.setCrossFamilyAggregator(agg);
}


void Engine::setFilter(const std::string &attribute, const std::string &variable, FunctionType minVal, FunctionType maxVal) {
  if(minVal > maxVal) {
    std::cout << "Error: filter min " << minVal << " must be less than or equal to filter max " << maxVal << std::endl;
    return;
  }

  // get index from global state
  LocalIndexType attributeID = mGlobalAttributeVariables.getAttributeVariableIndex(attribute, variable);
  // see if index exists
  if(attributeID == (LocalIndexType)-1) {
    std::cout << "could not set filter on " << attribute << " " << variable << std::endl;
    std::cout << "it was not found in the global attribute variable list." << std::endl;
    return;
  }

  // make sure loaded
  if(mActiveFilters.find(attributeID) == mActiveFilters.end()) {
    std::cout << "could not set filter on " << attribute << " " << variable << std::endl;
    std::cout << "because the associated data is not currently loaded." << std::endl;
    return;
  }

  if(!mGlobalRanges[attributeID].Initialized()) {
    std::cout << "Error: global range was not initialized" << std::endl;
    return;
  }

  // make sure it is a valid range
 /* if(!mGlobalRanges[attributeID].CompletelyContains(Range(minVal, maxVal))) {
    std::cout << "ignoring call: filter was outside of global filter range values for " << attribute << " " << variable << std::endl;
    std::cout << "min, max = " << minVal << ", " << maxVal << std::endl;
    return;
  }
  */

  Range newFilter(minVal, maxVal);
  if(newFilter != mActiveFilters[attributeID]) {
    mActiveFilters[attributeID] = newFilter;
    mPlot.setRegenerateData(true);
  }
}


void Engine::setXLogScale(bool value) {
  mPlot.setXLogScale(value);
}


void Engine::setYLogScale(bool value) {
  mPlot.setYLogScale(value);
}



void Engine::setXAxisRange(const Range &range) {
  mPlot.setXAxisRange(range);
}


void Engine::setYAxisRange(const Range &range) {
  mPlot.setYAxisRange(range);
}


void Engine::updateState() {
  if(mPlotAttributeID != mGlobalAttributeVariables.size()) {
    mPlot.regenerateData(mFamilies, mGlobalAttributeVariables, mPlotAttributeID, mActiveFilters, mGlobalRanges[mPlotAttributeID], mTimeWindow, mParameterRange[mSimplificationID], mRestrictToActiveTimestep, mActiveTimeStep, mSimplificationID);
  } else {
    Range dummyRange;
    mPlot.regenerateData(mFamilies, mGlobalAttributeVariables, mPlotAttributeID, mActiveFilters, dummyRange, mTimeWindow, mParameterRange[mSimplificationID], mRestrictToActiveTimestep, mActiveTimeStep, mSimplificationID);
  }

  writeSubSelectedIDsToFile();
}



void Engine::saveActivePlot() {
  mPlot.saveActivePlotData();
}


void Engine::clearSavedPlots() {
  mPlot.clearSavedPlots(false);
}


void Engine::savePlotToPLT(const std::string &filename) {
  mPlot.savePlotToPLT(filename);
}


void Engine::savePlotToAGR(const std::string &filename) {
  mPlot.savePlotToAGR(filename);
}


void Engine::clearPlotSelection() {
  mPlotSelection.clear();
}


void Engine::clearRangeSelection() {
  for(LocalIndexType i=0;  i < mFamilies.size(); i++) {
    mFamilies[i].clearSubSelectedIDs();
  }
  resetActiveFilters();
  mPlot.setRegenerateData(true);
}


void Engine::printState() const {
  std::cout << "===================================================================" << std::endl;
  std::cout << "Global State Variables: " << std::endl;
  std::cout << "===================================================================" << std::endl;
  /*for(uint32_t i=0; i < mParameterRange.size(); i++) {
    std::cout << "parameter range[" << i << "]: ";
    mParameterRange[i].Print();
    std::cout << std::endl;
  }*/
  std::cout << "global time window: ";
  mTimeWindow.Print();
  std::cout << std::endl;
  std::cout << "global attribute-variable list:" << std::endl;

  for(LocalIndexType i=0; i < mGlobalRanges.size(); i++) {
    std::cout << mGlobalAttributeVariables[i] << ":";
    mGlobalRanges[i].Print();
    std::cout << std::endl;
  }

  std::cout << "===================================================================" << std::endl;
  std::cout << "Active state values: " << std::endl;
  std::cout << "===================================================================" << std::endl;
  std::cout << "parameter: " << mParameter[mSimplificationID] << std::endl;
  std::cout << "active time step: " << mActiveTimeStep << std::endl;
  std::cout << "simplification id: " << mSimplificationID << std::endl;
  if(mPlotAttributeID != mGlobalAttributeVariables.size()) std::cout << "plot attribute: " << mPlotAttributeID << " " << mGlobalAttributeVariables[mPlotAttributeID] << std::endl;
  else std::cout << gHierarchyDefined << std::endl;
  std::cout << "plot type: " << plotTypeToString(mPlot.getPlotType()) << std::endl;
  std::cout << "plot resolution: " << mPlot.getResolution() << std::endl;
  std::cout << "cross family aggregator: " << mPlot.getCrossFamilyAggregatorName() << std::endl;
  std::cout << "cross clan aggregator: " << mPlot.getCrossClanAggregatorType() << std::endl;
  std::cout << "plot log x: " << mPlot.getXLogScale() << std::endl;
  std::cout << "plot log y: " << mPlot.getYLogScale() << std::endl;

  std::cout << "active attribute-variable:" << std::endl;

  /*for(typename std::map<StatisticsLocalIndexType, Range >::const_iterator itr=mActiveFilters.begin(); itr != mActiveFilters.end(); itr++) {
    std::cout << mGlobalAttributeVariables[itr->first] << ":";
    (itr->second).Print();
    std::cout << std::endl;
  }*/

  std::cout << "===================================================================" << std::endl;

}


Range Engine::getActiveFilter(LocalIndexType id) const {
  std::map<LocalIndexType, Range >::const_iterator itr = mActiveFilters.find(id);
  sterror(itr == mActiveFilters.end(), "Error: active filter not found");
  return itr->second;
}


void Engine::initializeAttributeVariablesFromList(std::set<LocalIndexType> &attributesToLoad) {
  for(LocalIndexType i = 0; i < mGlobalAttributeVariables.size(); i++) {
    if(attributesToLoad.find(i) != attributesToLoad.end()) loadAttributeVariable(mGlobalAttributeVariables[i].attribute(), mGlobalAttributeVariables[i].variable());
    else unloadAttributeVariable(mGlobalAttributeVariables[i].attribute(), mGlobalAttributeVariables[i].variable());
  }
}


void Engine::writeSubSelectedIDsToFile() {
  if(mSubselectionFileName == std::string("")) return;
  if(pickablePlot() && mPlotAttributeID != mGlobalAttributeVariables.size() && mPlotSelection.size() > 0) {
    std::vector<Range > ranges;
    mPlot.getRanges(mPlotSelection, ranges);
    mFamilies[mActiveTimeStep].writeSelectedFeaturesToFile(mPlotAttributeID, ranges, mSubselectionFileName, mSimplificationID);
  } else if(pickablePlot()) mFamilies[mActiveTimeStep].writeSubSelectedIDsToFile(mSubselectionFileName, mSimplificationID);
}


std::string Engine::getEngineStatus() const {
  if(mPlotAttributeID == mGlobalAttributeVariables.size() && mPlot.getPlotType() == CDF) {
    return std::string("Error:  Make sure you have loaded and specified the attribute/variable for which you would like to generate a CDF.");
  }
  if(mPlotAttributeID == mGlobalAttributeVariables.size() && mPlot.getPlotType() == WCDF) {
    return std::string("Error:  Make sure you have loaded and specified the attribute/variable for which you would like to generate a WCDF.");
  }
  if(mPlotAttributeID == mGlobalAttributeVariables.size() && mPlot.getPlotType() == HIST) {
    return std::string("Error:  Make sure you have loaded and specified the attribute/variable for which you would like to generate a histogram.");
  }
  if(mPlotAttributeID == mGlobalAttributeVariables.size() && mPlot.getCrossFamilyAggregatorType() == FEATURE_BASED) {
    return std::string("Error:  The cross family aggregator is attribute based but no attribute is specified.");
  }

  else return gValidState;
}


void Engine::resetActiveFilters() {
  for(std::map<LocalIndexType, Range >::iterator itr=mActiveFilters.begin(); itr != mActiveFilters.end(); itr++) {
    itr->second = mGlobalRanges[itr->first];
  }
}


std::string Engine::getSelectedSegmentInformation(int id) const {
  std::string info;

  for(std::map<LocalIndexType, Range >::const_iterator itr=mActiveFilters.begin(); itr != mActiveFilters.end(); itr++) {
    char val[256];
    sprintf(val, " %f \n", mFamilies[mActiveTimeStep].getAggregatedAttributes(itr->first, id, mSimplificationID));
    info += (mGlobalAttributeVariables[itr->first].attributeVariable() + std::string(val));
  }
  return info;
}


std::vector<std::string> Engine::getActiveAttributeList() const {
  std::vector<std::string> attrList;
  for(std::map<LocalIndexType, Range >::const_iterator itr=mActiveFilters.begin(); itr != mActiveFilters.end(); itr++) {
      attrList.push_back(mGlobalAttributeVariables[itr->first].attributeVariable());
  }
  return attrList;
}


void Engine::getObservations(LocalIndexType simpID, std::vector< std::map<LocalIndexType, std::vector<FunctionType> > > &observations, std::vector<FunctionType> &param) {
  if(param.size() != 0 && param.size() != mFamilies.size()) {
    std::cout << "either no or all parameter values should be set" << std::endl;
    std::cout << "param.size() = " << param.size() << std::endl;
    exit(0);
  }

  for(uint32_t i=0; i < mFamilies.size(); i++) {
    std::cout << "on family " << i << " of " << mFamilies.size() << std::endl;
    std::map<LocalIndexType, std::vector<FunctionType> > fileObs;
    if(param.size() > 0) mFamilies[i].updateParameter(param[i], mSimplificationID);
    mFamilies[i].getObservations(simpID, fileObs);
    observations.push_back(fileObs);
  }
}



};
