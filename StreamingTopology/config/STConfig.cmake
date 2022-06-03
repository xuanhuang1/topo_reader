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


##############################################################
#########   This file sets the available pre-processor #######
#########   macros to the standard configuration       #######
##############################################################



#
# Under Mac OS use the X-windows system rather
# than the frameworkes
#
TEST_CONFIG(ENABLE_PTHREADS FALSE BOOL "Enable threading")

#
# Disable the HDF5 interface and use a dummy instead
# 
TEST_CONFIG(ENABLE_HDF5 FALSE BOOL "Enable the HDF5 parser")

#
# Use incore arrays to store data instead of out-of-core
#
TEST_CONFIG(INCORE_ARRAYS TRUE BOOL "Use incore arrays rather than out-of-core mmap type arrays")

#
# Enable parallel computation 
#
TEST_CONFIG(ENABLE_PARALLEL TRUE BOOL "Compile the parallel test code")

#
# Enable VisusIO reader 
#
TEST_CONFIG(ENABLE_IDX FALSE BOOL "Enable the ViSUS I/O interface")

