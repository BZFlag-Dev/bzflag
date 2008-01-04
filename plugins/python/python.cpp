/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <Python.h>
#include <compile.h>
#include <eval.h>

#include "bzfsAPI.h"
#include "PyBZFlag.h"

BZ_GET_PLUGIN_VERSION

static char *ReadFile (const char *filename);

static Python::BZFlag *module_bzflag;
static PyObject *global_dict;
static char *code_buffer;

class PluginHandler : public bz_APIPluginHandler
{
public:
  virtual bool handle (bzApiString plugin, bzApiString param);
};

static void
AppendSysPath (char *directory)
{
  PyObject *mod_sys, *dict, *path, *dir;
  PyErr_Clear ();
  dir = Py_BuildValue ("s", directory);
  mod_sys = PyImport_ImportModule ("sys");
  dict = PyModule_GetDict (mod_sys);
  path = PyDict_GetItemString (dict, "path");

  if (!PyList_Check (path))
    return;
  PyList_Append (path, dir);

  if (PyErr_Occurred ())
    Py_FatalError ("could not build sys.path");
  Py_DECREF (mod_sys);
};

bool
PluginHandler::handle (bzApiString plugin, bzApiString param)
{
  const char *filename = plugin.c_str ();
  char *buffer = ReadFile (filename);
  code_buffer = buffer;

  PyErr_Clear();
  PyCodeObject *code = (PyCodeObject *) Py_CompileString (buffer, plugin.c_str(), Py_file_input);
  if (PyErr_Occurred ()) {
    PyErr_Print ();
    return false;
  }

  // eek! totally unportable - append the script's directory to sys.path,
  // in case there are any local modules
  if (strrchr (filename, '/')) {
    int len = strrchr (filename, '/') - filename;
    char *dir = new char[len + 1];
    strncpy (dir, filename, len);
    dir[len] = '\0';
    AppendSysPath (dir);
    free (dir);
  } else {
    AppendSysPath (".");
  }

  PyErr_Clear ();
  PyEval_EvalCode (code, global_dict, global_dict);
  if (PyErr_Occurred ()) {
    PyErr_Print ();
    return false;
  }

  return true;
};

static PluginHandler *py_handler;

BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
  // I would use assert here, but "Assertion `3 == 2' failed" is really not a useful error at all
  if (BZ_API_VERSION != 5) {
    fprintf (stderr, "Python plugin currently wraps the version 5 API, but BZFS is exporting version %d. Please complain loudly\n", BZ_API_VERSION);
    abort ();
  }

  Py_SetProgramName ("BZFlag");
  Py_Initialize ();
  PyEval_InitThreads ();

  module_bzflag = new Python::BZFlag ();

  py_handler = new PluginHandler ();
  if (!bz_registerCustomPluginHandler ("py", py_handler))
    fprintf (stderr, "couldn't register custom plugin handler\n");

  // set up the global dict
  // FIXME - should this be per-script?
  global_dict = PyDict_New ();
  PyDict_SetItemString (global_dict, "__builtins__", PyEval_GetBuiltins ());
  PyDict_SetItemString (global_dict, "__name__", PyString_FromString ("__main__"));

  return 0;
}

BZF_PLUGIN_CALL
int
bz_Unload (void)
{
  PyDict_Clear (global_dict);
  Py_DECREF (global_dict);
  Py_Finalize ();

  delete [] code_buffer;

  return 0;
}

static char *
ReadFile (const char *filename)
{
  FILE *f = fopen (filename, "r");

  unsigned int pos = ftell (f);
  fseek (f, 0, SEEK_END);
  unsigned int len = ftell (f);
  fseek (f, pos, SEEK_SET);

  char *buffer = new char[len + 1];
  fread (buffer, 1, len, f);

  buffer[len] = '\0';

  fclose (f);
  return buffer;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
