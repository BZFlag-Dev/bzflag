/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PyPlayer.h"

namespace Python
{

static void      Player_dealloc		(Player *player);
static PyObject *Player_getAttr		(Player *player, char *name);
static int       Player_setAttr		(Player *player, char *name, PyObject *v);
static int       Player_compare		(Player *a1, Player *a2);
static PyObject *Player_repr		(Player *player);
static PyObject *Player_ban		(Player *self, PyObject *args, PyObject *keywords);
static PyObject *Player_kick		(Player *self, PyObject *args, PyObject *keywords);
static PyObject *Player_kill		(Player *self, PyObject *args, PyObject *keywords);
static PyObject *Player_removeFlag	(Player *self, PyObject *args);
static PyObject *Player_fetchResource	(Player *self, PyObject *args);

static PyMethodDef Player_methods[] = {
  {"Ban",		(PyCFunction) Player_ban,	METH_VARARGS | METH_KEYWORDS,	NULL},
  {"Kick",		(PyCFunction) Player_kick,	METH_VARARGS | METH_KEYWORDS,	NULL},
  {"Kill",		(PyCFunction) Player_kill,	METH_VARARGS | METH_KEYWORDS,	NULL},
  {"RemoveFlag",	(PyCFunction) Player_removeFlag,METH_VARARGS,			NULL},
  {"FetchResource",	(PyCFunction) Player_fetchResource, METH_VARARGS,		NULL},
  {NULL,		NULL,				0,				NULL},
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
  (reprfunc) Player_repr,	// tp_repr
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
  Player_methods,		// tp_methods
  0,				// tp_members
};

PyObject *
CreatePlayer (int id)
{
  Player *p = (Player *) PyObject_NEW (Player, &Player_Type);
  p->record = bz_getPlayerByIndex (id);
  return (PyObject*) p;
}

static void
Player_dealloc (Player *player)
{
  bz_freePlayerRecord (player->record);
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
    attr = Py_BuildValue ("(fff)", player->record->pos[0], player->record->pos[1], player->record->pos[2]);
  } else if (strcmp (name, "rotation") == 0) {
    attr = Py_BuildValue ("f", player->record->rot);
  } else if (strcmp (name, "ipAddr") == 0) {
    attr = PyString_FromString (player->record->ipAddress.c_str());
  } else if (strcmp (name, "flag") == 0) {
    // skip the Py_None check at the end, since None is a valid return value.
    if (player->record->currentFlag.size () == 0)
      return Py_None;
    else
      attr = PyString_FromString (player->record->currentFlag.c_str ());
  } else if (strcmp (name, "flagHistory") == 0) {
    attr = PyList_New (0);
    for (unsigned int i = 0; i < player->record->flagHistory.size (); i++) {
      bzApiString str = player->record->flagHistory[i];
      PyList_Append (attr, PyString_FromString (str.c_str()));
    }
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
    for (unsigned int i = 0; i < player->record->groups.size (); i++) {
      bzApiString str = player->record->groups[i];
      PyList_Append (attr, PyString_FromString (str.c_str()));
    }
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
  return 0;
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
Player_ban (Player *self, PyObject *args, PyObject *keywords)
{
  return 0;
}

static PyObject *
Player_kick (Player *self, PyObject *args, PyObject *keywords)
{
  PyObject *notify = Py_True;
  char *reason;

  static char *kwlist[] = {"reason", "notify", NULL};
  if (!PyArg_ParseTupleAndKeywords (args, keywords, "s|o", kwlist, &reason, &notify)) {
    fprintf (stderr, "couldn't parse args\n");
    // FIXME - throw error
    return NULL;
  }

  bool result = bz_kickUser (self->record->playerID, reason, (notify == Py_True));
  return (result ? Py_True : Py_False);
}

static PyObject *
Player_kill (Player *self, PyObject *args, PyObject *keywords)
{
  PyObject *spawn = Py_False;

  static char *kwlist[] = {"spawnOnBase", NULL};
  if (!PyArg_ParseTupleAndKeywords (args, keywords, "|o", kwlist, &spawn)) {
    fprintf (stderr, "couldn't parse args\n");
    // FIXME - throw error
    return NULL;
  }

  bool result = bz_killPlayer (self->record->playerID, (spawn == Py_True));
  return (result ? Py_True : Py_False);
}

static PyObject *
Player_removeFlag (Player *self, PyObject *args)
{
  bool result = bz_removePlayerFlag (self->record->playerID);
  return (result ? Py_True : Py_False);
}

static PyObject *
Player_fetchResource (Player *self, PyObject *args)
{
  char *url;
  if (!PyArg_ParseTuple (args, "s", &url)) {
    fprintf (stderr, "couldn't parse args\n");
    // FIXME - throw error
    return NULL;
  }

  bool result = bz_sentFetchResMessage (self->record->playerID, url);
  return (result ? Py_True : Py_False);
}

};

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
