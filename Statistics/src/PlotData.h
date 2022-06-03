#ifndef PLOT_DATA_H
#define PLOT_DATA_H

#include <set>
#include <map>
#include <algorithm>
#include "StatisticsDefinitions.h"
#include "Aggregator.h"
#include "Range.h"


namespace Statistics {

enum PlotType {
  CDF = 1,
  WCDF = 2,
  HIST= 3,
  TIME= 4,
  PARAM = 5,
};

std::string plotTypeToString(PlotType plotType);

class PlotData {
  public: 
  PlotData() {};
  ~PlotData() {};
  PlotData(const PlotData &p);

  void print();
  void setDistributionData(const std::vector<FunctionType> &features, std::string &legend, Range &xAxisRange);
  void setNonDistributionData(const std::vector<FunctionType> &features, std::string &legend, std::vector<FunctionType> &xCoords, Range &yAxisRange);
  void clearData() { 
    mData.clear(); 
    clearCoordinates(); 
  }
  void clearCoordinates() { mCoordinates.clear(); }

  void dumpGNUPlotData(const std::string &filename, const std::string &comment=std::string("")) const;
  void dumpAGRPlotData(FILE* output) const;
  void plotDataOnXAxis(PlotType plotType, const LocalIndexType resolution, const Range &xAxisRange, Range &yAxisRange);
  std::pair<FunctionType, FunctionType> getCoordinate(LocalIndexType id) const { return mCoordinates[id]; }
  LocalIndexType getNumCoordinates() const { return mCoordinates.size(); }
  void getRanges(std::set<LocalIndexType> &plotSelection, std::vector<Range > &ranges);

  public: 
  // data is stored in a vector
  std::vector< FunctionType > mData;
  // vector of x, y coords for each set of attributes to be plotted
  // and vector of indices contributing to coord for each set of attributes to be plotted
  // [attribute][bins][x, y values] and [attribute][bins][list of ids]
  std::vector< std::pair<FunctionType, FunctionType> >  mCoordinates;

  std::string mLegend;
};


};

#endif
