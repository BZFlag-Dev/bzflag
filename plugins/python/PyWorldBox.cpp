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

#include "PyWorldBox.h"

namespace Python
{
namespace World
{

static void      Box_dealloc (Box *box);
static PyObject *Box_getAttr (Box *box, char *name);
static int       Box_setAttr (Box *box, char *name, PyObject *v);
static int       Box_compare (Box *a1, Box *a2);
static PyObject *Box_repr    (Box *box);

static PyMethodDef Box_methods[] = {
	{NULL, NULL, 0, NULL},
};

PyTypeObject Box_Type = {
	PyObject_HEAD_INIT(NULL)
	0,				// ob_size
	"BZFlag World Box",		// tp_name
	sizeof (Box),			// tp_basicsize
	0,				// tp_itemsize
	(destructor) Box_dealloc,	// tp_dealloc
	0,				// tp_print
	(getattrfunc) Box_getAttr,	// tp_getattr
	(setattrfunc) Box_setAttr,	// tp_setattr
	(cmpfunc) Box_compare,		// tp_compare
	(reprfunc) Box_repr,		// tp_repr
	0,				// tp_as_number
	0,				// tp_as_sequence
	0,				// tp_as_mapping
	0,				// tp_as_hash
	0,				// tp_call
	0,				// tp_str
	0,				// tp_getattro
	0,				// tp_setattro
	0,				// tp_as_buffer
	0,				// tp_flags
	0,				// tp_doc
	0,				// tp_traverse
	0,				// tp_clear
	0,				// tp_richcompare
	0,				// tp_weaklistoffset
	0,				// tp_iter
	0,				// tp_iternext
	Box_methods,			// tp_methods
	0,				// tp_members
};

PyObject *
CreateBox ()
{
	Box *b = (Box *) PyObject_NEW (Box, &Box_Type);
	return (PyObject *) b;
}

static void
Box_dealloc (Box *box)
{
	PyObject_DEL (box);
}

static PyObject *
Box_getAttr (Box *box, char *name)
{
	return NULL;
}

static int
Box_setAttr (Box *box, char *name, PyObject *v)
{
}

static int
Box_compare (Box *a1, Box *a2)
{
}

static PyObject *
Box_repr (Box *box)
{
	return NULL;
}

};
};
