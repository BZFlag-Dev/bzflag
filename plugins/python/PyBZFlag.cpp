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
	bzdb      = CreateBZDB ();

	PyModule_AddObject (module, "Event", event_sub->GetSubModule ());
	PyModule_AddObject (module, "Team",  team_sub->GetSubModule ());
	PyModule_AddObject (module, "BZDB",  bzdb);

	capture_handler.parent       = this;
	die_handler.parent           = this;
	spawn_handler.parent         = this;
	zone_entry_handler.parent    = this;
	zone_exit_handler.parent     = this;
	join_handler.parent          = this;
	part_handler.parent          = this;
	chat_handler.parent          = this;
	unknownslash_handler.parent  = this;
	getspawnpos_handler.parent   = this;
	getautoteam_handler.parent   = this;
	allowplayer_handler.parent   = this;
	tick_handler.parent          = this;
	generateworld_handler.parent = this;
	getplayerinfo_handler.parent = this;

	// Register event handlers
	bz_registerGeneralEvent (bz_eCaptureEvent,           &capture_handler);
	bz_registerGeneralEvent (bz_ePlayerDieEvent,         &die_handler);
	bz_registerGeneralEvent (bz_ePlayerSpawnEvent,       &spawn_handler);
	bz_registerGeneralEvent (bz_eZoneEntryEvent,         &zone_entry_handler);
	bz_registerGeneralEvent (bz_eZoneExitEvent,          &zone_exit_handler);
	bz_registerGeneralEvent (bz_ePlayerJoinEvent,        &join_handler);
	bz_registerGeneralEvent (bz_ePlayerPartEvent,        &part_handler);
	bz_registerGeneralEvent (bz_eChatMessageEvent,       &chat_handler);
	bz_registerGeneralEvent (bz_eUnknownSlashCommand,    &unknownslash_handler);
	bz_registerGeneralEvent (bz_eGetPlayerSpawnPosEvent, &getspawnpos_handler);
	bz_registerGeneralEvent (bz_eGetAutoTeamEvent,       &getautoteam_handler);
	bz_registerGeneralEvent (bz_eAllowPlayer,            &allowplayer_handler);
	bz_registerGeneralEvent (bz_eTickEvent,              &tick_handler);
	bz_registerGeneralEvent (bz_eGenerateWorldEvent,     &generateworld_handler);
	bz_registerGeneralEvent (bz_eGetPlayerInfoEvent,     &getplayerinfo_handler);

	// Create the dictionary for all the event handlers. Key is the int
	// event ID, value is a list of callables for our callbacks. Marshalling
	// is handled by the individual event handler classes
	event_listeners = PyDict_New ();
	CreateHandlerList (event_listeners, bz_eCaptureEvent);
	CreateHandlerList (event_listeners, bz_ePlayerDieEvent);
	CreateHandlerList (event_listeners, bz_ePlayerSpawnEvent);
	CreateHandlerList (event_listeners, bz_eZoneEntryEvent);
	CreateHandlerList (event_listeners, bz_eZoneExitEvent);
	CreateHandlerList (event_listeners, bz_ePlayerJoinEvent);
	CreateHandlerList (event_listeners, bz_ePlayerPartEvent);
	CreateHandlerList (event_listeners, bz_eChatMessageEvent);
	CreateHandlerList (event_listeners, bz_eUnknownSlashCommand);
	CreateHandlerList (event_listeners, bz_eGetPlayerSpawnPosEvent);
	CreateHandlerList (event_listeners, bz_eGetAutoTeamEvent);
	CreateHandlerList (event_listeners, bz_eAllowPlayer);
	CreateHandlerList (event_listeners, bz_eTickEvent);
	CreateHandlerList (event_listeners, bz_eGenerateWorldEvent);
	CreateHandlerList (event_listeners, bz_eGetPlayerInfoEvent);

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
