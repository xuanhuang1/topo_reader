########################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov,pascucci@sci.utah.edu.  
# LLNL-CODE-406031.  
# All rights reserved.  
#   
# This file is part of "Simple and Flexible Scene Graph Version 2.0."
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
########################################################################


# 
# Try to find libawesomium include and libraries
#

IF (WIN32)
	FIND_PATH( AWESOMIUM_INCLUDE_DIR Awesomium/WebCore.h
		${AWE_DIR}/include/
		DOC "The directory where Awesomium/WebCore.h resides")
	FIND_LIBRARY( AWESOMIUM_LIB
		NAMES AWESOMIUM awesomium
		PATHS
		${AWE_DIR}/build/lib
		DOC "The AWESOMIUM library")
ELSE (WIN32)
	FIND_PATH( AWESOMIUM_INCLUDE_DIR Awesomium/WebCore.h
		/usr/include/GL
		/usr/local/include/GL
		/sw/include/GL
		/opt/local/include/GL
		DOC "The directory where Awesomium/WebCore.h resides")
	FIND_LIBRARY( AWESOMIUM_LIB
		NAMES AWESOMIUM awesomium
		PATHS
        	/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The AWESOMIUM library")
ENDIF (WIN32)

IF (AWESOMIUM_LIB AND AWESOMIUM_INCLUDE_DIR)
   SET(AWESOMIUM_FOUND TRUE)
ELSE (AWESOMIUM_LIB AND AWESOMIUM_INCLUDE_DIR)
   SET(AWESOMIUM_FOUND FALSE)
ENDIF (AWESOMIUM_LIB AND AWESOMIUM_INCLUDE_DIR)
   


