/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"
#include "PyBZFlag.h"
#include "PyPlayer.h"

namespace Python
{

static PyObject *DebugMessage         (PyObject *self, PyObject *args);
static PyObject *FireWorldWeapon      (PyObject *self, PyObject *args);
static PyObject *GetCurrentTime       (PyObject *self, PyObject *args);
static PyObject *GetMaxWaitTime       (PyObject *self, PyObject *args);
static PyObject *GetPublic            (PyObject *self, PyObject *args);
static PyObject *GetPublicAddr        (PyObject *self, PyObject *args);
static PyObject *GetPublicDescription (PyObject *self, PyObject *args);
static PyObject *GetStandardSpawn     (PyObject *self, PyObject *args);
static PyObject *PlayClientSound      (PyObject *self, PyObject *args);
static PyObject *SendTextMessage      (PyObject *self, PyObject *args, PyObject *keywords);
static PyObject *SetMaxWaitTime       (PyObject *self, PyObject *args);

static struct PyMethodDef methods[] =
{
	// FIXME - docstrings
	{"DebugMessage",         (PyCFunction) DebugMessage,         METH_VARARGS,                 NULL},
	{"FireWorldWeapon",      (PyCFunction) FireWorldWeapon,      METH_VARARGS,                 NULL},
	{"GetCurrentTime",       (PyCFunction) GetCurrentTime,       METH_NOARGS,                  NULL},
	{"GetMaxWaitTime",       (PyCFunction) GetMaxWaitTime,       METH_NOARGS,                  NULL},
	{"GetPublic",            (PyCFunction) GetPublic,            METH_NOARGS,                  NULL},
	{"GetPublicAddr",        (PyCFunction) GetPublicAddr,        METH_NOARGS,                  NULL},
	{"GetPublicDescription", (PyCFunction) GetPublicDescription, METH_NOARGS,                  NULL},
	{"GetStandardSpawn",     (PyCFunction) GetStandardSpawn,     METH_VARARGS,                 NULL},
	{"PlayClientSound",      (PyCFunction) PlayClientSound,      METH_VARARGS,                 NULL},
	{"SendTextMessage",      (PyCFunction) SendTextMessage,      METH_VARARGS | METH_KEYWORDS, NULL},
	{"SetMaxWaitTime",       (PyCFunction) SetMaxWaitTime,       METH_VARARGS,                 NULL},
	{NULL,                   (PyCFunction) NULL,                 0,                            NULL},
};

void
BZFlag::RegisterEvent (Handler *handler, bz_eEventType event)
{
	handler->parent = this;
	bz_registerEvent (event, handler);
	PyDict_SetItem (event_listeners, PyInt_FromLong ((long) event), PyList_New (0));
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

	// Register event handlers
	event_listeners = PyDict_New ();
	RegisterEvent (&capture_handler,       bz_eCaptureEvent);
	RegisterEvent (&die_handler,           bz_ePlayerDieEvent);
	RegisterEvent (&spawn_handler,         bz_ePlayerSpawnEvent);
	RegisterEvent (&zone_entry_handler,    bz_eZoneEntryEvent);
	RegisterEvent (&zone_exit_handler,     bz_eZoneExitEvent);
	RegisterEvent (&join_handler,          bz_ePlayerJoinEvent);
	RegisterEvent (&part_handler,          bz_ePlayerPartEvent);
	RegisterEvent (&chat_handler,          bz_eChatMessageEvent);
	RegisterEvent (&unknownslash_handler,  bz_eUnknownSlashCommand);
	RegisterEvent (&getspawnpos_handler,   bz_eGetPlayerSpawnPosEvent);
	RegisterEvent (&getautoteam_handler,   bz_eGetAutoTeamEvent);
	RegisterEvent (&allowplayer_handler,   bz_eAllowPlayer);
	RegisterEvent (&tick_handler,          bz_eTickEvent);
	RegisterEvent (&generateworld_handler, bz_eGenerateWorldEvent);
	RegisterEvent (&getplayerinfo_handler, bz_eGetPlayerInfoEvent);

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
DebugMessage (PyObject *self, PyObject *args)
{
	int level;
	char *message;

	if (!PyArg_ParseTuple (args, "is", &level, &message)) {
		fprintf (stderr, "couldn't parse args\n");
		// FIXME - throw error
		return NULL;
	}

	bz_debugMessage (level, message);
	return NULL;
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
GetStandardSpawn (PyObject *self, PyObject *args)
{
	return Py_None;
}

static PyObject *
PlayClientSound (PyObject *self, PyObject *args)
{
	return Py_None;
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

	bool result = bz_sendTextMessage (to, from, message);

	return (result ? Py_True : Py_False);
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
