/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomBase.h"

/* local implementation headers */
#include "CmdLineOptions.h"
#include "TeamBases.h"

/* FIXME - external dependancies */
extern const int CtfTeams;
extern CmdLineOptions *clOptions;
extern BasesList bases;


CustomBase::CustomBase()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = BZDB.eval(StateDatabase::BZDB_BASESIZE);
  color = 0;
}


bool CustomBase::read(const char *cmd, std::istream& input) {
  if (strcmp(cmd, "color") == 0) {
    input >> color;
    if ((color < 0) || (color >= CtfTeams))
      return false;
  } else {
    if (!WorldFileObstacle::read(cmd, input))
      return false;
    if (!clOptions->flagsOnBuildings && ((pos[2] != 0) || (size[2] != 0))) {
      std::cout << "Dropping team base down to 0 because -fb not set\n";
      pos[2] = 0;
      size[2] = 0;
    }
  }
  return true;
}


void CustomBase::write(WorldInfo* world) const {
  float safety[] = { 0.0f, 0.0f, 0.0f };
  if (bases.find( color ) == bases.end())
    bases[color] = TeamBases((TeamColor)color);
  bases[color].addBase( pos, size, rotation, safety );

  world->addBase(pos[0], pos[1], pos[2], rotation,
		 fabs(size[0]), fabs(size[1]), fabs(size[2]), color,
		 driveThrough, shootThrough);
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
