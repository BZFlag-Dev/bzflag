/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "CustomTeleporter.h"

/* system implementation headers */
#include <math.h>

/* common interface headers */
#include "Teleporter.h"
#include "StateDatabase.h"
#include "ObstacleMgr.h"


CustomTeleporter::CustomTeleporter(const char* _telename)
{
  telename = _telename;
  size.x = 0.5f * BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
  size.y = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
  size.z = 2.0f * BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
  border = size.x * 2.0f;
  texSize = 0.0f; // use the default
}


bool CustomTeleporter::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "border") == 0) {
    input >> border;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    input >> texSize;
  }
  else {
    return WorldFileObstacle::read(cmd, input);
  }
  return true;
}


void CustomTeleporter::writeToGroupDef(GroupDefinition *groupdef) const
{
  Teleporter* tele =
    new Teleporter(transform, pos, rotation,
		   fabsf(size.x), fabsf(size.y), fabsf(size.z),
		   border, texSize,
		   driveThrough, shootThrough, ricochet);

  if (!telename.size() && name.size()) {
    tele->setName(name);
  } else {
    tele->setName(telename);
  }

  groupdef->addObstacle(tele);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
