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

#ifdef HAVE_ADNS_H
#include <adns.h>
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

enum RxStatus {
  ReadAll,
  ReadPart,
  ReadReset,
  ReadError,
  ReadDiscon
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


class PlayerInfo {
public:
  void        initPlayer(const struct sockaddr_in& clientAddr, int _fd,
			 int _playerIndex);
  void        resetPlayer(bool ctf);
  bool        isAccessVerified() const;
  // return false if player was not really in 
  bool        removePlayer(const char *reason);
  bool        gotAccessFailure();
  void        setLoginFail();
  void        reloadInfo();
  void        setPermissionRights();
  bool        hasSetGroupPermission(const std::string& group);
  bool        hasPermission(PlayerAccessInfo::AccessPerm right);
  void        setGroup(const std::string& group);
  void        resetGroup(const std::string& group);
  void        setAdmin();
  void        setRestartOnBase(bool on);
  bool        shouldRestartAtBase();
  std::string getName();
  bool        isRegistered() const;
  bool        isExisting();
  bool        isIdentifyRequired();
  bool        isAllowedToEnter();
  bool        isPasswordMatching(const char* pwd);
  uint8_t     getPlayerProperties();
  void        storeInfo(const char* pwd);
  void        setPassword(const std::string& pwd);
  void        initStatistics();
  bool        isConnected();
  int         pflush(fd_set *set);
  RxStatus    receive(size_t length);
  int         pwrite(const void *b, int l, int udpSocket);
  bool        setUdpIn(struct sockaddr_in &_uaddr);
  void        setUdpOut();
  void        createUdpCon(int remote_port);
  bool        isMyUdpAddrPort(struct sockaddr_in &uaddr);
  void        resetComm();
  void        closeComm();
  void        dropUnconnected();
  void        debugUdpInfo();
  void        debugUdpRead(int n, struct sockaddr_in &_uaddr, int udpSocket);
  void        debugAdd();
  void        fdSet(fd_set *read_set, fd_set *write_set, int &maxFile);
  int         fdIsSet(fd_set *set);
  void        getPlayerList(char *list); 
  const char *getTargetIP();
  int         sizeOfIP();
  void       *packAdminInfo(void *buf);
  void        debugUnknownPacket(int code);
  bool        isAtIP(const std::string& IP);
  void        debugHugePacket(int length);
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
  void        udpFillRead(void *buf, int len);
  void       *getUdpBuffer();
  void       *getTcpBuffer();
  void        cleanTcp();
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
  std::string reasonToKick();
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
  bool        passwordAttemptsMax();
#ifdef NETWORK_STATS
  void        countMessage(uint16_t code, int len, int direction);
#endif
private:
  void        cleanCallSign();
  void        cleanEMail();
  void        udpSend(int udpSocket, const void *b, size_t l);
  int         send(const void *buffer, size_t length);
  int         bufferedSend(int playerIndex, const void *buffer, size_t length);
#ifdef NETWORK_STATS
  void        dumpMessageStats();
#endif

  int         playerIndex;

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
#ifdef HAVE_ADNS_H
    // peer's network hostname (malloc/free'd)
    char *hostname;
    // adns query state for while we're looking up hostname
    adns_query adnsQuery;
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

    // input buffers
    // bytes read in current msg
    int tcplen;
    // current TCP msg
    char tcpmsg[MaxPacketLen];
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
    
    // DelayQueue for "Lag Flag"
    DelayQueue delayq;

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

