/*
 * output_feature_geometry.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: bremer5
 */

#include <vector>

#include "vtkSmartPointer.h"
#include "vtkUniformGrid.h"
#include "vtkMarchingCubes.h"
#include "vtkCell.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkPolyDataConnectivityFilter.h"

#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SegmentationHandle.h"
#include "SimplificationHandle.h"
#include "StatHandle.h"
#include "FileData.h"
#include "ValueElement.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"

using namespace std;
using namespace TopologyFileFormat;


int write_obj(const char* filename,vtkSmartPointer<vtkPolyData> surface)
{
  FILE *output = fopen(filename,"w");
  uint32_t i;
  double* p;
  vtkCell* cell;

  for (i=0;i<surface->GetNumberOfPoints();i++) {
    p = surface->GetPoint(i);
    fprintf(output,"v %f %f %f\n",p[0],p[1],p[2]);
  }

  for (i=0;i<surface->GetNumberOfPolys();i++) {
    cell = surface->GetCell(i);
    sterror(cell->GetNumberOfPoints()!=3,"Expected triangle.");

    fprintf(output,"f %lld %lld %lld\n",cell->GetPointId(0)+1,
    cell->GetPointId(1)+1,cell->GetPointId(2)+1);
  }

  fclose(output);

  return 1;
}

int write_obj(FILE* output,vtkSmartPointer<vtkPolyData> surface)
{
  static int size = 0;
  uint32_t i;
  double* p;
  vtkCell* cell;

  //fprintf(stderr,"WRiting more\n");
  for (i=0;i<surface->GetNumberOfPoints();i++) {
    p = surface->GetPoint(i);
    fprintf(output,"v %f %f %f\n",p[0],p[1],p[2]);
  }

  for (i=0;i<surface->GetNumberOfPolys();i++) {
    cell = surface->GetCell(i);
    sterror(cell->GetNumberOfPoints()!=3,"Expected triangle.");

    fprintf(output,"f %lld %lld %lld\n",cell->GetPointId(0)+1+size,
    cell->GetPointId(1)+1+size,cell->GetPointId(2)+1+size);
  }

  size += surface->GetNumberOfPoints();

  return 1;
}

//!Number of available input options (size of gOptions)1
#define NUM_OPTIONS 8


//!Array with the list of all available input options
static const char* gOptions[NUM_OPTIONS] = {
    "--help",

    "--i",
    "--o",
    "--per-feature-file",

    "--threshold",
    "--min-length",
    "--filter",
    "--smoothing-steps",

};

//! Name of the input file
const char* gInputFamilyName = NULL;

//! Name of the input file
const char* gInputSegmentationName = NULL;

//! Name of the output file
const char* gOutputFileName = NULL;

//! Name of the per feature output file
const char* gOutputPerFeatureName = NULL;

//! Minimal length of a feature in function value
FunctionType gParameterValue = -1;

//! Minimal length of a feature in function value
FunctionType gLengthThreshold = -1;

//! The name of the attribute to filter on
const char* gAttributeName = NULL;

//! The name of the attribute to filter on
const char* gAggregatorName = NULL;

//! The range of attribute values that are acceptable
FunctionType gFilterRange[2] = {0,0};

//! The number of smoothing steps
int32_t gSmoothingSteps = -1;

void print_help(FILE* output, const char* exec)
{
  fprintf(output,"Usage: %s [options]\nWhere options can be any of the following:\n\n",exec);

  fprintf(output,"--h \n\tPrint this help\n");
  fprintf(output,"--i <family-file> <segmentation-file>\n\tFilenames for the input\n");
  fprintf(output,"--o <filename.obj>\n\tFilename of the output file if not provided stdout will be used\n");
  fprintf(output,"--per-feature-file <name-template.obj>\n\t Template to be used for per-feature files\n");

  fprintf(output,"--threshold <float>\n\tThe parameter value used to define features\n");
  fprintf(output,"--min-length <float>\n\tThe minimal span in parameter values of feature that should be considered.\n");

  fprintf(output,"--filter <agg-name> <attribute-name> <low> <high>\n\t Additional attribute to use as filter\n");
  fprintf(output,"--smoothing-steps <int>\n\tNumber of smoothing steps desired (if any) \n");
}


/*! \brief Parse the command line input.
 *
 * This function parses the command line input containing the various
 * execution options and specifies the corresponding global variables used
 * during execution accordingly.
 * Note: The available execution options are defined in gOptions.
 * \param argc : The number of input arguments. (As given to main(...)).
 * \param argv : Array of lengths argc containing all input arguments.
 *               (As given to main(...)).
 * \return int : 0 in case of error and 1 in case of successs
 */
int parse_command_line(int argc, const char** argv)
{
  int i,j,option;

  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < NUM_OPTIONS;j++) {
      if(strcmp(gOptions[j],argv[i])==0) {
        option = j;
        break;
      }
    }

    switch (option) {

    case -1:  // Wrong input parameter
      fprintf(stderr,"\nError: Wrong input parameter \"%s\"\nTry %s --help\n\n",argv[i],argv[0]);
      return 0;
    case 0:   // --help
      print_help(stdout,argv[0]);
      return 0;
    case 1: // --i
      gInputFamilyName = argv[++i];
      gInputSegmentationName = argv[++i];
      break;
    case 2: // --o
      gOutputFileName = argv[++i];
      break;
    case 3: // --per-feature_file
      gOutputPerFeatureName = argv[++i];
      break;
    case 4: // --threshold
      gParameterValue = (FunctionType)atof(argv[++i]);
      break;
    case 5: // --min-length
      gLengthThreshold = (FunctionType)atof(argv[++i]);
      break;
    case 6: // --filter
      gAggregatorName = argv[++i];
      gAttributeName = argv[++i];
      gFilterRange[0] = (FunctionType)atof(argv[++i]);
      gFilterRange[1] = (FunctionType)atof(argv[++i]);
      break;
    case 7: // --smoothing-steps
      gSmoothingSteps = atoi(argv[++i]);
      break;
    default:
      return 0;
    }
  }

  return 1;
}



//! This executable will output triangulated surfaces describing features
/*! Given a feature family, a segmentation,a nd a threshold this executable
 *  will extract all fetaures at the given threshold, use a minimal enclosing
 *  bounding box to create a binary map, and finally use marching cubes to
 *  extract an iso-surface describing the feature geometry. The surfaces will
 *  be written in obj format
 *
 */

int main(int argc,const char *argv[])
{
  //Parse the command line input and define the execution settings
  if (parse_command_line(argc,argv) == 0) {
    print_help(stdout,argv[0]);
    return 0;
  }

  ClanHandle clan;
  FamilyHandle family;
  SimplificationHandle simp;
  StatHandle stat_handle;
  SegmentationHandle seg_handle;
  FeatureHierarchy hierarchy;
  FeatureSegmentation seg;

  clan.attach(gInputFamilyName);
  family = clan.family(0);
  simp = clan.family(0).simplification(0);

  hierarchy.initialize(simp);

  clan.attach(gInputSegmentationName);
  seg_handle = clan.family(0).segmentation();
  seg.initialize(seg_handle);


  Data<FunctionType> filter_data;

  if (gAttributeName != NULL) {

    std::vector<StatHandle> stats = family.aggregateList();

    for (uint8_t i=0;i<stats.size();i++) {
      fprintf(stderr,"Aggregate; %s %s\n",stats[i].species().c_str(),stats[i].stat().c_str());
    }

    stat_handle = family.aggregate(gAggregatorName,gAttributeName);
    filter_data.resize(stat_handle.elementCount());
    stat_handle.readData(filter_data);
  }

  if (seg_handle.domainType() != REGULAR_GRID) {
    fprintf(stderr,"Feature geometry is only supported for regular grids\n");
    return 0;
  }

  std::string desc = seg_handle.domainDescription();
  uint32_t dim[3];
  uint32_t dummy;

  sscanf(desc.c_str(),"%d %d %d %d",&dummy,dim,dim+1,dim+2);


  const char *basename = "feature_%04d.obj";
  char filename[100];

  hierarchy.parameter(gParameterValue);

  vector<Segment> segments;
  vtkSmartPointer<vtkUniformGrid> grid;
  vtkSmartPointer<vtkPolyData> surface;
  //vtkSmartPointer<vtkSmoothPolyDataFilter> filter = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
  vtkSmartPointer<vtkWindowedSincPolyDataFilter> filter = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();

  vtkSmartPointer<vtkMarchingCubes> mc = vtkSmartPointer<vtkMarchingCubes>::New();
  mc->ComputeNormalsOff();
  mc->ComputeGradientsOff();

  if (gSmoothingSteps > 0) {
    filter->SetNumberOfIterations(gSmoothingSteps);
    //filter->SetRelaxationFactor(0.25);
    filter->SetPassBand(0.1);
  }

  // A list of active features that comply with all filters and other flags
  std::vector<const Feature*> valid_features;
  FunctionType value;
  // If we do not have an explicit size threshold
  if (gLengthThreshold < 0) {
    // The we can use a living iterator to save some time
    for (FeatureHierarchy::LivingIterator it=hierarchy.beginLiving(gParameterValue);it!=hierarchy.endLiving();it++) {

      // If we wanted to filter based on a secondary attribute
      if (gAttributeName != NULL) {

        value = filter_data[it->id()];

        if ((value < gFilterRange[0]) || (value > gFilterRange[1]))
          continue;
      }

      valid_features.push_back(*it);
    }
  }
  else { // If we do have a length threshold
    // we need to look at all features because the length criterion
    // is not monotonic
    for (auto it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

      if (it->id() == 52) {
        fprintf(stderr,"Feature %d   [%f,%f]   agent %d  - %f\n",it->id(), it->lifeTime()[0],it->lifeTime()[1],
                it->agent()->id(),it->agent()->lifeTime()[0]);

        fprintf(stderr,"Feature %d   [%f,%f]   agent %d  - %f\n",it->rep(0)->id(), it->rep(0)->lifeTime()[0],it->rep(0)->lifeTime()[1],
                it->rep(0)->agent()->id(),it->rep(0)->agent()->lifeTime()[0]);
      }

      if (it->direction()) {// Ascending hierarchy (split trees)

        // If our local cut does not intersect this arc
        if ((it->lifeTime()[1] - it->agent()->lifeTime()[0] < gLengthThreshold)
            || (it->lifeTime()[0] - it->agent()->lifeTime()[0] >= gLengthThreshold)) {
          continue;
        }
      }
      else {
        if ((it->agent()->lifeTime()[1] - it->lifeTime()[0] < gLengthThreshold)
            || (it->agent()->lifeTime()[1] - it->lifeTime()[1] >= gLengthThreshold)) {
          continue;
        }
      }

      // If we wanted to filter based on a secondary attribute
      if (gAttributeName != NULL) {

        value = filter_data[it->id()];

        if ((value < gFilterRange[0]) || (value > gFilterRange[1]))
          continue;
      }

      fprintf(stderr,"\tFeature %d lifetime %f %f %f  %d\n",it->id(),it->agent()->lifeTime()[0],it->lifeTime()[1],filter_data[it->id()],it->repSize());

      valid_features.push_back(&(*it));

      //break;
    }
  }



  int count = 0;
  FILE* features = fopen(gOutputFileName,"w");
  for (auto it=valid_features.begin();it!=valid_features.end();it++) {

    seg.segmentation(*it,segments);

    int total_size = 0;
    for (uint32_t i=0;i<segments.size();i++)
      total_size += segments[i].size;


    fprintf(stderr,"Working on %d of %d features with %d vertices\n",it-valid_features.begin(),valid_features.size(),total_size);



    uint32_t tmp_dim[3] = {dim[0],dim[1],dim[2]};
    grid = segments_to_grid(segments,tmp_dim);

    if (grid->GetNumberOfPoints() == 0)
      continue;

    // If this bounding box touched the boundary skip this feature
    if (tmp_dim[0]*tmp_dim[1]*tmp_dim[2] == 0)
      continue;

    //fprintf(stderr,"Grid size %d from feature of size %f\n", grid->GetNumberOfPoints(), filter_data[(*it)->id()] );

    mc->SetInputData(grid);
    mc->SetValue(0,0.5);
    mc->Update();


    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connectivityFilter->SetInputData(mc->GetOutput());
    connectivityFilter->SetExtractionModeToLargestRegion();
    connectivityFilter->Update();

    surface = connectivityFilter->GetOutput();

    if (surface->GetNumberOfPolys() == 0)
      continue;

    if (gSmoothingSteps > 0) {
      //fprintf(stderr,"Smoothing %d steps\n",smooth);
      filter->SetInputData(surface);
      filter->Update();
      surface = filter->GetOutput();
    }

    if (gOutputPerFeatureName != NULL) {
      sprintf(filename,gOutputPerFeatureName,count++);
      write_obj(filename,surface);
    }
    write_obj(features,surface);

    //fprintf(stderr,"Points %lld Vertex %lld Lines %lld Polys %lld\n",surface->GetNumberOfPoints(),surface->GetNumberOfVerts(),
    //        surface->GetNumberOfLines(),surface->GetNumberOfPolys());

    //if (count > 100)
    //  break;
  }

  fclose(features);
  return 1;
}



