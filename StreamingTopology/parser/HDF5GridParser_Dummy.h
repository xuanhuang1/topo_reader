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


#ifndef HDF5GRIDPARSERDUMMY_H
#define HDF5GRIDPARSERDUMMY_H

#include <iostream>

#include "Parser.h"
#include "GenericData.h"
using namespace std; 


/*! \brief This is a dummy implementation of HDF5GridParser.
 *
 *  This version with the basic interface of the HDF5 can be
 *  used to compile code when HDF5 is not available.
 */
template <class DataClass = GenericData<float> >
class HDF5GridParser : public Parser<DataClass>
{

public: 
  //! Default constructor
  HDF5GridParser(const char* filename , std::string datasetName, int numGhostZones=0, bool periodic=false);
  //! Default constructor
  HDF5GridParser(const char* filename , std::string datasetName, int numGhostZones, vector<bool> periodic);
  //! Default constructor
  HDF5GridParser(const char* filename , std::string datasetName, vector< vector<int> > numGhostZones, bool periodic=false );

  //! Destructor
  virtual ~HDF5GridParser();
  
  //! Read the next token
  virtual FileToken getToken(){ return EMPTY; }

  //! Define which dimensions have periodic boundaries
  bool setPeriodicDimensions( const vector<bool>& periodic){ return false; }

  //! Get the total number of dimensions (including leading empty dimensions)
  int getNumDimensions(){ return 0; }

  //! Get the number of actually used dimensions
  int getNumActualDimensions(){ return 0; }
};

template <class DataClass>
HDF5GridParser<DataClass>::HDF5GridParser(const char* filename , std::string datasetName , int numGhostZones, bool periodic)
  : Parser<DataClass>(NULL,0)
{
  cerr<<"HDF5 is not avaiable in this binary."<<endl;
}

template <class DataClass>
HDF5GridParser<DataClass>::HDF5GridParser(const char* filename , std::string datasetName, vector< vector<int> > numGhostZones , bool periodic )
 : Parser<DataClass>(NULL,0)
{
  cerr<<"HDF5 is not avaiable in this binary."<<endl;
}

template <class DataClass>
HDF5GridParser<DataClass>::~HDF5GridParser()
{
}

#endif
