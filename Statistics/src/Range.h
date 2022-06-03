#ifndef RANGE_H
#define RANGE_H

#include "math.h"
#include <iostream>
#include "StatisticsDefinitions.h"

namespace Statistics {

class Range {
  public:

  Range() { Initialize(); }
  Range(const FunctionType low, const FunctionType high, const bool minC=true, const bool maxC=true) { initialized = true; min = low; max = high; minClosed = minC; maxClosed = maxC; }
  ~Range() { }
  Range(const Range &range) { Copy(range); }
  FunctionType Min() const { return min; }
  void Min(const FunctionType f) { min = f; }
  FunctionType Max() const { return max; }
  void Max(const FunctionType f) { max = f; }
  bool MinClosed() const { return minClosed; }
  void MinClosed(const bool val) { minClosed = val; }
  bool MaxClosed() const { return maxClosed; }
  void MaxClosed(const bool val) { maxClosed = val; }
  void Initialize() {
    min = gStatisticsMaxValue;
    max = gStatisticsMinValue; // set like this for updating purposes
    MinClosed(true);
    MaxClosed(true);
    initialized = false;
  }

  void UpdateRange(const FunctionType val) {
    if(val > 0 && fabs(gStatisticsMaxValue-val) < 10) return;
    if(val < 0 && fabs(val - gStatisticsMinValue) < 10) return;
    if(initialized) {
      if(min > val) min = val; 
      if(max < val) max = val; 
    } else {
      max = min = val;
      initialized = true;
    }
  }

  void UpdateRange(const Range &val) {
    if(val.min > 0 && fabs(gStatisticsMaxValue-val.min) < 10) return;
    if(val.min < 0 && fabs(val.min - gStatisticsMinValue) < 10) return;
    if(val.max > 0 && fabs(gStatisticsMaxValue-val.max) < 10) return;
    if(val.max < 0 && fabs(val.max - gStatisticsMinValue) < 10) return;
    if(initialized) {
      if(min > val.min) min = val.min; 
      if(max < val.max) max = val.max; 
    } else {
      max = val.max;
      min = val.min;
      initialized = true;
    }
  }

  bool operator==(const Range &r) const {
    return (min == r.min && max == r.max && minClosed == r.minClosed && maxClosed == r.maxClosed);
  } 
  bool operator!=(const Range &r) const {
    return !this->operator==(r); 
  } 
  void Copy(const Range &range) { min = range.min; max = range.max, minClosed = range.minClosed; maxClosed = range.maxClosed; initialized = range.initialized; };

  Range& operator=(const Range &r) { if(this != &r) Copy(r); return *this; }

  FunctionType Increment(FunctionType r) const { return (max-min)/r; }
  FunctionType Width() const { return (max-min); }
  FunctionType Midpoint() const { return min+0.5*(max-min); }


  bool Contains(const FunctionType val) const ;
  // must completely contain the range
  bool CompletelyContains(const Range & range) const ;
  // any overlap in ranges
  bool Intersects(const Range & range) const ;
  bool Disjoint(const Range & range) const ;

  void Print() const;

  bool Initialized() const { return initialized; }

  public: 
  FunctionType min, max;
  bool minClosed, maxClosed, initialized;
};

};


#endif
