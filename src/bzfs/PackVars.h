/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __PACKVARS_H__
#define __PACKVARS_H__

/* system interface headers */
#include <string>

/* common interface headers */
#include "Pack.h"
#include "StateDatabase.h"

/** class to send a bunch of BZDB variables via MsgSetVar.
 * dtor does the actual send
 */
class PackVars
{
public:
  PackVars(void *buffer, int playerIndex) : bufStart(buffer)
  {
    buf = nboPackUShort(bufStart, 0);//placeholder
    playerId = playerIndex;
    len = sizeof(uint16_t);
    count = 0;
  }

  ~PackVars()
  {
    if (len > sizeof(uint16_t)) {
      nboPackUShort(bufStart, count);
      directMessage(playerId, MsgSetVar, len, bufStart);
    }
  }

  // callback forwarder
  static void packIt(const std::string &key, void *pv)
  {
    reinterpret_cast<PackVars*>(pv)->sendPackVars(key);
  }

  void sendPackVars(const std::string &key)
  {
    std::string value = BZDB.get(key);
    int pairLen = key.length() + 1 + value.length() + 1;
    if ((pairLen + len) > (MaxPacketLen - 2*sizeof(uint16_t))) {
      nboPackUShort(bufStart, count);
      count = 0;
      directMessage(playerId, MsgSetVar, len, bufStart);
      buf = nboPackUShort(bufStart, 0); //placeholder
      len = sizeof(uint16_t);
    }

    buf = nboPackUByte(buf, (uint8_t)key.length());
    buf = nboPackString(buf, key.c_str(), key.length());
    buf = nboPackUByte(buf, (uint8_t)value.length());
    buf = nboPackString(buf, value.c_str(), value.length());
    len += pairLen;
    count++;
  }

private:
  void * const bufStart;
  void *buf;
  int playerId;
  unsigned int len;
  unsigned int count;
};


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
