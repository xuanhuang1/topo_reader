/*
 * MappedSegmentation.h
 *
 *  Created on: Apr 19, 2012
 *      Author: bremer5
 */

#ifndef MAPPEDSEGMENTATION_H_
#define MAPPEDSEGMENTATION_H_

#include "ArraySegmentation.h"

typedef FlexArray::MappedArray<GlobalIndexType,GlobalIndexType,LocalIndexType> SegmentationArray;
typedef FlexArray::MappedArray<FlexArray::MappedFunctionElement<GlobalIndexType,LocalIndexType>,GlobalIndexType,LocalIndexType> FunctionArray;

//! A segmentation that uses a sparse index space
class MappedSegmentation : public ArraySegmentation<SegmentationArray,FunctionArray>
{
public:

  typedef SegmentationArray SegmentationArrayType;
  typedef FunctionArray FunctionArrayType;

  //! Default constructor
  MappedSegmentation(const FunctionArray& function): ArraySegmentation<SegmentationArray,FunctionArray>(function){}

  //! Destructor
  ~MappedSegmentation() {}
};



#endif /* MAPPEDSEGMENTATION_H_ */
