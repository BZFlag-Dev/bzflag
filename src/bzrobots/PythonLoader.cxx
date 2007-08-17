/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PythonLoader.h"
#include "bzrobot_python_runtime.h"

PythonLoader::PythonLoader() :module(NULL), ctor(NULL), robot(NULL), initialized(false)
{
  Py_SetProgramName("bzrobots");
  Py_Initialize();
}
PythonLoader::~PythonLoader()
{
  /* Is this neccessary when we're calling Py_Finalize()? Can't hurt. :-) */
  Py_XDECREF(module);
  Py_XDECREF(ctor);
  Py_XDECREF(robot);

  Py_Finalize();
}

bool PythonLoader::initialize()
{
  /* We basically do: import sys; sys.path.append(".");
   * This is so it'll look in . for scripts to load. */
  PyObject *sys, *path;
  PyObject *sysString, *dotString;

  sysString = PyString_FromString("sys");
  sys = PyImport_Import(sysString);
  Py_XDECREF(sysString);

  if (!sys)
  {
    error = "Could not import 'sys'.";
    return false;
  }

  path = PyObject_GetAttrString(sys, "path");
  if (!path)
  {
    Py_XDECREF(sys); 
    error = "Could not get 'sys.path'.";
    return false;
  }

  dotString = PyString_FromString(".");
  PyList_Append(path, dotString);

  Py_XDECREF(path); Py_XDECREF(sys); 

  /* TODO:
   * I assume we should make it load (using PyImport_Import) the various
   * SWIGified APIs, so that they'll be "easily" accessible from the 
   * loaded modules, whopee. :-) */

  initialized = true;
  return true;
}

bool PythonLoader::load(std::string filename)
{
  if (!initialized)
  {
    if (!initialize())
      return false;
  }

  std::string::size_type extension_pos = filename.find_last_of(".");
  if (extension_pos == std::string::npos || extension_pos >= filename.length())
  {
    error = "Could not find a valid extension in filename '" + filename + "'.";
    return false;
  }
  std::string modulename = filename.substr(0, extension_pos);

  PyObject *file = PyString_FromString(modulename.c_str());

  Py_XDECREF(module);
  module = PyImport_Import(file);

  if (!module)
  {
    /* TODO: Do the same as PyErr_Print(), just into a string? 
     * See PyErr_Fetch()
     * (prints a traceback of the error-stack when Something Bad (tm)
     * happens. */
    error = "Could not load module from '" + modulename + "'";
    return false;
  }

  Py_XDECREF(ctor);
  ctor = PyObject_GetAttrString(module, "create");

  if (!ctor || !PyCallable_Check(ctor))
  {
    Py_XDECREF(ctor);
    ctor = NULL;
    error = "Can't find function 'create' in module from '" + filename + "'";
    return false;
  }

  Py_XDECREF(file);
  return true;
}

BZAdvancedRobot *PythonLoader::create(void)
{
  Py_XDECREF(robot);

  robot = PyObject_CallObject(ctor, NULL);
  if (!robot)
  {
    error = "Could not call constructor.";
    return NULL;
  }

  PySwigObject *holder = SWIG_Python_GetSwigThis(robot);
  return static_cast<BZAdvancedRobot *>(holder ? holder->ptr : 0);
}
void PythonLoader::destroy(BZAdvancedRobot * /*instance*/)
{
  Py_XDECREF(robot);
  robot = NULL;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
