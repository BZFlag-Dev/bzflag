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

#ifdef HAVE_ADNS_H
#include <adns.h>
#endif

// bzflag library headers
#include "global.h"
#include "TimeKeeper.h"
#include "Address.h"
#include "PlayerState.h"
#include "Team.h"
#include "Protocol.h"
#include "Flag.h"
#include "DelayQueue.h"

// bzfs-specific headers


// ??? - we need a compile-time flag for this (that is always on)?
#define TIMELIMIT

enum ClientState {
  PlayerNoExist,
  PlayerInLimbo,
  PlayerDead,
  PlayerAlive
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
  void        initPlayer(const struct sockaddr_in& clientAddr,
			 int _playerIndex);
  void        resetPlayer(bool ctf);
  // return false if player was not really in 
  bool        removePlayer();
  void        setRestartOnBase(bool on);
  bool        shouldRestartAtBase();
  void        resetComm();
  const char *getTargetIP();
  int         sizeOfIP();
  void       *packAdminInfo(void *buf);
  bool        isAtIP(const std::string& IP);
  bool        isPlaying();
  bool        exist();
  void        signingOn();
  bool        isAlive();
  bool        isDead();
  void        setAlive();
  void        setDead();
  bool        isBot();
  bool        isHuman();
  void       *packUpdate(void *buf);
  void        unpackEnter(void *buf);
  void        getLagStats(char* msg);
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
  void        dumpScore();
  float       scoreRanking();
  bool        setAndTestTK(float tkKickRatio);
  void        setOneMoreLoss();
  void        setOneMoreWin();
  void       *packScore(void *buf);
  bool        scoreReached(int score);
  bool        isFlagTransitSafe();
  in_addr     getIPAddress();
  void        delayQueueAddPacket(int length, const void *data, float time);
  bool        delayQueueGetPacket(int *length, void **data);
  void        delayQueueDequeuePackets();
  float       delayQueueNextPacketTime();
  const char *getClientVersion();
  void       *setClientVersion(size_t length, void *buf);
  std::string getIdleStat();
  bool        canBeRabbit(bool relaxing = false);
  void        setPaused(bool pauses);
  bool        isTooMuchIdling(TimeKeeper tm, float kickThresh);
  bool        hasStartedToNotRespond();
  int         updatePingLag(void *buf, float threshold, float max,
			    bool &warn, bool &kick);
  void        updateLagPlayerUpdate(float timestamp, bool ooo);
  bool        nextPing(float &waitTime);
  int         getNextPingSeqno();
  void        hasSent(char message[]);
  void        addFlagToHistory();
  void        handleFlagHistory(char message[]);
  void        addFlagToHistory(FlagType* type);
  bool        hasPlayedEarly();
  void        setPlayedEarly();
#ifdef HAVE_ADNS_H
  // return true if host is resolved
  bool        checkDNSResolution();
  const char *getHostname();
  static void startupResolver();
#endif

private:
  void        cleanCallSign();
  void        cleanEMail();

  int         playerIndex;

    bool restartOnBase;

    // peer's network address
    Address peer;
#ifdef HAVE_ADNS_H
    // peer's network hostname (malloc/free'd)
    char *hostname;
    // adns query state for while we're looking up hostname
    adns_query adnsQuery;
  static adns_state adnsState;
#endif
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
    // player's score
    int wins, losses, tks;

    TimeKeeper lastFlagDropTime;

    // TCP connection
    struct sockaddr_in taddr;

    // DelayQueue for "Lag Flag"
    DelayQueue delayq;

    std::string clientVersion;

    bool paused;
    TimeKeeper pausedSince;

    bool notResponding;

    // lag measurement
    float lagavg, jitteravg, lostavg, lagalpha, jitteralpha, lostalpha;
    int lagcount, laglastwarn, lagwarncount;
    bool pingpending;
    TimeKeeper nextping, lastping;
    int pingseqno, pingssent;

    // idle kick + jitter measurement
    float lasttimestamp;
    TimeKeeper lastupdate;
    TimeKeeper lastmsg;

    std::vector<FlagType*> flagHistory;

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

