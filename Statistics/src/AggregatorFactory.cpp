#include <cstring>
#include "AggregatorFactory.h"
#include "PeriodicTransformation.h"

namespace Statistics {

Attribute* Factory::make_aggregator_array(const char* name, bool preAggregated) const
{
  Attribute *arr = NULL;
  switch (type(name)) {
    case ATT_BASE:
      sterror(true, "cannot create a base aggregator array\n");
      return NULL;
    case ATT_VALUE:
      arr = new ValueArray();
      break;
    case ATT_VERTEXCOUNT:
      arr = new VertexCountArray();
      break;
    case ATT_MEAN:
      arr = new MeanArray();
      break;
    case ATT_MAX:
      arr = new MaximumArray();
      break;
    case ATT_MIN:
      arr = new MinimumArray();
      break;
    case ATT_SUM:
      arr = new SumArray();
      break;
    case ATT_VAR:
      arr = new VarianceArray();
      break;
    case ATT_SKEW:
      arr = new SkewnessArray();
      break;
    case ATT_KURT:
      arr = new KurtosisArray();
      break;
    case ATT_COV:
      arr = new CovarianceArray();
      break;
    case ATT_WMEAN:
      arr = new WeightedMeanArray();
      break;
    case ATT_PERMEAN:
      arr = new PeriodicMeanArray();
      break;
  }
  arr->preAggregated(preAggregated);

  /*
  else if (strcmp(name,"weighted")==0) {
    char dummy[10];
    char basename[20];
    sscanf(name,"%s %s",dummy,basename);

    if (strcmp(basename,"mean")==0)
        return new WeightedMean<FunctionType>();
    else
      sterror(true,"Basename \"%s\" not recognized for weight aggregator",basename);

    return NULL;
  }
  */

  if (strncmp(name,"periodic",8)==0) {
    int id;
    float period;
    char dummy[10],basename[20];

    sscanf(name,"%s %s %d %e",dummy,basename,&id,&period);
    PeriodicTransformation transform(period);

    dynamic_cast<PeriodicMeanArray*>(arr)->transformation(transform);
  }

  return arr;
}

Attribute* Factory::make_aggregator_map(const char* name, bool preAggregated) const
{
  Attribute * amap = NULL;
  switch (type(name)) {
    case ATT_BASE:
      sterror(true, "cannot create a base aggregator array\n");
      return NULL;
    case ATT_VALUE:
      amap =  new ValueMap();
      break;
    case ATT_VERTEXCOUNT:
      amap = new VertexCountMap();
      break;
    case ATT_MEAN:
      amap = new MeanMap();
      break;
    case ATT_MAX:
      amap = new MaximumMap();
      break;
    case ATT_MIN:
      amap = new MinimumMap();
      break;
    case ATT_SUM:
      amap = new SumMap();
      break;
    case ATT_VAR:
      amap = new VarianceMap();
      break;
    case ATT_SKEW:
      amap = new SkewnessMap();
      break;
    case ATT_KURT:
      amap = new KurtosisMap();
      break;
    case ATT_COV:
      amap = new CovarianceMap();
      break;
    case ATT_WMEAN:
      amap = new WeightedMeanMap();
      break;
    case ATT_PERMEAN:
      amap = new PeriodicMeanMap();
      break;
  }
  amap->preAggregated(preAggregated);
  return amap;
  return NULL;
}


Attribute* Factory::make_accessor_array(const Attribute* attribute,
                                        const char* base_name, const char* derived_name)
{
  std::cout << "base name = " << base_name << std::endl;
  std::cout << "derived name = " << derived_name << std::endl;
  switch (type(derived_name)) {
    case ATT_BASE:
      sterror(true, "cannot create a base aggregator array\n");
      return NULL;
    case ATT_VERTEXCOUNT: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,VertexCountAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MEAN: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,MeanAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorArray<MeanAggregator,MeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_VAR: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,VarianceAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorArray<MeanAggregator,VarianceAggregator>(attribute);
        case ATT_VAR:
          return new AccessorArray<VarianceAggregator,VarianceAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_SKEW: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,SkewnessAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorArray<MeanAggregator,SkewnessAggregator>(attribute);
        case ATT_VAR:
          return new AccessorArray<VarianceAggregator,SkewnessAggregator>(attribute);
        case ATT_SKEW:
          return new AccessorArray<SkewnessAggregator,SkewnessAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_KURT: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,KurtosisAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorArray<MeanAggregator,KurtosisAggregator>(attribute);
        case ATT_VAR:
          return new AccessorArray<VarianceAggregator,KurtosisAggregator>(attribute);
        case ATT_SKEW:
          return new AccessorArray<SkewnessAggregator,KurtosisAggregator>(attribute);
        case ATT_KURT:
          return new AccessorArray<KurtosisAggregator,KurtosisAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MAX: {
      switch(type(base_name)) {
        case ATT_MAX:
          return new AccessorArray<MaximumAggregator,MaximumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MIN: {
      switch(type(base_name)) {
        case ATT_MIN:
          return new AccessorArray<MinimumAggregator,MinimumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_SUM: {
      switch(type(base_name)) {
        case ATT_SUM:
          return new AccessorArray<SumAggregator,SumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_VALUE: {
      switch(type(base_name)) {
        case ATT_VALUE:
          return new AccessorArray<ValueAggregator,ValueAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_COV: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorArray<VertexCountAggregator,CovarianceAggregator>(attribute);
        case ATT_COV:
          return new AccessorArray<CovarianceAggregator,CovarianceAggregator >(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_WMEAN: {
      switch(type(base_name)) {
        case ATT_WMEAN:
          return new AccessorArray<WeightedMeanAggregator,WeightedMeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_PERMEAN: {
      switch(type(base_name)) {
        case ATT_PERMEAN:
          return new AccessorArray<PeriodicMeanAggregator,PeriodicMeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

  };
  std::cout << "returning null" << std::endl;

  return NULL;
}

Attribute* Factory::make_accessor_map(const Attribute* attribute,
                                      const char* base_name, const char* derived_name)
{
  switch (type(derived_name)) {
    case ATT_BASE:
      sterror(true, "cannot create a base aggregator array\n");
      return NULL;

    case ATT_VERTEXCOUNT: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorMap<VertexCountAggregator,VertexCountAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MEAN: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          std::cout << "about to return an AccessorMap<VertexCount, Mean>(attribute)" << std::endl;;
          return new AccessorMap<VertexCountAggregator,MeanAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorMap<MeanAggregator,MeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_VAR: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorMap<VertexCountAggregator,VarianceAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorMap<MeanAggregator,VarianceAggregator>(attribute);
        case ATT_VAR:
          return new AccessorMap<VarianceAggregator,VarianceAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_SKEW: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorMap<VertexCountAggregator,SkewnessAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorMap<MeanAggregator,SkewnessAggregator>(attribute);
        case ATT_VAR:
          return new AccessorMap<VarianceAggregator,SkewnessAggregator>(attribute);
        case ATT_SKEW:
          return new AccessorMap<SkewnessAggregator,SkewnessAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_KURT: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorMap<VertexCountAggregator,KurtosisAggregator>(attribute);
        case ATT_MEAN:
          return new AccessorMap<MeanAggregator,KurtosisAggregator>(attribute);
        case ATT_VAR:
          return new AccessorMap<VarianceAggregator,KurtosisAggregator>(attribute);
        case ATT_SKEW:
          return new AccessorMap<SkewnessAggregator,KurtosisAggregator>(attribute);
        case ATT_KURT:
          return new AccessorMap<KurtosisAggregator,KurtosisAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MAX: {
      switch(type(base_name)) {
        case ATT_MAX:
          return new AccessorMap<MaximumAggregator,MaximumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_MIN: {
      switch(type(base_name)) {
        case ATT_MIN:
          return new AccessorMap<MinimumAggregator,MinimumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_SUM: {
      switch(type(base_name)) {
        case ATT_SUM:
          return new AccessorMap<SumAggregator,SumAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_VALUE: {
      switch(type(base_name)) {
        case ATT_VALUE:
          return new AccessorMap<ValueAggregator,ValueAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_COV: {
      switch(type(base_name)) {
        case ATT_VERTEXCOUNT:
          return new AccessorMap<VertexCountAggregator,CovarianceAggregator>(attribute);
        case ATT_COV:
          return new AccessorMap<CovarianceAggregator,CovarianceAggregator >(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_WMEAN: {
      switch(type(base_name)) {
        case ATT_WMEAN:
          return new AccessorMap<WeightedMeanAggregator,WeightedMeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

    case ATT_PERMEAN: {
      switch(type(base_name)) {
        case ATT_PERMEAN:
          return new AccessorMap<PeriodicMeanAggregator,PeriodicMeanAggregator>(attribute);
        default:
          sterror(true,"%s is not a baseclass of %s",base_name,derived_name);
      };
    }

  };

  return NULL;
}

AggregatorType Factory::type(const char* name) const
{
  /*
  std::cout << "debug print cur name:" << name << std::endl;
  std::cout << "debug print Value:" << mValueArray.typeName() << std::endl;
  std::cout << "debug print VertexCount:" << mVertexCountArray.typeName() << std::endl;
  std::cout << "debug print mean:" << mMeanArray.typeName() << std::endl;
  std::cout << "debug print maximum:" << mMaximalFunctionValueArray.typeName() << std::endl;
  std::cout << "debug print minimum:" << mMinimalFunctionValueArray.typeName() << std::endl;
  std::cout << "debug print sum:" << mSumArray.typeName() << std::endl;
  std::cout << "debug print variance:" << mFunctionVarianceArray.typeName() << std::endl;
  std::cout << "debug print skewness:" << mFunctionSkewnessArray.typeName() << std::endl;
  std::cout << "debug print kurtosis:" << mFunctionKurtosisArray.typeName() << std::endl;
  std::cout << "debug print covariance:" << mCovarianceArray.typeName() << std::endl;
  std::cout << "debug print weighted mean:" << mWeightedMeanArray.typeName() << std::endl;
  std::cout << "debug print periodicmean:" << mPeriodicMeanArray.typeName() << std::endl;
  */

  if (strcmp(name,mValueArray.typeName())==0) {
    return ATT_VALUE;
  }
  else if (strcmp(name,mVertexCountArray.typeName())==0) {
    return ATT_VERTEXCOUNT;
  }
  else if (strcmp(name, mMeanArray.typeName())==0) {
    return ATT_MEAN;
  }
  else if (strcmp(name,mMaximalFunctionValueArray.typeName())==0) {
    return ATT_MAX;
  }
  else if (strcmp(name,mMinimalFunctionValueArray.typeName())==0) {
    return ATT_MIN;
  }
  else if (strcmp(name,mSumArray.typeName())==0) {
    return ATT_SUM;
  }
  else if (strcmp(name,mFunctionVarianceArray.typeName())==0) {
    return ATT_VAR;
  }
  else if (strcmp(name,mFunctionSkewnessArray.typeName())==0) {
    return ATT_SKEW;
  }
  else if (strcmp(name,mFunctionKurtosisArray.typeName())==0) {
    return ATT_KURT;
  }
  else if (strcmp(name,mCovarianceArray.typeName())==0) {
    return ATT_COV;
  }
  else if (strcmp(name,mWeightedMeanArray.typeName())==0) {
    return ATT_WMEAN;
  }
  else if (strncmp(name, mPeriodicMeanArray.typeName(),13)==0) {
    return ATT_PERMEAN;
  }
  else {
    fprintf(stderr,"Aggregator typename %s is not recognized",name);
    assert (false);
  }

  return ATT_VALUE;
}


}

