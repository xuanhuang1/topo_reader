/*
 * IDXParser.cpp
 *
 *  Created on: May 30, 2011
 *      Author: bremer5
 */

#include "IDXParser.h"

const uint32_t* effective_dimension(const char* idx_file, uint32_t low[],
                                    uint32_t high[], uint32_t res)
{
  // A static variable to ensure that a) we can pass out the pointer
  // and b) that the bulk of this function is only executed once
  static uint32_t dimension[3] = {0,0,0};

  if ((dimension[0] == 0) && (dimension[1] == 0) && (dimension[2] == 0)) {

    VisusFile* idx = VisusOpen(idx_file);

    assert (idx); // The file must be valid

    // Create the query_box;
    VisusBox query_box;

    for (int i=0;i<3;i++) {
      PGET(query_box.p1,i) = low[i];
      PGET(query_box.p2,i) = high[i];
    }

    // Create the full query at the given resolution
    VisusQuery *query;
    query = VisusCreateQuery(idx,query_box,res,0,res);

    // and ask for the number of samples in the first three dimension
    dimension[0] = PGET(query->nsamples,0);
    dimension[1] = PGET(query->nsamples,1);
    dimension[2] = PGET(query->nsamples,2);
  }

  return dimension;
}

template class IDXParser<>;
