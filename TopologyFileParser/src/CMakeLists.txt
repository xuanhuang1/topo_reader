
IF (TALASS_ENABLE_VTK)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ENDIF()

MESSAGE("VTK_MODULES_DIR " ${VTK_MODULES_DIR} ${VTK_INCLUDE_DIRS})

SET (TOPOLOGY_FILE_PARSER_HEADERS
    LocationHandle.h
    AnnotationHandle.h
    AssociationHandle.h
    AssociationKey.h
    ClanHandle.h
    ClanKey.h
    DataHandle.h
    EdgeHandle.h
    FamilyHandle.h
    FamilyKey.h
    FeatureData.h
    FeatureElement.h
    Feature.h
    FeatureHierarchy.h
    FeatureSegmentation.h
    Segment.h
    FileData.h
    FileElement.h
    FileHandle.h
    HandleCollection.h
    HandleKeys.h
    IndexHandle.h
    SegmentationHandle.h
    GeometryHandle.h
    SimplificationHandle.h
    SimplificationKey.h
    StatHandle.h
    ValueElement.h
    xmlParser.h
    SimplificationKey.h
    StatKey.h
    IntervalTree.h
    TrackingEdge.h
    FamilyCorrelation.h
    ComputeTracking.h
    KdTree.h
)

SET (TOPOLOGY_FILE_PARSER_SOURCES
    xmlParser.cpp

    Feature.cpp
    FeatureData.cpp
    FeatureHierarchy.cpp
    FeatureSegmentation.cpp
    Segment.cpp
       
    FileData.cpp
    FeatureElement.cpp
    FileHandle.cpp
    DataHandle.cpp
    StatHandle.cpp
    #AttributeHandle.cpp
    #AggregateHandle.cpp
    LocationHandle.cpp
    AnnotationHandle.cpp
    EdgeHandle.cpp
    SimplificationHandle.cpp
    AssociationHandle.cpp
    SegmentationHandle.cpp
    GeometryHandle.cpp
    IndexHandle.cpp
    FamilyHandle.cpp
    ClanHandle.cpp
    HandleKeys.cpp
    HandleCollection.cpp
    ClanKey.cpp
    FamilyKey.cpp
    SimplificationKey.cpp
    ValueElement.cpp
    StatKey.cpp
    IntervalTree.cpp
    FamilyCorrelation.cpp
    ComputeTracking.cpp
    KdTree.cpp
)

if(BUILD_SHARED_LIBS)
	ADD_LIBRARY(TopologyFileParser SHARED ${TOPOLOGY_FILE_PARSER_SOURCES} ${TOPOLOGY_FILE_PARSER_HEADERS})
else()
	ADD_LIBRARY(TopologyFileParser STATIC ${TOPOLOGY_FILE_PARSER_SOURCES} ${TOPOLOGY_FILE_PARSER_HEADERS})
endif()

SET(TOPO_LIBRARIES 
  TopologyFileParser
) 

IF (TALASS_ENABLE_VTK)

    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})   
    
    SET(TOPO_LIBRARIES 
      ${TOPO_LIBRARIES}
      ${VTK_LIBRARIES}
      )

ENDIF()

ADD_EXECUTABLE(test_feature_hierarchy test_feature_hierarchy.cpp)   
TARGET_LINK_LIBRARIES(test_feature_hierarchy ${TOPO_LIBRARIES})

INSTALL(FILES ${TOPOLOGY_FILE_PARSER_HEADERS}
        DESTINATION include/TopologyFileParser
)

INSTALL(TARGETS TopologyFileParser 
        EXPORT TopoFileParser
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

target_include_directories(TopologyFileParser
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

