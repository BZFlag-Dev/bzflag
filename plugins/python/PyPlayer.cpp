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

static void      Player_dealloc (Player *player);
static PyObject *Player_getAttr (Player *player, char *name);
static int       Player_setAttr (Player *player, char *name, PyObject *v);
static int       Player_compare (Player *a1, Player *a2);
static PyObject *Player_repr (Player *player);

static PyMethodDef Player_methods[] = {
	{NULL, NULL, 0, NULL},
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
	0, 0, 0, 0, 0, 0,
	0,				// tp_doc
	0, 0, 0, 0, 0, 0,
	Player_methods,			// tp_methods
	0,				// tp_members
};

PyObject *
CreatePlayer (int id)
{
	Player *p = (Player *) PyObject_NEW (Player, &Player_Type);
}

static void
Player_dealloc (Player *player)
{
	PyObject_DEL (player);
}

static PyObject *
Player_getAttr (Player *player, char *name)
{
	PyObject *attr = Py_None;
	player->record.update ();

	if (strcmp (name, "id") == 0)
		attr = Py_BuildValue ("i", player->record.playerID);
	else if (strcmp (name, "callsign") == 0)
		attr = PyString_FromString (player->record.callsign.c_str ());
	else if (strcmp (name, "team") == 0)
		attr = Py_BuildValue ("i", player->record.team);
	else if (strcmp (name, "position") == 0)
		attr = Py_BuildValue ("(f,f,f)", player->record.pos[0], player->record.pos[1], player->record.pos[2]);
	else if (strcmp (name, "rotation") == 0)
		attr = Py_BuildValue ("f", player->record.rot);
	else if (strcmp (name, "ipAddr") == 0)
		attr = PyString_FromString (player->record.ipAddress.c_str());
	else if (strcmp (name, "flag") == 0)
		// skip the Py_None check at the end, since None is a valid return value.
		if (player->record.currentFlag.length () == 0)
			return Py_None;
		else
			attr = PyString_FromString (player->record.currentFlag.c_str ());
	else if (strcmp (name, "flagHistory") == 0)
		// FIXME - create list
		attr = Py_None;
	else if (strcmp (name, "spawned") == 0)
		attr = player->record.spawned ? Py_True : Py_False;
	else if (strcmp (name, "verified") == 0)
		attr = player->record.verified ? Py_True : Py_False;
	else if (strcmp (name, "global") == 0)
		attr = player->record.globalUser ? Py_True : Py_False;
	else if (strcmp (name, "admin") == 0)
		attr = player->record.admin ? Py_True : Py_False;
	else if (strcmp (name, "groups") == 0)
		// FIXME - create list
		attr = Py_None;
	else if (strcmp (name, "wins") == 0)
		attr = Py_BuildValue ("i", player->record.wins);
	else if (strcmp (name, "losses") == 0)
		attr = Py_BuildValue ("i", player->record.losses);

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
}

static PyObject *
Player_repr (Player *player)
{
}

};
