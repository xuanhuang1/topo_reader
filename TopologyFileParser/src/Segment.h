/*
 * Segment.h
 *
 *  Created on: Jul 17, 2014
 *      Author: bremer5
 */

#ifndef SEGMENT_H
#define SEGMENT_H

#include <vector>
#include <string>

#ifdef TALASS_ENABLE_VTK

#include "vtkSmartPointer.h"
#include "vtkUniformGrid.h"

#endif

#include "TalassConfig.h"

namespace TopologyFileFormat
{


//! A segment encapsulates a pointer to a list of indices and the
//! number of indices present
struct Segment {
  LocalIndexType size;
  const GlobalIndexType* samples;

  uint32_t dim;
  const FunctionType* coordinates;
  const FunctionType* locations;
  const std::string* labels;

  Segment() : size(0),samples(NULL),dim(0),coordinates(NULL) {}
};



#ifdef TALASS_ENABLE_VTK

vtkSmartPointer<vtkUniformGrid> segments_to_grid(std::vector<Segment>& segments, uint32_t* dim, bool cell_centered = true, FunctionType grid_size=1);

#endif


}




#endif /* SEGMENT_H_ */
