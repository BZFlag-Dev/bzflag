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

// BZFlag common header
#include "common.h"

// interface header
#include "ShotUpdate.h"

// implementation headers
#include "Pack.h"

//
// ShotUpdate
//

void*			ShotUpdate::pack(void* buf) const
{
  buf = nboPackUInt8(buf, player);
  buf = nboPackUInt16(buf, id);
  buf = nboPackFVec3(buf, pos);
  buf = nboPackFVec3(buf, vel);
  buf = nboPackFloat(buf, dt);
  buf = nboPackInt16(buf, team);
  return buf;
}

void			ShotUpdate::pack(BufferedNetworkMessage *msg) const
{
  msg->packUInt8(player);
  msg->packUInt16(id);
  msg->packFVec3(pos);
  msg->packFVec3(vel);
  msg->packFloat(dt);
  msg->packInt16(team);
}

void*			ShotUpdate::unpack(void* buf)
{
  buf = nboUnpackUInt8(buf, player);
  buf = nboUnpackUInt16(buf, id);
  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFVec3(buf, vel);
  buf = nboUnpackFloat(buf, dt);
  short temp;
  buf = nboUnpackInt16(buf, temp);
  team = (TeamColor)temp;
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
  buf = nboPackFloat(buf, timeSent);
  buf = shot.pack(buf);
  buf = flagType->pack(buf);
  buf = nboPackFloat(buf, lifetime);
  buf = nboPackUInt8(buf, shotType);
  return buf;
}

void			FiringInfo::pack(BufferedNetworkMessage *msg) const
{
  msg->packFloat(timeSent);
  shot.pack(msg);
  flagType->pack(msg);
  msg->packFloat(lifetime);
  msg->packUInt8(shotType);
}

void*			FiringInfo::unpack(void* buf)
{
  buf = nboUnpackFloat(buf, timeSent);
  buf = shot.unpack(buf);
  buf = FlagType::unpack(buf, flagType);
  buf = nboUnpackFloat(buf, lifetime);
  unsigned char t = 0 ;
  buf = nboUnpackUInt8(buf, t);
  shotType = (ShotType)t;
  return buf;
}

void*			FiringInfo::unpackW(void* buf)
{
  buf = nboUnpackFloat(buf, timeSent);
  buf = shot.unpack(buf);
  buf = FlagType::unpack(buf, flagType);
  buf = nboUnpackFloat(buf, lifetime);
  unsigned char t = 0;
  buf = nboUnpackUInt8(buf, t);
  shotType = (ShotType)t;
  return buf;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
