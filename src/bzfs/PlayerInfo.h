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

// system headers
#include <string>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#endif

// bzflag library headers
#include "global.h"
#include "Permissions.h"
#include "TimeKeeper.h"
#include "Address.h"
#include "PlayerState.h"
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


#ifdef DEBUG
#define NETWORK_STATS
#endif
#ifdef NETWORK_STATS
struct MessageCount {
  public:
    uint32_t count;
    uint16_t code;
    uint16_t maxSize;
};
// does not include MsgNull
#define MessageTypes 38
#endif


#define SEND 1
#define RECEIVE 0


struct TeamInfo {
  public:
    Team team;
    TimeKeeper flagTimeout;
};


struct PlayerInfo {
  public:
    // player access
    PlayerAccessInfo accessInfo;
    bool Admin;
    bool restartOnBase;

    // player's registration name
    std::string regName;
    // time accepted
    TimeKeeper time;
    // socket file descriptor
    int fd;
    // peer's network address
    Address peer;
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

    // Last known position, vel, etc
    PlayerState lastState;

    TimeKeeper lastFlagDropTime;

    // input buffers
    // bytes read in current msg
    int tcplen;
    // current TCP msg
    char tcpmsg[MaxPacketLen];
    // bytes read in current msg
    int udplen;
    // current UDP msg
    char udpmsg[MaxPacketLen];

    // output buffer
    int outmsgOffset;
    int outmsgSize;
    int outmsgCapacity;
    char *outmsg;

    // UDP connection
    bool udpin; // udp inbound up, player is sending us udp
    bool udpout; // udp outbound up, we can send udp
    struct sockaddr_in uaddr;
    // TCP connection
    struct sockaddr_in taddr;

    // UDP message queue
    struct PacketQueue *uqueue;
    struct PacketQueue *dqueue;
    unsigned short lastRecvPacketNo;
    unsigned short lastSendPacketNo;

    std::string clientVersion;

    bool paused;
    TimeKeeper pausedSince;

    bool notResponding;
    bool toBeKicked;
    std::string toBeKickedReason;


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

    // number of times they have tried to /password
    int passwordAttempts;

#ifdef NETWORK_STATS
    // message stats bloat
    TimeKeeper perSecondTime[2];
    uint32_t perSecondCurrentBytes[2];
    uint32_t perSecondMaxBytes[2];
    uint32_t perSecondCurrentMsg[2];
    uint32_t perSecondMaxMsg[2];
    uint32_t msgBytes[2];
    struct MessageCount msg[2][MessageTypes];
#endif
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

