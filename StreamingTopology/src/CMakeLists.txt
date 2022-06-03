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

IF (TALASS_ENABLE_VTK)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ENDIF()



SET (ST_HEADERS

    STMappedArray.h

    Definitions.h
    Multiplicity.h
    IteratorBase.h
    Vertex.h
    UnionInfo.h
    MergeInfo.h
    SplitInfo.h
    UnionVertex.h
    MergeVertex.h
    SplitVertex.h
    ContourVertex.h
    SegmentationInfo.h
    SegmentedUnionVertex.h
    SegmentedContourVertex.h
    BranchInfo.h
    EnhancedUnionVertex.h
    EnhancedSegUnionVertex.h
    BoundaryMarker.h
        
    SetBranch.h

    UnionAlgorithm.h
    SegmentedUnionAlgorithm.h
    EnhancedUnionAlgorithm.h
    EnhancedSegUnionAlgorithm.h
    AcceleratedUnionAlgorithm.h
    AcceleratedSegUnionAlgorithm.h

    UnionSegmentation.h
    ArraySegmentation.h
    MappedSegmentation.h
    MTSegmentation.h
    STSegmentation.h
    
    TopoTreeInterface.h
    TopoTree.h
    UnionTree.h
    ParallelUnionTree.h
    MergeTree.h
    ParallelMergeTree.h
    EnhancedMergeTree.h
    AcceleratedMergeTree.h
    EnhancedSegMergeTree.h
    AcceleratedSegMergeTree.h
    SplitTree.h
    EnhancedSplitTree.h
    AcceleratedSplitTree.h
    ContourTreeBase.h
    ContourTree.h
#    ContourTree_TreeMerge.h
#    SegmentedContourTree_TreeMerge.h
#    SegmentedContourTree_FullTree.h
    SegmentedUnionTree.h
    SegmentedMergeTree.h
    SegmentedSplitTree.h
    EnhancedSegSplitTree.h
    AcceleratedSegSplitTree.h
    SortedUnionTree.h
    SortedMergeTree.h
    SortedSplitTree.h
    TreeScatter.h
    TreeGather.h
    GridGather.h

    Node.h
    TopoGraphInterface.h
    TopoGraph.h
    MultiResGraph.h
    GraphIO.h
    FileIO.h
    FeatureFamily.h
    ArcMetrics.h

    DomainDecomposition.h
    BlockDecomposition.h
    
    #TestStreamingAggregators.h
)

SET (ST_SRC

    Vertex.cpp
    UnionInfo.cpp
    UnionVertex.cpp
    MergeVertex.cpp
    SplitVertex.cpp
    ContourVertex.cpp
    SegmentedUnionVertex.cpp
    SegmentedContourVertex.cpp
    EnhancedUnionVertex.cpp
    EnhancedSegUnionVertex.cpp
    BoundaryMarker.cpp

    SetBranch.cpp

    UnionAlgorithm.cpp
    SegmentedUnionAlgorithm.cpp
    EnhancedUnionAlgorithm.cpp
    EnhancedSegUnionAlgorithm.cpp

    TopoTree.cpp
    UnionTree.cpp
    ParallelUnionTree.cpp
    MergeTree.cpp
    SplitTree.cpp
    ContourTreeBase.cpp 
    ContourTree.cpp
#    ContourTree_TreeMerge.h
#    SegmentedContourTree_TreeMerge.h
#    SegmentedContourTree_FullTree.h
    SegmentedUnionTree.cpp
    SegmentedMergeTree.cpp
    SegmentedSplitTree.cpp
    EnhancedSegMergeTree.cpp
    AcceleratedSegMergeTree.cpp
    EnhancedSegSplitTree.cpp
    AcceleratedSegSplitTree.cpp
    SortedUnionTree.cpp
    SortedMergeTree.cpp
    SortedSplitTree.cpp
    TreeScatter.cpp
    TreeGather.cpp
    GridGather.cpp

    ArraySegmentation.cpp
    MappedSegmentation.cpp
    MTSegmentation.cpp
    STSegmentation.cpp

    Node.cpp
    TopoGraph.cpp
    MultiResGraph.cpp
    GraphIO.cpp
    FileIO.cpp
    FeatureFamily.cpp

    DomainDecomposition.cpp
    BlockDecomposition.cpp
 
    #TestStreamingAggregators.cpp
)

INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../parser
        ${FLEXARRAY_INCLUDE_DIR} 
        ${TOPO_PARSER_INCLUDE_DIR}
        ${STATISTICS_INCLUDE_DIR}
        ${HDF5_INCLUDE_DIR}
)


ADD_LIBRARY(StreamingTopology STATIC ${ST_SRC})

INSTALL(FILES ${ST_HEADERS}
        DESTINATION ${PROJECT_INCLUDE_DIR}/StreamingTopology
)


INSTALL(TARGETS  StreamingTopology 
        ARCHIVE DESTINATION ${PROJECT_LIBRARY_DIR}
        LIBRARY DESTINATION ${PROJECT_LIBRARY_DIR}
        RUNTIME DESTINATION ${PROJECT_LIBRARY_DIR}
)
