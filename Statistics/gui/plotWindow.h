#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include<stdlib.h>

#include "GUIDriver.h"
#include "Engine.h"
#include "DisplayTools.h"

#ifdef MAC_OS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#ifdef WIN32
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "FL/Fl.H"
#include "FL/Fl_Gl_Window.H"
#include "FL/Fl_Text_Buffer.H"
#include "FL/Fl_Text_Display.H"

#include "math.h"



void idle_callback(void *v);

using namespace Statistics;

class PlotViewerUI;

const int BUFFERSIZE = 512;
class plotWindow : public Fl_Gl_Window {
public:

  GUIDriver *mGuiDriver;
  Engine *mEngine;
  Fl_Text_Display *mTextDisplay;

  GLuint mSelectBuffer[BUFFERSIZE];

  plotWindow(int x,int y,int w,int h);
  ~plotWindow() { delete mEngine; delete mGuiDriver; };
  void initialize(const char *datafile, bool useExistingOptFile, const char *optFileName, bool useThreshold, float threshold, Fl_Text_Display *d);
  
  // standard fltk
  void draw();	
  int handle(int);
  void resize(int, int, int, int);

  void drawScene();
  void setPerspective();
  void saveActivePlot() { mEngine->saveActivePlot(); };
  void clearSavedPlots() { mEngine->clearSavedPlots(); };
  void savePlotsToPLT(std::string &filename) { mEngine->savePlotToPLT(filename); };
  void savePlotsToAGR(std::string &filename) { mEngine->savePlotToAGR(filename); };
  void clearAllSelections() { 
    mEngine->clearRangeSelection(); 
    mEngine->clearPlotSelection();
    mGuiDriver->writeGuiInFile(); // call to update sliders
    mEngine->writeSubSelectedIDsToFile();
  }
  void setUpdateStateManually(bool val) { mGuiDriver->setUpdateStateManually(val); }
  void updateState() { 
    mEngine->writeSubSelectedIDsToFile();
    mGuiDriver->setStateUpdated(true);
    //std::cout << "plot window updating state: " << mGuiDriver->getStateUpdated() << std::endl; 
  };

  void boundingBox(CurDataType &left, CurDataType &right, CurDataType &bottom, CurDataType &top, CurDataType buffer=0.0) const;
  std::pair<CurDataType, CurDataType> glCoordinate(const std::pair<CurDataType, CurDataType> coord) const;
  void glDisplay() const;

  void displayAxes() const;
  void displayPlot(const PlotData&data, float red, float green, float blue, float lineWidth) const;
  void displayVertex(const PlotData &data, uint32_t selected_vert, float red, float green, float blue) const;
  void displayLabels() const;
  void displayPickedRegion() const;
  void displayErrorMessage(const std::string &message);
  void printStats();
  

  void startPicking();
  int stopPicking();
  int processHits(int hits);
  void pickPlotRegion();
  
 
};
#endif
