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

#ifndef __PLAYERINFO_H__
#define __PLAYERINFO_H__

// bzflag global header
#include "global.h"

// system headers
#include <string>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#endif

// bzflag library headers
#include "TimeKeeper.h"
#include "Team.h"
#include "Protocol.h"
#include "Flag.h"

// bzfs-specific headers


// ??? - we need a compile-time flag for this (that is always on)?
#define TIMELIMIT

enum ClientState {
  PlayerNoExist,
  PlayerInLimbo,
  PlayerDead,
  PlayerAlive
};

enum PlayerReplayState {
  ReplayNone,
  ReplayReceiving,
  ReplayStateful
};

#define SEND 1
#define RECEIVE 0


struct TeamInfo {
  public:
    Team team;
    TimeKeeper flagTimeout;
};


class PlayerInfo {
public:
  PlayerInfo();
  void        initPlayer(int _playerIndex);
  void        resetPlayer(bool ctf);
  // return false if player was not really in 
  bool        removePlayer();
  void        setRestartOnBase(bool on);
  bool        shouldRestartAtBase();
  bool        isPlaying();
  bool        exist();
  void        signingOn();
  bool        isAlive();
  bool        isDead();
  void        setAlive();
  void        setDead();
  bool        isPaused();
  bool        isNotResponding();
  bool        isNotPlaying();
  bool        isBot();
  bool        isHuman();
  void       *packUpdate(void *buf);
  void       *packId(void *buf);
  void        unpackEnter(void *buf);
  const char *getCallSign() const;
  bool        isCallSignReadable();
  const char *getEMail() const;
  bool        isEMailReadable();
  void       *packVirtualFlagCapture(void *buf);
  bool        isTeam(TeamColor team) const;
  bool        isObserver() const;
  TeamColor   getTeam();
  void        setTeam(TeamColor team);
  void        wasARabbit();
  void        wasNotARabbit();
  bool        isARabbitKill(PlayerInfo &victim);
  void        resetFlag();
  bool        haveFlag() const;
  int         getFlag() const;
  void        setFlag(int flag);
  bool        isFlagTransitSafe();
  const char *getClientVersion();
  void       *setClientVersion(size_t length, void *buf);
  std::string getIdleStat();
  bool        canBeRabbit(bool relaxing = false);
  void        setPaused(bool pauses);
  bool        isTooMuchIdling(TimeKeeper tm, float kickThresh);
  bool        hasStartedToNotRespond();
  void        hasSent(char message[]);
  void        addFlagToHistory();
  bool        hasPlayedEarly();
  void        setPlayedEarly();
  void        setReplayState(PlayerReplayState state);
  void        updateIdleTime();
  PlayerReplayState getReplayState();
private:
  void        cleanCallSign();
  void        cleanEMail();

  int         playerIndex;

    bool restartOnBase;

    // current state of player
    ClientState state;
    // type of player
    PlayerType type;
    // player's pseudonym
    char callSign[CallSignLen];
    // player's email address
    char email[EmailLen];
    // player's team
    TeamColor team;
    // true for dead rabbit until respawn
    bool wasRabbit;
    // flag index player has
    int flag;

    TimeKeeper lastFlagDropTime;

    std::string clientVersion;

    bool paused;
    TimeKeeper pausedSince;

    bool notResponding;
    
    // Has the player been sent any replay 'faked' state
    PlayerReplayState replayState;

    // idle kick
    TimeKeeper lastmsg;
    TimeKeeper lastupdate;

    // player played before countdown started
    bool playedEarly;
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

