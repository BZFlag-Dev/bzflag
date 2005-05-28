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

#include "PyEvents.h"
#include "PyBZFlag.h"

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

};
