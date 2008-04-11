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

// implementation header
#include "Weapon.h"

Weapon::Weapon()
{
  type = Flags::Null;
  pos[0] = pos[1] = pos[2] = 0.0f;
  dir = 0.0f;
  initDelay = 0.0f;
  delay.clear();
}


void* Weapon::unpack(void* buf)
{
  uint16_t delayCount;

  buf = FlagType::unpack(buf, type);
  buf = nboUnpackFloatVector(buf, pos);
  buf = nboUnpackFloat(buf, dir);
  buf = nboUnpackFloat(buf, initDelay);
  buf = nboUnpackUShort(buf, delayCount);

  int i;
  for (i = 0; i < delayCount; i++) {
    float delayValue;
    buf = nboUnpackFloat(buf, delayValue);
    delay.push_back(delayValue);
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
