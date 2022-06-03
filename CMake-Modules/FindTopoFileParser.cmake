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
# TOPO_PARSER_FOUND          - system has TopologyFileParser
# TOPO_PARSER_INCLUDE_DIR    - path to TopoFileDefinitions.h
# TOPO_PARSER_LIBRARIES      - path to libTopologyFileParser.a
#
#

IF (NOT DEFINED TOPO_PARSER_FOUND)
   SET (TOPO_PARSER_FOUND FALSE)
ENDIF ()

IF ((NOT DEFINED TALASS_SUPER_BUILD) AND (NOT TOPO_PARSER_FOUND))

   FIND_PATH(TOPO_PARSER_INCLUDE_DIR TopoFileDefinitions.h
      ${CMAKE_SOURCE_DIR}/../TopologyFileParser/build/include/TopologyFileParser
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
      PATH_SUFFIXES TopologyFileParser
    )

    FIND_LIBRARY(TOPO_PARSER_LIBRARIES TopologyFileParser
       ${CMAKE_SOURCE_DIR}/../TopologyFileParser/build/lib
       /usr/lib
       /sw/lib
    )



    IF (TOPO_PARSER_INCLUDE_DIR AND TOPO_PARSER_LIBRARIES)

       SET(TOPO_PARSER_FOUND TRUE)
       IF (CMAKE_VERBOSE_MAKEFILE)
          MESSAGE("Using TOPO_PARSER_INCLUDE_DIR = " ${TOPO_PARSER_INCLUDE_DIR}) 
          MESSAGE("Using TOPO_PARSER_LIBRARIES   = " ${TOPO_PARSER_LIBRARIES}) 
       ENDIF (CMAKE_VERBOSE_MAKEFILE)

    ELSE()
   
        MESSAGE("ERROR TopologyFileParser library not found on the system")
 
    ENDIF()

ELSEIF (NOT TOPO_PARSER_FOUND)

   FIND_PATH(TOPO_PARSER_SOURCE_DIR TopologyFileParser/CMakeLists.txt
      ${CMAKE_SOURCE_DIR}/../
   )

   ADD_SUBDIRECTORY(${TOPO_PARSER_SOURCE_DIR}/TopologyFileParser ${PROJECT_BINARY_DIR}/TopologyFileParser)

   SET(TOPO_PARSER_FOUND TRUE)
   SET(TOPO_PARSER_INCLUDE_DIR ${TOPO_PARSER_SOURCE_DIR}/TopologyFileParser/src)
   SET(TOPO_PARSER_LIBRARIES TopologyFileParser)

   IF (CMAKE_VERBOSE_MAKEFILE)
      MESSAGE("Using TOPO_PARSER_INCLUDE_DIR = " ${TOPO_PARSER_INCLUDE_DIR}) 
      MESSAGE("Using TOPO_PARSER_LIBRARIES   = " ${TOPO_PARSER_LIBRARIES}) 
   ENDIF (CMAKE_VERBOSE_MAKEFILE)

   GET_PROPERTY(LOCAL_DEFINITIONS DIRECTORY ${TOPO_PARSER_SOURCE_DIR}/TopologyFileParser PROPERTY COMPILE_DEFINITIONS)
   SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${LOCAL_DEFINITIONS})

   GET_PROPERTY(LOCAL_INCLUDES DIRECTORY ${TOPO_PARSER_SOURCE_DIR}/TopologyFileParser PROPERTY INCLUDE_DIRECTORIES)
   SET_PROPERTY(DIRECTORY PROPERTY INCLUDE_DIRECTORIES ${LOCAL_INCLUDES})

ENDIF()
                         
