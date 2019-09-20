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
    virtual const char* Name()
    {
        return "Thief Control";
    }
    virtual void Init( const char* config );
    virtual void Event( bz_EventData *eventData );
private:
    void dropThief(bz_FlagTransferredEventData_V1 *data);
};

BZ_PLUGIN(ThiefControl)

void ThiefControl::Init(const char* /*config*/ )
{
    bz_debugMessage(4, "thiefControl plugin loaded");
    Register(bz_eFlagTransferredEvent);
}

void ThiefControl::dropThief(bz_FlagTransferredEventData_V1* data)
{
    const std::string noStealMsg = "Flag dropped. Don't steal from teammates!";
    data->action = data->DropThief;
    bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
}

void ThiefControl::Event (bz_EventData * eventData)
{
    bz_FlagTransferredEventData_V1 *data = (bz_FlagTransferredEventData_V1 *) eventData;

    if (data->eventType != bz_eFlagTransferredEvent)
        return;

    bz_eTeamType fromTeam = bz_getPlayerTeam(data->fromPlayerID);
    bz_eTeamType toTeam = bz_getPlayerTeam(data->toPlayerID);

    if (fromTeam == eNoTeam || toTeam == eNoTeam)
      return;

    switch (bz_getGameType())
    {

    case eFFAGame:
        if (toTeam == fromTeam && toTeam != eRogueTeam)
        {
            dropThief(data);
        }
        break;

    case eCTFGame:
    {
        if (toTeam == fromTeam && toTeam != eRogueTeam)
        {
            bz_ApiString flagT = bz_ApiString(data->flagType);

            // Allow teammates to steal team flags
            // This will allow someone to steal a team flag in order
            // to possibly capture it faster.
            if (flagT != "R*" && flagT != "G*" && flagT != "B*" && flagT != "P*")
            {
                dropThief(data);
            }
        }
    }
    break;

    case eRabbitGame:
        if (toTeam == fromTeam)
        {
            dropThief(data);
        }
        break;

    default:
        break;

    }

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
