#include "plotWindow.h"
#include <math.h>
#include <iostream>

#ifdef WIN32
#include <FTGL.h>
#include <FTGLPixmapFont.h>
#else
#include <FTGL/ftgl.h>
#include <FTGL/FTGLPixmapFont.h>
#endif
/*
#include "PortableTimer.h"
PortableTimer timer;*/
 
 

void idle_callback(void *val) {
  plotWindow *p = reinterpret_cast<plotWindow *>(val);

  if(p->mGuiDriver->guiOptUpdated()) {
    //std::cout << "gui opt updated" << std::endl;
    std::set<LocalIndexType> attributesToLoad = p->mGuiDriver->identifyAttributeVariablesToLoad();
    p->mEngine->initializeAttributeVariablesFromList(attributesToLoad);
    p->mGuiDriver->writeGuiInFile();
    p->damage(1);
    //std::cout << "wrote gui in file" << std::endl;
  }
  if(p->mGuiDriver->guiOutUpdated()) {
    std::cout << "gui out updated" << std::endl;
    portableSleep(1);
/*    timer.Init();
    timer.StartTimer();*/
    p->mGuiDriver->loadStateFromFile();
 /*   timer.EndTimer();
    printf("@@@@@@@@@@@@Time at end of guiupdated: %f \n", timer.GetTimeSecond());*/
    p->mEngine->printState();
    p->damage(1);
  }
  if(p->mGuiDriver->selectedSegmentUpdated()) {
    std::string info = p->mGuiDriver->loadSelectedSegmentFromFile();
    if(info != std::string("")){
      p->mTextDisplay->show();
      Fl_Text_Buffer *buff= new Fl_Text_Buffer();
      buff->text(info.c_str());
      p->mTextDisplay->buffer(buff);
    } else p->mTextDisplay->hide();

    p->damage(1);
  }
  Fl::repeat_timeout(0.01, idle_callback, p);

}

plotWindow::plotWindow(int x,int y,int w,int h) : Fl_Gl_Window(x,y,w,h){
//  Fl::add_idle(idle_callback,this);
  Fl::add_timeout(0.1, idle_callback, this);
};

void plotWindow::resize(int a, int b, int c, int d) {
	Fl_Gl_Window::resize(a, b, c, d);
	damage(1);
}

int plotWindow::handle(int e) {
  int retval = Fl_Gl_Window::handle(e);
  if(e == FL_PUSH) {
    //if(Fl::event_state(FL_SHIFT)) {
      pickPlotRegion();
      retval = 1;
    //} 
    Fl_Gl_Window::damage(1); 
   }
   return retval;
}


void plotWindow::draw() {
  
  if (!valid()) {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glLoadIdentity(); 
    glViewport(0,0,w(),h());
    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  }

  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();

  setPerspective();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix(); 
  drawScene();
  glPopMatrix();
};



void plotWindow::setPerspective() {
  CurDataType left, right, bottom, top;
  boundingBox(left, right, bottom, top, 0.3);

  //printf("l, r, b, t: %f %f %f %f\n", left, right, bottom, top);
  glOrtho(left, right, bottom, top,-100,100);
}

void plotWindow::initialize(const char *datafile, bool useOptionFile, const char *optionFile, bool useThreshold, float threshold, Fl_Text_Display *d) {
   mEngine = new Engine(std::string(datafile), useThreshold, threshold);
   mGuiDriver = new GUIDriver(mEngine, useOptionFile, optionFile);
   mEngine->setSubSelectionFileName(mGuiDriver->getEngineFilename());
   mTextDisplay = d;
   if(useOptionFile) updateState();
   redraw();
   //std::cout << "done initializing" << std::endl;

}

void plotWindow::drawScene() {

  std::string engineState = mEngine->getEngineStatus();
  std::string guiState = mGuiDriver->getStatus();
  if(engineState != gValidState) {
    //std::cout << "should be seeing error message" << std::endl;
    displayErrorMessage(engineState);
  } else if(guiState != gValidState) {
    displayErrorMessage(guiState);
  } else {
    glDisplay();
  }
}


void plotWindow::boundingBox(CurDataType &left, CurDataType &right, CurDataType &bottom, CurDataType &top, CurDataType buffer) const { 

  Range xRange = mEngine->getXAxisRange();
  Range yRange = mEngine->getYAxisRange();
  
  // add buffer for axes (this should match what is in display axis for consistency sake)
  xRange.UpdateRange(xRange.Max() + xRange.Width()*.1);
  yRange.UpdateRange(yRange.Max() + yRange.Width()*.1);

  std::pair<CurDataType, CurDataType> glCoordMin = glCoordinate(std::pair<CurDataType, CurDataType>(xRange.Min(), yRange.Min()));
  std::pair<CurDataType, CurDataType> glCoordMax = glCoordinate(std::pair<CurDataType, CurDataType>(xRange.Max(), yRange.Max()));

  left = glCoordMin.first-(glCoordMax.first-glCoordMin.first)*buffer;
  right = glCoordMax.first+(glCoordMax.first-glCoordMin.first)*buffer;
  bottom = glCoordMin.second-(glCoordMax.second-glCoordMin.second)*buffer;
  top = glCoordMax.second+(glCoordMax.second-glCoordMin.second)*buffer;

}

std::pair<CurDataType, CurDataType> plotWindow::glCoordinate(const std::pair<CurDataType, CurDataType> coord) const {
  std::pair<CurDataType, CurDataType> glCoord;
  CurDataType xScaleFactor, yScaleFactor;
  const CurDataType SCALINGFACTOR = 0.75;

  Range xAxisRange = mEngine->getXAxisRange();
  Range yAxisRange = mEngine->getYAxisRange();
  bool logXAxis = mEngine->getXLogScale();
  bool logYAxis = mEngine->getYLogScale();

  if(xAxisRange.Width() < 1.) {
    xScaleFactor = 1.0/xAxisRange.Width();
    //std::cout << "scaling x by " << xScaleFactor << std::endl;
  } else xScaleFactor = 1.0;
  if(yAxisRange.Width() < 1.) {
    yScaleFactor = 1.0/yAxisRange.Width();
    //std::cout << "scaling y by " << yScaleFactor << std::endl;
  } else yScaleFactor = 1.0;

  glCoord.first = (coord.first-xAxisRange.Min())*xScaleFactor+0.1; 
  // new x range = 0.1-->xAxisRange.Width()+0.1  
  CurDataType newXMax = (xAxisRange.Width())*xScaleFactor+0.1;
  CurDataType newXMin = 0.1;

  if(logXAxis) {
    glCoord.first = log(glCoord.first); 
    newXMin = log(newXMin);
    newXMax = log(newXMax);
  }

  glCoord.second = (coord.second-yAxisRange.Min())*yScaleFactor+0.1;
  CurDataType newYMax = (yAxisRange.Width())*yScaleFactor+0.1;
  CurDataType newYMin = 0.1;

  if(logYAxis) {
    glCoord.second = log(glCoord.second); 
    newYMin = log(newYMin);
    newYMax = log(newYMax);
  }

  // scale y
  glCoord.second = glCoord.second*SCALINGFACTOR*(newXMax-newXMin)/(newYMax-newYMin);

  return glCoord;
}

void plotWindow::displayAxes() const {
  Range xAxis = mEngine->getXAxisRange();
  Range yAxis = mEngine->getYAxisRange();

  std::pair<CurDataType, CurDataType> glCoord;

  glColor3f(.75, .75, .75);
  // X axis
  //std::cout << "X Axis bounds: ";
  glBegin(GL_LINES);
    glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), yAxis.Min()));
    glVertex3f(glCoord.first, glCoord.second, 0.0);
   // std::cout << "(" << glCoord.first << ", " << glCoord.second << ", 0) --> ";
    glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Max()+.1*xAxis.Width(), yAxis.Min()));
    glVertex3f(glCoord.first, glCoord.second, 0.0);
   // std::cout << "(" << glCoord.first << ", " << glCoord.second << ", 0)" << std::endl;
  glEnd();

  // Y axis 
  //std::cout << "Y Axis bounds: ";
  glBegin(GL_LINES);
    glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), yAxis.Min()));
    glVertex3f(glCoord.first, glCoord.second, 0.0);
   // std::cout << "(" << glCoord.first << ", " << glCoord.second << ", 0) --> ";
    glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), yAxis.Max()+.1*yAxis.Width()));
    glVertex3f(glCoord.first, glCoord.second, 0.0);
    //std::cout << "(" << glCoord.first << ", " << glCoord.second << ", 0)" << std::endl;
  glEnd();

  
  std::pair<CurDataType, CurDataType> minVals = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), yAxis.Min()));
  std::pair<CurDataType, CurDataType> maxVals = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Max(), yAxis.Max()));

  // X axis tics
  CurDataType cur = xAxis.Min(); 
  CurDataType delta = xAxis.Increment(4);
  if (delta > 0) { // IF there is anything to draw at all
    do {
      glBegin(GL_LINES);
      glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(cur, yAxis.Min()));
      glVertex3f(glCoord.first, glCoord.second-(maxVals.first-minVals.first)*0.01, 0.0);
      //      std::cout << "(" << glCoord.first << ", " << glCoord.second-(maxVals.first-minVals.first)*0.01 << ", 0) --> ";
      glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(cur, yAxis.Min()));
      glVertex3f(glCoord.first, glCoord.second+(maxVals.first-minVals.first)*0.01, 0.0);
      //      std::cout << "(" << glCoord.first << ", " << glCoord.second+(maxVals.first-minVals.first)*0.01 << ", 0)" << std::endl;
      glEnd();
      cur += delta;
    } while(cur <= xAxis.Max());
  }

  // Y axis  tics
  cur = yAxis.Min(); 
  delta = yAxis.Increment(4);
  if (delta > 0) {
    do {
      glBegin(GL_LINES);
      glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), cur));
      glVertex3f(glCoord.first-(maxVals.second-minVals.second)*0.01, glCoord.second, 0.0);
      //      std::cout << "(" << glCoord.first-(maxVals.second-minVals.second)*0.01 << ", " << glCoord.second << ", 0) --> ";
      glCoord = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), cur));
      //      std::cout << "(" << glCoord.first+(maxVals.second-minVals.second)*0.01 << ", " << glCoord.second << ", 0)" << std::endl;
      glVertex3f(glCoord.first+(maxVals.second-minVals.second)*0.01, glCoord.second, 0.0);
      glEnd();
      cur += delta;
    } while(cur <= yAxis.Max());
  }
}

void plotWindow::glDisplay() const {
  glDisable(GL_LIGHTING);

  
  // draw axes
  displayAxes();
   
  // display labels
  displayLabels();
   
  // display active plot
  displayPlot(mEngine->getActivePlotData(), 0.0, 0.0, 0.0, 2);

  // display saved plots
  for(uint32_t i=0; i < mEngine->getNumSavedPlots(); i++) {
    float red, green, blue;
    colorLookup(i, red, green, blue);
    displayPlot(mEngine->getSavedPlotData(i), red, green, blue, 2);
  }

  if(mEngine->pickablePlot()) {
    //std::cout << "trying to display selection" << std::endl;
    displayPickedRegion();
  }

  glEnable(GL_LIGHTING);

}

void plotWindow::displayPickedRegion() const {
  PlotData data = mEngine->getActivePlotData();
  if(data.getNumCoordinates() == 0) return;
  glPushMatrix();
  glEnable(GL_LIGHTING);

  material(205./256., 0., 65./256., 0.2, 0, 1);
  glNormal3d(0., 0., 1.0);

  Range yAxis = mEngine->getYAxisRange();
  std::pair<CurDataType, CurDataType> glMinCoord = glCoordinate(std::pair<CurDataType, CurDataType>(0.0, yAxis.Min()));

  for(uint32_t i =0; i < data.getNumCoordinates()-1; i++) {
    if(mEngine->plotSelectionContains(i)) { 
      glBegin(GL_QUADS);
      std::pair<CurDataType, CurDataType> glCoord = glCoordinate(data.getCoordinate(i));
      std::pair<CurDataType, CurDataType> nextglCoord = glCoordinate(data.getCoordinate(i+1));
      glVertex3f(glCoord.first, glMinCoord.second, 0.0);
      if(mEngine->getPlotType() == HIST) {
        glVertex3f(glCoord.first, nextglCoord.second, 0.0);
      } else if(mEngine->getPlotType() == CDF) {
        glVertex3f(glCoord.first, glCoord.second, 0.0);
      }
      glVertex3f(nextglCoord.first, nextglCoord.second, 0.0);
      glVertex3f(nextglCoord.first, glMinCoord.second, 0.0);
      glEnd();
    } 
  }
  glDisable(GL_LIGHTING);
  glPopMatrix();
}

void plotWindow::displayPlot(const PlotData &data, float red, float green, float blue, float lineWidth) const {

  glPushMatrix();
  glLineWidth(lineWidth);
  glColor3f(red, green, blue);			
  glBegin(GL_LINE_STRIP);
  for(uint32_t i =0; i < data.getNumCoordinates(); i++) {
    std::pair<CurDataType, CurDataType> glCoord = glCoordinate(data.getCoordinate(i));
  //std::cout << "drawing vert: (" << glCoord.first << ", " << glCoord.second << ", 0) " << std::endl;
    glVertex3f(glCoord.first, glCoord.second, 0.0);
    if(mEngine->getPlotType() == HIST) {
      if(i < data.getNumCoordinates()-1) {
        std::pair<CurDataType, CurDataType> nextglCoord = glCoordinate(data.getCoordinate(i+1));
      //std::cout << "drawing vert: (" << nextglCoord.first << ", " << glCoord.second << ", 0) " << std::endl;
        glVertex3f(glCoord.first, nextglCoord.second, 0.0);
      } else {
        Range yAxis = mEngine->getYAxisRange();
        std::pair<CurDataType, CurDataType> glMinCoord = glCoordinate(std::pair<CurDataType, CurDataType>(0.0, yAxis.Min()));
        glVertex3f(glCoord.first, glMinCoord.second, 0.0);
      }
    }
  }
  glEnd();
  if(mEngine->getPlotType() != HIST) {
    for(uint32_t i =0; i < data.getNumCoordinates(); i++) {
      displayVertex(data, i, 0, 0, 0);
    }
  }
  glPopMatrix();
}

void plotWindow::displayVertex(const PlotData &data, uint32_t selected_vert, float red, float green, float blue) const {
  std::pair<CurDataType, CurDataType> glCoord = glCoordinate(data.getCoordinate(selected_vert));
  glPushMatrix();
  glColor3f(red, green, blue);			
  glPointSize(4);
  glBegin(GL_POINTS);
    glVertex3f(glCoord.first, glCoord.second, 0.0);
  glEnd();

  glPointSize(1);
  glPopMatrix();
}

void plotWindow::displayLabels() const {
  FTGLPixmapFont font("Verdana.ttf");
  if(font.Error()) {
    std::cout << "error! Font file Verdana.ttf missing" << std::endl;
    return;
  }

  CurDataType DEFAULT_HEIGHT = 535;
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  
  CurDataType titleLeft, titleRight, titleTop, titleBottom;
  boundingBox(titleLeft, titleRight, titleBottom, titleTop, 0.1);

  CurDataType labelLeft, labelRight, labelTop, labelBottom;
  boundingBox(labelLeft, labelRight, labelBottom, labelTop, 0.25);

  Range xAxis = mEngine->getXAxisRange();
  Range yAxis = mEngine->getYAxisRange();
  std::pair<CurDataType, CurDataType> plotMin = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Min(), yAxis.Min()));
  std::pair<CurDataType, CurDataType> plotMax = glCoordinate(std::pair<CurDataType, CurDataType>(xAxis.Max(), yAxis.Max()));

  char xAxisMin[256], xAxisMax[256], yAxisMin[256], yAxisMax[256];
  sprintf(xAxisMin, "%1.3e", xAxis.Min());
  sprintf(xAxisMax, "%1.3e", xAxis.Max());
  sprintf(yAxisMin, "%1.3e", yAxis.Min());
  sprintf(yAxisMax, "%1.3e", yAxis.Max());


  glColor3f(0, 0, 0);
  font.FaceSize(15*viewport[3]/DEFAULT_HEIGHT);
 
  glPushMatrix();
    // Display Title
    glRasterPos2f(titleLeft, titleTop);
    font.Render(mEngine->getPlotTitle().c_str());
    std::cout << mEngine->getPlotTitle().c_str() << std::endl;

    // Display XAxisLabel
    glRasterPos2f(0.4*(labelLeft+labelRight), labelBottom);
    font.Render(mEngine->getXAxisLabel().c_str());
    //std::cout << mEngine->getXAxisLabel().c_str() << std::endl;
    
    // Display YAxisLabel
    glRasterPos2f(labelLeft, 0.5*(labelTop+labelBottom));
    font.Render(mEngine->getYAxisLabel().c_str());
    //std::cout << mEngine->getYAxisLabel().c_str() << std::endl;
    
    // Display XAxisMin
    glRasterPos2f(plotMin.first, titleBottom);
    font.Render(xAxisMin);
    //std::cout << xAxisMin << std::endl;
    
    // Display XAxisMax
    glRasterPos2f(plotMax.first, titleBottom);
    font.Render(xAxisMax);
    //std::cout << xAxisMax << std::endl;
    
    // Display YAxisMin
    glRasterPos2f(labelLeft+0.175*(titleLeft-labelLeft), plotMin.second);
    font.Render(yAxisMin);
    //std::cout << yAxisMin << std::endl;
    
    // Display YAxisMax
    glRasterPos2f(labelLeft+0.175*(titleLeft-labelLeft), plotMax.second);
    font.Render(yAxisMax);
    //std::cout << yAxisMax << std::endl;
  //glPopMatrix();
  glPopMatrix();
}


void plotWindow::displayErrorMessage(const std::string &message) {

  glClearColor(1.0, 1.0, 1.0, 0.0);
  glLoadIdentity(); 
  glViewport(0,0,w(),h());

  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();

  mEngine->setXAxisRange(Range(-10, 10));
  mEngine->setYAxisRange(Range(-10, 10));

  setPerspective();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  glDisable(GL_LIGHTING);
  FTGLPixmapFont font("Verdana.ttf");
  if(font.Error()) {
    std::cout << "error!" << std::endl;
    return;
  }

  CurDataType DEFAULT_HEIGHT = 535;
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  
  CurDataType messageLeft, messageRight, messageTop, messageBottom;
  boundingBox(messageLeft, messageRight, messageBottom, messageTop, 0.05);

  glColor3f(0, 0, 0);
  font.FaceSize(15*viewport[3]/DEFAULT_HEIGHT);
 
  glPushMatrix();
    // Display Message
    glRasterPos2f(messageLeft, messageTop);
    font.Render(message.c_str());

  glPopMatrix();
  glEnable(GL_LIGHTING);
}



void plotWindow::pickPlotRegion() {
  if(!mEngine->pickablePlot()) return; 

  PlotData data = mEngine->getActivePlotData();
  if(data.getNumCoordinates() == 0) return;

  std::cout << "about to start picking!" << std::endl;
  startPicking();
  glPushName(0);
  
  Range yAxis = mEngine->getYAxisRange();
  std::pair<CurDataType, CurDataType> glMinCoord = glCoordinate(std::pair<CurDataType, CurDataType>(0.0, yAxis.Min()));

  for(uint32_t i =0; i < data.getNumCoordinates()-1; i++) {
    glLoadName(i);
    glBegin(GL_QUADS);
    std::pair<CurDataType, CurDataType> glCoord = glCoordinate(data.getCoordinate(i));
    std::pair<CurDataType, CurDataType> nextglCoord = glCoordinate(data.getCoordinate(i+1));
    glVertex3f(glCoord.first, glMinCoord.second, 0.0);
    if(mEngine->getPlotType() == HIST) {
      glVertex3f(glCoord.first, nextglCoord.second, 0.0);
    } else if(mEngine->getPlotType() == CDF) {
      glVertex3f(glCoord.first, glCoord.second, 0.0);
    }
    glVertex3f(nextglCoord.first, nextglCoord.second, 0.0);
    glVertex3f(nextglCoord.first, glMinCoord.second, 0.0);
    glEnd();

    glBegin(GL_POINTS);
    if(mEngine->getPlotType() == CDF) glVertex3f(0.5*(glCoord.first+nextglCoord.first), 0.5*glMinCoord.second+0.25*glCoord.second+0.25*nextglCoord.second, 0.);
    if(mEngine->getPlotType() == HIST) glVertex3f(0.5*(glCoord.first+nextglCoord.first), 0.5*(glMinCoord.second+nextglCoord.second), 0.);
    glEnd();
  }


  int hits = stopPicking();
  std::cout << "hits = " << hits << std::endl;
  if(hits > 0) {
    int selectedRegion = processHits(hits);
    mEngine->updatePlotSelection(selectedRegion);
    mEngine->writeSubSelectedIDsToFile();
   }
}

int plotWindow::processHits(int hits) {
  int selected;
  GLuint names, *ptr, minZ,*ptrNames, numberOfNames;
  ptr = (GLuint *) mSelectBuffer;
  minZ = 0xffffffff;
  for(int i = 0; i < hits; i++) {	
    names = *ptr;
    ptr++;
    if(*ptr < minZ) {
      numberOfNames = names;
      minZ = *ptr;
      ptrNames = ptr+2;
    }
    ptr += names+2;
  }
  std::cout << "The closest hit is ";
  ptr = ptrNames;
  for(unsigned int j = 0; j < numberOfNames; j++,ptr++) {
    std::cout << *ptr << " ";
    selected = *ptr;
  }
  std::cout << std::endl;
  return selected;
}

void plotWindow::startPicking() {
  int x = Fl::event_x();
  int y = Fl::event_y();
  glSelectBuffer(BUFFERSIZE, mSelectBuffer);
  glRenderMode(GL_SELECT);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  GLint view[4];
  glGetIntegerv(GL_VIEWPORT, view);

  gluPickMatrix(x,view[3]-y,15,15,view);

  setPerspective();
	

  glMatrixMode(GL_MODELVIEW);
  glInitNames(); 
}

int plotWindow::stopPicking() {
  // restoring the original projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();

  // returning to normal rendering mode
  return (glRenderMode(GL_RENDER));
}


