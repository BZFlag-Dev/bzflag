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

/* the common header */
#include "common.h"

/* interface header */
#include "CustomZone.h"

/* system headers */
#include <string>
#include <sstream>
#include <math.h>

/* local implementation headers */
#include "EntryZones.h"
#include "Flag.h"
#include "Team.h"


CustomZone::CustomZone()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
}


bool CustomZone::read(const char *cmd, std::istream& input) {
  if (strcmp(cmd, "flag") == 0) {
    std::string args, flag;

    std::getline(input, args);
    std::istringstream  parms(args);

    while (parms >> flag) {
      FlagType *type;

      if (flag == "good") {
	FlagSet &fs = Flag::getGoodFlags();
	for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	  FlagType *f = *it;
	  if (f->endurance != FlagNormal) { // Null and Team flags
	    qualifiers.push_back(f->flagAbbv);
	  }
	}
      }
      else if (flag == "bad") {
	FlagSet &fs = Flag::getBadFlags();
	for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	  FlagType *f = *it;
	  if (f->endurance != FlagNormal) { // Null and Team flags
	    qualifiers.push_back(f->flagAbbv);
	  }
	}
      }
      else {
	type = Flag::getDescFromAbbreviation(flag.c_str());
	if (type == Flags::Null)
	  return false;
	qualifiers.push_back(flag);
      }
    }
    input.putback('\n');
    if (qualifiers.size() == 0)
      return false;
  }
  else if ((strcmp(cmd, "team") == 0) || (strcmp(cmd, "safety") == 0)) {
    std::string args;
    int color;

    std::getline(input, args);
    std::istringstream  parms(args);

    while (parms >> color) {
      if ((color < 0) || (color >= CtfTeams))
	return false;
      std::string qual = std::string(Team::getName((TeamColor)color));
      if (strcmp(cmd, "safety") == 0) {
	qual = EntryZones::getSafetyPrefix() + qual;
      }
      qualifiers.push_back(qual);
    }
    input.putback('\n');
    if (qualifiers.size() == 0)
      return false;
  }
  else if (!WorldFileLocation::read(cmd, input))
      return false;

  return true;
}


void CustomZone::writeToWorld(WorldInfo* worldInfo) const
{
  worldInfo->addZone( this );
}

void CustomZone::getRandomPoint(float *pt) const
{
  float x = (float)((bzfrand() * (2.0f * size[0])) - size[0]);
  float y = (float)((bzfrand() * (2.0f * size[1])) - size[1]);
  pt[2] = (float)(bzfrand() * size[2]);

  pt[0] = x * cosf(rotation) - y * sinf(rotation);
  pt[1] = x * sinf(rotation) + y * cosf(rotation);

  pt[0] += pos[0];
  pt[1] += pos[1];
  pt[2] += pos[2];
}

float CustomZone::getDistToPoint (const float *_pos) const
{
  // FIXME - should use proper minimum distance from
  // the zone edge, and maybe -1.0f if its inside the zone
  float v[3], dist;
  v[0] = _pos[0] - pos[0];
  v[1] = _pos[1] - pos[1];
  v[2] = _pos[2] - pos[2];
  dist = sqrtf (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

  return dist;
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
