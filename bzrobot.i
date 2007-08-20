%module(directors="1") bzrobot
%{
#include "BZAdvancedRobot.h"
%}

%feature("director");

%include "BZAdvancedRobot.h"
