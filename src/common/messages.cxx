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
#include "messages.h"

/* system implementation headers */
#include <sstream>
#include <string>

#include <stdio.h>
#include <string.h>

/* common implementation headers */
#include "Pack.h"


PlayerAddMessage::PlayerAddMessage()
{
  playerID = -1;
  team = -1;
  type = -1;
  wins = 0;
  losses = 0;
  tks = 0;
}

bool PlayerAddMessage::unpack ( void* buf )
{
  unsigned char id;
  int16_t _team;
  uint16_t _type, _wins, _losses, _tks;
  char _callsign[_CallSignLen] = {0};

  buf = nboUnpackUInt8(buf, id);
  buf = nboUnpackUInt16(buf, _type);
  buf = nboUnpackInt16(buf, _team);
  buf = nboUnpackUInt16(buf, _wins);
  buf = nboUnpackUInt16(buf, _losses);
  buf = nboUnpackUInt16(buf, _tks);
  buf = nboUnpackString(buf, _callsign, _CallSignLen);

  playerID = id;
  team = _team;
  type = _type;
  wins = _wins;
  losses = _losses;
  tks = _tks;

  callsign = _callsign;

  return true;
}

void*  PlayerAddMessage::pack ( void* buf )
{
  int16_t _team;
  uint16_t _type, _wins, _losses, _tks;
  char _callsign[_CallSignLen] = {0};

  unsigned char id;

  id = playerID;
  _team = team;
  _type = type;
  _wins = wins;
  _losses = losses;
  _tks = tks;

  strncpy(_callsign,callsign.c_str(),callsign.size() > _CallSignLen-1 ? _CallSignLen-1 :  callsign.size());

  buf = nboPackUInt8(buf, id);
  buf = nboPackUInt16(buf, _type);
  buf = nboPackUInt16(buf, _type);
  buf = nboPackInt16(buf, _team);
  buf = nboPackUInt16(buf, _wins);
  buf = nboPackUInt16(buf, _losses);
  buf = nboPackUInt16(buf, _tks);
  buf = nboPackString(buf, _callsign, _CallSignLen);

  return buf;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
