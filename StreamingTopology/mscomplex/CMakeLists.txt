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



SET (ST_MSCOMPLEX_HEADERS

 
    EdgeConstructor.h

    EmbeddedVertex.h
    GradientInfo.h
    MCVertex.h
    MSCVertex.h
    Slopes.h
    GradientComplexInterface.h
    GradientComplex.h
    ThreadedGradientComplex.h
    SlopeComplex.h
    MCAssembly.h
    MSCAssembly.h
    GraphNode.h
    Graph.h
    HierarchicalMorseComplex.h
    ComplexAssembly.h
    StableComplexAssembly.h
    UnstableComplexAssembly.h

)

SET (ST_MSCOMPLEX_SRC

    EdgeConstructor.cpp
    EmbeddedVertex.cpp
    GradientInfo.cpp
    MCVertex.cpp
    MSCVertex.cpp
    Slopes.cpp
    GradientComplex.cpp
    ThreadedGradientComplex.cpp
    SlopeComplex.cpp
    MCAssembly.cpp
    MSCAssembly.cpp
    GraphNode.cpp
    Graph.cpp
    HierarchicalMorseComplex.cpp
    ComplexAssembly.cpp
    StableComplexAssembly.cpp
    UnstableComplexAssembly.cpp

)

INCLUDE_DIRECTORIES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_SOURCE_DIR}/../parser
   ${FLEXARRAY_INCLUDE_DIR} 
   ${TOPO_PARSER_INCLUDE_DIR}
   ${STATISTICS_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIR}
)

ADD_LIBRARY(StreamingMSComplex STATIC ${ST_MSCOMPLEX_SRC})

INSTALL(TARGETS  StreamingMSComplex
        ARCHIVE DESTINATION ${PROJECT_LIBRARY_DIR}
        LIBRARY DESTINATION ${PROJECT_LIBRARY_DIR}
        RUNTIME DESTINATION ${PROJECT_LIBRARY_DIR}
)
