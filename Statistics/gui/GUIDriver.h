#ifndef GUIDRIVER_H
#define GUIDRIVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <set>
#include <iostream>
#include <cstdio>
#include "Engine.h"

using namespace Statistics;

void portableSleep(uint32_t time);


class GUIDriver  {
private:
  GUIDriver() { mEngine = NULL; };
public:
  GUIDriver(Engine* eg, bool useExistingOptFile, const char *optFileName);
  ~GUIDriver();
  bool guiOptUpdated();
  bool guiOutUpdated();
  bool selectedSegmentUpdated();
  void writeGuiInFile();
  void loadStateFromFile();
  void setUpdateStateManually(bool val) { mUpdateStateManually = val; }
  bool getUpdateStateManually() const { return mUpdateStateManually; }
  void setStateUpdated(bool val) { mStateUpdated = val; }
  bool getStateUpdated() const { return mStateUpdated; }
  std::string getGuiOptFilename() const { return mGuiOptFilename; }
  std::string getSelectedSegmentFilename() const { return mSelectedSegmentFilename; }
  std::string getEngineFilename() const { return mEngineFilename; }

  uint32_t aggregatorToInt(const std::string &agg);
  std::string intToAggregator(uint32_t id);

  std::set<LocalIndexType> identifyAttributeVariablesToLoad() const;

  std::string getStatus() const;
  void resetSliders();
  bool getSelectedSegment() const { return mSelectedSegment; }
  std::string loadSelectedSegmentFromFile();


public:
	
  Engine* mEngine;

  std::string mGuiInFilename;
  std::string mGuiOutFilename;
  std::string mGuiOptFilename;
  std::string mSelectedSegmentFilename;
  std::string mEngineFilename;

  time_t mLastGuiOptTime;
  time_t mLastGuiOutTime;
  time_t mLastSelectedSegmentTime;

  bool mUpdateStateManually, mStateUpdated;

  int mSelectedSegment;

};


#endif
