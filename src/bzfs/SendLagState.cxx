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
#include "SendLagState.h"

// system headers
#include <string>
#include <vector>

// common headers
#include "bzfio.h"
#include "BzTime.h"
#include "NetMessage.h"

// local headers
#include "bzfs.h"
#include "GameKeeper.h"


static float updateRate = 2.0f;

static BzTime lastSend = BzTime::getCurrent();


//============================================================================//

float SendLagState::getWaitTime()
{
  const BzTime nowTime = BzTime::getCurrent();
  return updateRate - (nowTime - lastSend);
}


void SendLagState::update()
{
  if (BZDB.isSet("_sendLagRate")) {
    updateRate = BZDB.eval("_sendLagRate");
  }

  if (updateRate <= 0.0f) {
    return;
  }

  BzTime nowTime = BzTime::getCurrent();
  if ((nowTime - lastSend) < updateRate) {
    return;
  }
  lastSend = nowTime;

  NetMessage netMsg;

  uint8_t entryCount = 0;
  netMsg.packUInt8(0); // dummy value

  for (int id = 0; id < curMaxPlayers; id++) {
    GameKeeper::Player* gkPlayer = GameKeeper::Player::getPlayerByIndex(id);
    if (!gkPlayer) {
      continue;
    }
    const LagInfo& lagInfo = gkPlayer->lagInfo;
    const uint16_t lag     = uint16_t(lagInfo.getLag());    // in milliseconds
    const uint16_t jitter  = uint16_t(lagInfo.getJitter()); // in milliseconds
    const uint8_t  pktLoss = uint8_t(lagInfo.getLoss());    // in percent
    netMsg.packUInt8(id);
    netMsg.packUInt16(lag);
    netMsg.packUInt16(jitter);
    netMsg.packUInt8(pktLoss);
    entryCount++;
  }

  nboPackUInt8(netMsg.getData(), entryCount);

  logDebugMessage(6, "MsgLagState sent with %i entries\n", (int) entryCount);

  netMsg.broadcast(MsgLagState);
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
