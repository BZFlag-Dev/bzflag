/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "Weapon.h"

// common headers
#include "Pack.h"


Weapon::Weapon() {
  type = Flags::Null;
  pos = fvec3(0.0f, 0.0f, 0.0f);
  dir = 0.0f;
  initDelay = 0.0f;
  delay.clear();
}


void* Weapon::unpack(void* buf) {
  uint16_t delayCount;

  buf = FlagType::unpack(buf, type);
  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFloat(buf, dir);
  buf = nboUnpackFloat(buf, initDelay);
  buf = nboUnpackUInt16(buf, delayCount);

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
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
