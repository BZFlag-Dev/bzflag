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

#include "ShotUpdate.h"

//
// ShotUpdate
//

void*			ShotUpdate::pack(void* buf) const
{
  buf = player.pack(buf);
  buf = nboPackUShort(buf, id);
  buf = nboPackVector(buf, pos);
  buf = nboPackVector(buf, vel);
  buf = nboPackFloat(buf, dt);
  return buf;
}

void*			ShotUpdate::unpack(void* buf)
{
  buf = player.unpack(buf);
  buf = nboUnpackUShort(buf, id);
  buf = nboUnpackVector(buf, pos);
  buf = nboUnpackVector(buf, vel);
  buf = nboUnpackFloat(buf, dt);
  return buf;
}

//
// FiringInfo
//

FiringInfo::FiringInfo()
{
  // do nothing -- must be prepared before use by unpack() or assignment
}

void*			FiringInfo::pack(void* buf) const
{
  buf = shot.pack(buf);
  buf = nboPackUShort(buf, uint16_t(flag));
  buf = nboPackFloat(buf, lifetime);
  return buf;
}

void*			FiringInfo::unpack(void* buf)
{
  uint16_t _flag;
  buf = shot.unpack(buf);
  buf = nboUnpackUShort(buf, _flag);
  flag = FlagId(_flag);
  buf = nboUnpackFloat(buf, lifetime);
  return buf;
}

