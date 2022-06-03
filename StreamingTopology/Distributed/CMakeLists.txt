SET(DISTRIBUTED_BINARY_DIR ${PROJECT_BINARY_DIR} CACHE STRING "Distributed build directory")
SET(DISTRIBUTED_SOURCE_DIR ${PROJECT_SOURCE_DIR} CACHE STRING "Distributed source directory")
SET(DISTRIBUTED_INCLUDE ${DISTRIBUTED_BINARY_DIR}/include CACHE STRING "Distributed include directory")
SET(DISTRIBUTED_LIBRARIES ${DISTRIBUTED_BINARY_DIR}/lib CACHE STRING "Distributed lib directory")
SET(DISTRIBUTED_SRC ${DISTRIBUTED_SOURCE_DIR}/src CACHE STRING "Distributed build src directory")


IF (NOT PROJECT_INSTALL_PREFIX)
    SET(PROJECT_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
ENDIF (NOT PROJECT_INSTALL_PREFIX)


INCLUDE(${CMAKE_SOURCE_DIR}/../config/FindFlexArray.cmake)
IF (NOT FLEXARRAY_FOUND)
   MESSAGE("Build error: FlexArray libraries not found. Try setting -DADDITIONAL_INCLUDE_PATH")
ENDIF (NOT FLEXARRAY_FOUND)


SET (DIST_HEADERS
  FiFoBuffer.h
  TopoInputStream.h
  SerialInputStream.h
  TopoOutputStream.h
  SerialOutputStream.h
  TopoCommunicator.h
  SerialCommunicator.h
  Token.h
  Algorithm.h
  PatchAlgorithm.h
  LocalComputeAlgorithm.h
  ScatterAlgorithm.h
  GatherAlgorithm.h
  ControlFlow.h
  ModuloFlow.h
  ModuloScatter.h
  ModuloGather.h
  TopoControl.h
  SerialControl.h
  PointIndex.h
  DistributedDefinitions.h
  MergeTree.h
  StreamingGatherAlgorithm.h
  Box.h
  
)

SET( DIST_SOURCES
  FiFoBuffer.cpp
  SerialInputStream.cpp
  TopoOutputStream.cpp
  SerialOutputStream.cpp
  SerialCommunicator.cpp
  Token.cpp
  Algorithm.cpp
  PatchAlgorithm.cpp
  LocalComputeAlgorithm.cpp
  ScatterAlgorithm.cpp
  GatherAlgorithm.cpp
  ModuloFlow.cpp
  ModuloScatter.cpp
  ModuloGather.cpp
  SerialControl.cpp
  PointIndex.cpp
  MergeTree.cpp
  StreamingGatherAlgorithm.cpp
 )
 
INCLUDE_DIRECTORIES (
   ${FLEXARRAY_INCLUDE_DIR}
)

 
ADD_LIBRARY(parallelMT STATIC ${DIST_HEADERS} ${DIST_SOURCES})


INSTALL(TARGETS  parallelMT 
        ARCHIVE DESTINATION ${PROJECT_INSTALL_PREFIX}/lib
        LIBRARY DESTINATION ${PROJECT_INSTALL_PREFIX}/lib
        RUNTIME DESTINATION ${PROJECT_INSTALL_PREFIX}/bin/gui
)

 SUBDIRS(utilities)
