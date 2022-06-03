/*
 * GatherAlgorithm.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "GatherAlgorithm.h"
#include "StreamingGatherAlgorithm.h"

GatherAlgorithm* GatherAlgorithm::make(GatherAlgorithmType type, bool invert)
{
  switch (type) {
    case GATHER_STREAMING_SORT:
      return new StreamingGatherAlgorithm(invert);
  }

  return NULL;
}
