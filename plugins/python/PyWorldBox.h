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

#include <Python.h>
#include "bzfsAPI.h"

#ifndef __PYTHON_WORLD_BOX_H__
#define __PYTHON_WORLD_BOX_H__

namespace Python
{
namespace World
{

typedef struct
{
  PyObject_HEAD

  float pos[3];
  float rot;
  float scale[3];
  bool drive_through;
  bool shoot_through;
} Box;

PyObject *CreateBox ();

};
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
