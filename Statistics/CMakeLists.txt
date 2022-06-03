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

TALASS_PROJECT(Statistics)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/config/StatisticsConfig.cmake)

# too many warnings under windows
if(WIN32)
  set(CMAKE_C_WARNING_LEVEL   0)
  set(CMAKE_CXX_WARNING_LEVEL 0)    
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /W0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
endif()



#Check if OpenMP support is available and set flags.
IF (TALASS_USE_OPENMP)
FIND_PACKAGE(OpenMP)
if (OPENMP_FOUND)
  SET (HAVE_OPENMP 1)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()
endif()

FIND_PACKAGE(TopoFileParser)

INCLUDE (CMakeDetermineSystem)

#this check sets the preprocessor option MAC_OS
#opengl include files are stored in OpengGL/*.h instead of GL/*.h
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  ADD_DEFINITIONS (-DMAC_OS)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

IF (TALASS_ENABLE_VTK)
    FIND_PACKAGE(VTK COMPONENTS vtkCommonDataModel vtkFiltersCore vtkIOXML NO_MODULE)
    INCLUDE(${VTK_USE_FILE})
ENDIF()
        


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(utilities)

IF (TALASS_ENABLE_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF ()

IF (TALASS_ENABLE_PYTHON)
    ADD_SUBDIRECTORY(python)
ENDIF()  

