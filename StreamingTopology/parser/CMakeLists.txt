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



SET (ST_PARSER_HEADERS


    Parser.h
    GridEdgeParser.h
    PeriodicGridEdgeParser.h
    GridParser.h
    GridParser.h
    PeriodicGridParser.h
    HDF5GridParser.h
    HDF5GridParser_Dummy.h
    ImplicitGridParser.h
    ImplicitPeriodicGridParser.h
    ImplicitGridEdgeParser.h
    PMS2DParser.h
    ImplicitPMS2DParser.h
    SortedGridParser.h
    SMAParser.h
    SMBParser.h
    BinaryParser.h
    DistributedBinaryParser.h
    CompactBinaryParser.h
    CompactDistributedBinaryParser.h
    SharedBinaryParser.h
    GenericData.h
)

SET (ST_PARSER_SRC

    Parser.cpp
    GridEdgeParser.cpp
    PeriodicGridEdgeParser.cpp
    GridParser.cpp
    PeriodicGridParser.cpp
    SubGridParser.cpp
    HDF5GridParser.cpp
    HDF5GridParser_Dummy.cpp
    ImplicitGridParser.cpp
    ImplicitPeriodicGridParser.h
    ImplicitGridEdgeParser.cpp
    PMS2DParser.cpp
    ImplicitPMS2DParser.cpp
    SortedGridParser.cpp
    SMAParser.cpp
    SMBParser.cpp
    BinaryParser.cpp
    DistributedBinaryParser.cpp
    CompactBinaryParser.cpp
    CompactDistributedBinaryParser.cpp
    SharedBinaryParser.cpp
 )

INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${FLEXARRAY_INCLUDE_DIR} 
        ${TOPO_PARSER_INCLUDE_DIR}
        ${HDF5_INCLUDE_DIR}
)

ADD_LIBRARY(StreamingParser STATIC ${ST_PARSER_SRC})


INSTALL(FILES ${ST_PARSER_HEADERS}
        DESTINATION ${PROJECT_INCLUDE_DIR}/StreamingTopology
)


INSTALL(TARGETS  StreamingParser 
        ARCHIVE DESTINATION ${PROJECT_LIBRARY_DIR}
        LIBRARY DESTINATION ${PROJECT_LIBRARY_DIR}
        RUNTIME DESTINATION ${PROJECT_LIBRARY_DIR}
)
