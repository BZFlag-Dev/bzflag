/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __GAMEKEEPER_H__
#define __GAMEKEEPER_H__

// bzflag global header
#include "common.h"

// common interface headers
#include "DelayQueue.h"
#include "PlayerInfo.h"
#include "PlayerState.h"

// implementation-specific bzfs-specific headers
#include "CmdLineOptions.h"
#include "FlagHistory.h"
#include "Permissions.h"
#include "LagInfo.h"
#include "Score.h"
#include "RecordReplay.h"
#include "NetHandler.h"

const int PlayerSlot = MaxPlayers + ReplayObservers;

/** This class is meant to be the container of all the global entity that lives
    into the game and methods to act globally on those.
    Up to now it contain players. Flag class is only there as a TODO
*/
class GameKeeper {
public:
  class Player {
  public:
    Player(int _playerIndex, const struct sockaddr_in &clientAddr, int fd);
    ~Player();

    int            getIndex();
    static Player *getPlayerByIndex(int _playerIndex);
    static int     count();
    static void    updateLatency(float &waitTime);
    static void    dumpScore();
    static int     anointRabbit(int oldRabbit);
    static std::vector<int> allowed(PlayerAccessInfo::AccessPerm right,
				    int targetPlayer = -1);
    static int     getPlayerIDByName(const std::string &name);
    static void    reloadAccessDatabase();

    bool           loadEnterData(void *buf,
				 uint16_t &rejectCode,
				 char *rejectMsg);
    void          *packAdminInfo(void *buf);
    void          *packPlayerUpdate(void *buf);
    void           signingOn(bool ctf);

    // players 
    PlayerInfo        player;
    // Net Handler
    NetHandler       *netHandler;
    // player lag info
    LagInfo          *lagInfo;
    // player access
    PlayerAccessInfo  accessInfo;
    // Last known position, vel, etc
    PlayerState      *lastState;
    // DelayQueue for "Lag Flag"
    DelayQueue        delayq;
    // FlagHistory
    FlagHistory       flagHistory;
    // Score
    Score             score;
  private:
    static Player *playerList[PlayerSlot];
    int    playerIndex;
  };
  class Flag {
  };
};

inline int GameKeeper::Player::getIndex()
{
  return playerIndex;
}

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
