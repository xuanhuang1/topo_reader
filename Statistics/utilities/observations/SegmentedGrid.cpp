#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <fstream>
#include "SegmentedGrid.h"
#include "Covariance_J9.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>

using namespace Statistics;


void SegmentedGrid::readSegmentation(string &baseFilename) {
  string segName = baseFilename+string(".seg");
  string mapName = baseFilename+string(".map");
  vector<Segment> allFeatures;
  vector<Feature> features = mHierarchy->allFeatures();
  allFeatures.resize(features.size());

  FILE *segFile = fopen(segName.c_str(), "rb");
  if(segFile == NULL) {
    cout << "error: segfile " << segName << " not available!" << endl;
    exit(1);
  }

  uint32_t buffer[1024];
  uint32_t pos = 0;
  uint32_t size;
  uint32_t id;

  Cell c;
  uint32_t count=0;
  FILE *mapFile = fopen(mapName.c_str(), "rb");

  uint32_t *dptr;
  if (mapFile != NULL)
  {
    fseek(mapFile, 0, SEEK_END);
    uint32_t ss = ftell(mapFile)/sizeof(uint32_t);
    fclose(mapFile);

    dptr = new uint32_t[ss];
    uint32_t i = 0;

    mapFile = fopen(mapName.c_str(), "rb");
    while (ss > 0)
    {
      fread(&dptr[i], sizeof(uint32_t), 1, mapFile);
      count++;
      i++, ss--;
    }
  }

//  std::cout<<"\n size: "<<mSegments.size()<<"\t count:"<<count<<std::endl;
  count = 0;
 
  size = fread(buffer,sizeof(uint32_t),1024,segFile);
  while (size > 0) 
  {
    for (pos=0;pos<size;pos++) 
    {
      if (buffer[pos] != GNULL) 
      {
        id = count;
          
        if (mapFile != NULL)
          id = dptr[count];

        allFeatures[buffer[pos]].elements.push_back(id);
      }
      count++;
    }
    size = fread(buffer,sizeof(uint32_t),1024,segFile);   
  }
  delete [] dptr;

  // now store the elements for only the active features
 for(FeatureHierarchy::ActiveIterator mIte = mHierarchy->beginActive(); mIte != mHierarchy->endActive(); mIte++) 
  {
    for(uint32_t i=0; i < allFeatures[mHierarchy->mappedIndex(mIte->id())].elements.size(); i++) {
        mSegments[mHierarchy->mappedIndex((mIte.rep())->id())].elements.push_back(allFeatures[mHierarchy->mappedIndex(mIte->id())].elements[i]);
    }
  }

}

void SegmentedGrid::swapHierarchy(FeatureHierarchy *cur, string &baseFilename) {
  mHierarchy = cur;
  mSegments.clear();
  readSegmentation(baseFilename);
}  

map<LocalIndexType, map<SegmentedGrid::Cell, LocalIndexType> > SegmentedGrid::getTileIDs() const {
  map<LocalIndexType, map<SegmentedGrid::Cell, LocalIndexType> > tileIDs;
  for(map<LocalIndexType, Segment>::const_iterator itr=mSegments.begin(); itr != mSegments.end(); itr++) {
    tileIDs[itr->first] = getTileIDs(itr->first);
  }
  return tileIDs;
}

                                           
map<SegmentedGrid::Cell, LocalIndexType> SegmentedGrid::getTileIDs(LocalIndexType featureID) const {
  map<SegmentedGrid::Cell, LocalIndexType> tileIDs;

  map<LocalIndexType, Segment>::const_iterator itr = mSegments.find(featureID);
  if(itr == mSegments.end()) {
    cout << "error: featureID does not exist!!!" << std::endl;
  }
  for(uint32_t i=0; i < itr->second.elements.size(); i++) {
    LocalIndexType id = itr->second.elements[i];
    SegmentedGrid::Cell coordInGrid;
    SegmentedGrid::Cell coordInTile;
    int accum = 1;
    for(int j=0; j < DIM; j++) {
      coordInGrid.pos[j] =  (id % (mGridDim[j]*accum)) / accum;       
      coordInTile.pos[j] = coordInGrid.pos[j] / (mGridDim[j]/mTileDim[j]);
      accum *= mGridDim[j];
    }
    map<SegmentedGrid::Cell, LocalIndexType>::iterator itr = tileIDs.find(coordInTile);
    if(itr == tileIDs.end()) {
      tileIDs.insert(map<SegmentedGrid::Cell, LocalIndexType>::value_type(coordInTile, 1));
    } else {
      (itr->second)++;
    }

  }

  return tileIDs;
}

map<LocalIndexType, SegmentedGrid::OOBB > SegmentedGrid::getOBBs() const {
  map<LocalIndexType, SegmentedGrid::OOBB > obbs;
  for(map<LocalIndexType, Segment>::const_iterator itr=mSegments.begin(); itr != mSegments.end(); itr++) {
    obbs[itr->first] = getOBBs(itr->first);
    getOBBs(itr->first);
  }
  return obbs;
}

SegmentedGrid::OOBB SegmentedGrid::getOBBs(LocalIndexType featureID) const {

  SegmentedGrid::OOBB obb;
  map<LocalIndexType, Segment>::const_iterator itr = mSegments.find(featureID);
  int sz = DIM*(DIM+1)*0.5;
  Covariance_J9Array cov;
  cov.resize(sz);

  if(itr == mSegments.end()) {
    cout << "error: featureID does not exist!!!" << std::endl;
  }
  for(uint32_t i=0; i < itr->second.elements.size(); i++) {
    LocalIndexType id = itr->second.elements[i];
    SegmentedGrid::Cell coordInGrid;
    int accum = 1;
    for(int j=0; j < DIM; j++) {
      coordInGrid.pos[j] =  (id % (mGridDim[j]*accum)) / accum;       
      accum *= mGridDim[j];
    }
    obb.center = obb.center+coordInGrid;
    int count=0;
    for(int j=0; j < DIM; j++) {
      for(int k=j; k < DIM; k++) {
        cov[count++].addVertex(coordInGrid.pos[j], coordInGrid.pos[k], j);
      }
    }
  }
  obb.center = obb.center / itr->second.elements.size();
  // Now populate the gsl matrix and compute eigenvectors
  int count=0;
  double *data = new double[DIM*DIM];
  for(int i=0; i < DIM; i++) {
    for(int j=i; j < DIM; j++) { 
      data[i*DIM+j] = cov[count].value();
      data[j*DIM+i] = cov[count].value();
      count++;
    }
  }
     
  gsl_matrix_view m = gsl_matrix_view_array (data, DIM, DIM);
     
  gsl_vector *eval = gsl_vector_alloc (DIM);
  gsl_matrix *evec = gsl_matrix_alloc (DIM, DIM);
     
  gsl_eigen_symmv_workspace * w = gsl_eigen_symmv_alloc (DIM);
  gsl_eigen_symmv (&m.matrix, eval, evec, w);
  gsl_eigen_symmv_free (w);
  gsl_eigen_symmv_sort (eval, evec, GSL_EIGEN_SORT_ABS_ASC);
       
  for(int i = 0; i < DIM; i++) {
    SegmentedGrid::Eigen eig_i;
    eig_i.value = gsl_vector_get (eval, i);
    gsl_vector_view evec_i = gsl_matrix_column (evec, i);
    for(int j=0; j < DIM; j++) {
      eig_i.vector[j] = gsl_vector_get(&(evec_i.vector), j);
    } 
  //  printf ("eigenvalue = %g\n", eig_i.value);
  //  printf ("eigenvector = \n");
  //  gsl_vector_fprintf (stdout, &evec_i.vector, "%g");
    
    obb.eigens[i] = eig_i;
  }

     
  gsl_vector_free (eval);
  gsl_matrix_free (evec);
  delete [] data;   
   
  return obb;
}

