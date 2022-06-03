/*
 * process_statistics.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: bremer5
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <cmath>
#include <stack>
#include <algorithm>
#include <iterator>

#include "Value.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"
#include "ValueElement.h"
#include "HandleCollection.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;


//! Number of available input options
#define NUM_OPTIONS 12

//! Array with the list of options
static const char* gOptions[NUM_OPTIONS] = {

    "--help",
    "--i",
    "--family-agg",
    "--time-agg",
    "--type",
    "--attribute",
    "--stat",
    "--o",
    "--low",
    "--high",
    "--filter",
    "--l"
};

//! The list of file names to process
vector<string> gFileNames;

//! The aggregator used to aggregate between families
Attribute* gCrossFamily = NULL;

//! The aggregator used across time
Attribute* gCrossTime = NULL;

//! The parameter at which to evaluate the hierarchies
double gParameter = 0;

//! The parameter range at which to evaluate
double gRange[2] = {0,0};

//! The number of steps used for the range
uint32_t gStepCount = 2;

//! Number of buckets of a histogram
uint32_t gNumBuckets = 1;

//! The lower bound of the accepted values
FunctionType gLowerBound = -10e34;

//! The upper bound of the accepted values
FunctionType gUpperBound = 10e34;

enum PlotType {
  CDF = 0,
  HIST = 1,
  TIME = 2,
  PARAM = 3,
  LIST = 4,
  UCDF = 5,
  WCDF = 6,
  LPARAM = 7,
  EPARAM = 8,
};
#define NUM_PLOT_TYPES 9
const char* gPlotTypeOptions[NUM_PLOT_TYPES] = {
    "cdf",
    "hist",
    "time",
    "param",
    "list",
    "ucdf",
    "wcdf",
    "log-param",
    "exp-param",
};

PlotType gPlotType = LIST;

string gAttribute = "Unknown";
string gStatistics = "Unknown";

string gFilterAttribute = "Unknown";
string gFilterStatistics = "Unknown";


const char* gOutputFileName = NULL;

void print_help(FILE* output, const char* exe)
{
  fprintf(output,"Usage: %s <options> where options are \n\n",exe);
  fprintf(output,"\t--i <file0.family> ... <fileN.family>: input files\n");
  fprintf(output,"\t--l <filelist.txt>: a text file containing the sorted list of input files (absolute paths)\n");
  fprintf(output,"\t--o <filename> : output file (default: stdout)\n");
  fprintf(output,"\t--family-agg <aggregator-name>: aggregator used to accumulate values within a time step\n");
  fprintf(output,"\t--time-agg <aggregator-name>: aggregator used to accumulate values across time\n");
  fprintf(output,"\t--type <sub-options>: the type of data desired\n");
  fprintf(output,"\t\t list      <parameter>               : A list of values at the given parameter\n");
  fprintf(output,"\t\t cdf       <parameter>               : A CDF at the given parameter\n");
  fprintf(output,"\t\t ucdf      <parameter>               : Unnormalized CDF at the given parameter\n");
  fprintf(output,"\t\t wcdf      <parameter>               : Weighted CDF at the given parameter\n");
  fprintf(output,"\t\t hist      <parameter> <num-buckets> : A histogram at the given parameter\n");
  fprintf(output,"\t\t time      <parameter>               : A plot over time\n");
  fprintf(output,"\t\t param     <min> <max> <step>        : A plot by parameter with the given number of steps\n");
  fprintf(output,"\t\t log-param <min> <max> <step>        : A plot by parameter with the given number of steps using a logarithmic sampling\n");
  fprintf(output,"\t\t exp-param <min> <max> <step>        : A plot by parameter with the given number of steps using an exponential sampling\n\n");


  fprintf(output,"\t--attribute <name>: the attribute of interest (e.g. temperature)\n");
  fprintf(output,"\t--stat <name>: the statistics of interest (e.g. mean)\n\n");


  fprintf(output,"\t--filter <attribute> <statistic>: the attribute and statistics of the values used for filtering\n");
  fprintf(output,"\t--low  <value>: lower bound of the accepted filter values\n");
  fprintf(output,"\t--high <value>: upper bound of the accepted filter values\n");
}

int parse_command_line(int argc, const char** argv)
{
  int i,j,option;
  Factory fac;

  if (argc < 2) {
    print_help(stdout,argv[0]);
    return 0;
  }


  for (i=1;i<argc;i++) {
    option = -1;
    for (j=0; j < NUM_OPTIONS;j++) {
      if(strcmp(gOptions[j],argv[i])==0)
        option= j;
    }
    switch (option) {

      case -1:  // Wrong input parameter
        fprintf(stderr,"\nError: Wrong input parameter \"%s\"\nTry %s --help\n\n",argv[i],argv[0]);
        return 0;
      case 0:   // --help
        print_help(stdout,argv[0]);
        return 0;
      case 1: // --input
      {
        istringstream iss(argv[++i]);
        copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter(gFileNames));

        //gFileNames.push_back(string(argv[++i]));

        while ((i < argc-1) && (strncmp("--",argv[i+1],2) != 0)) {
          istringstream iss(argv[++i]);
          copy(istream_iterator<string>(iss),
               istream_iterator<string>(),
               back_inserter(gFileNames));
          //gFileNames.push_back(string(argv[++i]));
        }

        break;
      }

      // ------------------------------------------------------------------------
      // added by Harsh on Aug 19, 2019 to support a large number of input files
      case 11: {  //--l

        std::string fname = argv[++i];
        std::ifstream infile(fname);
        std::string line;
        while (std::getline(infile, line)) {
            if (line.rfind(".family") == (line.length()-7)){
                gFileNames.push_back(line);
            }
        }
        infile.close();
        //std::cout << " got " << gFileNames.size() << " files!\n";
        break;
      }
      // ------------------------------------------------------------------------

      case 2: {// --family-agg
        string name(argv[++i]);

        gCrossFamily = fac.make_aggregator(name);
        if (gCrossFamily == NULL) {
          fprintf(stderr,"\nError: family aggergator \"%s\" not recognized\n",argv[i-1]);
          return 0;
        }

        break;
      }
      case 3: {// --time-agg
        string tname(argv[++i]);

        gCrossTime = fac.make_aggregator_array(tname,true);
        if (gCrossTime == NULL) {
          fprintf(stderr,"\nError: time aggergator \"%s\" not recognized\n",argv[i-1]);
          exit(0);
        }

        break;
      }
      case 4: {// --type
        i++;
        for (j=0; j < NUM_PLOT_TYPES;j++) {
          if(strcmp(gPlotTypeOptions[j],argv[i])==0) {
            gPlotType = (PlotType)j;
            break;
          }
        }
        if (j == NUM_PLOT_TYPES) {
          fprintf(stderr,"Sorry, the plot type \"%s\"is not recognized .....\n",argv[i]);
          return 0;
        }

        switch (gPlotType)  {
          case CDF:
          case UCDF:
          case WCDF:
          case LIST:
          case TIME:
            gParameter = atof(argv[++i]);
            break;
          case HIST:
            gParameter = atof(argv[++i]);
            gNumBuckets = atoi(argv[++i]);
            break;
          case PARAM:
          case LPARAM:
          case EPARAM:
            gRange[0] = atof(argv[++i]);
            gRange[1] = atof(argv[++i]);
            gStepCount = atoi(argv[++i]);

            if (gRange[1] < gRange[0])
              std::swap(gRange[0],gRange[1]);
            break;
        }

        break;
      }
      case 5: // --attribute
        gAttribute = string(argv[++i]);
        break;

      case 6: // --stat
        gStatistics = string(argv[++i]);
        break;

      case 7: // --o
        gOutputFileName = argv[++i];
        break;

      case 8: // --low
        gLowerBound = atof(argv[++i]);
        break;

      case 9: // --high
        gUpperBound = atof(argv[++i]);
        break;

      case 10: // --filter
        gFilterAttribute = string(argv[++i]);
        gFilterStatistics = string(argv[++i]);
        break;

      default:
        exit(0);
    }
  }

  return 1;
}

int plot_cdf(const vector<FamilyHandle>& families, PlotType type,uint32_t num_buckets = 0)
{
  vector<FunctionType> cdf;
  vector<FamilyHandle>::const_iterator fIt;
  Factory fac;


  Attribute* data = fac.make_aggregator_array(gStatistics,true);
  Attribute* filter = NULL;

  if (gFilterAttribute != "Unknown")
    filter = fac.make_aggregator_array(gFilterStatistics,true);


  for (fIt=families.begin();fIt!=families.end();fIt++) {

    if (!fIt->providesAggregate(gStatistics,gAttribute)) {
      fprintf(stderr,"Warning: Family \"%s\" does not contain the attribute or statistics\n",fIt->variableName().c_str());
      continue;
    }

    StatHandle stat = fIt->aggregate(gStatistics,gAttribute);
    data->resize(stat.elementCount());
    stat.readData(data);

    // If we have a filter defined
    if (filter != NULL) {
      StatHandle fstat = fIt->aggregate(gFilterStatistics,gFilterAttribute);
      filter->resize(fstat.elementCount());
      fstat.readData(filter);

    }

    FeatureHierarchy hierarchy;
    FeatureHierarchy::LivingIterator it;
    hierarchy.initialize(fIt->simplification(0));
    hierarchy.parameter(gParameter);

    fprintf(stderr,"Processing time step %e at parameter %e with %d features\n",fIt->time(),gParameter,hierarchy.livingSize(gParameter));

    if (gCrossFamily != NULL)
      gCrossFamily->reset();

    for (it=hierarchy.beginLiving(gParameter);it!=hierarchy.endLiving();it++) {

      LocalIndexType id = (*it)->id();

      if ((filter == NULL) ||
          (((*filter)[(*it)->id()].value() >= gLowerBound) &&
           ((*filter)[(*it)->id()].value() <= gUpperBound))) {

        if (gCrossFamily != NULL)
          gCrossFamily->addVertex((*data)[(*it)->id()].value(),(*it)->id());
        else {
          //fprintf(stderr,"feature %d %e\n",id,(*data)[id].value());
          cdf.push_back((*data)[id].value());
        }
      }
    }

    if (gCrossFamily != NULL)
      cdf.push_back(gCrossFamily->value());
  }

  if ((type == CDF) || (type == UCDF) || (type == WCDF) || (type == HIST))
    sort(cdf.begin(),cdf.end());

  FILE* output = stdout;
  if (gOutputFileName != NULL)
    output = fopen(gOutputFileName,"w");

  if (type == CDF) { // output cdf

    FunctionType sum=0;
    for (int i=0;i<cdf.size();i++) {
      sum += cdf[i];
      fprintf(output,"%f %f\n",cdf[i],(i+1)/(float)cdf.size());
    }
  }
  else if (type == UCDF) { // output cdf

    FunctionType sum=0;
    for (int i=0;i<cdf.size();i++) {
      sum += cdf[i];
      fprintf(output,"%f %d\n",cdf[i],i+1);
    }
  }
  else if (type == WCDF) { // output cdf

    FunctionType sum=0;
    for (int i=0;i<cdf.size();i++) {
      sum += cdf[i];
      fprintf(output,"%f %f\n",cdf[i],sum);
    }
  }
  else if (type == LIST) { // output cdf

    for (int i=0;i<cdf.size();i++) {
      fprintf(output,"%f\n",cdf[i]);
    }
  }
  else if (type == HIST) { // output histogram

    FunctionType delta = (cdf.back() - cdf[0]) / num_buckets;
    uint32_t bucket = 0;
    uint32_t count;
    uint32_t i=0;

    fprintf(stderr,"min %f max %f delta %f\n",cdf[0],cdf.back(),delta);
    while (i < cdf.size()) {
      bucket++;
      count = 0;
      while ((i < cdf.size()) && (cdf[i] <= (cdf[0] + bucket*delta))) {
        count++;
        i++;
      }

      fprintf(output,"%f %d\n",cdf[0] + (bucket-1)*delta,count);
      fprintf(stdout,"%f %d\n",cdf[0] + (bucket-1)*delta,count);
    }
    fprintf(output,"%f %d\n",cdf[0] + (bucket)*delta,0);
    fprintf(stdout,"%f %d\n",cdf[0] + (bucket)*delta,0);


  }
  else
    assert (false);

  if (gOutputFileName != NULL)
    fclose(output);

  return 1;
}

int plot_time(const vector<FamilyHandle>& families)
{
  map<FunctionType,FunctionType> time_map;
  map<FunctionType,FunctionType>::iterator mIt;
  vector<FamilyHandle>::const_iterator fIt;
  Factory fac;
  Attribute* data = fac.make_aggregator_array(gStatistics,true);

  Attribute* filter = NULL;
  if (gFilterAttribute != "Unknown")
    filter = fac.make_aggregator_array(gFilterStatistics,true);


  if (gCrossFamily == NULL) {
    fprintf(stderr,"Error: Must have a family-aggregator defined to do a cross time plot\n");
    return 0;
  }

  for (fIt=families.begin();fIt!=families.end();fIt++) {

    if (!fIt->providesAggregate(gStatistics,gAttribute)) {
      fprintf(stderr,"Warning: Family \"%s\" does not contain the attribute or statistics\n",fIt->variableName().c_str());
      continue;
    }

    StatHandle stat = fIt->aggregate(gStatistics,gAttribute);
    data->resize(stat.elementCount());
    stat.readData(data);

    // If we have a filter defined
    if (filter != NULL) {
      StatHandle fstat = fIt->aggregate(gFilterStatistics,gFilterAttribute);
      filter->resize(fstat.elementCount());
      fstat.readData(filter);

    }

    FeatureHierarchy hierarchy;
    FeatureHierarchy::LivingIterator it;
    hierarchy.initialize(fIt->simplification(0));

    if (gParameter < hierarchy.minParameter() || (gParameter > hierarchy.maxParameter()))
      fprintf(stderr,"Warning: Parameter %e out of range [%e,%e] at time %e\n",gParameter,
              hierarchy.minParameter(),hierarchy.maxParameter(),fIt->time());

    hierarchy.parameter(gParameter);

    gCrossFamily->reset();

    for (it=hierarchy.beginLiving(gParameter);it!=hierarchy.endLiving();it++) {

      if ((filter == NULL) ||
          (((*filter)[(*it)->id()].value() >= gLowerBound) &&
           ((*filter)[(*it)->id()].value() <= gUpperBound))) {
        //fprintf(stderr,"time %d feature %d: %e\n",fIt->timeIndex(),(*it)->id(),(*data)[(*it)->id()].value());
        gCrossFamily->addVertex((*data)[(*it)->id()].value(),(*it)->id());
      }
    }

    if (gCrossFamily->value() != gCrossFamily->initialValue())
      time_map[fIt->time()] = gCrossFamily->value();
    else {
      fprintf(stderr,"Warning: Time step %d does not contain any features\n",fIt->timeIndex());
      time_map[fIt->time()] = 0;
    }
  }


  FILE* output = stdout;
  if (gOutputFileName != NULL)
    output = fopen(gOutputFileName,"w");

  for (mIt=time_map.begin();mIt!=time_map.end();mIt++)
    fprintf(output,"%e %e\n",mIt->first,mIt->second);

  if (gOutputFileName != NULL)
    fclose(output);

  return 1;
}

int plot_parameter(const vector<FamilyHandle>& families, PlotType type)
{
  vector<FamilyHandle>::const_iterator fIt;
  Factory fac;
  Attribute* data = fac.make_aggregator_array(gStatistics,true);

  Attribute* filter = NULL;
  if (gFilterAttribute != "Unknown")
    filter = fac.make_aggregator_array(gFilterStatistics,true);

  double delta;

  double base;

  if (gRange[0] < 0)
    base = 1e-5;
  else
    base = gRange[0];



  switch (type) {
    case PARAM:
      delta = (gRange[1] - gRange[0]) / (gStepCount - 1);
      break;
    case LPARAM:
      delta = (log10(gRange[1] - gRange[0] + base) - log10(gRange[0] - gRange[0] + base)) / (gStepCount - 1);
      //delta = (log10(gRange[1]) - log10(gRange[0])) / (gStepCount - 1);
      //delta = (log10(gRange[1]) - log10(gRange[0])) / (gStepCount - 1);
      break;
    case EPARAM:
      delta = 999 / (gStepCount -1);
      break;
    default:
      fprintf(stderr,"Unrecognized plot type\n");
      assert(false);
      break;
  }

  if (gCrossTime == NULL) {
    fprintf(stderr,"Error: A parameter plot requires a time-agg to be specified\n");
    return 0;
  }
  gCrossTime->resize(gStepCount);

  for (fIt=families.begin();fIt!=families.end();fIt++) {

    if (!fIt->providesAggregate(gStatistics,gAttribute)) {
      fprintf(stderr,"Warning: Family \"%s\" does not contain the attribute or statistics\n",fIt->variableName().c_str());
      continue;
    }

    StatHandle stat = fIt->aggregate(gStatistics,gAttribute);
    data->resize(stat.elementCount());
    stat.readData(data);

    // If we have a filter defined
    if (filter != NULL) {
      StatHandle fstat = fIt->aggregate(gFilterStatistics,gFilterAttribute);
      filter->resize(fstat.elementCount());
      fstat.readData(filter);

    }
    FeatureHierarchy hierarchy;
    FeatureHierarchy::LivingIterator it;
    hierarchy.initialize(fIt->simplification(0));


    FunctionType p;
    for (uint32_t i=0;i<gStepCount;i++) {
      gCrossFamily->reset();

      switch (type) {
        case PARAM:
          p = gRange[0] + i*delta;
          break;
        case LPARAM:
          p = pow(10,log10(gRange[0] - gRange[0] + base) + i*delta) + gRange[0] - base;
          //p = pow(10,i*delta) + gRange[0] - 1;
          //p = pow(10,log10(gRange[0]) + i*delta);
          break;
        case EPARAM:
          p = log10(1 + i*delta)* (gRange[1] - gRange[0]) / 3 + gRange[0];
          break;
        default:
          break;
      }
      fprintf(stderr,"Processing %f  delta=%e\n",p,delta);

      hierarchy.parameter(p);

      //fprintf(stderr,"Processing time step %e at parameter %e with %d features\n",fIt->time(),p,hierarchy.livingSize(p));


      for (it=hierarchy.beginLiving(p);it!=hierarchy.endLiving();it++) {
        //fprintf(stderr,"[%f,%f] value = %e,%e \n",it->lifeTime()[0],it->lifeTime()[1],(*data)[(*it)->id()].value(),gCrossFamily->value());
        if ((filter == NULL) ||
            (((*filter)[(*it)->id()].value() >= gLowerBound) &&
             ((*filter)[(*it)->id()].value() <= gUpperBound))) {

          gCrossFamily->addVertex((*data)[(*it)->id()].value(),(*it)->id());
        }
      }

      if (gCrossFamily->value() != gCrossFamily->initialValue())
        (*gCrossTime)[i].addVertex(gCrossFamily->value(),0);
     else {
        fprintf(stderr,"Warning: Time step %d does not contain any features\n",fIt->timeIndex());
      }

    }
  }

  FILE* output = stdout;
  if (gOutputFileName != NULL)
    output = fopen(gOutputFileName,"w");

  switch (type) {

    case PARAM:
      for (uint32_t i=0;i<gStepCount;i++)
        fprintf(output,"%.15e %.15e\n",gRange[0]+i*delta,(*gCrossTime)[i].value());
      break;
    case LPARAM:
      for (uint32_t i=0;i<gStepCount;i++) {
        fprintf(output,"%.15e %.15e\n", pow(10,log10(gRange[0] - gRange[0] + base) + i*delta) + gRange[0] - base,(*gCrossTime)[i].value());
        //fprintf(output,"%.15e %.15e\n", pow(10,log10(gRange[0]) + i*delta),(*gCrossTime)[i].value());
        //fprintf(output,"%.15e %.15e\n", pow(10,log10(gRange[0]) + i*delta),(*gCrossTime)[i].value());
      }
      break;
    case EPARAM:
      for (uint32_t i=0;i<gStepCount;i++)
        fprintf(output,"%.15e %.15e\n",log10(1 + i*delta)* (gRange[1] - gRange[0]) / 3 + gRange[0],(*gCrossTime)[i].value());
      break;
    default:
      assert(false);
      break;
  }
  if (gOutputFileName != NULL)
    fclose(output);


  return 1;
}

int main(int argc, const char* argv[])
{
  if (!parse_command_line(argc,argv))
    exit(0);

  HandleCollection collection;

  collection.initialize(gFileNames);

  FunctionType global_range[2] = {1e34,-1e34};
  FunctionType range[2];
  vector<ClanKey> clans;
  vector<FamilyHandle> families;
  vector<FamilyHandle>::const_iterator fIt;

  collection.getClanKeys(clans);

  assert (clans.size() == 1);

  collection.getFamilyHandles(clans[0],families);

  for (fIt=families.begin();fIt!=families.end();fIt++) {
    fIt->simplification(0).range(range);
    global_range[0] = std::min(global_range[0],range[0]);
    global_range[1] = std::max(global_range[1],range[1]);
  }

  //gParameter = std::max(gParameter,global_range[0]);
  //gParameter = std::min(gParameter,global_range[1]);

  gRange[0] = std::max(gRange[0],(double)global_range[0]);
  gRange[1] = std::min(gRange[1],(double)global_range[1]);

  switch (gPlotType) {
    case CDF:
    case UCDF:
    case WCDF:
      plot_cdf(families,gPlotType);
      break;
    case TIME:
      plot_time(families);
      break;
    case PARAM:
    case LPARAM:
    case EPARAM:
      plot_parameter(families,gPlotType);
      break;
    case HIST:
      plot_cdf(families,gPlotType,gNumBuckets);
      break;
    case LIST:
      plot_cdf(families,gPlotType);
      break;

  }




  return 1;
}
