#######################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov, jcbenne@sandia.gov
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





#
# Try to find the VISUS libraries
# Once done this will define
#
# VISUS_FOUND          - system has ViSUS
# VISUS_INCLUDE_DIR    - path to Visus include directory
# VISUS_LIBRARIES      - all VISUS libraries
# GLEW_INCLUDE_DIR     - path to glew included with visus
# GLEW_LIBRARIES       - path to glew included with visus
#
#

MESSAGE("visus path: " ${VISUS_PATH})

FIND_PATH(VISUS_INCLUDE_DIR  visuscpp libs
                             ${VISUS_PATH}/include
)

FIND_PATH(GLEW_INCLUDE_DIR   glew.h 
                             ${VISUS_INCLUDE_DIR}/libs/glew/GL
)

###########################################
# Need to specify whether nvisusio was compiled against Qt or Juce.
###########################################

SET(VISUS_GUI_LIBRARY "juce" CACHE STRING "one of juce, qt")
SET_PROPERTY(CACHE VISUS_GUI_LIBRARY PROPERTY STRINGS juce qt)
IF (${VISUS_GUI_LIBRARY} STREQUAL "qt")
  SET(VISUS_QT TRUE) 
ELSEIF (${VISUS_GUI_LIBRARY} STREQUAL "juce") 
  SET(VISUS_JUCE TRUE)
ENDIF()


FIND_LIBRARY(VISUS_KERNEL_LIB    visuskernel            ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_DB_LIB        visusdb                ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_IDX_LIB       visusidx               ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_DATAFLOW_LIB  visusdataflow          ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_APPKIT_LIB    visusappkit            ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_GUI_LIB       visusgui               ${VISUS_PATH}/lib)
IF (VISUS_JUCE)
 FIND_LIBRARY(VISUS_GUI_IMPL_LIB Juce                   ${VISUS_PATH}/lib)
ENDIF()
FIND_LIBRARY(VISUS_CURL_LIB      curl                   ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_FREEIMAGE_LIB FreeImage              ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_XML_LIB       tinyxml                ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_LIBZ_LIB      libz                   ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_GLEW_LIB      glew                   ${VISUS_PATH}/lib)
FIND_LIBRARY(VISUS_SSL_LIB       ssl)
FIND_LIBRARY(VISUS_CRYPTO_LIB    crypto)

SET(VISUS_LIBRARIES 
    ${VISUS_APPKIT_LIB}
    ${VISUS_IDX_LIB}
    ${VISUS_GUI_LIB}
    ${VISUS_DATAFLOW_LIB}
    ${VISUS_DB_LIB}
    ${VISUS_KERNEL_LIB}
    ${VISUS_GUI_IMPL_LIB}
    ${VISUS_CURL_LIB}
    ${VISUS_FREEIMAGE_LIB}
    ${VISUS_XML_LIB}
    ${VISUS_LIBZ_LIB}
    ${VISUS_GLEW_LIB}
    ${VISUS_SSL_LIB}
    ${VISUS_CRYPTO_LIB}
)

#Set these so other Find*.cmake don't end up finding them elsewhere.
SET(GLEW_LIBRARIES   ${VISUS_GLEW_LIB})

# On apple we need a bunch of other frameworks
IF (APPLE) 
   FIND_LIBRARY(COREMIDI_FRAMEWORK CoreMidi)
   FIND_LIBRARY(COREAUDIO_FRAMEWORK CoreAudio)
   FIND_LIBRARY(COREFOUNDATION_FRAMEWORK CoreFoundation)
   FIND_LIBRARY(QUARTZCORE_FRAMEWORK QuartzCore)
   FIND_LIBRARY(COCOA_FRAMEWORK Cocoa)
   FIND_LIBRARY(IOKIT_FRAMEWORK IoKit)
   IF (VISUS_QT)
      FIND_LIBRARY(QT_OPENGL_FRAMEWORK QtOpenGL)
      FIND_LIBRARY(QT_GUI_FRAMEWORK QtGui)
      FIND_LIBRARY(QT_CORE_FRAMEWORK QtCore)
   ENDIF()

   SET (VISUS_LIBRARIES 
       ${VISUS_LIBRARIES}
       ${COREMIDI_FRAMEWORK}
       ${COREAUDIO_FRAMEWORK}
       ${COREFOUNDATION_FRAMEWORK}
       ${QUARTZCORE_FRAMEWORK}
       ${COCOA_FRAMEWORK}
       ${IOKIT_FRAMEWORK}
       ${QT_OPENGL_FRAMEWORK}
       ${QT_GUI_FRAMEWORK}
       ${QT_CORE_FRAMEWORK}
   )
ENDIF() 



###########################################
# windows
###########################################

IF (WIN32)
  SET(VISUS_WINDOWS 1)
  ADD_DEFINITIONS(-DVISUS_WINDOWS=1 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN=1)
  SET(CMAKE_CXX_FLAGS "/Oi ${CMAKE_CXX_FLAGS}") 
  
    SET (VISUS_LIBRARIES 
      ${VISUS_LIBRARIES}
      Vfw32.lib
      Version.lib
      Imm32.lib
      Winmm.lib;
      shlwapi.lib
      Wininet.lib
  )

###########################################
# osx (desktop)
###########################################

ELSEIF (APPLE)
  SET(VISUS_APPLE 1)
  SET(VISUS_OSX 1)
  ADD_DEFINITIONS(-DVISUS_APPLE=1 -DVISUS_OSX=1)  
  IF("${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "10.5")
    SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk" CACHE STRING "SDKROOT")
  ELSE()
    SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk" CACHE STRING "SDKROOT")	
  ENDIF() 

###########################################
# linux
###########################################

ELSEIF (UNIX)
  SET(VISUS_LINUX 1)
  ADD_DEFINITIONS(-DVISUS_LINUX=1)

	IF (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		ADD_DEFINITIONS(-D_DEBUG=1)
	ENDIF()

  SET (VISUS_LIBRARIES 
    ${VISUS_LIBRARIES}
    freetype
    ssl
	rt
	pthread
	dl
  )

#  SET(CMAKE_C_FLAGS         ${CMAKE_C_FLAGS}   "-fPIC")
#  SET(CMAKE_CXX_FLAGS       ${CMAKE_CXX_FLAGS} "-fPIC")
ENDIF()


IF (VISUS_INCLUDE_DIR)

   SET(VISUS_FOUND "YES")
   IF (CMAKE_VERBOSE_MAKEFILE)
      MESSAGE("Using VISUS_INCLUDE_DIR  = " ${VISUS_INCLUDE_DIR}) 
      MESSAGE("Using GLEW_INCLUDE_DIR   = " ${GLEW_INCLUDE_DIR}) 
      MESSAGE("Found VISUS_LIBRARIES    = " ${VISUS_LIBRARIES}) 
      MESSAGE("Found GLEW_LIBRARIES     = " ${GLEW_LIBRARIES}) 
   ENDIF (CMAKE_VERBOSE_MAKEFILE)

ELSE ()
   
   MESSAGE("ERROR Visus library not found on the system")
 
ENDIF ()
                         
