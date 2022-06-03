#ifndef PLOT_H
#define PLOT_H

#include "FeatureFamilyData.h"
#include "StatisticsDefinitions.h"
#include "Aggregator.h"
#include "Range.h"
#include "PlotData.h"
#include "AttributeVariableList.h"
#include "FamilyAggregator.h"
#include <set>
#include <vector>

namespace Statistics {


class Plot{
  public: Plot();
  ~Plot();
  Plot(const Plot &p);

  void setResolution(const LocalIndexType r);
  void setPlotType(const std::string &type);
  void setCrossFamilyAggregator(const FamilyAggregator& );
  void setCrossClanAggregator(const std::string &);
  void setUpdatePlot(bool val) { mUpdatePlot = true; }
  void setRegenerateData(bool val) { mRegenerateData = true; }
  void setXLogScale(bool val) { mXLogScale = val; }
  void setYLogScale(bool val) { mYLogScale = val; }
  void setXAxisRange(const Range &range) { mXAxisRange = range; }
  void setYAxisRange(const Range &range) { mYAxisRange = range; }
  void getRanges(std::set<LocalIndexType> &plotSelection, std::vector<Range > &ranges);


  FunctionType getResolution() const { return mResolution; }
  FunctionType getXLogScale() const { return mXLogScale; }
  FunctionType getYLogScale() const { return mYLogScale; }
  Range getXAxisRange() const { return mXAxisRange; }
  Range getYAxisRange() const { return mYAxisRange; }
  PlotType getPlotType() const { return mPlotType; }
  std::string getCrossFamilyAggregatorName() const {  return mCrossFamilyAggregator.name(); }
  FamilyAggregationType getCrossFamilyAggregatorType() const {  return mCrossFamilyAggregator.type(); }
  std::string getCrossClanAggregatorType() const {  return mCrossClanAggregator->typeName(); } 
  PlotData getActiveData() const { return mActiveData; }
  PlotData getSavedData(LocalIndexType id) const { return mSavedData[id]; }
  LocalIndexType getNumSavedPlots() const { return mSavedData.size(); }
  std::string getTitle() const { return mTitle; }
  std::string getXAxisLabel() const { return mXAxisLabel; }
  std::string getYAxisLabel() const { return mYAxisLabel; }



  void print() { for(LocalIndexType i=0; i < mSavedData.size(); i++) { mSavedData[i].print(); } }
  void saveActivePlotData() { mSavedData.push_back(mActiveData); }
  void clearSavedPlots(bool initRanges = true) { 
    for(uint32_t i=0; i < mSavedData.size(); i++) { mSavedData[i].clearData(); }
    mSavedData.clear(); 
    if(initRanges) {
      mXAxisRange.Initialize();
      mYAxisRange.Initialize();
    }
  }
  void savePlotToPLT(const std::string &filename = std::string(""));
  void savePlotToAGR(const std::string &filename = std::string(""));
  void regenerateData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range & timeRange, Range &parameterRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID);
  void generateSpeciesDistributionData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range& dataRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID);
  void generateTimeSeriesData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range& timeRange, LocalIndexType simpID);
  void generateParameterData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range &parameterRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID);
  
  public: 
  // appropriate info here includes feature family id, timestep range, simplification sequence
  std::string mTitle;
  std::string mXAxisLabel, mYAxisLabel;
  Range mXAxisRange, mYAxisRange;
  bool mXLogScale, mYLogScale;
  PlotType mPlotType;
  LocalIndexType mResolution;

  // one vector of data per saved plot
  std::vector<PlotData > mSavedData;
  // active plot data 
  PlotData mActiveData;

  //! aggregator used for reduction in time and threshold plots
  FamilyAggregator mCrossFamilyAggregator;

  //! aggregator used for reduction in threshold plots
  Attribute* mCrossClanAggregator;

  // flags to indicate when data and plots should be regenerated
  bool mUpdatePlot, mRegenerateData;


};


};

#endif
