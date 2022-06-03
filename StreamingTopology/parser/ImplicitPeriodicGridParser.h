/*
 * implicitPeriodicGridParser.h
 *
 *  Created on: Nov 19, 2013
 *      Author: bremer5
 */

#ifndef IMPLICITPERIODICGRIDPARSER_H
#define IMPLICITPERIODICGRIDPARSER_H

#include "PeriodicGridParser.h"
#include "ImplicitGridParser.h"


/*! This class implements a grid parser that both attaches an
 *  implicit grid as well as the periodic edges
 *
 */
template<class DataClass = GenericData<float> >
class ImplicitPeriodicGridParser: public virtual ImplicitGridParser<DataClass>,
public virtual PeriodicGridParser<DataClass>
{
public:

  //! Default constructor
  /*! Default constructor which saves the stream address and
   *  dimensions for later access
   *  @param function: file stream of the raw function to be read
   *  @param dim: index of samples in each dimension
   *  @param period: bitmask to determine which directions are periodic
   *  @param edim: number of coordinates per vertex (N/A)
   *  @param fdim: index of the coordinate that should be the function (N/A)
   *  @param adims: indices of the coordinates that should be preserved (N/A)
   *  @param attributes: file pointers to any additional attributes of interest
   *  @param map_file: optional file pointer to store the index map if compactifying
   */
  ImplicitPeriodicGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                             uint8_t period, uint32_t fdim, const std::vector<uint32_t>& adims = std::vector<uint32_t>(),
                             bool compact=true, FILE* map_file=NULL);

  virtual ~ImplicitPeriodicGridParser() {}

protected:

  //! Read the next plane of data and if necessary add
  //! planes of x-,y-, and z-coordinates
  virtual int readDataPlane();

};

template <class DataClass>
ImplicitPeriodicGridParser<DataClass>::ImplicitPeriodicGridParser(const std::vector<FILE*>& attributes, uint32_t dim_x, uint32_t dim_y, uint32_t dim_z,
                                                                  uint8_t period, uint32_t fdim, const std::vector<uint32_t>& adims, bool compact, FILE* map_file)
                                                                  : GridParser<DataClass>(attributes,dim_x,dim_y, dim_z, fdim,adims,compact,map_file),
                                                                    ImplicitGridParser<DataClass>(attributes,dim_x,dim_y, dim_z, fdim,adims,compact,map_file),
                                                                    PeriodicGridParser<DataClass>(attributes,dim_x,dim_y, dim_z, period, fdim,adims,compact,map_file)
{
}

template <class DataClass>
int ImplicitPeriodicGridParser<DataClass>::readDataPlane()
{
  ImplicitGridParser<DataClass>::readDataPlane();

  // This is copied from PeriodicGridParser to avoid ambiguous overload
  if (this->mK == 1)
    memcpy(this->mFirstPlaneMap,this->mIndexMap[0],this->mDimX*this->mDimY*sizeof(GlobalIndexType));

  return 1;
}


#endif /* IMPLICITPERIODICGRIDPARSER_H_ */
