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

// implementation header
#include "EntryZone.h"

EntryZone::EntryZone()
{
  pos = fvec3(0.0f, 0.0f, 0.0f);
  size = fvec3(1.0f, 1.0f, 1.0f);
  rot = 0.0f;
  flags.clear();
  teams.clear();
  safety.clear();
  return;
}


void* EntryZone::unpack(void* buf)
{
  uint16_t flagCount, teamCount, safetyCount;

  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFVec3(buf, size);
  buf = nboUnpackFloat(buf, rot);
  buf = nboUnpackUInt16(buf, flagCount);
  buf = nboUnpackUInt16(buf, teamCount);
  buf = nboUnpackUInt16(buf, safetyCount);

  int i;
  for (i = 0; i < flagCount; i++) {
    FlagType *type;
    buf = FlagType::unpack (buf, type);
    flags.push_back(type);
  }
  for (i = 0; i < teamCount; i++) {
    int16_t team;
    buf = nboUnpackInt16(buf, team);
    teams.push_back((TeamColor)team);
  }
  for (i = 0; i < safetyCount; i++) {
    int16_t safetyTeam;
    buf = nboUnpackInt16(buf, safetyTeam);
    safety.push_back((TeamColor)safetyTeam);
  }

  return buf;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
