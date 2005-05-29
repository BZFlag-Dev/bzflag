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

#include "PyPlayer.h"

namespace Python
{

static void      Player_dealloc    (Player *player);
static PyObject *Player_getAttr    (Player *player, char *name);
static int       Player_setAttr    (Player *player, char *name, PyObject *v);
static int       Player_compare    (Player *a1, Player *a2);
static PyObject *Player_repr       (Player *player);
static PyObject *Player_ban        (Player *self, PyObject *args);
static PyObject *Player_kick       (Player *self, PyObject *args);
static PyObject *Player_kill       (Player *self, PyObject *args);
static PyObject *Player_removeFlag (Player *self, PyObject *args);

static PyMethodDef Player_methods[] = {
	{"Ban",        (PyCFunction) Player_ban,        METH_VARARGS, NULL},
	{"Kick",       (PyCFunction) Player_kick,       METH_VARARGS, NULL},
	{"Kill",       (PyCFunction) Player_kill,       METH_VARARGS, NULL},
	{"RemoveFlag", (PyCFunction) Player_removeFlag, METH_VARARGS, NULL},
	{NULL,         NULL,                            0,            NULL},
};

PyTypeObject Player_Type = {
	PyObject_HEAD_INIT(NULL)
	0,				// ob_size
	"BZFlag Player",		// tp_name
	sizeof (Player),		// tp_basicsize
	0,				// tp_itemsize
	(destructor) Player_dealloc,	// tp_dealloc
	0,				// tp_print
	(getattrfunc) Player_getAttr,	// tp_getattr
	(setattrfunc) Player_setAttr,	// tp_setattr
	(cmpfunc) Player_compare,	// tp_compare
	(reprfunc) Player_repr,		// tp_repr
	0,				// tp_as_number - FIXME - should this be ID?
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
	Player_methods,			// tp_methods
	0,				// tp_members
};

PyObject *
CreatePlayer (int id)
{
	Player *p = (Player *) PyObject_NEW (Player, &Player_Type);
	p->record = new bz_PlayerRecord ();
	bz_getPlayerByIndex (id, p->record);
	return (PyObject*) p;
}

static void
Player_dealloc (Player *player)
{
	delete player->record;
	PyObject_DEL (player);
}

static PyObject *
Player_getAttr (Player *player, char *name)
{
	PyObject *attr = Py_None;
	player->record->update ();

	if (strcmp (name, "id") == 0) {
		attr = Py_BuildValue ("i", player->record->playerID);
	} else if (strcmp (name, "callsign") == 0) {
		attr = PyString_FromString (player->record->callsign.c_str ());
	} else if (strcmp (name, "team") == 0) {
		attr = Py_BuildValue ("i", player->record->team);
	} else if (strcmp (name, "position") == 0) {
		attr = Py_BuildValue ("(f,f,f)", player->record->pos[0], player->record->pos[1], player->record->pos[2]);
	} else if (strcmp (name, "rotation") == 0) {
		attr = Py_BuildValue ("f", player->record->rot);
	} else if (strcmp (name, "ipAddr") == 0) {
		attr = PyString_FromString (player->record->ipAddress.c_str());
	} else if (strcmp (name, "flag") == 0) {
		// skip the Py_None check at the end, since None is a valid return value.
		if (player->record->currentFlag.length () == 0)
			return Py_None;
		else
			attr = PyString_FromString (player->record->currentFlag.c_str ());
	} else if (strcmp (name, "flagHistory") == 0) {
		attr = PyList_New (0);
		for (std::vector<std::string>::iterator it = player->record->flagHistory.begin (); it != player->record->flagHistory.end (); it++)
			PyList_Append (attr, PyString_FromString (it->c_str()));
	} else if (strcmp (name, "spawned") == 0) {
		attr = player->record->spawned ? Py_True : Py_False;
	} else if (strcmp (name, "verified") == 0) {
		attr = player->record->verified ? Py_True : Py_False;
	} else if (strcmp (name, "global") == 0) {
		attr = player->record->globalUser ? Py_True : Py_False;
	} else if (strcmp (name, "admin") == 0) {
		attr = player->record->admin ? Py_True : Py_False;
	} else if (strcmp (name, "groups") == 0) {
		attr = PyList_New (0);
		for (std::vector<std::string>::iterator it = player->record->groups.begin (); it != player->record->groups.end (); it++)
			PyList_Append (attr, PyString_FromString (it->c_str()));
	} else if (strcmp (name, "wins") == 0) {
		attr = Py_BuildValue ("i", player->record->wins);
	} else if (strcmp (name, "losses") == 0) {
		attr = Py_BuildValue ("i", player->record->losses);
	} else if (strcmp (name, "__members__") == 0) {
		attr = Py_BuildValue ("[s,s,s,s,s,s,s,s,s,s,s,s,s,s,s]",
				      "id", "callsign", "team", "position", "rotation", "ipAddr",
				      "flag", "flagHistory", "spawned", "verified", "global",
				      "admin", "groups", "wins", "losses");
	}

	if (attr == Py_None)
		return Py_FindMethod (Player_methods, (PyObject *) player, name);

	return attr;
}

static int
Player_setAttr (Player *player, char *name, PyObject *v)
{
}

static int
Player_compare (Player *a1, Player *a2)
{
	return ((a1->record->playerID == a2->record->playerID) &&
		(a1->record->callsign == a2->record->callsign));
}

static PyObject *
Player_repr (Player *player)
{
	return PyString_FromFormat ("[Player \"%s\" (%d)]", player->record->callsign.c_str (), player->record->playerID);
}

static PyObject *
Player_ban (Player *self, PyObject *args)
{
}

static PyObject *
Player_kick (Player *self, PyObject *args)
{
}

static PyObject *
Player_kill (Player *self, PyObject *args)
{
}

static PyObject *
Player_removeFlag (Player *self, PyObject *args)
{
}

};
