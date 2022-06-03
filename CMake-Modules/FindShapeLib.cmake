#
# Try to find the ShapeLib libraries
# Once done this will define
#
# SHAPELIB_FOUND          - system has ShapeLib
# SHAPELIB_INCLUDE_DIR    - path to include/FeatureFamilyParser.h
# SHAPELIB_LIBRARIES      - path to ShapeLib library
#
#

FIND_PATH(SHAPELIB_INCLUDE_DIR ShapeLib/TopoFileDefinitions.h
      ${PROJECT_INSTALL_PREFIX}/include
      ${STATISTICS_SOURCE_DIR}/../ShapeLib/build/include
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
)

FIND_LIBRARY(SHAPELIB_LIBRARIES ShapeLib
       ${PROJECT_INSTALL_PREFIX}/lib
       ${STATISTICS_SOURCE_DIR}/../ShapeLib/build/lib
       /usr/lib
       /sw/lib
)


IF (SHAPELIB_INCLUDE_DIR AND SHAPELIB_LIBRARIES)

   SET(SHAPELIB_FOUND "YES")
   IF (CMAKE_VERBOSE_MAKEFILE)
      MESSAGE("Using SHAPELIB_INCLUDE_DIR = " ${SHAPELIB_INCLUDE_DIR}) 
      MESSAGE("Using SHAPELIB_LIBRARIES   = " ${SHAPELIB_LIBRARIES}) 
   ENDIF (CMAKE_VERBOSE_MAKEFILE)

ELSE (SHAPELIB_INCLUDE_DIR AND SHAPELIB_LIBRARIES)
   
   MESSAGE("ERROR ShapeLib library not found on the system")
 
ENDIF (SHAPELIB_INCLUDE_DIR AND SHAPELIB_LIBRARIES)
                         
