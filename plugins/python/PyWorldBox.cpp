/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PyWorldBox.h"

namespace Python
{
namespace World
{

static void      Box_dealloc (Box *box);
static PyObject *Box_getAttr (Box *box, char *name);
static int       Box_setAttr (Box *box, char *name, PyObject *v);
static int       Box_compare (Box *a1, Box *a2);
static PyObject *Box_repr    (Box *box);

static PyMethodDef Box_methods[] = {
  {NULL, NULL, 0, NULL},
};

PyTypeObject Box_Type = {
  PyObject_HEAD_INIT(NULL)
  0,				// ob_size
  "BZFlag World Box",		// tp_name
  sizeof (Box),			// tp_basicsize
  0,				// tp_itemsize
  (destructor) Box_dealloc,	// tp_dealloc
  0,				// tp_print
  (getattrfunc) Box_getAttr,	// tp_getattr
  (setattrfunc) Box_setAttr,	// tp_setattr
  (cmpfunc) Box_compare,	// tp_compare
  (reprfunc) Box_repr,		// tp_repr
  0,				// tp_as_number
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
  Box_methods,			// tp_methods
  0,				// tp_members
};

PyObject *
CreateBox ()
{
  Box *b = (Box *) PyObject_NEW (Box, &Box_Type);
  return (PyObject *) b;
}

static void
Box_dealloc (Box *box)
{
  PyObject_DEL (box);
}

static PyObject *
Box_getAttr (Box *box, char *name)
{
  PyObject *attr = Py_None;

  if (strcmp (name, "position") == 0) {
    attr = Py_BuildValue ("(fff)", box->pos[0], box->pos[1], box->pos[2]);
  } else if (strcmp (name, "rotation") == 0) {
    attr = Py_BuildValue ("f", box->rot);
  } else if (strcmp (name, "scale") == 0) {
    attr = Py_BuildValue ("(fff)", box->scale[0], box->scale[1], box->scale[2]);
  } else if (strcmp (name, "drive_through") == 0) {
    attr = box->drive_through ? Py_True : Py_False;
  } else if (strcmp (name, "shoot_through") == 0) {
    attr = box->shoot_through ? Py_True : Py_False;
  }

  if (attr == Py_None)
    return Py_FindMethod (Box_methods, (PyObject *) box, name);

  return attr;
}

static int
Box_setAttr (Box *box, char *name, PyObject *v)
{
  return 0;
}

static int
Box_compare (Box *a1, Box *a2)
{
  return 0;
}

static PyObject *
Box_repr (Box *box)
{
  return PyString_FromFormat ("[Box - position=(%f, %f, %f), rotation=%f, scale=(%f, %f, %f), %s%s%s]", box->pos[0], box->pos[1], box->pos[2], box->rot, box->scale[0], box->scale[1], box->scale[2], box->drive_through ? "drive-through" : "", box->drive_through && box->shoot_through ? ", " : "", box->shoot_through ? "shoot-through" : "");
}

};
};

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
