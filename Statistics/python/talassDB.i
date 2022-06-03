%module talassDB

%include "typemaps.i"
%include "stdint.i"

%{
#include "StatisticsDB.h"
%}

// Needed to understand the GlobalIndexType
%include "TalassConfig.h"

// Needed to understand the TopologyFileFormat::TimeIndexType
%include "HandleKeys.h"
%include "StatisticsDB.h"
