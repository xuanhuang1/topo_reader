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

SET(ST_PARALLEL_HEADERS
    AddressHandler.h
    Block2Stream.h
)

SET(ST_PARALLEL_SRC
    Block2Stream.cpp
)


#INCLUDE_DIRECTORIES(
#   ${STREAMING_TOPOLOGY_SRC}
#   ${STREAMING_TOPOLOGY_SOURCE_DIR}/parser
#   ${FLEXARRAY_INCLUDE_DIR}
#   ${STATISTICS_INCLUDE_DIR}    
#   ${TOPO_PARSER_INCLUDE_DIR}
#)

ADD_LIBRARY(ParallelTopology STATIC ${ST_PARALLEL_SRC})

#SET(LINK_LIBRARIES
#    ParallelTopology
#    StreamingTopology 
#    StreamingParser
#    ${STATISTICS_LIBRARIES}
#    ${TOPO_PARSER_LIBRARIES}
#    ${FLEXARRAY_LIBRARIES}
#    ${PTHREAD_LIBRARIES}
#)


SET(LINK_LIBRARIES_BLOCK
   ParallelTopology
)

#ADD_EXECUTABLE(build_threaded_tree  build_threaded_tree.cpp)   
#TARGET_LINK_LIBRARIES(build_threaded_tree ${LINK_LIBRARIES})

ADD_EXECUTABLE(block_to_stream  block_to_stream.cpp)   
TARGET_LINK_LIBRARIES(block_to_stream ${LINK_LIBRARIES_BLOCK})

#INSTALL(TARGETS ParallelTopology build_threaded_tree block_to_stream
#        ARCHIVE DESTINATION ${PROJECT_LIBRARY_DIR}
#        LIBRARY DESTINATION ${PROJECT_LIBRARY_DIR}
#        RUNTIME DESTINATION ${PROJECT_LIBRARY_DIR}
#)


