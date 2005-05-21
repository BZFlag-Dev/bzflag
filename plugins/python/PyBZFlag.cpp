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

void
TickHandler::process (bz_EventData *eventData)
{
	PyObject *listeners = parent->GetListeners (bz_eTickEvent);

	bz_TickEventData *ted = (bz_TickEventData*) eventData;
	if (listeners == NULL || !PyList_Check (listeners)) {
		// FIXME - throw error
		fprintf (stderr, "tick listeners is not a list!\n");
		return;
	}

	PyObject *arglist = Py_BuildValue("(d)", ted->time);

	// Call out to all of our listeners
	int size = PyList_Size (listeners);
	for (int i = 0; i < size; i++) {
		PyObject *handler = PyList_GetItem (listeners, i);
		if (!PyCallable_Check (handler)) {
			// FIXME - throw error
			fprintf (stderr, "tick listener is not callable\n");
			Py_DECREF (arglist);
			return;
		}
		PyEval_CallObject (handler, arglist);
	}
	Py_DECREF (arglist);
}

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

void
CreateHandlerList (PyObject *dict, int event)
{
	PyDict_SetItem (dict, PyInt_FromLong (event), PyList_New (0));
}

BZFlag::BZFlag ()
{
	module = Py_InitModule3 ("BZFlag", methods, NULL);
	Py_INCREF (Py_None);
	Py_INCREF (Py_False);
	Py_INCREF (Py_True);

	// Create and add submodules
	event_sub = new Event ();
	team_sub  = new Team ();

	PyModule_AddObject (module, "Event", event_sub->GetSubModule ());
	PyModule_AddObject (module, "Team",   team_sub->GetSubModule ());

	// Register event handlers
	tick_handler.parent = this;
	bz_registerGeneralEvent (bz_eTickEvent, &tick_handler);

	// Create the dictionary for all the event handlers. Key is the int
	// event ID, value is a list of callables for our callbacks. Marshalling
	// is handled by the individual event handler classes
	event_listeners = PyDict_New ();
	CreateHandlerList (event_listeners, bz_eTickEvent);

	PyModule_AddObject (module, "Events", event_listeners);
}

PyObject *
BZFlag::GetListeners (int event)
{
	PyObject *o = PyInt_FromLong (event);
	PyObject *ret = PyDict_GetItem (event_listeners, o);
	Py_DECREF (o);

	return ret;
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
	return Py_None;
}

static PyObject *
GetCurrentTime (PyObject *self, PyObject *args)
{
	printf ("GetCurrentTime ()\n");
	return Py_None;
}

};
