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
# STREAMING_TOPO_FOUND          - system has StreamingTopology
# STREAMING_TOPO_INCLUDE_DIR    - path to StreamingTopology/TopoGraph.h
# STREAMING_topo_LIBRARY        - path to libStreamingTopology.a
# STREAMING_parser_LIBRARY      - path to libStreamingParser.a
# STREAMING_TOPO_LIBRARIES      - path to libStreamingTopology.a libStreamingParser
#
#


IF (NOT DEFINED STREAMING_TOPO_FOUND)
   SET (STREAMING_TOPO_FOUND FALSE)
ENDIF ()

IF ((NOT DEFINED TALASS_SUPER_BUILD) AND (NOT STREAMING_TOPO_FOUND))


    FIND_PATH(STREAMING_TOPO_INCLUDE_DIR StreamingTopology/TopoGraph.h
        ${CMAKE_SOURCE_DIR}/../StreamingTopology/build/include
        /usr/include
        /usr/X11/include
        /usr/X11R6/include
    )

    FIND_LIBRARY(STREAMING_topo_LIBRARY StreamingTopology
        ${CMAKE_SOURCE_DIR}/../StreamingTopology/build/lib
        /usr/lib
        /sw/lib
    )

    FIND_LIBRARY(STREAMING_parser_LIBRARY StreamingParser
        ${CMAKE_SOURCE_DIR}/../StreamingTopology/build/lib
        /usr/lib
        /sw/lib
    )

    IF (STREAMING_TOPO_INCLUDE_DIR AND STREAMING_topo_LIBRARY AND STREAMING_parser_LIBRARY)

        SET(STREAMING_TOPO_LIBRARIES ${STREAMING_topo_LIBRARY} ${STREAMING_parser_LIBRARY})

        SET(STREAMING_TOPO_FOUND TRUE)
        IF (CMAKE_VERBOSE_MAKEFILE)
            MESSAGE("Using STREAMING_TOPO_INCLUDE_DIR = " ${STREAMING_TOPO_INCLUDE_DIR}) 
            MESSAGE("Using STREAMING_TOPO_LIBRARIES   = " ${STREAMING_TOPO_LIBRARIES}) 
        ENDIF (CMAKE_VERBOSE_MAKEFILE)

    ELSE (STREAMING_TOPO_INCLUDE_DIR AND STREAMING_topo_LIBRARY AND STREAMING_parser_LIBRARY)
   
        MESSAGE("ERROR StreamingTopology libraries not found on the system")
        MESSAGE("Using STREAMING_TOPO_INCLUDE_DIR = " ${STREAMING_TOPO_INCLUDE_DIR}) 
        MESSAGE("Using STREAMING_topo_LIBRARY   = " ${STREAMING_topo_LIBRARY}) 
        MESSAGE("Using STREAMING_parser_LIBRARY   = " ${STREAMING_parser_LIBRARY}) 
 
    ENDIF ()
    
ELSEIF (NOT STREAMING_TOPO_FOUND)

    FIND_PATH(STREAMING_TOPO_SOURCE_DIR StreamingTopology/CMakeLists.txt
        ${CMAKE_SOURCE_DIR}/../
    )

    ADD_SUBDIRECTORY(${STREAMING_TOPO_SOURCE_DIR}/StreamingTopology ${PROJECT_BINARY_DIR}/StreamingTopology)

    SET(STREAMING_TOPO_FOUND TRUE)
    SET(STREAMING_TOPO_INCLUDE_DIR 
        ${STREAMING_TOPO_SOURCE_DIR}/StreamingTopology/src
        ${STREAMING_TOPO_SOURCE_DIR}/StreamingTopology/parser
    )
        
    SET(STREAMING_TOPO_LIBRARIES StreamingTopology StreamingParser)

    GET_PROPERTY(LOCAL_DEFINITIONS DIRECTORY ${STREAMING_TOPO_SOURCE_DIR}/StreamingTopology PROPERTY COMPILE_DEFINITIONS)
    MESSAGE("COMPILE_DEFINITIONS = " ${LOCAL_DEFINITIONS})

    SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${LOCAL_DEFINITIONS})
            

ENDIF ()
                         
