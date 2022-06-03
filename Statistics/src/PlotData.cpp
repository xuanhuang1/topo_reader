#include "PlotData.h"

namespace Statistics {

  std::string plotTypeToString(PlotType plotType) {
    if(plotType == CDF) return std::string("CDF");
    if(plotType == WCDF) return std::string("WCDF");
    if(plotType == HIST) return std::string("HIST");
    if(plotType == TIME) return std::string("TIME");
    if(plotType == PARAM) return std::string("PARAM");
    else return std::string("INVALID PLOT TYPE");
  }



  // don't dump shifted data - let gnuplot handle it
  void PlotData::dumpGNUPlotData(const std::string &filename, const std::string &comment) const {
     FILE *file = fopen(filename.c_str(), "w");
     fprintf(file, "#%s\n", comment.c_str());
     for(LocalIndexType i=0; i < mCoordinates.size(); i++) {
        fprintf(file, "%f\t%f\n", mCoordinates[i].first, mCoordinates[i].second);
     }
     fclose(file);
  }

  void PlotData::dumpAGRPlotData(FILE* output) const
  {
    for(LocalIndexType i=0; i < mCoordinates.size(); i++) {
      fprintf(output, "%f\t%f\n", mCoordinates[i].first, mCoordinates[i].second);
    }

    fprintf(output,"s_ type XY\n");
  }


  PlotData::PlotData(const PlotData &p) {
    mData = p.mData;
    mCoordinates = p.mCoordinates;
    mLegend = p.mLegend;
  }


  void PlotData::setDistributionData(const std::vector<FunctionType> &features, std::string &legend, Range &xAxisRange) {
    // store data
    clearData();
    mData.resize(features.size());
    for(LocalIndexType i=0; i < features.size(); i++) {
       mData[i] = features[i];
       xAxisRange.UpdateRange(mData[i]);
    }
    mLegend = legend;
  }

  void PlotData::setNonDistributionData(const std::vector<FunctionType> &features, std::string &legend, std::vector<FunctionType> &xCoords, Range &yAxisRange) {
    // store data
    clearData();
    mData.resize(features.size());
    for(LocalIndexType i=0; i < features.size(); i++) {
       mData[i] = features[i];
       mCoordinates.push_back(std::pair<FunctionType, FunctionType>(xCoords[i], mData[i]));
       yAxisRange.UpdateRange(mData[i]);
    }
    mLegend = legend;
  }

  void PlotData::plotDataOnXAxis(PlotType plotType, const LocalIndexType resolution, const Range &xAxisRange, Range &yAxisRange) {
    clearCoordinates();
    std::sort(mData.begin(), mData.end());
    FunctionType delta = (xAxisRange.Max() - xAxisRange.Min()) / (FunctionType) resolution;

    FunctionType sum = mData.size();
    FunctionType lastY;

    FunctionType current = xAxisRange.Min(), accumulated = 0;
    LocalIndexType i=0, start = 0;

    if (plotType == HIST) {
      sum = 1;
    } else if (plotType == CDF) {
      sum = mData.size();
    } else if (plotType == WCDF) {
      sum = 0;
      for (i=0;i<mData.size();i++)
        sum += mData[i];
    }


    lastY = 0;
    yAxisRange.UpdateRange(lastY);

    mCoordinates.push_back(std::pair<FunctionType, FunctionType>(current, lastY));
    std::cout << "(" << mCoordinates.back().first << ", " << mCoordinates.back().second << ") " << std::endl;
    unsigned int bucket = 0;
    i = 0;
    while(i<mData.size()){
      bucket++;
      start = i;
      current += delta;
      while ((i < mData.size()) && (mData[i] <= current)) {
        if (plotType != WCDF)
          accumulated += 1;
        else
          accumulated += mData[i];

        i++;
      }
      while ((i < mData.size()) && bucket == resolution) {
        current = xAxisRange.Max();
        if (plotType != WCDF)
          accumulated += 1;
        else
          accumulated += mData[i];
        i++;
      }
      if(plotType == HIST) lastY = FunctionType(i-start)/sum;
      else if (plotType == CDF || plotType == WCDF) lastY = accumulated/sum;

      mCoordinates.push_back(std::pair<FunctionType, FunctionType>(current, lastY));
      std::cout << "(" << mCoordinates.back().first << ", " << mCoordinates.back().second << ") " << std::endl;
      yAxisRange.UpdateRange(lastY);
    }
  }

  void PlotData::getRanges(std::set<LocalIndexType> &plotSelection, std::vector<Range > &ranges) {
    if(mCoordinates.size() == 0) return;

    for(uint32_t i=0; i < mCoordinates.size()-1; i++) {
      if(plotSelection.find(i) != plotSelection.end()) {
        ranges.push_back(Range(mCoordinates[i].first, mCoordinates[i+1].first));
      }
    }
  }

  void PlotData::print() {
    std::cout << "Data: " << std::endl;
    for(LocalIndexType i=0; i < mData.size(); i++) {
       std::cout << "\t" << mData[i] << std::endl;
    }
    std::cout << "mCoordinates: " << std::endl;
    for(LocalIndexType i=0; i < mCoordinates.size(); i++) {
       std::cout << "\t(" << mCoordinates[i].first << ", " << mCoordinates[i].second << ")" << std::endl;
    }
  }


}


