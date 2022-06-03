/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov 
* OCEC-08-107
* All rights reserved.  
*   
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*   
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*   
*  
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/


#ifndef HDF5GRIDPARSER_H
#define HDF5GRIDPARSER_H

#ifdef ST_DISABLE_HDF5

#include "HDF5GridParser_Dummy.h"

#else



#include <stack>
#include <queue>
#include <cstdio>

#include "Parser.h"
#include "GenericData.h"

#include <iostream>
#include <vector>
#include <map>
using namespace std; 

#include "H5Cpp.h"

/*! \brief Read HDF5 Grid data
 *
 * The expected data organization is a n-D array describing the
 * data of a regular grid in m-D space (with m<=n). Depending
 * on the dataset, the first few dimensions may not be used,
 * i.e., they may have a size of 1. E.g., in the case of 
 * 2D or 3D physics data the following layout may be given:
 *
 * [ ][ ][vy][vx][y][x] //First two dimensions have a size of 1
 * 
 * [vz][vy][vx][z][y][x] //All dimensions have a size >1
 *
 * The dataset may have ghost cells at the beginning and end
 * of each dimensions. In the case of the physics data, e.g., 
 * 3 ghost cells in each direction and dimensions, i.e., a
 * total of 6 ghost cells per data dimension. In case of 56 cells,
 * we have a layout of [ghost][actual data][ghost] =
 * [0 1 2][3...52][53 54 55]. The code supports arbitary numbers
 * of ghost zones at the beginning and end of each dimension.
 *
 * A dataset may furthmore have periodic boundaries. The reader
 * supports periodic boundaries on a per variable type, e.g.,
 * the data may be periodic in x,y and not periodic in vx, vy.
 * This is, e.g., the case in the physics dataset.
 *
 * Shortcomings of the current implementation:
 *
 * If a mFMin/mFMax are set, then edges pointing from a valid to
 * an invalid point will still be created. Also, since a point is
 * scheduled for finalization only after all its edges have been
 * send, all invalid points will still be issued for finalization.
 *
 * If a periodic dataset is given, then the ID of all boundary points
 * are stored in mBoundaryPoints which may be expensive. Furthermore,
 * all boundary points are finalized at the very end, after all
 * vertices have been red, which may not be optimal since some
 * of the boundary points could be finalized earlier.
 *
 * TODO:
 * - Check whether all boundary edges are added
 */
template <class DataClass = GenericData<float> >
class HDF5GridParser : public Parser<DataClass>
{

public:
  /*! \brief Default constructor
   *  @param filename : Name of the file to be opened.
   *  @param datasetName: Name of the dataset that should be opened.
   *  @param numGhostZones: Number of ghost zones in each direction.
   *  @param periodic : Is the dataset periodic, i.e., do we need to
   *                    add edges conecting oposite boundaries.
   */
  HDF5GridParser(const char* filename , std::string datasetName, int numGhostZones=0, bool periodic=false);
  
  /*! \brief Default constructor
   *  @param filename : Name of the file to be opened.
   *  @param datasetName: Name of the dataset that should be opened.
   *  @param numGhostZones: Number of ghost zones in each direction.
   *  @param periodic : Define which data dimensions are actually periodic
   */
  HDF5GridParser(const char* filename , std::string datasetName, int numGhostZones, vector<bool> periodic);

  /*! \brief Default constructor
   *  @param filename : Name of the file to be opened.
   *  @param datasetName: Name of the dataset that should be opened.
   *  @param numGhostZones: Number of ghost zones in each direction for 
   *                        each dimension.
   *  @param periodic : Is the dataset periodic, i.e., do we need to
   *                    add edges conecting oposite boundaries.
   */
  HDF5GridParser(const char* filename , std::string datasetName, vector< vector<int> > numGhostZones, bool periodic=false );

  //! Destructor
  virtual ~HDF5GridParser();

   /*! \brief Used to define the periodic data dimensions.
    *
    *  NOTE: This function should be called directly after
    *        creation, before any data is read. Otherwise
    *        parts of the data may be read with different
    *        settings.
    *
    *  \param periodic : Vector of length #Dimensions
    *                    (see getNumDimensions() ) defining
    *                    for each dimensions whether it is
    *                    periodic or not.
    */
  bool setPeriodicDimensions( const vector<bool>& periodic);

  /*! \brief Get the number of data dimensions
   *  \return int : The number of data dimensions
   */
  int getNumDimensions(){ return mDimensions.size(); }

  /*! \brief Get the number of actual data dimensions
   *  \return int : The number of data dimensions used.
   *                Leading data dimensions with lenght 1
   *                are considered "empty" dimensions.
   */
  int getNumActualDimensions(){ return mActualDimensions; }

  //! Read the next token
  virtual FileToken getToken();

private:
   //! HDF5 file from which data is loaded
  H5::H5File* mFile;

  //! Dataset from which the data is read
  H5::DataSet mDataset;

  //! Name of the file
  H5std_string mFilename;

  //! Name of the dataset that is read
  H5std_string mDatasetName;

  //! The HDF5 dataspace the data is read from
  H5::DataSpace mDataspace;

  //! The number of data dimensions of the dataset
  vector<hsize_t> mDimensions;

  //! The number of actual data dimensions
  unsigned int mActualDimensions;

  /*! Number of ghost zones in each dimension
   *
   * For each dimension mNumGhostZones[i] the number of
   * ghost cells at the begining mNumGhostZones[i][0]
   * and the end mNumGhostZomes[i][1] are stored.
   */
  vector< vector<int> > mNumGhostZones;

  //! Along which boundaries should we add periodic edges
  vector<bool> mPeriodicDimensions;
  //! Does the dataset have any periodic dimensions
  bool mPeriodic;


  //! List of all boundary points visited so far (only updated if mPeriodic=true)
  vector<GlobalIndexType> mBoundaryPoints;

  /*! \brief The index of the current hyperslap.
   * 
   * The first five entries mark the hyperslap that is currently stored
   * in mCurrentHyperslapData. The last index marks the current element
   * within the hyperslap.
   */
  vector<int> mCurrentHyperslap;
  
  //! Last non-ghost index in the hyperslap data
  int mLastNonGhostIndex;

  //! The data of the current hyperslap
  float* mCurrentHyperslapData;

  //! Keep a list of vertices that need to be finalized
  vector<GlobalIndexType> mScheduleFinalize;

  //! Keep a list of edges that need to be processed
  vector< vector<GlobalIndexType> > mScheduleEdge;

  //! List of "invalid" edges
  //map<GlobalIndexType , bool> mNonValidPoints;

  

private:
  /*! \brief Initial setup of the file and data
   *  \param filename : The name of the file to be read.
   *  \param datasetName : The name of the dataset to be read.
   */
  void initalize();

  //! Read the data of the next hyperslap to be processed
  bool readNextHyperslap();

  //! Get the index of the next hyperslap
  bool getNextHyperslap();

  //! Check whether the current hyperslap is the last one.
  bool isLastHyperslap();

  //! Schedule the negative diagonal neighbor of the current point for finalization
  void scheduleFinalize();

  //! Compute the index of a vertex based on the given hyperslap coordinate
  inline GlobalIndexType getIdFromHyperslap(const vector<int>& slap) {
    if (slap.size() != mDimensions.size()) {
      return -1;
    }
    GlobalIndexType id = 0;
    vector<int> factors;
    factors.resize(mDimensions.size(), 1);
    for (int i = int(mDimensions.size()) - 2; i >= 0; --i) {
      factors[i] = factors[i + 1] * mDimensions[i + 1];
    }
    for (unsigned int i = 0; i < mDimensions.size(); ++i) {
      id += slap[i] * factors[i];
    }
    return id;
  }

  //! Get the hyperslap index for a given ID. The vector given is modified
  inline void getHyperslapFromId(GlobalIndexType id , vector<int>& slap){
    slap.resize( mDimensions.size() , 0 );
    int numPointsPerDim=1;
    for( int i=int(mDimensions.size())-int(mActualDimensions) ; i<int(mDimensions.size()) ; ++i ){
      numPointsPerDim=1;
      for(unsigned int j=i+1; j<mDimensions.size() ; ++j ){
        numPointsPerDim*=mDimensions[j];
      }
      slap[i] = int( double(id)/double(numPointsPerDim) );
      id -= slap[i] * numPointsPerDim;
    }
  }

  //! Check whether a given point is a boundary point
  inline bool checkBoundary( const vector<int>& slap ){
    for( int i=int(mDimensions.size())-int(mActualDimensions) ; i<int(mDimensions.size()) ; ++i ){
      if(slap[i] == mNumGhostZones[i][0]){ return true; }
      if(slap[i] == int(mDimensions[i])-int(mNumGhostZones[i][1])-1 ){ return true; }
    }
    return false;
  }

  //! Check whether a given point is a boundary point
  inline bool checkBoundary( GlobalIndexType id ){
    vector<int> slap;
    getHyperslapFromId(id , slap);
    return checkBoundary( slap );
  }

  //! Check whether a given point is on a periodic boundary
  inline bool checkPeriodicBoundary( const vector<int>& slap ){
    for( int i=int(mDimensions.size())-int(mActualDimensions) ; i<int(mDimensions.size()) ; ++i ){
      if( mPeriodicDimensions[i] ){
        if(slap[i] == mNumGhostZones[i][0]){ return true; }
        if(slap[i] == int(mDimensions[i])-int(mNumGhostZones[i][1])-1 ){ return true; }
      }
    }
    return false;
  }

  //! Check whether a given point is on a periodic boundary
  inline bool checkPeriodicBoundary( GlobalIndexType id ){
    vector<int> slap;
    getHyperslapFromId(id , slap);
    return checkPeriodicBoundary( slap );
  }

  //! Check whether the given hyperslap coordinate points to a ghost cell
  inline bool checkGhost(const vector<int>& slap) {
    //Check whether the neighbor is a ghost point
    for (unsigned int i = 0; i < slap.size(); ++i) {
      if( slap[i] < mNumGhostZones[i][0] || slap[i]>=(int(mDimensions[i])-int(mNumGhostZones[i][1])) )
      {
        return true;
      }
    }
    return false;
  }

  //! Check whether the given id belongs to a ghost cell
  inline bool checkGhost(GlobalIndexType id){
    //Convert the id to a hyperslap index
    vector<int> slap;
    getHyperslapFromId(id,slap);
    //Check whether the hyperslap is a ghost cell
    return checkGhost( slap );
  }

  //! Print the given hyperslap index to cout
  inline void printHyperslap(const vector<int>& slap) {
    cout << "[ ";
    for (unsigned int i = 0; i < slap.size(); ++i)
      cout << slap[i] << " ";
    cout << "] ";
  }

  //! Print which dimensions are set to be periodic
  inline void printPeriodicDimensions(){
    cerr<<"Periodic: ";
    for(unsigned int i=0; i<mPeriodicDimensions.size() ; ++i ){
      cerr<<mPeriodicDimensions[i]<<" ";
    }
    cerr<<endl;
  }

  /*! \brief Test function
   *
   *  Read all hyperslaps iteratively using readNextHyperslap and
   *  print the data of each hyperslap to cout.
   */
  void testRead();
};

template <class DataClass>
HDF5GridParser<DataClass>::HDF5GridParser(const char* filename , std::string datasetName , int numGhostZones, bool periodic)
  : Parser<DataClass>(NULL,0), mFilename(filename) , mDatasetName(datasetName) , mPeriodic(periodic), mCurrentHyperslapData(NULL)
{
  //Initalize the file and all variables
  initalize();

  //TEST TEST TEST: Reduce dimensions to ease debugging
  /*mDimensions[0]=1;
  mDimensions[1]=1;
  mDimensions[2]=10;
  mDimensions[3]=10;
  mDimensions[4]=10;
  mDimensions[5]=10;
  numGhostZones=0;
  mActualDimensions=4;
  mPeriodic=true;*/

  //Define the number of ghost zones
  int numGhosts=0;
  for(unsigned int i=0 ; i<mDimensions.size() ; ++i)
  {
    numGhosts = (int(mDimensions[i]) > numGhostZones) ? numGhostZones : 0;
    mNumGhostZones[i][0] = numGhosts;
    mNumGhostZones[i][1] = numGhosts;
  }

  //Define the largest non-ghost index
  mLastNonGhostIndex = mDimensions.back() - mNumGhostZones.back()[1] -1;

  //Define the periodic dimensions
  mPeriodicDimensions.resize( mDimensions.size() , mPeriodic );
 
  //testRead();
}

 template <class DataClass>
HDF5GridParser<DataClass>::HDF5GridParser(const char* filename , std::string datasetName, int numGhostZones, vector<bool> periodic)
  : Parser<DataClass>(NULL,0), mFilename(filename) , mDatasetName(datasetName) , mCurrentHyperslapData(NULL)
{
  //Initalize the file and all variables
  initalize();

  //Define the number of ghost zones
  int numGhosts=0;
  for(unsigned int i=0 ; i<mDimensions.size() ; ++i)
  {
    numGhosts = (int(mDimensions[i]) > numGhostZones) ? numGhostZones : 0;
    mNumGhostZones[i][0] = numGhosts;
    mNumGhostZones[i][1] = numGhosts;
  }

  //Define the largest non-ghost index
  mLastNonGhostIndex = mDimensions.back() - mNumGhostZones.back()[1] -1;

  //Set the periodic dimensions
  bool ok = setPeriodicDimensions( periodic );
  if( !ok ){
    sterror( 0 , "HDF5GridParser: The given array for defining the periodic dimensions was invalid.")
  }
}

template <class DataClass>
HDF5GridParser<DataClass>::HDF5GridParser(const char* filename , std::string datasetName, vector< vector<int> > numGhostZones , bool periodic )
 : Parser<DataClass>(NULL,0), mFilename(filename) , mDatasetName(datasetName) , mPeriodic(periodic), mCurrentHyperslapData(NULL)
{
  //Initalize the file and all variables
  initalize();
  //Define the number of ghost zones
  mNumGhostZones = numGhostZones;
  //Define the largest non-ghost index
  mLastNonGhostIndex = mDimensions.back() - mNumGhostZones.back()[1] -1;
  //Define the periodic dimensions
  mPeriodicDimensions.resize( mDimensions.size() , mPeriodic );
}


template <class DataClass>
void HDF5GridParser<DataClass>::initalize()
{
  mFile = new H5::H5File(mFilename, H5F_ACC_RDONLY);
  try{
    mDataset = mFile->openDataSet( mDatasetName );
  }catch(H5::FileIException){
    sterror( 0, "Dataset not found in HDF5 file.")
  }catch( H5::DataTypeIException ){
    sterror( 0 , "Datset could not be opened in HDF5 file." );
  }

  H5T_class_t type_class = mDataset.getTypeClass();
  if( !type_class == H5T_FLOAT)
  {
    sterror( 0 , "Error: Currently only float data is supported for HDF5 grids." );
  }

  //Get the endian order
  H5::FloatType floattype = mDataset.getFloatType();
  H5std_string order_string;
  //H5T_order_t order = floattype.getOrder( order_string );
  floattype.getOrder( order_string );
  cout <<"Endian: "<< order_string << endl;

  //Get the dataspace of the dataset
  mDataspace = mDataset.getSpace();

  //Get the number of dimensions
  int ndims = mDataspace.getSimpleExtentNdims();
  //Get the size in each dimension
  hsize_t dims_out[ndims];
  mDataspace.getSimpleExtentDims( dims_out, NULL);
  cout<<"Number of dimensions: "<<ndims<<" : ";
  mActualDimensions=0;
  mDimensions.resize(ndims);
  //Save the number of dimensions. Define the number of
  //actual dimensions.
  mNumGhostZones.resize( mDimensions.size() );
  for(int i=0 ; i<ndims; ++i)
  {
    cout <<   (unsigned long)(dims_out[i]);
    cout << " ";
    if( dims_out[i] > 1 && mActualDimensions==0 ){
        mActualDimensions = ndims - i;
    }
    mNumGhostZones[i].resize( 2 , 0 );
    mDimensions[i] = dims_out[i];
  }
  cout<<endl;
  cout<<"Actual dimensions: "<<mActualDimensions<<endl;

  //Initalize the data array
  mCurrentHyperslapData = new float[ mDimensions.back() ];

}

template<class DataClass>
bool HDF5GridParser<DataClass>::setPeriodicDimensions( const vector<bool>& periodic )
{
  //If a non-empty vector is given, then it should specify a value for each dimension
  if( periodic.size() == mDimensions.size() ){
    //Save the new settings and check if any of the boundaries is set as periodic
    mPeriodicDimensions = periodic;
    mPeriodic = false;
    for( unsigned int i=0; i<mPeriodicDimensions.size() ; ++i ){
      if( mPeriodicDimensions[i] ){
        mPeriodic = true;
        break;
      }
    }
    printPeriodicDimensions();
    return true;
  }else{
    //Error, if we have a missmatch between periodic.size() and mDimensions.size()
    if( periodic.size() > 0 ){
      return false;
    }
    //If an empty vector is given then all dimensions are set to be non-periodic
    mPeriodicDimensions.resize( mDimensions.size() , false );
    mPeriodic = false;
    printPeriodicDimensions();
    return true;
  }

}

template <class DataClass>
HDF5GridParser<DataClass>::~HDF5GridParser()
{
  if( mFile ){
    mFile->close();
    delete mFile;
  }
  delete[] mCurrentHyperslapData;
}

template <class DataClass>
FileToken HDF5GridParser<DataClass>::getToken()
{
  //Check if we need to read more data
  if( ( mCurrentHyperslap.size() == 0 || mCurrentHyperslap.back() > mLastNonGhostIndex ) &&
      !isLastHyperslap()
    )
  {
    readNextHyperslap();
  }

  //Add an EDGE if possible
  //We need to do this first, because the list of edges and the finalize
  //list is updated when adding a vertex, i.e., we need to make sure that
  //all edges are done first before we go to the finalize list
  if (mScheduleEdge.size() > 0) {
    this->mPath = mScheduleEdge.back();
    mScheduleEdge.pop_back();
    //cout<<"EDGE: "<<this->mPath[0]<<" , "<<this->mPath[1]<<endl;
    return EDGE;
  }

  //FINALIZE a vertex if possible
  //We need to do this next, because otherwise we would not get to this
  //step until all vertecies have been added.
  if (mScheduleFinalize.size() > 0) {
    //for(unsigned int i=0; i<mScheduleFinalize.size() ; ++i){
      //If the vertex to be finalized is valid
      //if( mNonValidPoints.find( mScheduleFinalize[i] ) == mNonValidPoints.end() ){
        this->mFinal = mScheduleFinalize.back();
        mScheduleFinalize.pop_back();
        //cout<<"FINALIZE: "<<this->mFinal<<endl;
        return FINALIZE;
      //}else{
      //  mNonValidPoints.erase( this->mFinal ); //There will be no more edges pointing to this point
      //}
    //}
  }

  //Add a VERTEX if possible
  //If we can't do anything else, then add a vertex and update the list
  //of edges that need to be added next and the list of verticies that
  //can be finalized afterwards.
  if( (mCurrentHyperslap.back() <= mLastNonGhostIndex ) )
  {
    //Search for a valid point
    GlobalIndexType tempId;
    while( (mCurrentHyperslapData[mCurrentHyperslap.back()] < this->mFMin) ||
           (mCurrentHyperslapData[mCurrentHyperslap.back()] > this->mFMax) )
    {
      //Define the current point as invalid
      tempId = getIdFromHyperslap(mCurrentHyperslap);
      //mNonValidPoints[tempId]=true;
      //cout<<"INVALID "<<tempId<<" "<<mCurrentHyperslapData[ mCurrentHyperslap.back() ]<<endl;
      
      //Add the diagonal negative neighbor to the list of finalized vertices
      scheduleFinalize();

      //Go to the next hyperslap
      mCurrentHyperslap[ mDimensions.size()-1 ]++;
      //If we reached the end of the current hyperslap then try to load the next one
      if( mCurrentHyperslap.back() > mLastNonGhostIndex ){
        if( !isLastHyperslap() ){
          readNextHyperslap();
        }else{
          //We are at the end and need to finish things
          if( mScheduleFinalize.size()>0 || mScheduleEdge.size()>0 ){
            return getToken();
          }else{
            //There is nothing left to do
            //cout<<"EMPTY 1"<<endl;
            return EMPTY;
          }
        }
      }
    }
    //If we found a valid vertex
    //Update the data for the vertex
    this->mId   = getIdFromHyperslap( mCurrentHyperslap );
    this->mData = mCurrentHyperslapData[ mCurrentHyperslap.back() ];

    //Update the list of edges that need to be processed
    vector<int> tempHyperslap;
    for( unsigned int i=0; i<mDimensions.size() ; ++i )
    {
      vector<GlobalIndexType> tempEdge;
      tempHyperslap = mCurrentHyperslap;
      int maxIndex = mDimensions[i] - mNumGhostZones[i][1] -1;
      //Add negative edge if necessary
      tempHyperslap[i]--;
      if ((tempHyperslap[i] >= mNumGhostZones[i][0]) &&
          (tempHyperslap[i] <= maxIndex )
         ) 
      {
        tempEdge.resize(2);
        tempEdge[0] = this->mId;
        tempEdge[1] = getIdFromHyperslap(tempHyperslap);
        //if( mNonValidPoints.find(tempEdge[1]) == mNonValidPoints.end() ){
          mScheduleEdge.push_back(tempEdge);
        //}
      }
    }

    //If the dataset is periodic then add the respective missing edges
    if( mPeriodic )
    {
      vector<GlobalIndexType> tempEdge;
      tempEdge.resize(2);
      tempEdge[0] = this->mId;
      vector<int> tempHyperslap;
      //Check for each used dimension whether the point is on that boundary
      for( int i=int(mDimensions.size())-int(mActualDimensions) ; i<int(mDimensions.size()) ; ++i )
      {
        //If this dimensions is periodic
        if (mPeriodicDimensions[i]) {
          //If we are at the upper boundary of dimension i
          //then add the periodic edge.
          //If we are at the lower boundary then our periodic
          //neighbor has not been processed yet and we can not
          //add the edge yet.
          if (mCurrentHyperslap[i] == int(mDimensions[i]) - int(mNumGhostZones[i][1]) - 1) {
            tempHyperslap = mCurrentHyperslap;
            tempHyperslap[i] = mNumGhostZones[i][0];
            tempEdge[1] = getIdFromHyperslap(tempHyperslap);
            mScheduleEdge.push_back(tempEdge);
          }
        }
      }
      if( checkPeriodicBoundary( mCurrentHyperslap ) ){
        mBoundaryPoints.push_back(this->mId);
      }
    }

    //Schedule finalization of the negative diagonal neighbor
    scheduleFinalize();

    //Update the hyperslap index for the next call of getToken()
    mCurrentHyperslap[ mDimensions.size()-1 ]++;

    //Return that the data for the next vertex is ready
    //cout<<"VERTEX "<<this->mId<<endl;
    return VERTEX;
  }

  //If every previous check failed then this means that no more data is left,
  //i.e., all vertices and edges have been added and all vertices have been
  //finalized.
  //cout<<"EMPTY 2"<<endl;
  return EMPTY;

}

template <class DataClass>
bool HDF5GridParser<DataClass>::readNextHyperslap()
{
  if( getNextHyperslap() )
  {
    //cout<<"Processing Hyperslap: ";
    //printHyperslap( mCurrentHyperslap );
    //cout<<endl;
    int index = int(mCurrentHyperslap.size()) - 2;
    index = index<0?0:index;
    if (mCurrentHyperslap[index] == mNumGhostZones[index][0]) {
      cout << "Processed all hyperslaps < " << getIdFromHyperslap(mCurrentHyperslap) << " ";
      printHyperslap(mCurrentHyperslap);
      cout << endl;
    }


    //Define the dataspace hyperslap
    hsize_t offset[ mDimensions.size() ];
    hsize_t count [ mDimensions.size() ];
    for( unsigned int i=0; i<mDimensions.size() ; ++i){
      offset[i] = hsize_t(mCurrentHyperslap[i]);
      count[i]  = 1;
    }
    offset[ mDimensions.size()-1 ] = 0;
    count[ mDimensions.size()-1 ]  = mDimensions.back();
    mDataspace.selectHyperslab( H5S_SELECT_SET, count , offset );

    //Define the memory space
    hsize_t     dimsm[1];
    dimsm[0]  = mDimensions.back();
    H5::DataSpace memspace( 1 , dimsm );

    hsize_t      offset_out[1];       // hyperslab offset in memory
    hsize_t      count_out[1];        // size of the hyperslab in memory
    offset_out[0] = 0;
    count_out[0]  = mDimensions.back();
    memspace.selectHyperslab( H5S_SELECT_SET, count_out, offset_out );

    //Read the current hyperslap 
    mDataset.read( mCurrentHyperslapData , H5::PredType::NATIVE_FLOAT , memspace  , mDataspace );

    //Define the current index
    mCurrentHyperslap[ mCurrentHyperslap.size()-1 ]=mNumGhostZones.back()[0];

    return true;
  }
  else{
    cerr<<"HDF5GridParser<DataClass>::readNextHyperslap(): No more hyperslaps available"<<endl;
    return false;
  }
}

template <class DataClass>
bool HDF5GridParser<DataClass>::getNextHyperslap()
{
  //If there is no current hyperslap yet
  if(mCurrentHyperslap.size() ==0 ){
    mCurrentHyperslap.resize( mDimensions.size() );
    for( unsigned int i=0; i<mDimensions.size() ; ++i ){
      mCurrentHyperslap[i] = mNumGhostZones[i][0];
    }
    //cout<<"HDF5GridParser<DataClass>::getNextHyperslap(): Init current hyperslap"<<endl;
    return true;
  }
  
  //If we are already at the last hyperslap
  if( isLastHyperslap() ){
    //cout<<"HDF5GridParser<DataClass>::getNextHyperslap(): Reached last hyperslap"<<endl;
    return false;
  }

  //Move to the next hyperslap.
  for (int i = int(mDimensions.size()) - 2; i >= 0; i--) {
    if (mCurrentHyperslap[i] < (int(mDimensions[i]) - mNumGhostZones[i][0] - 1) )
    {
      mCurrentHyperslap[i] = mCurrentHyperslap[i] + 1;
      break;
    } else {
      mCurrentHyperslap[i] = mNumGhostZones[i][0];
    }
  }
  mCurrentHyperslap[ mCurrentHyperslap.size()-1 ]=mNumGhostZones.back()[0];
  return true;
}

template <class DataClass>
bool HDF5GridParser<DataClass>::isLastHyperslap()
{
  if( mCurrentHyperslap.size() > 0){
    bool re=true;
    for(int i=0; i<int(mCurrentHyperslap.size())-1 ; ++i){
      re = re && ( mCurrentHyperslap[i] == (int(mDimensions[i])-mNumGhostZones[i][1]-1) );
    }
    return re;
  }else{
    return false;
  }
}

template <class DataClass>
void HDF5GridParser<DataClass>::scheduleFinalize()
{
  //Create the negative diagonal neighbor
  int dimLimit = int(mDimensions.size()) - int(mActualDimensions);
  vector<int> tempHyperslap = mCurrentHyperslap;
  for(int i = mDimensions.size() - 1; i >= dimLimit; --i) {
    --tempHyperslap[i];
  }
  
  //Check whether the neighbor is a ghost point
  bool ghostPoint = checkGhost( tempHyperslap );
  bool periodicBoundary   = checkPeriodicBoundary( tempHyperslap );
  bool schedule   = !ghostPoint && !(mPeriodic && periodicBoundary);
  //Schedule the point for finalization
  if( schedule ){
    mScheduleFinalize.push_back(getIdFromHyperslap(tempHyperslap));
  }

  //If the current hyperslap is at the boundary then we need
  //to finalize additional points in the previous hyperslaps
  //If we are at the boundary in the top-most dimensions then
  //extra care needs to be taken during finalization.
  for (int dim1 = mDimensions.size() - 1; dim1 > dimLimit; dim1--) {
    int lastIndex = (mDimensions[dim1] - mNumGhostZones[dim1][1] - 1); //Last non ghost
    if (mCurrentHyperslap[dim1] == lastIndex) {
      tempHyperslap=mCurrentHyperslap;
      for(int dim2 = dim1-1; dim2 >= dimLimit; --dim2) {
        --tempHyperslap[dim2];
      }
      ghostPoint = checkGhost( tempHyperslap );
      periodicBoundary   = checkPeriodicBoundary( tempHyperslap );
      schedule   = !ghostPoint && !(mPeriodic && periodicBoundary);
      if( schedule ){
        mScheduleFinalize.push_back(getIdFromHyperslap(tempHyperslap));
      }
    }
  }
  
  //If we have reached the last data vertex (i.e., all vertices and edges have been added)
  int lastIndex = (mDimensions.back() - mNumGhostZones.back()[1] - 1); //Last non ghost
  if (isLastHyperslap() && mCurrentHyperslap.back() == lastIndex)
  {
    //If we are not periodic then we now need to finalize the complete top-most level
    if (!mPeriodic)
    {
      //Compute the last and first valid index within the final
      //level containing valid (i.e.,non-ghost) points
      int numPointsPerLevel = 1; //Total number of points on the last level
      for (unsigned int i = dimLimit + 1; i < mDimensions.size(); ++i) {
        numPointsPerLevel *= mDimensions[i];
      }
      int numGhostsBefore = 0; // mDimensions.back(); //number of ghost rows at the beginning
      int numGhostsAfter = 0; //mDimensions.back(); //number of ghost rows at the end
      for (int i = int(mDimensions.size()) - 2; i > dimLimit; --i) {
        numGhostsBefore *= mNumGhostZones[i][0];
        numGhostsAfter *= mNumGhostZones[i][1];
      }

      int lastId = numPointsPerLevel * (mDimensions[dimLimit] - mNumGhostZones[dimLimit][1]) - 1;
      int firstId = lastId - numPointsPerLevel + 1;
      lastId -= numGhostsAfter;
      firstId += numGhostsBefore;
      //Schedule all non-ghost points of the final level for finalization
      mScheduleFinalize.reserve(mScheduleFinalize.size() + numPointsPerLevel);
      //int relId;
      for (int i = firstId; i <= lastId; ++i) {
        //relId = i%mDimensions.back();
        //Schedule the point if it is not a ghost point
        //if( relId>=mNumGhostZones.back()[0] && relId <=lastIndex ){
        if (!checkGhost(i)) {
          mScheduleFinalize.push_back(i);
        }
      }
    }
    //If the dataset is periodic, then we now need to finalize all boundary vertices.
    else{
      mScheduleFinalize.insert( mScheduleFinalize.begin() ,
                                mBoundaryPoints.begin(),
                                mBoundaryPoints.begin()+mBoundaryPoints.size());
      cout<<"Scheduled "<<mBoundaryPoints.size()<<" boundary points"<<endl;
    }
  }
}

/********************************************************
 *  Test functions
 ******************************************************/
template <class DataClass>
void HDF5GridParser<DataClass>::testRead()
{
  cerr<<"HDF5GridParser<DataClass>::testRead(): start"<<endl;
  //read the data
  while( !isLastHyperslap() )
  {
    bool ok = readNextHyperslap();
    if (ok) {
      printHyperslap(mCurrentHyperslap);
      for (unsigned int i = 0; i < mDimensions.back(); ++i) {
        if( i==0 ){
          cout<<"[ ";
        }
        if( int(i)==mNumGhostZones.back()[0] || int(i)==mLastNonGhostIndex+1 ){
          cout<<"] [ ";
        }
        cout << mCurrentHyperslapData[i];
        cout << " ";
      }
      cout <<"]"<< endl;
    } else {
      cout << "Hyperslap not read" << endl;
      break;
    }
  }
  cerr << "HDF5GridParser<DataClass>::testRead(): finished"<<endl;
}
#endif

#endif
