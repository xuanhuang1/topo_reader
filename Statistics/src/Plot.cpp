#include "Plot.h"
namespace Statistics {



Plot::Plot() {
  mResolution = 20;
  mPlotType = CDF;
  Factory aggFactory;
  mCrossClanAggregator = aggFactory.make_aggregator("min");
  mCrossFamilyAggregator = FamilyAggregator("featureCount");
  mUpdatePlot = mRegenerateData = true;
  mXLogScale = mYLogScale = false;
}


void Plot::setResolution(const LocalIndexType r) {
  if(r != mResolution) {
    mResolution = r;
    if(mPlotType == PARAM) {
      mRegenerateData = true;
    }
    mUpdatePlot = true;
  }
}


void Plot::setPlotType(const std::string &type) {
  if(!(type == std::string("CDF") || type == std::string("HIST") || type == std::string("WCDF") ||
       type == std::string("TIME") || type == std::string("PARAM"))) {
    std::cout << "invalid plot type" << std::endl;
    return;
  }

  if(type != plotTypeToString(mPlotType)) {
    if(type == std::string("CDF")) mPlotType = CDF;
    if(type == std::string("HIST")) mPlotType = HIST;
    if(type == std::string("WCDF")) mPlotType = WCDF;
    if(type == std::string("TIME")) mPlotType = TIME;
    if(type == std::string("PARAM")) mPlotType = PARAM;
    mRegenerateData = mUpdatePlot = true;
    clearSavedPlots();
  }
}


void Plot::setCrossClanAggregator(const std::string &aggregator) {
  if(!(aggregator ==  std::string("min") || aggregator == std::string("max") || aggregator == std::string("mean"))) {
    std::cout << "Error: " << aggregator << " not currently supported, please choose from min, max, or mean." << std::endl;
    return;
  }
  if(std::string(mCrossClanAggregator->typeName()) != aggregator) {
    delete mCrossClanAggregator;
    Factory aggFactory;
    mCrossClanAggregator = aggFactory.make_aggregator(aggregator.c_str());
    mRegenerateData = mUpdatePlot = true;
  }
}


void Plot::setCrossFamilyAggregator(const FamilyAggregator& agg)
{
  if (mCrossFamilyAggregator != agg) {
    mCrossFamilyAggregator = agg;
    mRegenerateData = mUpdatePlot = true;
  }
}




Plot::~Plot() {
  delete mCrossClanAggregator;
}


Plot::Plot(const Plot &p) {
  mTitle = p.mTitle;
  mXAxisLabel = p.mXAxisLabel;
  mYAxisLabel = p.mYAxisLabel;
  mXAxisRange = p.mXAxisRange;
  mYAxisRange = p.mYAxisRange;
  mXLogScale = p.mXLogScale;
  mYLogScale = p.mYLogScale;
  mResolution = p.mResolution;
  mPlotType = p.mPlotType;
  mSavedData = p.mSavedData;
  mActiveData= p.mActiveData;
  mUpdatePlot= p.mUpdatePlot;
  mRegenerateData= p.mRegenerateData;
  mCrossFamilyAggregator = p.mCrossFamilyAggregator;
  mCrossClanAggregator = p.mCrossClanAggregator->clone();
}


void Plot::savePlotToPLT(const std::string &filename) {

   if(mSavedData.size() == 0) {
     std::cout << "no saved data --- not generating a plot file" << std::endl;
     return;
   }

   std::string baseFilename;
   if(filename == "")  baseFilename = mActiveData.mLegend;
   else baseFilename = filename;

   // make data name
   std::vector<std::string> dataName(mSavedData.size());
   for(LocalIndexType i=0; i < dataName.size(); i++) {
     char fileNum[256];
     sprintf(fileNum, "%d", i);
     dataName[i] = baseFilename + std::string("_") + std::string(fileNum) + std::string(".dat");
   }

   // make script name
   std::string scriptName = baseFilename + std::string(".plt");
   // make image name
   std::string imageName = baseFilename + std::string(".pdf");

   // generate gnu script file
   FILE *file = fopen(scriptName.c_str(), "w");
   fprintf(file, "set terminal pdf color enhanced\n");
   fprintf(file, "set output '%s'\n", imageName.c_str());
   fprintf(file, "set title \"%s\"\n", mTitle.c_str());
   fprintf(file, "set xlabel '%s'\n", mXAxisLabel.c_str());
   fprintf(file, "set ylabel '%s'\n", mYAxisLabel.c_str());
   if(mXLogScale) fprintf(file, "set logscale x\n");
   if(mYLogScale) fprintf(file, "set logscale y\n");
   fprintf(file, "set mxtics'\n");

   FunctionType yoffset = (mYAxisRange.Width())*0.1;
   FunctionType xoffset = (mXAxisRange.Width())*0.1;
   if(!mYLogScale || mYAxisRange.Min() > 0) fprintf(file, "set yrange[%f : %f]\n", mYAxisRange.Min(), mYAxisRange.Max() +yoffset);
   else fprintf(file, "set yrange[%f : %f]\n", mYAxisRange.Min() + yoffset*.1, mYAxisRange.Max() +yoffset);
   if(!mXLogScale || mXAxisRange.Min() > 0) fprintf(file, "set xrange[%f : %f]\n", mXAxisRange.Min(), mXAxisRange.Max());
   else fprintf(file, "set xrange[%f : %f]\n",mXAxisRange.Min() + xoffset*.1, mXAxisRange.Max());
   fprintf(file, "set key out vert\n");
   fprintf(file, "set key right top\n");

   std::cout << "dumping plot data to file " << scriptName  << std::endl;

   fprintf(file, "plot ");
   for(LocalIndexType i=0; i < mSavedData.size(); i++) {
     if(mPlotType == HIST) {
       fprintf(file, "'%s' with steps", dataName[i].c_str());
     } else {
       fprintf(file, "'%s' with lines", dataName[i].c_str());
     }
     if(i < mSavedData.size()-1) fprintf(file, ", ");
   }
   fprintf(file, "\n");

   fclose(file);

   // dump out data
   for(LocalIndexType i=0; i < mSavedData.size(); i++) {
      mSavedData[i].dumpGNUPlotData(dataName[i]);
   }
}


void Plot::savePlotToAGR(const std::string &filename) {

   if(mSavedData.size() == 0) {
     std::cout << "no saved data --- not generating a plot file" << std::endl;
     return;
   }

   std::string baseFilename;
   if(filename == "")  baseFilename = mActiveData.mLegend;
   else baseFilename = filename;

   // make data name
   std::vector<std::string> dataName(mSavedData.size());
   for(LocalIndexType i=0; i < dataName.size(); i++) {
     char fileNum[256];
     sprintf(fileNum, "%d", i);
     dataName[i] = baseFilename + std::string("_") + std::string(fileNum) + std::string(".dat");
   }

   // make script name
   std::string scriptName = baseFilename + std::string(".agr");
   // make image name
   std::string imageName = baseFilename + std::string(".pdf");

   // generate gnu script file
   FILE *file = fopen(scriptName.c_str(), "w");

   for(LocalIndexType i=0; i < mSavedData.size(); i++) {
     mSavedData[i].dumpAGRPlotData(file);
     if(mPlotType == HIST)
       fprintf(file, "@s_ line type 3");
   }

   fprintf(file,"@ title \"%s\"\n",mTitle.c_str());
   fprintf(file,"@ xaxis label \"%s\"\n",mXAxisLabel.c_str());
   fprintf(file,"@ yaxis label \"%s\"\n",mYAxisLabel.c_str());
   if(mXLogScale)
     fprintf(file, "@ xaxis scale logarithmic\n");
   if(mYLogScale)
     fprintf(file, "@ yaxis scale logarithmic\n");

   /*
   FunctionType yoffset = (mYAxisRange.Width())*0.1;
   FunctionType xoffset = (mXAxisRange.Width())*0.1;
   if(!mYLogScale || mYAxisRange.Min() > 0)
     fprintf(file, "set yrange[%f : %f]\n", mYAxisRange.Min(), mYAxisRange.Max() +yoffset);
   else
     fprintf(file, "set yrange[%f : %f]\n", mYAxisRange.Min() + yoffset*.1, mYAxisRange.Max() +yoffset);

   if(!mXLogScale || mXAxisRange.Min() > 0)
     fprintf(file, "set xrange[%f : %f]\n", mXAxisRange.Min(), mXAxisRange.Max());
   else
     fprintf(file, "set xrange[%f : %f]\n",mXAxisRange.Min() + xoffset*.1, mXAxisRange.Max());

   std::cout << "dumping plot data to file " << scriptName  << std::endl;
    */
   fclose(file);

   // dump out data
   for(LocalIndexType i=0; i < mSavedData.size(); i++) {
      mSavedData[i].dumpGNUPlotData(dataName[i]);
   }
}


void Plot::regenerateData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range &timeRange, Range &parameterRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID) {
  // at the accumulated Aggregators in FeatureFamilyData

  if(mRegenerateData) {
    if(mSavedData.size() == 0) {
      mXAxisRange.Initialize();
      mYAxisRange.Initialize();
    }

    if(mPlotType == CDF || mPlotType == HIST || mPlotType == WCDF) {
      if(featureFamilyID == names.size()) {
        //std::cout << "feature count can only be plotted as a time series or parameter series -- please update your plotting selections." << std::endl;
        return;
      }
      generateSpeciesDistributionData(families, names, featureFamilyID, filters, dataRange, restrictToActiveTimestep, activeTimestep, simpID);
    }
    else {
      if(featureFamilyID == names.size() && mCrossFamilyAggregator.type() == FEATURE_BASED) return;

      if(mPlotType == TIME) {
        generateTimeSeriesData(families, names, featureFamilyID, filters, dataRange, timeRange, simpID);
      }
      else if(mPlotType == PARAM) {
        generateParameterData(families, names, featureFamilyID, filters, dataRange, parameterRange, restrictToActiveTimestep, activeTimestep, simpID);
      }
    }
    mUpdatePlot = true;
    mRegenerateData = false;
    if(mYAxisRange.Width() == 0) {
      mYAxisRange.UpdateRange(mYAxisRange.Min()+0.5);
      mYAxisRange.UpdateRange(mYAxisRange.Min()-0.5);
    }
    if(mXAxisRange.Width() == 0) {
      mXAxisRange.UpdateRange(mXAxisRange.Min()+0.5);
      mXAxisRange.UpdateRange(mXAxisRange.Min()-0.5);
   }

  }

  if(mUpdatePlot) {
    mUpdatePlot = false;
    if(mPlotType == CDF || mPlotType == HIST || mPlotType == WCDF) {
     mActiveData.plotDataOnXAxis(mPlotType, mResolution, mXAxisRange, mYAxisRange);
    }
  }
}


void Plot::getRanges(std::set<LocalIndexType> &plotSelection, std::vector<Range > &ranges) {
  if(mPlotType != CDF && mPlotType != HIST && mPlotType != WCDF) return;
  mActiveData.getRanges(plotSelection, ranges);
}


void Plot::generateSpeciesDistributionData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID) {
  // first is feature id (local feature id + timestep), second is aggregated quantity
  std::vector<FunctionType> features;
  if(restrictToActiveTimestep) families[activeTimestep].getFilteredFeatures(featureFamilyID, filters, features, simpID);
  else {
    for(LocalIndexType i = 0; i < families.size(); i++) {
      families[i].getFilteredFeatures(featureFamilyID, filters, features, simpID);
    }
  }

  std::string cleanedUpName = names.cleanedUpName(featureFamilyID);
  if(mPlotType == CDF) mTitle = std::string("CDF of ") + cleanedUpName;
  else if(mPlotType == HIST) mTitle = std::string("Histogram of ") + cleanedUpName;
  else if(mPlotType == WCDF) mTitle = std::string("Weighted CDF of ") + cleanedUpName;
  if(restrictToActiveTimestep) {
    char tstep[256];
    sprintf(tstep, "%d", activeTimestep);
    mTitle += std::string(" at timestep ") + std::string(tstep);
  } else {
    mTitle += std::string(" across all timesteps");
  }
  std::string legend = mTitle;
  mXAxisLabel = cleanedUpName;
  mYAxisLabel = std::string("count");

  mActiveData.setDistributionData(features, legend, mXAxisRange);

}


void Plot::generateTimeSeriesData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range &timeRange, LocalIndexType simpID) {

  std::vector<FunctionType> features;
  std::vector<FunctionType> time;
  bool addDataPoint = true;

  for(unsigned int i=0; i < families.size(); i++) {
    std::vector<FunctionType> curFeatures;
    addDataPoint = true;
    switch (mCrossFamilyAggregator.type()) {
      case FEATURE_BASED:
        families[i].getFilteredFeatures(featureFamilyID, filters, curFeatures, simpID);
        if(curFeatures.size() > 0) {
          features.push_back(mCrossFamilyAggregator.evaluate(curFeatures));
        } else addDataPoint = false;
        break;
      case HIERARCHY_BASED:
        // Deprecated
        //families[i].filterFeatures(featureFamilyID, filters, simpID);
        //features.push_back(mCrossFamilyAggregator.evaluate(families[i].mHierarchies[simpID]));
        break;
      case UNDEFINED_AGG:
        stwarning("Family aggregator is uninitialized return 0")
        features.push_back(0);
        break;
    }

    if(addDataPoint) time.push_back(i);
  }

  std::string legend = std::string("time series");
  std::string cleanedUpName;
  if(mCrossFamilyAggregator.type() == FEATURE_BASED) {
    cleanedUpName = names.cleanedUpName(featureFamilyID);
    mTitle = std::string("Time series: ") + mCrossFamilyAggregator.name() + std::string(" ") + cleanedUpName + std::string(" vs timestep");
    mYAxisLabel = cleanedUpName;
  }
  else {
    mTitle = std::string("Time series: ") + mCrossFamilyAggregator.name() + std::string(" vs timestep");
    mYAxisLabel = mCrossFamilyAggregator.name();
  }
  mXAxisLabel = std::string("time step");
  mXAxisRange = timeRange;

  mActiveData.setNonDistributionData(features, legend, time, mYAxisRange);
}


void Plot::generateParameterData(std::vector<FeatureFamilyData > &families, AttributeVariableList &names, LocalIndexType featureFamilyID, std::map<LocalIndexType, Range > &filters, Range &dataRange, Range &parameterRange, bool restrictToActiveTimestep, LocalIndexType activeTimestep, LocalIndexType simpID) {

  FunctionType curParam = parameterRange.Min();
  FunctionType delta = parameterRange.Width()/mResolution;

  std::vector<FunctionType> features;
  std::vector<FunctionType> param;

  while(curParam <= parameterRange.Max()) {

    Attribute* paramAgg = mCrossClanAggregator->clone();

    if(restrictToActiveTimestep) {
      families[activeTimestep].updateParameter(curParam, simpID);

      std::vector<FunctionType> curFeatures;

      switch (mCrossFamilyAggregator.type()) {
        case FEATURE_BASED:
          families[activeTimestep].getFilteredFeatures(featureFamilyID, filters, curFeatures, simpID);
          if(curFeatures.size() > 0) paramAgg->addVertex(mCrossFamilyAggregator.evaluate(curFeatures),activeTimestep);
          break;
        case HIERARCHY_BASED:
          //Depracated
          //families[activeTimestep].filterFeatures(featureFamilyID, filters, simpID);
          //paramAgg->addVertex(mCrossFamilyAggregator.evaluate(families[activeTimestep].mHierarchies[simpID]), activeTimestep);
          break;
        case UNDEFINED_AGG:
          stwarning("Family aggregator is uninitialized return 0")
          paramAgg->addVertex(0,activeTimestep);
          break;
      }

    } else {

      for(unsigned int j= 0; j < families.size(); j++) {
        families[j].updateParameter(curParam, simpID);

        std::vector<FunctionType> curFeatures;

        switch (mCrossFamilyAggregator.type()) {
          case FEATURE_BASED:
            families[j].getFilteredFeatures(featureFamilyID, filters, curFeatures, simpID);
            if(curFeatures.size() > 0) paramAgg->addVertex(mCrossFamilyAggregator.evaluate(curFeatures),j);
            break;
          case HIERARCHY_BASED:
            //Deprecated
            //families[j].filterFeatures(featureFamilyID, filters, simpID);
            //paramAgg->addVertex(mCrossFamilyAggregator.evaluate(families[j].mHierarchies[simpID]),j);
            break;
          case UNDEFINED_AGG:
            stwarning("Family aggregator is uninitialized return 0")
            paramAgg->addVertex(0,activeTimestep);
            break;
        }
      }
    }

    if(paramAgg->value() != paramAgg->initialValue()) {
      features.push_back(paramAgg->value());
      param.push_back(curParam);
    }

    curParam += delta;
  }

  std::string legend = std::string("parameter series");
  std::string cleanedUpName;
  if(mCrossFamilyAggregator.type() == FEATURE_BASED) {
    cleanedUpName = names.cleanedUpName(featureFamilyID);
    mTitle = std::string("Parameter plot: ") + std::string(mCrossClanAggregator->typeName()) + std::string(" of ") + mCrossFamilyAggregator.name() + std::string(" ") + cleanedUpName;
    if(restrictToActiveTimestep) {
      char tstep[256];
      sprintf(tstep, "%d", activeTimestep);
      mTitle = mTitle + std::string(" at timestep ") + std::string(tstep)  + std::string(" vs parameter value");
    } else {
      mTitle += std::string(" over all timesteps vs parameter value");
    }
    mYAxisLabel = cleanedUpName;
  }
  else {
    mTitle = std::string("Parameter plot: ") + std::string(mCrossClanAggregator->typeName()) + std::string(" of ") + mCrossFamilyAggregator.name();
    if(restrictToActiveTimestep) {
      char tstep[256];
      sprintf(tstep, "%d", activeTimestep);
      mTitle = mTitle + std::string(" at timestep ") + std::string(tstep)  + std::string(" vs parameter value");
    } else {
      mTitle += std::string(" over all timesteps vs parameter value");
    }
    mYAxisLabel = mCrossFamilyAggregator.name();
  }
  mXAxisLabel = std::string("parameter");
  mXAxisRange = parameterRange;

  mActiveData.setNonDistributionData(features, legend, param, mYAxisRange);

}




}

