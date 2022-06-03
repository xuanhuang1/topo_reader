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
# Try to find the Talass UI library
# Once done this will define
#
# TALASS_UI_FOUND          - system has TalassUI
# TALASS_UI_INCLUDE_DIR    - path to TalassUI includes
# TALASS_UI_LIBRARIES      - path to TalassUI libraries
#
#

IF (NOT DEFINED TALASS_UI_FOUND)
   SET (TALASS_UI_FOUND FALSE)
ENDIF ()

IF ((NOT DEFINED TALASS_SUPER_BUILD) AND (NOT TALASS_UI_FOUND))

   FIND_PATH(TALASS_UI_INCLUDE_DIR TalassModule.h
      ${CMAKE_SOURCE_DIR}/../UI/build/include/talass
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
      PATH_SUFFIXES UI
    )

    FIND_LIBRARY(TALASS_UI_LIBRARIES UI
       ${CMAKE_SOURCE_DIR}/../UI/build/lib
       /usr/lib
       /sw/lib
    )

    IF (TALASS_UI_INCLUDE_DIR AND TALASS_UI_LIBRARIES)

       SET(TALASS_UI_FOUND TRUE)
       IF (CMAKE_VERBOSE_MAKEFILE)
          MESSAGE("Using TALASS_UI_INCLUDE_DIR = " ${TALASS_UI_INCLUDE_DIR}) 
          MESSAGE("Using TALASS_UI_LIBRARIES   = " ${TALASS_UI_LIBRARIES}) 
       ENDIF (CMAKE_VERBOSE_MAKEFILE)

    ELSE()
   
        MESSAGE("ERROR TalassUI library not found on the system")
 
    ENDIF()

ELSEIF (NOT TALASS_UI_FOUND)

   FIND_PATH(TALASS_UI_SOURCE_DIR UI/CMakeLists.txt
      ${CMAKE_SOURCE_DIR}/../
   )

   ADD_SUBDIRECTORY(${TALASS_UI_SOURCE_DIR}/UI ${PROJECT_BINARY_DIR}/UI)

   SET(TALASS_UI_FOUND TRUE)
   SET(TALASS_UI_INCLUDE_DIR ${TALASS_UI_SOURCE_DIR}/UI/src)
   SET(TALASS_UI_LIBRARIES TalassDataReader TalassCore ${TOPO_PARSER_LIBRARIES})

   IF (CMAKE_VERBOSE_MAKEFILE)
      MESSAGE("Using TALASS_UI_INCLUDE_DIR = " ${TALASS_UI_INCLUDE_DIR}) 
      MESSAGE("Using TALASS_UI_LIBRARIES   = " ${TALASS_UI_LIBRARIES}) 
   ENDIF (CMAKE_VERBOSE_MAKEFILE)

   GET_PROPERTY(LOCAL_DEFINITIONS DIRECTORY ${TALASS_UI_SOURCE_DIR}/UI PROPERTY COMPILE_DEFINITIONS)
   SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${LOCAL_DEFINITIONS})


ENDIF()
                         
