/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"
#include "OSFile.h"
#include <Python.h>
#include "PyBZFlag.h"

static char *ReadFile (const char *filename);

static Python::BZFlag *module_bzflag;
static PyObject *global_dict;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
	Py_SetProgramName ("BZFlag");
	Py_Initialize ();

	module_bzflag = new Python::BZFlag ();

	char *buffer = ReadFile (commandLine);

	PyCodeObject *code = (PyCodeObject *) Py_CompileString (buffer, commandLine, Py_file_input);

	global_dict = PyDict_New ();
	PyDict_SetItemString (global_dict, "__builtins__", PyEval_GetBuiltins ());
	PyDict_SetItemString (global_dict, "__name__", PyString_FromString ("__main__"));
	PyEval_EvalCode (code, global_dict, global_dict);
}

BZF_PLUGIN_CALL
int
bz_Unload (void)
{
	PyDict_Clear (global_dict);
	Py_DECREF (global_dict);
	Py_Finalize ();
}

static char *
ReadFile (const char *filename)
{
	OSFile osf;
	osf.open (filename, "r");
	char *buffer = new char[osf.size ()];
	osf.read (buffer, osf.size ());
	osf.close ();
	return buffer;
}
