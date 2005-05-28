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
static int       BZDB_setAttr       (BZDB *bzdb, char *name, PyObject *v);
static PyObject *BZDB_repr          (BZDB *bzdb);
static int       BZDB_length        (BZDB *bzdb);
static PyObject *BZDB_subscript     (BZDB *bzdb, PyObject *key);
static int       BZDB_ass_subscript (BZDB *bzdb, PyObject *key, PyObject *value);

static PyMethodDef BZDB_methods[] = {
	{NULL, NULL, 0, NULL},
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
	0,				// tp_getattr
	0,				// tp_setattr
	0,				// tp_compare
	(reprfunc) BZDB_repr,		// tp_repr
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

static void
BZDB_dealloc (BZDB *bzdb)
{
}

static PyObject *
BZDB_getAttr (BZDB *bzdb, char *name)
{
}

static int
BZDB_setAttr (BZDB *bzdb, char *name, PyObject *v)
{
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
}

static int
BZDB_ass_subscript (BZDB *bzdb, PyObject *key, PyObject *value)
{
}

}
