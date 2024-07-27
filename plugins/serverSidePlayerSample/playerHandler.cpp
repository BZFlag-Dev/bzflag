/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
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
    bz_debugMessage(3, "PlayerHandler::added");
    std::string name = format("Sample %d", playerIndex);
    setPlayerData(name.c_str(), NULL, "bot sample", eObservers);
    joinGame();
}

void PlayerHandler::textMessage(int dest, int source, const char *text)
{
    if (dest == getPlayerID())
        sendChatMessage(text,source);
}

void PlayerHandler::playerSpawned(int player, const float[3], float)
{
    std::string playerName = bz_getPlayerCallsign(player);
    std::string msg = "Oh look, " + playerName + " decided to join us!";
    sendChatMessage(msg.c_str());
}

void PlayerHandler::shotFired(int player, unsigned short)
{
    std::string playerName = bz_getPlayerCallsign(player);
    std::string msg = "Hey, " + playerName + " I bet you think you are special now!";
    sendChatMessage(msg.c_str());
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
