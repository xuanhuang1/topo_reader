/*
 * Segment.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: bremer5
 */


#ifdef TALASS_ENABLE_VTK
#include "vtkCharArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#endif

#include "Segment.h"

namespace TopologyFileFormat
{

#ifdef TALASS_ENABLE_VTK

vtkSmartPointer<vtkUniformGrid> segments_to_grid(std::vector<Segment>& segments, uint32_t* dim, bool cell_centered,FunctionType grid_size)
{
  FunctionType low[3],high[3]; // The extents of the bounding box
  vtkSmartPointer<vtkUniformGrid> grid = vtkSmartPointer<vtkUniformGrid>::New();
  uint32_t i;
  std::vector<Segment>::iterator seg;

  low[0] = low[1] = low[2] = 10e34;
  high[0] = high[1] = high[2] = -10e34;

  for (seg=segments.begin();seg!=segments.end();seg++) {

    if (seg->size == 0) { // If this segment contains no vertices
      continue;
    }

    // Otherwise, we expect coordinates to be defined
    if (seg->coordinates == NULL) {
      sterror(true,"Segment does not contain coordinates. Cannot create grid.");
      return grid;
    }

    for (i=0;i<seg->size;i++) {
      //fprintf(stderr,"P = %f %f %f\n",seg->coordinates[3*i],seg->coordinates[3*i+1],seg->coordinates[3*i+2]);
      low[0] = std::min(low[0],seg->coordinates[3*i]);
      low[1] = std::min(low[1],seg->coordinates[3*i+1]);
      low[2] = std::min(low[2],seg->coordinates[3*i+2]);

      high[0] = std::max(high[0],seg->coordinates[3*i]);
      high[1] = std::max(high[1],seg->coordinates[3*i+1]);
      high[2] = std::max(high[2],seg->coordinates[3*i+2]);
    }
  }

  if (low[0] > high[0]) // If we didn't have any vertex
    return grid;

  // If this bounding box touches the global boundary
  if ((low[0] == 0) || (low[1] == 0) || (low[2] == 0)
      || (high[0] == dim[0]-1) || (high[1] == dim[1]-1) || (high[2] == dim[2]+1)) {

    // Indicate the boundary artifact by setting the dimensions to 0
    dim[0] = dim[1] = dim[2] = 0;
  }
  // Convert the FunctionType coordinates into integer extents assuming the given grids_size
  int extent[6];

  // Make sure you pad the extents by 1 in each direction
  extent[0] = extent[2] = extent[4] = 0;
  extent[1] = (int)round((high[0] - low[0]) / grid_size) + 2;
  extent[3] = (int)round((high[1] - low[1]) / grid_size) + 2;
  extent[5] = (int)round((high[2] - low[2]) / grid_size) + 2;

  // For a cell centered grid we write out the enclosing dual grid
  // which is yet one cell larger
  if (cell_centered) {
    extent[1]++;
    extent[3]++;
    extent[5]++;
  }


  //fprintf(stderr,"BBox  %d %d %d   x   %d %d %d\n",extent[0],extent[2],extent[4],extent[1],extent[3],extent[5]);
  grid->SetExtent(extent);
  grid->SetOrigin(low[0],low[1],low[2]);
  //grid->SetDataDescription( VTK_XYZ_GRID);
  //grid->AllocateScalars(VTK_CHAR,1);

  //char *data = (char*)grid->GetScalarPointer();

  // For simplicity convert the extent to dimensions
  extent[1]++;
  extent[3]++;
  extent[5]++;

  vtkSmartPointer<vtkFloatArray> data = vtkSmartPointer<vtkFloatArray>::New();
  data->SetNumberOfValues(extent[1]*extent[3]*extent[5]);
  data->SetNumberOfComponents(1);

  for (i=0;i<extent[1]*extent[3]*extent[5];i++)
    data->SetTuple1(i,0);

  int index[3];

  for (seg=segments.begin();seg!=segments.end();seg++) {
    // Now set all actual samples to 1
    for (uint32_t i=0;i<seg->size;i++) {
      index[0] = (int)round((seg->coordinates[3*i+0] - low[0]) / grid_size) + 1;
      index[1] = (int)round((seg->coordinates[3*i+1] - low[1]) / grid_size) + 1;
      index[2] = (int)round((seg->coordinates[3*i+2] - low[2]) / grid_size) + 1;

      data->SetTuple1((index[2]+0)*extent[3]*extent[1] + (index[1] + 0)*extent[1] + (index[0] + 0),1);

      if (cell_centered) {
        data->SetTuple1((index[2]+0)*extent[3]*extent[1] + (index[1] + 0)*extent[1] + (index[0] + 1),1);
        data->SetTuple1((index[2]+0)*extent[3]*extent[1] + (index[1] + 1)*extent[1] + (index[0] + 0),1);
        data->SetTuple1((index[2]+0)*extent[3]*extent[1] + (index[1] + 1)*extent[1] + (index[0] + 1),1);

        data->SetTuple1((index[2]+1)*extent[3]*extent[1] + (index[1] + 0)*extent[1] + (index[0] + 0),1);
        data->SetTuple1((index[2]+1)*extent[3]*extent[1] + (index[1] + 0)*extent[1] + (index[0] + 1),1);
        data->SetTuple1((index[2]+1)*extent[3]*extent[1] + (index[1] + 1)*extent[1] + (index[0] + 0),1);
        data->SetTuple1((index[2]+1)*extent[3]*extent[1] + (index[1] + 1)*extent[1] + (index[0] + 1),1);
      }


      //data[index[2]*extent[3]*extent[1] + index[1]*extent[1] + index[0]] = 1;
    }
  }

  grid->GetPointData()->SetScalars(data);
  grid->GetCellData()->SetScalars(data);

  return grid;
}

#endif

}







