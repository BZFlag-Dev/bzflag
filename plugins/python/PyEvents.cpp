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
PythonHandler::emit (PyObject *arglist, int event)
{
	PyObject *listeners = parent->GetListeners (event);

	if (listeners == NULL || !PyList_Check(listeners)) {
		// FIXME - throw error
		fprintf (stderr, "tick listeners is not a list!\n");
		return;
	}

	// Call out
	int size = PyList_Size (listeners);
	for (int i = 0; i < size; i++) {
		PyObject *handler = PyList_GetItem (listeners, i);
		if (!PyCallable_Check (handler)) {
			// FIXME - throw error
			fprintf (stderr, "%d listener is not callable\n", event);
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
}

void
CaptureHandler::process (bz_EventData *eventData)
{
	bz_CTFCaptureEventData *ced = (bz_CTFCaptureEventData *) eventData;

	PyObject *arglist = Py_BuildValue ("(iii(fff)fd",
			ced->teamCaped,
			ced->teamCaping,
			ced->playerCaping,
			ced->pos[0],
			ced->pos[1],
			ced->pos[2],
			ced->rot,
			ced->time);
	emit (arglist, bz_eCaptureEvent);
	Py_DECREF (arglist);
}

void
DieHandler::process (bz_EventData *eventData)
{
	bz_PlayerDieEventData *pded = (bz_PlayerDieEventData *) eventData;

	PyObject *arglist = Py_BuildValue ("iiiis(fff)fd",
			pded->playerID,
			pded->teamID,
			pded->killerID,
			pded->killerTeamID,
			pded->flagKilledWith.c_str(),
			pded->pos[0],
			pded->pos[1],
			pded->pos[2],
			pded->rot,
			pded->time);
	emit (arglist, bz_ePlayerDieEvent);
	Py_DECREF (arglist);
}

void
JoinHandler::process (bz_EventData *eventData)
{
	bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
	parent->AddPlayer (pjped->playerID);

	PyObject *arglist = Py_BuildValue ("iissd",
			pjped->playerID,
			pjped->teamID,
			pjped->callsign.c_str (),
			pjped->reason.c_str (),
			pjped->time);

	emit (arglist, bz_ePlayerJoinEvent);
	Py_DECREF (arglist);
}

void
PartHandler::process (bz_EventData *eventData)
{
	bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
	parent->RemovePlayer (pjped->playerID);

	PyObject *arglist = Py_BuildValue ("iissd",
			pjped->playerID,
			pjped->teamID,
			pjped->callsign.c_str (),
			pjped->reason.c_str (),
			pjped->time);

	emit (arglist, bz_ePlayerPartEvent);
	Py_DECREF (arglist);
}

void
TickHandler::process (bz_EventData *eventData)
{
	bz_TickEventData *ted = (bz_TickEventData*) eventData;

	PyObject *arglist = Py_BuildValue ("(d)", ted->time);
	emit (arglist, bz_eTickEvent);
	Py_DECREF (arglist);
}

};
