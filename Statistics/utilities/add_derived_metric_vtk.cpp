#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include <stack>

#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkPolyData.h"


#include "Value.h"
#include "ClanHandle.h"
#include "AggregatorFactory.h"
#include "Attribute.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"

using namespace std;
using namespace TopologyFileFormat;
using namespace Statistics;


void accumulate(const Feature* f, Attribute& attr)
{
  vector<const Feature*> order;
  vector<const Feature*>::reverse_iterator it;
  stack<const Feature*> front;
  const Feature* top;


  front.push(f);

  while (!front.empty()) {
    top = front.top();
    front.pop();

    order.push_back(top);

    for (int i=0;i<top->conSize();i++)
      front.push(top->con(i));
  }

  for (it=order.rbegin();it!=order.rend();it++) {
    if ((*it)->repSize() > 0)
      attr.addSegment((*it)->rep(0)->id(),(*it)->id());
  }

}


/*!
 *
 */

int main(int argc, const char* argv[])
{
  if (argc < 4) {
    fprintf(stderr,"Usage: %s <family> <segmentation> <vtkDataSet.vtp> [<metric> <name> ... <metric> <name>]\n",argv[0]);

   exit(0);
  }


  ClanHandle fclan,sclan;
  FamilyHandle family;
  FeatureHierarchy hierarchy;

  SegmentationHandle seg_handle;
  FeatureSegmentation seg;


  fclan.attach(argv[1]);
  family = fclan.family(0);
  hierarchy.initialize(family.simplification(0));

  hierarchy.parameter(1.0);

  sclan.attach(argv[2]);
  seg_handle = sclan.family(0).segmentation();
  seg.initialize(seg_handle);


  int length = strlen(argv[3]);
  const char* ext = argv[3] + length - 3;

  vtkSmartPointer<vtkPolyData> surface;

  if (strcmp(ext,"vtp") == 0) {
    vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName(argv[3]);
    reader->Update();
    surface = reader->GetOutput();
  }

  vector<Attribute* > metrics;
  vector<vtkDataArray* > data;

  int i = 4;
  while (i < argc-1) {


    if (strcmp(argv[i],"volume") == 0)
      metrics.push_back(new VertexCountArray(hierarchy.featureCount()));
    else if (strcmp(argv[i],"mean") == 0)
      metrics.push_back(new MeanArray(hierarchy.featureCount()));
    else if (strcmp(argv[i],"min") == 0)
      metrics.push_back(new MinimumArray(hierarchy.featureCount()));
    else if (strcmp(argv[i],"max") == 0)
      metrics.push_back(new MaximumArray(hierarchy.featureCount()));
    else if (strcmp(argv[i],"variance") == 0)
      metrics.push_back(new VarianceArray(hierarchy.featureCount()));
    else if (strcmp(argv[i],"skewness") == 0)
      metrics.push_back(new SkewnessArray(hierarchy.featureCount()));
    else {
      fprintf(stderr,"Metric not recognized %s \n",argv[i]);
      exit(0);
    }

    data.push_back(surface->GetPointData()->GetArray(argv[i+1]));
    i += 2;
  }



  std::vector<Feature>::const_iterator it;
  Segment s;

  // For all features
  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    // Get the list of vertices for this feature
    s = seg.elementSegmentation(it->id());

    // For all vertices of the feature
    for (LocalIndexType k=0;k<s.size;k++) {

      // For all metrics
      for (int i=0;i<metrics.size();i++) {

        // Add the value of the corresponding point
        (*metrics[i])[it->id()].addVertex(data[i]->GetTuple1(s.samples[k]),s.samples[k]);
      }
    }
  }

  fprintf(stderr,"Done computing ... starting accumulating\n");


  for (it=hierarchy.allFeatures().begin();it!=hierarchy.allFeatures().end();it++) {

    // For all roots
    if (it->repSize() == 0) {

      for (int i=0;i<metrics.size();i++) {
        accumulate(&(*it),*metrics[i]);
      }
    }
  }


  fprintf(stderr,"Done accumulating... starting output\n");

  // Now output each metric individually
  for (int i=0;i<metrics.size();i++) {

    // Create a plain value array
    ValueArray vals(hierarchy.featureCount());

    // Transfer the values
    for (int k=0;k<hierarchy.featureCount();k++)
      vals[k].addVertex((*metrics[i])[k].value(),0);


    // Create the corresponding stat handle

    StatHandle derived_handle;
    derived_handle.aggregated(true);
    derived_handle.stat(vals[0].typeName());

    char name[100];
    sprintf(name,"%s(%s)",metrics[i]->typeName(),data[i]->GetName());
    derived_handle.species(name);

    derived_handle.encoding(false);



    // Initialize the data
    derived_handle.setData(&vals);

    // And append it to the file
    fclan.family(0).append(derived_handle);
  }
}


