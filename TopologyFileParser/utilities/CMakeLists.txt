
INCLUDE_DIRECTORIES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_BINARY_DIR}
)


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



ADD_EXECUTABLE(merge_distributed_files merge_distributed_files.cpp)   
TARGET_LINK_LIBRARIES(merge_distributed_files ${TOPO_LIBRARIES})

ADD_EXECUTABLE(change_clan_attributes change_clan_attributes.cpp)   
TARGET_LINK_LIBRARIES(change_clan_attributes ${TOPO_LIBRARIES})

ADD_EXECUTABLE(change_family_attributes change_family_attributes.cpp)   
TARGET_LINK_LIBRARIES(change_family_attributes ${TOPO_LIBRARIES})

ADD_EXECUTABLE(tutorial_1 tutorial_1.cpp)   
TARGET_LINK_LIBRARIES(tutorial_1 ${TOPO_LIBRARIES})

ADD_EXECUTABLE(file_output_example file_output_example.cpp)   
TARGET_LINK_LIBRARIES(file_output_example ${TOPO_LIBRARIES})

ADD_EXECUTABLE(compute_tracking_graph compute_tracking_graph.cpp)   
TARGET_LINK_LIBRARIES(compute_tracking_graph ${TOPO_LIBRARIES})

ADD_EXECUTABLE(localizer_to_family localizer_to_family.cpp)   
TARGET_LINK_LIBRARIES(localizer_to_family ${TOPO_LIBRARIES})

ADD_EXECUTABLE(output_features output_features.cpp)   
TARGET_LINK_LIBRARIES(output_features ${TOPO_LIBRARIES})

IF (TALASS_ENABLE_VTK)
  ADD_EXECUTABLE(output_feature_geometry output_feature_geometry.cpp)   
  TARGET_LINK_LIBRARIES(output_feature_geometry ${TOPO_LIBRARIES})
endif()



INSTALL(TARGETS output_features change_family_attributes change_clan_attributes file_output_example 
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
)


