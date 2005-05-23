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

};
