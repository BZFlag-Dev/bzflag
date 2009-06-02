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
#include "CustomBase.h"

// system headers
#include <string.h>

/* common implementation headers */
#include "global.h" // for CtfTeams
#include "BaseBuilding.h"
#include "ObstacleMgr.h"


CustomBase::CustomBase()
{
  pos = fvec3(0.0f, 0.0f, 0.0f);
  rotation = 0.0f;
  size.x = size.y = BZDB.eval(StateDatabase::BZDB_BASESIZE);
  color = 0;

  triggerWorldWep = false;
  worldWepType = "SW";
}


bool CustomBase::read(const char *cmd, std::istream& input) {
  if ((strcasecmp(cmd, "team")  == 0) ||
      (strcasecmp(cmd, "color") == 0)) {
    input >> color;
    if ((color <= 0) || (color >= CtfTeams)) {
      return false;
    }
  } else if (strcasecmp(cmd, "oncap") == 0) {
    triggerWorldWep = true;
    input >> worldWepType;
  } else {
    if (!WorldFileObstacle::read(cmd, input)) {
      return false;
    }
  }
  return true;
}


void CustomBase::writeToGroupDef(GroupDefinition *groupdef) const
{
  const fvec3 absSize(fabsf(size.x), fabsf(size.y), fabsf(size.z));
  BaseBuilding* base = new BaseBuilding(pos, rotation, absSize, color, ricochet);
  base->setName(name.c_str());
  groupdef->addObstacle(base);

  if (triggerWorldWep) {
    FlagType* flagType = Flag::getDescFromAbbreviation(worldWepType.c_str());
    WorldWeaponGlobalEventHandler* handler =
      new WorldWeaponGlobalEventHandler(flagType, &pos, rotation, 0, (TeamColor)color);
    worldEventManager.addEvent(bz_eCaptureEvent, handler);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
