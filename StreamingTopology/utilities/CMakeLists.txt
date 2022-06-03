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




INCLUDE_DIRECTORIES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_SOURCE_DIR}/../parser
   ${CMAKE_CURRENT_SOURCE_DIR}/../mscomplex
   ${STATISTICS_INCLUDE_DIR}  
   ${TOPO_PARSER_INCLUDE_DIR}
   ${FLEXARRAY_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIR}
)

SET(LINK_LIBRARIES
        StreamingTopology 
        StreamingParser
       ${STATISTICS_LIBRARIES}
       ${TOPO_PARSER_LIBRARIES}
       ${FLEXARRAY_LIBRARIES}
       ${PTHREAD_LIBRARIES}
       ${HDF5_LIBRARIES}
)

IF (TALASS_ENABLE_VTK)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    
    SET(LINK_LIBRARIES
      ${LINK_LIBRARIES}
      ${VTK_LIBRARIES}
      )
ENDIF()


#ADD_EXECUTABLE(streaming_topology  streaming_topology.cpp)   
#TARGET_LINK_LIBRARIES(streaming_topology StreamingTopology)

#ADD_EXECUTABLE(streaming_segmentation  streaming_segmentation.cpp)   
#TARGET_LINK_LIBRARIES(streaming_segmentation StreamingTopology)

ADD_EXECUTABLE(build_topo_tree  ManPage.cpp build_topo_tree.cpp)   
TARGET_LINK_LIBRARIES(build_topo_tree ${LINK_LIBRARIES})

#ADD_EXECUTABLE(build_gradient_complex2D  build_gradient_complex.cpp)   
#TARGET_LINK_LIBRARIES(build_gradient_complex2D StreamingMSComplex ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(build_gradient_complex2D PROPERTIES COMPILE_FLAGS -DDIMENSION=2)

#ADD_EXECUTABLE(build_gradient_complex3D  build_gradient_complex.cpp)   
#TARGET_LINK_LIBRARIES(build_gradient_complex3D StreamingMSComplex ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(build_gradient_complex3D PROPERTIES COMPILE_FLAGS -DDIMENSION=3)

#ADD_EXECUTABLE(build_gradient_complex4D  build_gradient_complex.cpp)   
#TARGET_LINK_LIBRARIES(build_gradient_complex4D StreamingMSComplex ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(build_gradient_complex4D PROPERTIES COMPILE_FLAGS -DDIMENSION=4)

#ADD_EXECUTABLE(build_gradient_complex5D  build_gradient_complex.cpp)   
#TARGET_LINK_LIBRARIES(build_gradient_complex5D StreamingMSComplex ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(build_gradient_complex5D PROPERTIES COMPILE_FLAGS -DDIMENSION=5)

#ADD_EXECUTABLE(build_gradient_complex6D  build_gradient_complex.cpp)   
#TARGET_LINK_LIBRARIES(build_gradient_complex6D StreamingMSComplex ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(build_gradient_complex6D PROPERTIES COMPILE_FLAGS -DDIMENSION=6)




ADD_EXECUTABLE(obj2sma  obj2sma.cpp)   
TARGET_LINK_LIBRARIES(obj2sma ${LINK_LIBRARIES})

ADD_EXECUTABLE(smb2smb  smb2smb.cpp)   
TARGET_LINK_LIBRARIES(smb2smb ${LINK_LIBRARIES})

ADD_EXECUTABLE(binary2binary  binary2binary.cpp)   
TARGET_LINK_LIBRARIES(binary2binary ${LINK_LIBRARIES})

ADD_EXECUTABLE(mef2sma  mef2sma.cpp)   
TARGET_LINK_LIBRARIES(mef2sma ${LINK_LIBRARIES})

# Deprecated
#ADD_EXECUTABLE(aggregate_feature_families   aggregate_feature_families.cpp)   
#TARGET_LINK_LIBRARIES(aggregate_feature_families ${LINK_LIBRARIES})
#SET_TARGET_PROPERTIES(aggregate_feature_families PROPERTIES COMPILE_FLAGS ${ADDITIONAL_COMPILE_FLAGS})

ADD_EXECUTABLE(raw2grid   raw2grid.cpp)   
TARGET_LINK_LIBRARIES(raw2grid ${LINK_LIBRARIES})

ADD_EXECUTABLE(pms2smb   pms2smb.cpp)   
TARGET_LINK_LIBRARIES(pms2smb ${LINK_LIBRARIES})

#ADD_EXECUTABLE(process_family process_family.cpp)   
#TARGET_LINK_LIBRARIES(process_family ${LINK_LIBRARIES})

ADD_EXECUTABLE(bin2pts bin2pts.cpp)   
TARGET_LINK_LIBRARIES(bin2pts ${LINK_LIBRARIES})
ADD_EXECUTABLE( stream_localizer_to_family stream_localizer_to_family.cpp)   
TARGET_LINK_LIBRARIES(stream_localizer_to_family ${LINK_LIBRARIES})

INSTALL(TARGETS  build_topo_tree obj2sma smb2smb binary2binary mef2sma raw2grid pms2smb stream_localizer_to_family
        ARCHIVE DESTINATION ${PROJECT_EXE_DIR}
        LIBRARY DESTINATION ${PROJECT_EXE_DIR}
        RUNTIME DESTINATION ${PROJECT_EXE_DIR}
)

ADD_EXECUTABLE(probe_binary  probe_binary.cpp)   

ADD_EXECUTABLE(extract_block  extract_block.cpp)   

