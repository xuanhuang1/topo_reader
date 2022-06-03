#######################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov, jcbenne@sandia.gov
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





#
# Try to find the Statistics libraries
# Once done this will define
#
# STATISTICS_FOUND          - system has Statistics
# STATISTICS_INCLUDE_DIR    - path to Statistics/SegmentAggregator.h
# STATISTICS_LIBRARIES      - path to libStatistics.a
#
#

IF (NOT DEFINED STATISTICS_FOUND)
   SET (STATISTICS_FOUND FALSE)
ENDIF ()


IF ((NOT DEFINED TALASS_SUPER_BUILD) AND (NOT STATISTICS_FOUND))

    FIND_PATH(STATISTICS_INCLUDE_DIR Attribute.h
       ${CMAKE_SOURCE_DIR}/../Statistics/build/include
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
      PATH_SUFFIXES Statistics
   )

   FIND_LIBRARY(STATISTICS_LIBRARIES Statistics
       ${CMAKE_SOURCE_DIR}/../Statistics/build/lib
       /usr/lib
       /sw/lib
   )

   IF (STATISTICS_INCLUDE_DIR AND STATISTICS_LIBRARIES)

      SET(STATISTICS_FOUND TRUE)
      IF (CMAKE_VERBOSE_MAKEFILE)
         MESSAGE("Using STATISTICS_INCLUDE_DIR = " ${STATISTICS_INCLUDE_DIR}) 
         MESSAGE("Using STATISTICS_LIBRARIES   = " ${STATISTICS_LIBRARIES}) 
      ENDIF (CMAKE_VERBOSE_MAKEFILE)

   ELSE (STATISTICS_INCLUDE_DIR AND STATISTICS_LIBRARIES)
   
      MESSAGE("ERROR Statistics library not found on the system")
 
   ENDIF (STATISTICS_INCLUDE_DIR AND STATISTICS_LIBRARIES)

ELSEIF (NOT STATISTICS_FOUND) 

   FIND_PATH(STATISTICS_SOURCE_DIR Statistics/CMakeLists.txt
      ${CMAKE_SOURCE_DIR}/../
   )

   ADD_SUBDIRECTORY(${STATISTICS_SOURCE_DIR}/Statistics ${PROJECT_BINARY_DIR}/Statistics)

   SET(STATISTICS_FOUND TRUE)
   SET(STATISTICS_INCLUDE_DIR ${STATISTICS_SOURCE_DIR}/Statistics/src)
   SET(STATISTICS_LIBRARIES Statistics)


    GET_PROPERTY(LOCAL_DEFINITIONS DIRECTORY ${STATISTICS_SOURCE_DIR}/Statistics PROPERTY COMPILE_DEFINITIONS)
    SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${LOCAL_DEFINITIONS})

ENDIF()
