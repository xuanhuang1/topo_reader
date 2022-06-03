INCLUDE_DIRECTORIES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${TOPO_PARSER_INCLUDE_DIR}
)


SET(LINK_LIBRARIES
    Statistics
    ${TOPO_PARSER_LIBRARIES}
)

IF (TALASS_ENABLE_VTK)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    
    SET(LINK_LIBRARIES
      ${LINK_LIBRARIES}
      ${VTK_LIBRARIES}
      )
ENDIF()


ADD_EXECUTABLE(buildTestTree  buildTestTree.cpp)   
TARGET_LINK_LIBRARIES(buildTestTree ${LINK_LIBRARIES})

#ADD_EXECUTABLE(batchExplorer  batchExplorer.cpp)   
#TARGET_LINK_LIBRARIES(batchExplorer ${LINK_LIBRARIES})

#ADD_EXECUTABLE(cmdLineProc  cmdLineProcessing.cpp)   
#TARGET_LINK_LIBRARIES(cmdLineProc ${LINK_LIBRARIES})

#ADD_EXECUTABLE(testCovariance  testCovariance.cpp)   
#TARGET_LINK_LIBRARIES(testCovariance ${LINK_LIBRARIES})

ADD_EXECUTABLE(add_derived_attribute  add_derived_attribute.cpp)   
TARGET_LINK_LIBRARIES(add_derived_attribute ${LINK_LIBRARIES})

ADD_EXECUTABLE(add_mean_position add_mean_position.cpp)
TARGET_LINK_LIBRARIES(add_mean_position ${LINK_LIBRARIES})

ADD_EXECUTABLE(build_transformed_volume build_transformed_volume.cpp)   
TARGET_LINK_LIBRARIES(build_transformed_volume ${LINK_LIBRARIES})

ADD_EXECUTABLE(add_derived_metric add_derived_metric.cpp)   
TARGET_LINK_LIBRARIES(add_derived_metric ${LINK_LIBRARIES})

#ADD_EXECUTABLE(dump_feature_count dump_feature_count.cpp)   
#TARGET_LINK_LIBRARIES(dump_feature_count ${LINK_LIBRARIES})

ADD_EXECUTABLE(dump_by_parameter dump_by_parameter.cpp)   
TARGET_LINK_LIBRARIES(dump_by_parameter ${LINK_LIBRARIES})

ADD_EXECUTABLE(dump_values_at_parameter dump_values_at_parameter.cpp)   
TARGET_LINK_LIBRARIES(dump_values_at_parameter ${LINK_LIBRARIES})

ADD_EXECUTABLE(dump_cdf_at_parameter dump_cdf_at_parameter.cpp)   
TARGET_LINK_LIBRARIES(dump_cdf_at_parameter ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_center_dist  add_center_dist.cpp)   
#TARGET_LINK_LIBRARIES(add_center_dist ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_confidence_hierarchy  add_confidence_hierarchy.cpp)   
#TARGET_LINK_LIBRARIES(add_confidence_hierarchy ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_circumference add_circumference.cpp)   
#TARGET_LINK_LIBRARIES(add_circumference ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_area add_area.cpp)   
#TARGET_LINK_LIBRARIES(add_area ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_boundary_metric add_boundary_metric.cpp)   
#TARGET_LINK_LIBRARIES(add_boundary_metric ${LINK_LIBRARIES})

#ADD_EXECUTABLE(add_bbox_volume add_bbox_volume.cpp)   
#TARGET_LINK_LIBRARIES(add_bbox_volume ${LINK_LIBRARIES})

ADD_EXECUTABLE(process_statistics process_statistics.cpp)   
TARGET_LINK_LIBRARIES(process_statistics ${LINK_LIBRARIES})

#ADD_EXECUTABLE(extract_largest_component extract_largest_component.cpp)   
#TARGET_LINK_LIBRARIES(extract_largest_component ${LINK_LIBRARIES})

#ADD_EXECUTABLE(attribute_by_parameter attribute_by_parameter.cpp)   
#TARGET_LINK_LIBRARIES(attribute_by_parameter ${LINK_LIBRARIES})

IF (TALASS_ENABLE_VTK)

    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})   

    ADD_EXECUTABLE(add_derived_metric_vtk add_derived_metric_vtk.cpp)   
    TARGET_LINK_LIBRARIES(add_derived_metric_vtk ${LINK_LIBRARIES} ${VTK_LIBRARIES})

ENDIF()



INSTALL(TARGETS   process_statistics
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
)

#IF (TALASS_ENABLE_OBSERVATIONS)
#    ADD_DEFINITIONS(-DOBSERVATIONS)
#    ADD_SUBDIRECTORY(observations)
#ENDIF ()

