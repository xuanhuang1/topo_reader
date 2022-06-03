/*
 * LocalComputeAlgorithm.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "LocalComputeAlgorithm.h"

LocalComputeAlgorithm* LocalComputeAlgorithm::make(LocalAlgorithmType type, bool invert)
{
  switch (type) {
    case LOCAL_SORTED_UF:
      return NULL;
  }

  return NULL;
}


