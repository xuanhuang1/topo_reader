#ifndef ENGINE_H
#define ENGINE_H

#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "ClanHandle.h"
#include "FamilyHandle.h"
#include "FeatureHierarchy.h"
#include "HandleCollection.h"
#include "FeatureFamilyData.h"
#include "Plot.h"
#include "Range.h"
#include "Aggregator.h"
#include "AttributeVariableList.h"
#include "FamilyAggregator.h"

namespace Statistics {


const std::string gHierarchyDefined = std::string("hierarchyDefined");
const std::string gValidState = std::string("valid");

class Engine  {
private:
  Engine() {};
public:
  Engine(const std::string &dataFileName, bool useThreshold=false, float threshold=-1.0);
  Engine(const HandleCollection &hc, bool useThreshold=false, float threshold=-1.0);
  void initFamily(uint32_t timestep, bool useThreshold=false, float threshold=-1.0);

  ~Engine() {};

  void printAvailableAttributeVariables() const {
    std::cout << mGlobalAttributeVariables << std::endl;
  };

  void initializeAttributeVariablesFromList(std::set<LocalIndexType> &attributesToLoad);
  void loadAttributeVariable(const std::string &attribute, const std::string &variable);
  void unloadAttributeVariable(const std::string &attribute, const std::string &variable);
  void setActiveTimeStep(LocalIndexType timestep);
  void setRestrictToActiveTimeStep(bool val);
  bool setParameter(FunctionType parameter, LocalIndexType simpID);
  void setPlotResolution(LocalIndexType resolution);
  void setPlotType(const std::string &plotType);
  void setPlotFeature(const std::string &attribute, const std::string &variable);
  void setCrossClanAggregator(const std::string &aggregator);
  void setCrossFamilyAggregator(const FamilyAggregator& agg);
  void setFilter(const std::string &attribute, const std::string &variable, FunctionType minVal, FunctionType maxVal);
  void setXLogScale(bool value);
  void setYLogScale(bool value);
  void setXAxisRange(const Range &range);
  void setYAxisRange(const Range &range);
  void updateState();
  void printState() const;
  void saveActivePlot();
  void clearSavedPlots();
  void savePlotToPLT(const std::string &filename);
  void savePlotToAGR(const std::string &filename);
  void clearPlotSelection();
  void clearRangeSelection();
  void writeSubSelectedIDsToFile();
  void updateParameterForActiveFamilies();
  void resetActiveFilters();
  void setSubSelectionFileName(const std::string &filename) { mSubselectionFileName = filename; }
  std::vector<std::string> getActiveAttributeList() const;
  FeatureHierarchy* getHierarchy(LocalIndexType fileID, LocalIndexType simpID) const { return mFamilies[fileID].getHierarchy(simpID); };
  void getObservations(LocalIndexType simpID, std::vector< std::map<LocalIndexType, std::vector<FunctionType> > > &observations, std::vector<FunctionType> &param);//=std::vector<FunctionType>()) ;
  
  std::string getEngineStatus() const;
  std::string getSelectedSegmentInformation(int id) const;
  bool pickablePlot() const { return (mPlot.getPlotType() == HIST || mPlot.getPlotType() == CDF || mPlot.getPlotType() == WCDF); }

  Range getParameterRange() const { return mParameterRange[mSimplificationID]; }
  FunctionType getParameter() const { return mParameter[mSimplificationID]; }
  Range getTimeWindow() const { return mTimeWindow; }
  FunctionType getActiveTimeStep() const { return mActiveTimeStep; }
  AttributeVariableList getGlobalAttributeVariables() const { return mGlobalAttributeVariables; }
  std::vector<Range > getGlobalRanges() const { return mGlobalRanges; }
  Range getActiveFilter(LocalIndexType id) const;
  std::map<LocalIndexType, Range > getActiveFilters() const { return mActiveFilters; };
  LocalIndexType getPlotAttributeID() const { return mPlotAttributeID; }
  LocalIndexType getPlotResolution() const { return mPlot.getResolution(); }
  PlotType getPlotType() const { return mPlot.getPlotType(); }
  FunctionType getXLogScale() const { return mPlot.getXLogScale(); }
  FunctionType getYLogScale() const { return mPlot.getYLogScale(); }
  Range getXAxisRange() const { return mPlot.getXAxisRange(); }
  Range getYAxisRange() const { return mPlot.getYAxisRange(); }
  PlotData getActivePlotData() const { return mPlot.getActiveData(); }
  PlotData getSavedPlotData(LocalIndexType id) const { return mPlot.getSavedData(id); }
  LocalIndexType getNumSavedPlots() const { return mPlot.getNumSavedPlots(); }
  FamilyAggregationType getCrossFamilyAggregatorType() const {  return mPlot.getCrossFamilyAggregatorType(); } 
  std::string getCrossFamilyAggregatorName() const {  return mPlot.getCrossFamilyAggregatorName(); } 
  std::string getCrossClanAggregatorType() const {  return mPlot.getCrossClanAggregatorType(); } 
  bool getRestrictToActiveTimeStep() const { return mRestrictToActiveTimestep; }
  std::set<uint32_t> getPlotSelection() const { return mPlotSelection; }
  void updatePlotSelection(uint32_t val) { if(mPlotSelection.find(val) != mPlotSelection.end()) mPlotSelection.erase(val); else mPlotSelection.insert(val); }
  bool plotSelectionContains(uint32_t val) { return mPlotSelection.find(val) != mPlotSelection.end(); }
  LocalIndexType getSimplificationID() const { return mSimplificationID; }
  LocalIndexType getNumSimplificationSequences() const { return mParameter.size(); }
  
 

  std::string getPlotTitle() const { return mPlot.getTitle(); }
  std::string getXAxisLabel() const { return mPlot.getXAxisLabel(); }
  std::string getYAxisLabel() const { return mPlot.getYAxisLabel(); }
  


public:
  // one per timestep
  std::vector<FeatureFamilyData > mFamilies;
  std::vector<TopologyFileFormat::FamilyHandle>  mReaders;

  // plot
  Plot mPlot;
  
  //Parameter info - we need one for each simplification sequence
  std::vector<Range >mParameterRange;
  std::vector<FunctionType> mParameter;

  // global and active timewindows
  Range mTimeWindow;
  LocalIndexType mActiveTimeStep;

  // list of attribute variables and associated global ranges
  AttributeVariableList mGlobalAttributeVariables;
  std::vector<Range > mGlobalRanges;
  
  // map of active Filters
  // first == index into mGlobalAttributeVariables
  // second == active filter range
  std::map<LocalIndexType, Range > mActiveFilters;

  // index into mGlobalAttributeVariables -- the data we are plotting
  LocalIndexType mPlotAttributeID;

  bool mRestrictToActiveTimestep;

  std::string mSubselectionFileName;
  std::set<uint32_t> mPlotSelection;

  LocalIndexType mSimplificationID;

  };



};

#endif
