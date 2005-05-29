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

#include "PyBZDB.h"

namespace Python
{

static void      BZDB_dealloc       (BZDB *bzdb);
static PyObject *BZDB_getAttr       (BZDB *bzdb, char *name);
static PyObject *BZDB_repr          (BZDB *bzdb);
static int       BZDB_length        (BZDB *bzdb);
static PyObject *BZDB_subscript     (BZDB *bzdb, PyObject *key);
static int       BZDB_ass_subscript (BZDB *bzdb, PyObject *key, PyObject *value);
static PyObject *BZDB_bool          (BZDB *self, PyObject *args);
static PyObject *BZDB_double        (BZDB *self, PyObject *args);
static PyObject *BZDB_int           (BZDB *self, PyObject *args);

static PyMethodDef BZDB_methods[] = {
	{"GetBool",   (PyCFunction) BZDB_bool,   METH_VARARGS, NULL},
	{"GetDouble", (PyCFunction) BZDB_double, METH_VARARGS, NULL},
	{"GetInt",    (PyCFunction) BZDB_int,    METH_VARARGS, NULL},
	{NULL,        NULL,                      0,            NULL},
};

PyMappingMethods BZDB_mapping = {
	(inquiry) BZDB_length,			// mp_length
	(binaryfunc) BZDB_subscript,		// mp_subscript
	(objobjargproc) BZDB_ass_subscript,	// mp_ass_subscript
};

PyTypeObject BZDB_Type = {
	PyObject_HEAD_INIT(NULL)
	0,				// ob_size
	"BZDB",				// tp_name
	sizeof (BZDB),			// tp_basicsize
	0,				// tp_itemsize
	(destructor) BZDB_dealloc,	// tp_dealloc
	0,				// tp_print
	(getattrfunc) BZDB_getAttr,	// tp_getattr
	0,				// tp_setattr
	0,				// tp_compare
	0,				// tp_repr - FIXME?
	0,				// tp_as_number
	0,				// tp_as_sequence
	&BZDB_mapping,			// tp_as_mapping
	0,				// tp_as_hash
	0,				// tp_call
	0,				// tp_str
	0,				// tp_getattro
	0,				// tp_setattro
	0,				// tp_flags
	0,				// tp_doc
};

PyObject *
CreateBZDB ()
{
	BZDB *bzdb = (BZDB *) PyObject_NEW (BZDB, &BZDB_Type);
	return ((PyObject *) bzdb);
}

static void
BZDB_dealloc (BZDB *bzdb)
{
	PyObject_DEL (bzdb);
}

static PyObject *
BZDB_getAttr (BZDB *bzdb, char *name)
{
	return Py_FindMethod (BZDB_methods, (PyObject *) bzdb, name);
}

static PyObject *
BZDB_repr (BZDB *bzdb)
{
}

static int
BZDB_length (BZDB *bzdb)
{
}

static PyObject *
BZDB_subscript (BZDB *bzdb, PyObject *key)
{
	if (!PyString_Check (key)) {
		// FIXME - throw KeyError
		return NULL;
	}
	char *k = PyString_AsString (key);
	return PyString_FromString (bz_getBZDBString (k).c_str ());
}

static int
BZDB_ass_subscript (BZDB *bzdb, PyObject *key, PyObject *value)
{
	if (!PyString_Check (key)) {
		// FIXME - throw error
		return 0;
	}
	if (!PyString_Check (value)) {
		// FIXME - throw error
		return 0;
	}
	// FIXME - need API for this
	return 1;
}

static PyObject *
BZDB_bool (BZDB *self, PyObject *args)
{
	char *key;
	if (!PyArg_ParseTuple (args, "s", &key)) {
		// FIXME - throw KeyError
		return NULL;
	}
	return (bz_getBZDBBool (key) ? Py_True : Py_False);
}

static PyObject *
BZDB_double (BZDB *self, PyObject *args)
{
	char *key;
	if (!PyArg_ParseTuple (args, "s", &key)) {
		// FIXME - throw KeyError
		return NULL;
	}
	return PyFloat_FromDouble (bz_getBZDBDouble (key));
}

static PyObject *
BZDB_int (BZDB *self, PyObject *args)
{
	char *key;
	if (!PyArg_ParseTuple (args, "s", &key)) {
		// FIXME - throw KeyError
		return NULL;
	}
	return PyInt_FromLong ((long) bz_getBZDBInt (key));
}

}
