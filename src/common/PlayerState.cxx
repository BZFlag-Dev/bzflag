/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "PlayerState.h"
#include "Pack.h"

PlayerState::PlayerState()
: status(DeadStatus), azimuth(0.0f), angVel(0.0f)
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  velocity[0] = velocity[0] = velocity[2] = 0.0f;
}

void*	PlayerState::pack(void* buf) const
{
  buf = nboPackShort(buf, int16_t(status));
  buf = nboPackVector(buf, pos);
  buf = nboPackVector(buf, velocity);
  buf = nboPackFloat(buf, azimuth);
  buf = nboPackFloat(buf, angVel);
  return buf;
}

void*	PlayerState::unpack(void* buf)
{
  int16_t inStatus;
  buf = nboUnpackShort(buf, inStatus);
  buf = nboUnpackVector(buf, pos);
  buf = nboUnpackVector(buf, velocity);
  buf = nboUnpackFloat(buf, azimuth);
  buf = nboUnpackFloat(buf, angVel);
  status = short(inStatus);
  return buf;
}

