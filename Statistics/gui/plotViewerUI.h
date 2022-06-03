// generated by Fast Light User Interface Designer (fluid) version 1.0110

#ifndef plotViewerUI_h
#define plotViewerUI_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "plotWindow.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

class plotViewerUI {
public:
  plotViewerUI(const char *datafile, bool useExistingOptFile, const char *optfile, bool useThreshold, float threshold);
  Fl_Double_Window *mainWin;
  plotWindow *plotWin;
  Fl_Group *Controls;
  Fl_Text_Display *selectedSegmentText;
  Fl_Group *stateBehavior;
  Fl_Round_Button *updateStateManually;
private:
  void cb_updateStateManually_i(Fl_Round_Button*, void*);
  static void cb_updateStateManually(Fl_Round_Button*, void*);
public:
  Fl_Button *updateState;
private:
  void cb_updateState_i(Fl_Button*, void*);
  static void cb_updateState(Fl_Button*, void*);
public:
  Fl_Group *subselection;
  Fl_Button *clearAllSelections;
private:
  void cb_clearAllSelections_i(Fl_Button*, void*);
  static void cb_clearAllSelections(Fl_Button*, void*);
public:
  Fl_Group *plotting;
  Fl_Button *saveActivePlot;
private:
  void cb_saveActivePlot_i(Fl_Button*, void*);
  static void cb_saveActivePlot(Fl_Button*, void*);
public:
  Fl_Button *clearSavedPlots;
private:
  void cb_clearSavedPlots_i(Fl_Button*, void*);
  static void cb_clearSavedPlots(Fl_Button*, void*);
public:
  Fl_Button *savePlotsToPLT;
private:
  void cb_savePlotsToPLT_i(Fl_Button*, void*);
  static void cb_savePlotsToPLT(Fl_Button*, void*);
public:
  Fl_Input *filename;
  Fl_Button *savePlotsToAGR;
private:
  void cb_savePlotsToAGR_i(Fl_Button*, void*);
  static void cb_savePlotsToAGR(Fl_Button*, void*);
public:
  void show(int argc, char **argv);
};
#endif