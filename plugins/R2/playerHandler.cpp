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

#include "playerHandler.h"
#include "plugin_utils.h"

void PlayerHandler::added(int playerIndex)
{
    bz_debugMessage(3, "R2::added");
    std::string name = "R2-";
    if (playerIndex >= 10)
        name += format("%d", playerIndex);
    else
        name += format("D%d", playerIndex);

    std::string motto;
    for (int i = 0; i < 32; i++)
    {
        if (rand() % 2 == 1)
            motto += "1";
        else
            motto += "0";
    }

    setPlayerData(name.c_str(), nullptr, motto.c_str(), eRogueTeam);
    joinGame();
}

void PlayerHandler::textMessage(int dest, int source, const char *text)
{
    if (dest == getPlayerID())
    {
        sendChatMessage("WHAHAHAAHAHAHAHAAAAA?A?A?A?!?!", source);
    }
}

void PlayerHandler::playerSpawned(int player, const float pos[3], float rot)
{

}

void PlayerHandler::shotFired(int player, unsigned short shotID)
{
}


void PlayerHandler::startPlay()
{
    // if we are not spawned, spawn!
    respawn();
}


void PlayerHandler::spawned()
{
    sendChatMessage("Beep bawooodddd!");
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
