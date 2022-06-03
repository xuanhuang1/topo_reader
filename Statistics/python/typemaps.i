%{
#define SWIG_FILE_WITH_INIT
%}

%include "std_vector.i"
%include "std_string.i"
%include "numpy.i"

%init %{
import_array();
%}

namespace std {
  %template(vectors) vector<string>;
};

%typemap(in,numinputs=0) std::vector<Statistics::TimeStep>* values (std::vector<Statistics::TimeStep> tmp_steps)
{
  $1 = &tmp_steps;
}


%typemap(argout) std::vector<Statistics::TimeStep>* values
{
  //fprintf(stderr,"Found %d time steps\n",$1->size());

  $result = PyList_New($1->size());

  for (int i=0;i<$1->size();i++) {
    PyObject* step = PyList_New(3);
    PyList_SET_ITEM(step,0,PyFloat_FromDouble((*$1)[i].mTime));

    PyObject* data = PyList_New((*$1)[i].mValues.size());
    PyObject* ids = PyList_New((*$1)[i].mValues.size());
    for (int k=0;k<PyList_Size(data);k++) {
      PyList_SET_ITEM(data,k,PyFloat_FromDouble((*$1)[i].mValues[k]));
      PyList_SET_ITEM(ids,k,PyInt_FromLong((*$1)[i].mIds[k]));
    }
    
    PyList_SET_ITEM(step,1,ids);
    PyList_SET_ITEM(step,2,data);
    PyList_SET_ITEM($result, i, step);
  }
}  

