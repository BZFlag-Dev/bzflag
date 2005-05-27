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
#include "PyPlayer.h"

namespace Python
{

void
CaptureHandler::process (bz_EventData *eventData)
{
	PyObject *listeners = parent->GetListeners (bz_eCaptureEvent);

	bz_CTFCaptureEventData *ced = (bz_CTFCaptureEventData *) eventData;
	if (listeners == NULL || !PyList_Check(listeners)) {
		// FIXME - throw error
		fprintf (stderr, "tick listeners is not a list!\n");
		return;
	}

	PyObject *arglist = Py_BuildValue ("(iii(fff)fd",
			ced->teamCaped,
			ced->teamCaping,
			ced->playerCaping,
			ced->pos[0],
			ced->pos[1],
			ced->pos[2],
			ced->rot,
			ced->time);

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
		PyErr_Clear ();
		PyEval_CallObject (handler, arglist);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
			return;
		}
	}
	Py_DECREF (arglist);
}

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

	PyObject *arglist = Py_BuildValue ("(d)", ted->time);

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
		PyErr_Clear ();
		PyEval_CallObject (handler, arglist);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
			return;
		}
	}
	Py_DECREF (arglist);
}

void
JoinHandler::process (bz_EventData *eventData)
{
	bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
	parent->AddPlayer (pjped->playerID);

	PyObject *listeners = parent->GetListeners (bz_ePlayerJoinEvent);
	if (listeners == NULL || !PyList_Check (listeners)) {
		// FIXME - throw error
		fprintf (stderr, "join listeners is not a list!\n");
		return;
	}

	// Call out to all of our listeners
	int size = PyList_Size (listeners);
	for (int i = 0; i < size; i++) {
		PyObject *handler = PyList_GetItem (listeners, i);
		if (!PyCallable_Check (handler)) {
			// FIXME - throw error
			fprintf (stderr, "join listener is not callable\n");
			return;
		}
		PyErr_Clear ();
		PyEval_CallObject (handler, NULL);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
			return;
		}
	}
}

void
PartHandler::process (bz_EventData *eventData)
{
	bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
	parent->RemovePlayer (pjped->playerID);

	PyObject *listeners = parent->GetListeners (bz_ePlayerPartEvent);
	if (listeners == NULL || !PyList_Check (listeners)) {
		// FIXME - throw error
		fprintf (stderr, "part listeners is not a list!\n");
		return;
	}

	// Call out to all of our listeners
	int size = PyList_Size (listeners);
	for (int i = 0; i < size; i++) {
		PyObject *handler = PyList_GetItem (listeners, i);
		if (!PyCallable_Check (handler)) {
			// FIXME - throw error
			fprintf (stderr, "part listener is not callable\n");
			return;
		}
		PyErr_Clear ();
		PyEval_CallObject (handler, NULL);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
			return;
		}
	}
}

static PyObject *FireWorldWeapon      (PyObject *self, PyObject *args);
static PyObject *GetCurrentTime       (PyObject *self, PyObject *args);
static PyObject *GetMaxWaitTime       (PyObject *self, PyObject *args);
static PyObject *GetPublic            (PyObject *self, PyObject *args);
static PyObject *GetPublicAddr        (PyObject *self, PyObject *args);
static PyObject *GetPublicDescription (PyObject *self, PyObject *args);
static PyObject *SendTextMessage      (PyObject *self, PyObject *args, PyObject *keywords);
static PyObject *SetMaxWaitTime       (PyObject *self, PyObject *args);

static struct PyMethodDef methods[] =
{
	// FIXME - docstrings
	{"FireWorldWeapon",      (PyCFunction) FireWorldWeapon, METH_VARARGS,                 NULL},
	{"GetCurrentTime",       (PyCFunction) GetCurrentTime,  METH_NOARGS,                  NULL},
	{"GetMaxWaitTime",       (PyCFunction) GetMaxWaitTime,  METH_NOARGS,                  NULL},
	{"GetPublic",            (PyCFunction) GetPublic,       METH_NOARGS,                  NULL},
	{"GetPublicAddr",        (PyCFunction) GetPublic,       METH_NOARGS,                  NULL},
	{"GetPublicDescription", (PyCFunction) GetPublic,       METH_NOARGS,                  NULL},
	{"SendTextMessage",      (PyCFunction) SendTextMessage, METH_VARARGS | METH_KEYWORDS, NULL},
	{"SetMaxWaitTime",       (PyCFunction) SetMaxWaitTime,  METH_VARARGS,                 NULL},
	{NULL,                   (PyCFunction) NULL,            0,                            NULL},
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
	tick_handler.parent    = this;
	join_handler.parent    = this;
	part_handler.parent    = this;
	capture_handler.parent = this;
	bz_registerGeneralEvent (bz_eCaptureEvent,    &capture_handler);
	bz_registerGeneralEvent (bz_eTickEvent,       &tick_handler);
	bz_registerGeneralEvent (bz_ePlayerJoinEvent, &join_handler);
	bz_registerGeneralEvent (bz_ePlayerPartEvent, &part_handler);

	// Create the dictionary for all the event handlers. Key is the int
	// event ID, value is a list of callables for our callbacks. Marshalling
	// is handled by the individual event handler classes
	event_listeners = PyDict_New ();
	CreateHandlerList (event_listeners, bz_eTickEvent);
	CreateHandlerList (event_listeners, bz_ePlayerJoinEvent);
	CreateHandlerList (event_listeners, bz_ePlayerPartEvent);

	PyModule_AddObject (module, "Events", event_listeners);

	// Create the players list
	players = PyDict_New ();

	PyModule_AddObject (module, "Players", players);
}

PyObject *
BZFlag::GetListeners (int event)
{
	PyObject *o = PyInt_FromLong (event);
	PyObject *ret = PyDict_GetItem (event_listeners, o);
	Py_DECREF (o);

	return ret;
}

bool PlayerExists (std::vector<int> h, int n)
{
	for (std::vector<int>::iterator it = h.begin (); it != h.end (); it++) {
		if (n == *it)
			return true;
	}
	return false;
}

void
BZFlag::AddPlayer (int id)
{
	PyObject *pyp = CreatePlayer (id);
	PyDict_SetItem (players, PyInt_FromLong (id), pyp);
}

void
BZFlag::RemovePlayer (int id)
{
	PyDict_DelItem (players, PyInt_FromLong (id));
}

static PyObject *
FireWorldWeapon (PyObject *self, PyObject *args)
{
	printf ("FireWorldWeapon ()\n");
	char *flag;
	float lifetime;
	int from_player;
	return Py_None;
}

static PyObject *
GetCurrentTime (PyObject *self, PyObject *args)
{
	printf ("GetCurrentTime ()\n");
	return Py_None;
}

static PyObject *
GetMaxWaitTime (PyObject *self, PyObject *args)
{
	return Py_BuildValue ("f", bz_getMaxWaitTime ());
}

static PyObject *
GetPublic (PyObject *self, PyObject *args)
{
	return (bz_getPublic () ? Py_True : Py_False);
}

static PyObject *
GetPublicAddr (PyObject *self, PyObject *args)
{
	return PyString_FromString (bz_getPublicAddr ().c_str ());
}

static PyObject *
GetPublicDescription (PyObject *self, PyObject *args)
{
	return PyString_FromString (bz_getPublicDescription ().c_str ());
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

	// FIXME - retval?
	return Py_None;
}

static PyObject *
SetMaxWaitTime (PyObject *self, PyObject *args)
{
	float time;
	if (!PyArg_ParseTuple (args, "f", &time)) {
		fprintf (stderr, "couldn't parse args\n");
		// FIXME - throw error
		return NULL;
	}

	bz_setMaxWaitTime (time);
	return Py_None;
}

};
