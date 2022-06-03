# - Config file for the STREAMSTAT package
# It defines the following variables
#  STREAMSTAT_INCLUDE_DIRS - include directories for DataFlow
#  STREAMSTAT_LIBRARIES    - libraries to link against
 
if(NOT TOPOPARSER_FOUND)

  # Compute paths
  get_filename_component(TOPOPARSER_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
 
  # include the main exported targets
  include("${TOPOPARSER_CMAKE_DIR}/TopoFileParser.cmake")

  set(TOPOPARSER_FOUND TRUE)

endif()
