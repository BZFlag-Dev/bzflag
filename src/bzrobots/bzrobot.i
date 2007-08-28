%module(directors="1") bzrobot
%{
#include "BZAdvancedRobot.h"
#include "Tank.h"
%}

%feature("director");

%include "std_vector.i"
%include "std_string.i"
namespace std {
       %template (VecTank) vector< Tank >;
}

%typemap(out) float [ANY] {
    int i;
    $result = PyList_New($1_dim0);
    for (i = 0; i < $1_dim0; i++) {
        PyObject *o = PyFloat_FromDouble((double) $1[i]);
        PyList_SetItem($result,i,o);
    }
}

%typemap(memberin) float [ANY] {
    int i;
    for (i = 0; i < $1_dim0; i++) {
        $1[i] = $input[i];
    }
}

%include "BZAdvancedRobot.h"
%include "Tank.h"
