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

/* interface header */
#include "CustomGate.h"

/* system implementation headers */
#include <math.h>

/* common interface headers */
#include "Teleporter.h"
#include "StateDatabase.h"
#include "ObstacleMgr.h"


CustomGate::CustomGate(const char* _telename)
{
  telename = _telename;
  size[0] = 0.5f * BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
  size[1] = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
  size[2] = 2.0f * BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
  border = size[0] * 2.0f;
  horizontal = false;
}


bool CustomGate::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "border") == 0)
    input >> border;
  else if (strcasecmp(cmd, "horizontal") == 0)
    horizontal = true;
  else
    return WorldFileObstacle::read(cmd, input);
  return true;
}


void CustomGate::writeToGroupDef(GroupDefinition *groupdef) const
{
  Teleporter* tele =
    new Teleporter(pos, rotation,
		   fabsf(size[0]), fabsf(size[1]), fabsf(size[2]),
		   border, horizontal, driveThrough, shootThrough);

  tele->setName(telename);

  groupdef->addObstacle(tele);
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
