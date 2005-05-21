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

#include <Python.h>
#include "PyBZFlag.h"

namespace Python
{

static PyObject *SendTextMessage (PyObject *self, PyObject *args);
static PyObject *FireWorldWeapon (PyObject *self, PyObject *args);
static PyObject *GetCurrentTime  (PyObject *self, PyObject *args);

static struct PyMethodDef methods[] =
{
	// FIXME - docstrings
	{"SendTextMessage", SendTextMessage, METH_VARARGS, NULL},
	{"FireWorldWeapon", FireWorldWeapon, METH_VARARGS, NULL},
	{"GetCurrentTime",  GetCurrentTime,  METH_NOARGS,  NULL},
	{NULL,              NULL,            0,            NULL},
};

BZFlag::BZFlag ()
{
	module = Py_InitModule3 ("BZFlag", methods, NULL);

}

static PyObject *
SendTextMessage (PyObject *self, PyObject *args)
{
	printf ("SendTextMessage ()\n");
}

static PyObject *
FireWorldWeapon (PyObject *self, PyObject *args)
{
	printf ("FireWorldWeapon ()\n");
}

static PyObject *
GetCurrentTime (PyObject *self, PyObject *args)
{
	printf ("GetCurrentTime ()\n");
}

};
