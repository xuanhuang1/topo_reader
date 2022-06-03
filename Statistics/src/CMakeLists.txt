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

INCLUDE_DIRECTORIES(
  ${TOPO_PARSER_INCLUDE_DIR}
)

IF (TALASS_ENABLE_VTK)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ENDIF()



SET (STATISTICS_HEADERS
    StatisticsDefinitions.h
    AggregatorFactory.h
    Aggregator.h
    Accessor.h
    Attribute.h
    PeriodicTransformation.h
    Value.h
    VertexCount.h
    Mean.h
    WeightedMean.h
    FunctionVariance.h
    FunctionSkewness.h
    FunctionKurtosis.h
    MinimalFunctionValue.h


    MaximalFunctionValue.h
    IntegratedFunctionValue.h
    Covariance.h
    Covariance_J9.h
    PeriodicMean.h
    #FamilyAggregator.h
    Range.h
    #Engine.h
    #Plot.h
    #PlotData.h
    AttributeVariableList.h
    #FeatureFamilyData.h
    StatisticsDB.h
)

SET (STATISTICS_SRC

    AggregatorFactory.cpp
    Accessor.cpp
    Attribute.cpp
    Value.cpp
    PeriodicTransformation.cpp
    VertexCount.cpp
    Mean.cpp
    WeightedMean.cpp
    FunctionVariance.cpp
    FunctionSkewness.cpp
    FunctionKurtosis.cpp
    MaximalFunctionValue.cpp
    MinimalFunctionValue.cpp
    IntegratedFunctionValue.cpp
    Covariance.cpp
    Covariance_J9.cpp
    PeriodicMean.cpp
    #FamilyAggregator.cpp
    Range.cpp
    #Engine.cpp
    #Plot.cpp
    #PlotData.cpp
    AttributeVariableList.cpp
    #FeatureFamilyData.cpp
    StatisticsDB.cpp
)

ADD_LIBRARY(Statistics STATIC ${STATISTICS_SRC} ${STATISTICS_HEADERS})


INSTALL(FILES ${STATISTICS_HEADERS}
        DESTINATION ${PROJECT_INCLUDE_DIR}/Statistics
)

INSTALL(TARGETS Statistics 
        DESTINATION ${PROJECT_LIBRARY_DIR}
)
 
