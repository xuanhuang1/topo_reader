#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <vector>

#include "Covariance_J9.h"
#include "Covariance.h"
#include "FunctionVariance.h"

using namespace std;
using namespace Statistics;


int main(int argc, const char** argv)
{
  CovarianceAggregator cov;
  Covariance_J9Base covJ9;
  VarianceAggregator var;
  for(int i=0; i < 1000; i++) {
    cov.addVertex(float(i), float(i), i);
    covJ9.addVertex(float(i), float(i), i);
    var.addVertex(float(i), i);
    cout << cov.value() << " " << covJ9.value() << " " << var.value() << endl;
  }

}
