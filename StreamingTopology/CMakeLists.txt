#######################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov 
# OCEC-08-107
# All rights reserved.  
#   
# This file is part of "Streaming Topological Graphs Version 1.0."
# Please also read BSD_ADDITIONAL.txt.
#   
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   
# @ Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the disclaimer below.
# @ Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the disclaimer (as noted below) in
#   the documentation and/or other materials provided with the
#   distribution.
# @ Neither the name of the LLNS/LLNL nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#   
#  
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
# LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING
#
#######################################################################



cmake_minimum_required(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMake-Modules ${CMAKE_MODULE_PATH})
INCLUDE(TalassSetup)



TALASS_PROJECT(StreamingTopology)


INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/config/STConfig.cmake)


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)


if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")# -stdlib=libc++")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")# -stdlib=libc++")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

IF (TALASS_ENABLE_PTHREADS)
   FIND_PACKAGE(PThread)
ELSE ()
  ADD_DEFINITIONS(-DST_DISABLE_PTHREADS)
ENDIF ()


IF (TALASS_ENABLE_HDF5)
  FIND_PACKAGE(HDF5)
ELSE ()
  ADD_DEFINITIONS(-DST_DISABLE_HDF5)
ENDIF ()

IF (TALASS_INCORE_ARRAYS)
  ADD_DEFINITIONS(-DST_INCORE_ARRAYS)
ENDIF ()

IF (TALASS_ENABLE_PARALLEL)
  ADD_DEFINITIONS(-DST_ENABLE_PARALLEL)
ENDIF ()

IF (TALASS_ENABLE_VISUSIO)
  ADD_DEFINITIONS(-DST_ENABLE_VISUSIO)
  FIND_PACKAGE(VisusIO)
ENDIF ()

IF (TALASS_ENABLE_VTK)
    FIND_PACKAGE(VTK COMPONENTS vtkCommonDataModel vtkFiltersCore NO_MODULE)
    INCLUDE(${VTK_USE_FILE})
    MESSAGE("VTK_INCLUDE_DIRS = ", ${VTK_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(FlexArray)
FIND_PACKAGE(TopoFileParser)
FIND_PACKAGE(Statistics)


ADD_SUBDIRECTORY(parser)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(mscomplex)
ADD_SUBDIRECTORY(utilities)

IF (TALASS_ENABLE_PARALLEL)
    ADD_SUBDIRECTORY(parallel)
ENDIF (TALASS_ENABLE_PARALLEL)
