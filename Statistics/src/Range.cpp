#include "Range.h"
namespace Statistics {




bool Range::Contains(const FunctionType val) const  {
   if(MinClosed() && val < min) {
     //std::cout << "min closed && " << val << " < " << min << std::endl;
     return false;
   }
   if(!MinClosed() && val <= min) {
     //std::cout << "min open && " << val << " <= " << min << std::endl;
     return false;
   }
   if(MaxClosed() && val > max) {
     //std::cout << "max closed && " << val << " > " << max << std::endl;
     return false;
   }
   if(!MaxClosed() && val >= max) {
     //std::cout << "max open && " << val << " >= " << max << std::endl;
     return false;
   }
   return true;
}

bool Range::CompletelyContains(const Range & range) const  {
   if(MinClosed() && range.Min() < min) {
     //std::cout << "min closed && " << range.Min() << " < " << min << std::endl;
     return false;
   }
   if(!MinClosed() && range.Min() <= min) {
     //std::cout << "min open && " << range.Min() << " <= " << min << std::endl;
     return false;
   }
   if(MaxClosed() && range.Max() > max) {
     //std::cout << "max closed && " << range.Max() << " > " << max << std::endl;
     return false;
   }
   if(!MaxClosed() && range.Max() >= max) {
     //std::cout << "max open && " << range.Max() << " >= " << max << std::endl;
     return false;
   }
   return true;
}

bool Range::Intersects(const Range & range)  const {
   if(MinClosed() && range.Min() < min) {
     if(range.MaxClosed() && range.Max() < min) return false;
     if(!range.MaxClosed() && range.Max() <= min) return false;
   }
   if(!MinClosed() && range.Max() <= min) return false;

   if(MaxClosed() && range.Max() > max) {
     if(range.MinClosed() && range.Min() > max) return false;
     if(!range.MinClosed() && range.Min() >= max) return false;
   }
   if(!MaxClosed() && range.Min() >= max) return false;

  return true;
}

bool Range::Disjoint(const Range & range) const  {
  return !(CompletelyContains(range));
}

void Range::Print() const {
  if(MinClosed()) std::cout << "[";
  else std::cout << "(";

  std::cout << min << ", " << max;


  if(MaxClosed()) std::cout << "]";
  else std::cout << ")";
}

};
