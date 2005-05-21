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
#include "PyBZFlag.h"

namespace Python
{

static PyObject *SendTextMessage (PyObject *self, PyObject *args, PyObject *keywords);
static PyObject *FireWorldWeapon (PyObject *self, PyObject *args);
static PyObject *GetCurrentTime  (PyObject *self, PyObject *args);

static struct PyMethodDef methods[] =
{
	// FIXME - docstrings
	{"SendTextMessage", (PyCFunction) SendTextMessage, METH_VARARGS | METH_KEYWORDS, NULL},
	{"FireWorldWeapon", (PyCFunction) FireWorldWeapon, METH_VARARGS,                 NULL},
	{"GetCurrentTime",  (PyCFunction) GetCurrentTime,  METH_NOARGS,                  NULL},
	{NULL,              (PyCFunction) NULL,            0,                            NULL},
};

BZFlag *BZFlag::instance = NULL;
int BZFlag::References = 0;

BZFlag *
BZFlag::GetInstance ()
{
	if (instance == NULL)
		instance = new BZFlag ();
	References++;
	return instance;
}

void
BZFlag::DeRef ()
{
	References--;
}

BZFlag::BZFlag ()
{
	module = Py_InitModule3 ("BZFlag", methods, NULL);
	Py_INCREF (Py_None);
	Py_INCREF (Py_False);
	Py_INCREF (Py_True);

	event_sub = new Event ();

	PyModule_AddObject (module, "Event", event_sub->GetSubModule ());
}

static PyObject *
SendTextMessage (PyObject *self, PyObject *args, PyObject *keywords)
{
	int to, from;
	char *message;

	static char *kwlist[] = {"to", "from", "message", NULL};

	if (!PyArg_ParseTupleAndKeywords (args, keywords, "iis", kwlist, &to, &from, &message)) {
		fprintf (stderr, "couldn't parse args\n");
		// FIXME - throw error
		return NULL;
	}

	printf ("SendTextMessage (%d, %d, %s)\n", to, from, message);
	bool result = bz_sendTextMessage (to, from, message);
	return Py_None;
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
