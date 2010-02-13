/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __PYTHONSCRIPT_H__
#define __PYTHONSCRIPT_H__

#include "common.h"

/* system interface headers */
#include <string>

/* ugly hack for pyconfig.h */
#ifndef HAVE_NCURSES_H
  #include <Python.h>
#else
  #undef HAVE_NCURSES_H
  #include <Python.h>
  #ifndef HAVE_NCURSES_H
    #define HAVE_NCURSES_H
  #endif
#endif


/* local interface headers */
#include "RobotScript.h"
#include "Robot.h"

#ifdef SWIG_VERSION_BCD
# if SWIG_VERSION_BCD < 0x010337
#  define SwigPyObject PySwigObject
# endif
#endif

class PythonLoader : public RobotScript {
  PyObject *module, *ctor;
  PyObject *pyrobot;

  bool initialized;
  bool initialize();
  bool addSysPath(std::string path);

  public:
    PythonLoader();
    ~PythonLoader();
    bool load(std::string filename);
    BZRobots::Robot *create(void);
    void destroy(BZRobots::Robot *instance);
};

#endif /* __PYTHONSCRIPT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
