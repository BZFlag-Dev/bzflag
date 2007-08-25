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

%include "BZAdvancedRobot.h"
%include "Tank.h"
