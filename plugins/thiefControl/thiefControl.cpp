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

// thiefControl.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class ThiefControl : public bz_Plugin
{
public:
  virtual const char* Name() {return "Thief Control";}
  virtual void Init( const char* config );
  virtual void Event( bz_EventData *eventData );
};

BZ_PLUGIN(ThiefControl)

void ThiefControl::Init(const char* /*config*/ )
{
  bz_debugMessage(4, "thiefControl plugin loaded");
  Register(bz_eFlagTransferredEvent);
}

void ThiefControl::Event (bz_EventData * eventData)
{
  bz_FlagTransferredEventData_V1 *data = (bz_FlagTransferredEventData_V1 *) eventData;
  const std::string noStealMsg = "Flag dropped. Don't steal from teammates!";

  if (!data)
    return;

  if (data->eventType != bz_eFlagTransferredEvent)
    return;

  bz_BasePlayerRecord *playerFrom = bz_getPlayerByIndex(data->fromPlayerID);

  if (!playerFrom)
    return;

  bz_BasePlayerRecord *playerTo = bz_getPlayerByIndex(data->toPlayerID);

  if (!playerTo) {
    bz_freePlayerRecord(playerFrom);
    return;
  }

  switch (bz_getGameType()) {

  case eFFAGame:
    if (playerTo->team == playerFrom->team && playerTo->team != eRogueTeam) {
      data->action = data->DropThief;
      bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
    }
    break;

  case eCTFGame:
    {
      if (playerTo->team == playerFrom->team && playerTo->team != eRogueTeam) {
	bz_ApiString flagT = bz_ApiString(data->flagType);

	// Allow teammates to steal team flags
	// This will allow someone to steal a team flag in order
	// to possibly capture it faster.
	if (flagT != "R*" && flagT != "G*" && flagT != "B*" && flagT != "P*") {
	  data->action = data->DropThief;
	  bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
	}
      }
    }
    break;

  case eRabbitGame:
    if (playerTo->team == playerFrom->team) {

      data->action = data->DropThief;
      bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
    }
    break;

  default:
    break;

  }

  bz_freePlayerRecord(playerTo);
  bz_freePlayerRecord(playerFrom);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
