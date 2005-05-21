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
#include <Python.h>
#include "PyBZFlag.h"

static char *ReadFile (const char *filename);

static Python::BZFlag *module_bzflag;
static PyObject *global_dict;
static char *code_buffer;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
	Py_Initialize ();
	Py_SetProgramName ("BZFlag");

	module_bzflag = new Python::BZFlag ();

	char *buffer = ReadFile (commandLine);
	code_buffer = buffer;

	PyCodeObject *code = (PyCodeObject *) Py_CompileString (buffer, commandLine, Py_file_input);
	if (PyErr_Occurred ()) {
		fprintf (stderr, "error compiling!\n");
		PyErr_Print ();
		return 1;
	}

	global_dict = PyDict_New ();
	PyDict_SetItemString (global_dict, "__builtins__", PyEval_GetBuiltins ());
	PyDict_SetItemString (global_dict, "__name__", PyString_FromString ("__main__"));
	PyEval_EvalCode (code, global_dict, global_dict);
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
