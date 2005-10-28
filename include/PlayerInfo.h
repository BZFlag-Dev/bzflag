/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

// common interface headers
#include "TimeKeeper.h"
#include "Team.h"
#include "Protocol.h"
#include "Flag.h"
#include "WordFilter.h"


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
  PlayerInfo(int _playerIndex);

  int   getPlayerIndex( void ) {return playerIndex;}
  void	setLastMsg(std::string msg);
  const std::string& getLastMsg() const;
  TimeKeeper  getLastMsgTime() const;
  void	incSpamWarns();
  int	getSpamWarns() const;
  void	resetPlayer(bool ctf);
  void	setRestartOnBase(bool on);
  bool	shouldRestartAtBase();
  bool	isPlaying() const;
  void	signingOn();
  bool	isAlive() const;
  void	setAlive();
  void	setDead();
  bool	isPaused() const;
  bool	isAutoPilot() const;
  bool	isBot() const;
  bool	isHuman() const;
  void  *packUpdate(void *buf);
  void  *packId(void *buf);
  bool	unpackEnter(void *buf, uint16_t &rejectCode, char *rejectMsg);
  const char *getCallSign() const;
  const char *getEMail() const;
  const char *getToken() const;
  void	clearToken();
  void       *packVirtualFlagCapture(void *buf);
  bool	isTeam(TeamColor team) const;
  bool	isObserver() const;
  TeamColor   getTeam();
  void	setTeam(TeamColor team);
  void	wasARabbit();
  void	wasNotARabbit();
  bool	isARabbitKill(PlayerInfo &victim) const;
  void	resetFlag();
  bool	haveFlag() const;
  int	 getFlag() const;
  void	setFlag(int flag);
  bool	isFlagTransitSafe();
  const char *getClientVersion();
  void getClientVersionNumbers(int& major, int& major, int& revision);
  std::string getIdleStat();
  bool	canBeRabbit(bool relaxing = false);
  void	setPaused(bool paused);
  void	setAutoPilot(bool autopilot);
  bool	isTooMuchIdling(float kickThresh);
  bool	hasStartedToNotRespond();
  void	hasSent();
  bool	hasPlayedEarly();
  void	setPlayedEarly(bool early = true);
  void	setReplayState(PlayerReplayState state);
  void	updateIdleTime();
  PlayerReplayState getReplayState();
  static void setCurrentTime(TimeKeeper tm);
  static void setFilterParameters(bool	callSignFiltering,
				  WordFilter &filterData,
				  bool	simpleFiltering);

  void setTrackerID(unsigned short int t);
  unsigned short int trackerID();
  static TimeKeeper now;
  int endShotCredit;

  PlayerType getType( void ) {return type;}

private:

  void	cleanEMail();
  bool	isCallSignReadable();
  bool	isEMailReadable();

  int	 playerIndex;

  bool restartOnBase;

  // current state of player
  ClientState state;
  // type of player
  PlayerType type;
  // player's pseudonym
  char callSign[CallSignLen];
  // token from db server
  char token[TokenLen];
  // player's email address
  char email[EmailLen];
  // version information from client
  char clientVersion[VersionLen];
  int clientVersionMajor;
  int clientVersionMinor;
  int clientVersionRevision;

  // player's team
  TeamColor team;
  // true for dead rabbit until respawn
  bool wasRabbit;
  // flag index player has
  int flag;

  TimeKeeper lastFlagDropTime;


  // spam prevention
  std::string lastMsgSent;
  int spamWarns;
  TimeKeeper lastMsgTime;

  bool paused;
  TimeKeeper pausedSince;

  bool autopilot;

  bool notResponding;

  // Has the player been sent any replay 'faked' state
  PlayerReplayState replayState;

  // idle kick
  TimeKeeper lastmsg;
  TimeKeeper lastupdate;

  // player played before countdown started
  bool playedEarly;

  // tracker id for position tracking
  unsigned short int tracker;

  // Error string
  std::string errorString;

  // just need one of these for
  static WordFilter serverSpoofingFilter;

  static bool	callSignFiltering;
  static WordFilter *filterData;
  static bool	simpleFiltering;
};

inline bool PlayerInfo::isPlaying() const {
  return state > PlayerInLimbo;
}

inline bool PlayerInfo::isHuman() const {
  return type == TankPlayer;
}

inline bool PlayerInfo::haveFlag() const {
  return flag >= 0;
}

inline int PlayerInfo::getFlag() const {
  return flag;
}

inline const std::string& PlayerInfo::getLastMsg() const {
  return lastMsgSent;
}

inline TimeKeeper PlayerInfo::getLastMsgTime() const {
  return lastMsgTime;
}

inline int PlayerInfo::getSpamWarns() const {
  return spamWarns;
}

inline void PlayerInfo::incSpamWarns() {
  ++spamWarns;
}

inline void PlayerInfo::setLastMsg(std::string msg) {
  lastMsgSent = msg;
  lastMsgTime = now;
}

inline bool PlayerInfo::isAlive() const {
  return state == PlayerAlive;
}

inline bool PlayerInfo::isPaused() const {
  return paused;
}

inline bool PlayerInfo::isAutoPilot() const {
  return autopilot;
}

inline bool PlayerInfo::isBot() const {
  return type == ComputerPlayer;
}

inline bool PlayerInfo::isARabbitKill(PlayerInfo &victim) const {
  return wasRabbit || victim.team == RabbitTeam;
}

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
