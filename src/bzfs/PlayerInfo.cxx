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

/* interface header */
#include "PlayerInfo.h"

// system headers
#include <errno.h>

/* system implementation headers */
#include <assert.h>

// implementation-specific bzflag headers
#include "TextUtils.h"

#define MAX_FLAG_HISTORY (10)

void PlayerInfo::initPlayer(const struct sockaddr_in& clientAddr,
			    int _playerIndex) {
  playerIndex      = _playerIndex;
  AddrLen addr_len = sizeof(clientAddr);

  // store address information for player
  memcpy(&taddr, &clientAddr, addr_len);

  state = PlayerInLimbo;
  paused = false;
  pausedSince = TimeKeeper::getNullTime();
};

void PlayerInfo::resetPlayer(bool ctf) {
  wasRabbit = false;

  delayq.dequeuePackets();

  lagavg = 0;
  lagcount = 0;
  laglastwarn = 0;
  lagwarncount = 0;
  lagalpha = 1;

  jitteravg = 0;
  jitteralpha = 1;

  lostavg = 0;
  lostalpha = 1;

  lasttimestamp = 0.0f;
  lastupdate = TimeKeeper::getCurrent();
  lastmsg	 = TimeKeeper::getCurrent();

  nextping = TimeKeeper::getCurrent();
  nextping += 10.0;
  pingpending = false;
  pingseqno = 0;
  pingssent = 0;
  
  replayState = ReplayNone;

#ifdef TIMELIMIT
  playedEarly = false;
#endif

  restartOnBase = ctf;
}

bool PlayerInfo::removePlayer() {

  bool wasPlaying = state > PlayerInLimbo;

  delayq.dequeuePackets();

  callSign[0] = 0;

  flagHistory.clear();

  state = PlayerNoExist;

  return wasPlaying;
}

void PlayerInfo::setRestartOnBase(bool on) {
  restartOnBase = on;
};

bool PlayerInfo::shouldRestartAtBase() {
  return restartOnBase;
};

void PlayerInfo::resetComm() {
    state = PlayerNoExist;
    delayq.init();
};

bool PlayerInfo::isPlaying() {
  return state > PlayerInLimbo;
};

bool PlayerInfo::exist() {
  return state != PlayerNoExist;
};
void PlayerInfo::signingOn() {
  state = PlayerDead;
  flag = -1;
  wins = 0;
  losses = 0;
  tks = 0;  
};

bool PlayerInfo::isAlive() {
  return state == PlayerAlive;
};

bool PlayerInfo::isDead() {
  return state == PlayerDead;
};

void PlayerInfo::setAlive() {
  state = PlayerAlive;
  flag = -1;
};

void PlayerInfo::setDead() {
  state = PlayerDead;
};

bool PlayerInfo::isBot() {
  return type == ComputerPlayer;
};

bool PlayerInfo::isHuman() {
  return type == TankPlayer;
};

void *PlayerInfo::packUpdate(void *buf) {
  buf = nboPackUShort(buf, uint16_t(type));
  buf = nboPackUShort(buf, uint16_t(team));
  buf = nboPackUShort(buf, uint16_t(wins));
  buf = nboPackUShort(buf, uint16_t(losses));
  buf = nboPackUShort(buf, uint16_t(tks));
  buf = nboPackString(buf, callSign, CallSignLen);
  buf = nboPackString(buf, email, EmailLen);
  return buf;
};

void PlayerInfo::unpackEnter(void *buf) {
  // data: type, team, name, email
  uint16_t _type;
  int16_t _team;
  buf = nboUnpackUShort(buf, _type);
  buf = nboUnpackShort(buf, _team);
  type = PlayerType(_type);
  team = TeamColor(_team);
  buf = nboUnpackString(buf, callSign, CallSignLen);
  buf = nboUnpackString(buf, email, EmailLen);
  cleanCallSign();
  cleanEMail();
  DEBUG1("Player %s [%d] has joined from %s:%d\n",
	 callSign, playerIndex, inet_ntoa(taddr.sin_addr),
	 ntohs(taddr.sin_port));
};

void PlayerInfo::getLagStats(char* msg) {
  if ((state > PlayerInLimbo) && (type == TankPlayer)) {
    sprintf(msg,"%-16s : %3d +- %2dms", callSign,
	    int(lagavg * 1000),
	    int(jitteravg * 1000));
    if (lostavg >= 0.01f)
      sprintf(msg + strlen(msg), " %d%% lost/ooo", int(lostavg * 100));
  } else {
    msg[0] = 0;
  }
};

const char *PlayerInfo::getCallSign() const {
  return callSign;
};

void PlayerInfo::cleanCallSign() {
  // strip leading whitespace from callsign
  char *sp = callSign;
  char *tp = sp;
  while (isspace(*sp))
    sp++;

  // strip any non-printable characters and ' and " from callsign
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"')) {
      *tp++ = *sp;
    }
  } while (*++sp);
  *tp = *sp;

  // strip trailing whitespace from callsign
  while (isspace(*--tp)) {
    *tp=0;
  }
};

bool PlayerInfo::isCallSignReadable() {
  // callsign readability filter, make sure there are more alphanum than non
  // keep a count of alpha-numerics
  int alnumCount = 0;
  const char *sp = callSign;
  do {
    if (isalnum(*sp)) {
      alnumCount++;
    }
  } while (*++sp);
  int callsignlen = strlen(callSign);
  return (callsignlen <= 4) || ((float)alnumCount / (float)callsignlen > 0.5f);
};

const char *PlayerInfo::getEMail() const {
  return email;
};

void PlayerInfo::cleanEMail() {
  // strip leading whitespace from email
  char *sp = email;
  char *tp = sp;
  while (isspace(*sp))
    sp++;

  // strip any non-printable characters and ' and " from email
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"')) {
      *tp++ = *sp;
    }
  } while (*++sp);
  *tp = *sp;

  // strip trailing whitespace from email
  while (isspace(*--tp)) {
    *tp=0;
  }
};

bool PlayerInfo::isEMailReadable() {
  // email/"team" readability filter, make sure there are more
  // alphanum than non
  int emailAlnumCount = 0;
  char *sp = email;
  do {
    if (isalnum(*sp)) {
      emailAlnumCount++;
    }
  } while (*++sp);
  int emaillen = strlen(email);
  return (emaillen <= 4) || (((float)emailAlnumCount / (float)emaillen) > 0.5);
};

void *PlayerInfo::packVirtualFlagCapture(void *buf) {
  buf = nboPackUShort(buf, uint16_t(int(team) - 1));
  buf = nboPackUShort(buf, uint16_t(1 + (int(team) % 4)));
  return buf;
};

bool PlayerInfo::isTeam(TeamColor _team) const {
  return team == _team;
};

bool PlayerInfo::isObserver() const {
  return team == ObserverTeam;
};

TeamColor PlayerInfo::getTeam() {
  return team;
};

void PlayerInfo::setTeam(TeamColor _team) {
  team = _team;
};

void PlayerInfo::wasARabbit() {
  team = RogueTeam;
  wasRabbit = true;
};

void PlayerInfo::wasNotARabbit() {
  wasRabbit = false;
};

bool PlayerInfo::isARabbitKill(PlayerInfo &victim) {
  return wasRabbit || victim.team == RabbitTeam;
};

void PlayerInfo::resetFlag() {
  flag = -1;
  lastFlagDropTime = TimeKeeper::getCurrent();
};

bool PlayerInfo::haveFlag() const {
  return flag >= 0;
}
int PlayerInfo::getFlag() const {
  return flag;
};

void PlayerInfo::setFlag(int _flag) {
  flag = _flag;
};

void PlayerInfo::dumpScore() {
  if (state > PlayerInLimbo)
    std::cout << wins << '-' << losses << ' ' << callSign << std::endl;
};

float PlayerInfo::scoreRanking() {
  int sum = wins + losses;
  if (sum == 0)
    return 0.5;
  float average = (float)wins/(float)sum;
  // IIRC that is how wide is the gaussian
  float penalty = (1.0f - 0.5f / sqrt((float)sum));
  return average * penalty;
};

bool PlayerInfo::setAndTestTK(float tkKickRatio) {
  tks++;
  return (tks >= 3) && (tkKickRatio > 0) && // arbitrary 3
    ((wins == 0) || (tks * 100 / wins > tkKickRatio));
};

void PlayerInfo::setOneMoreLoss() {
  losses++;
};

void PlayerInfo::setOneMoreWin() {
  wins++;
};

void *PlayerInfo::packScore(void *buf) {
  buf = nboPackUByte(buf, playerIndex);
  buf = nboPackUShort(buf, wins);
  buf = nboPackUShort(buf, losses);
  buf = nboPackUShort(buf, tks);
  return buf;
};

bool PlayerInfo::scoreReached(int score) {
  return wins - losses >= score;
};

bool PlayerInfo::isFlagTransitSafe() {
  return TimeKeeper::getCurrent() - lastFlagDropTime >= 2.0f;
};

void PlayerInfo::delayQueueAddPacket(int length, const void *data,
				     float time) {
  delayq.addPacket(length, data, time);
};

bool PlayerInfo::delayQueueGetPacket(int *length, void **data) {
  return delayq.getPacket(length, data);
};

void PlayerInfo::delayQueueDequeuePackets() {
  delayq.dequeuePackets();
};

float PlayerInfo::delayQueueNextPacketTime() {
  return delayq.nextPacketTime();
};

const char *PlayerInfo::getClientVersion() {
  return clientVersion.c_str();
};

void *PlayerInfo::setClientVersion(size_t length, void *buf) {
  char *versionString = new char[length];
  buf = nboUnpackString(buf, versionString, length);
  clientVersion = std::string(versionString);
  delete[] versionString;
  DEBUG2("Player %s [%d] sent version string: %s\n", 
	 callSign, playerIndex, clientVersion.c_str());
  return buf;
}

std::string PlayerInfo::getIdleStat() {
  TimeKeeper now = TimeKeeper::getCurrent();
  std::string reply;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    reply = string_util::format("%-16s : %4ds", callSign,
				int(now - lastupdate));
    if (paused) {
      reply += string_util::format("  paused %4ds", int(now - pausedSince));
    }
  }
  return reply;
};

bool PlayerInfo::canBeRabbit(bool relaxing) {
  if (paused || notResponding || (team != ObserverTeam))
    return false;
  return relaxing ? (state > PlayerInLimbo) : (state == PlayerAlive);
};

void PlayerInfo::setPaused(bool _paused) {
  paused = _paused;
  pausedSince = TimeKeeper::getCurrent();
};

bool PlayerInfo::isTooMuchIdling(TimeKeeper tm, float kickThresh) {
  bool idling = false;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    int idletime = (int)(tm - lastupdate);
    int pausetime = 0;
    if (paused && tm - pausedSince > idletime)
      pausetime = (int)(tm - pausedSince);
    idletime = idletime > pausetime ? idletime : pausetime;
    if (idletime > (tm - lastmsg < kickThresh ? 3 * kickThresh : kickThresh)) {
      DEBUG1("Kicking player %s [%d] idle %d\n", callSign, playerIndex,
	     idletime);
      idling = true;
    }
  }
  return idling;
};

bool PlayerInfo::hasStartedToNotRespond() {
  float notRespondingTime = BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME);
  bool startingToNotRespond = false;
  if (state > PlayerInLimbo) {
    bool oldnr = notResponding;
    notResponding = (TimeKeeper::getCurrent() - lastupdate)
      > notRespondingTime;
    if (!oldnr && notResponding)
      startingToNotRespond = true;
  }
  return startingToNotRespond;
}

void PlayerInfo::updateLagPlayerUpdate(float timestamp, bool ooo) {
  if (ooo) {
    lostavg   = lostavg * (1 - lostalpha) + lostalpha;
    lostalpha = lostalpha / (0.99f + lostalpha);
  }
  TimeKeeper now = TimeKeeper::getCurrent();

  // don't calc jitter if more than 2 seconds between packets
  if (lasttimestamp > 0.0f && timestamp - lasttimestamp < 2.0f) {
    const float jitter = fabs(now - lastupdate - (timestamp - lasttimestamp));
    // time is smoothed exponentially using a dynamic smoothing factor
    jitteravg   = jitteravg * (1 - jitteralpha) + jitteralpha * fabs(jitter);
    jitteralpha = jitteralpha / (0.99f + jitteralpha);
    lostavg     = lostavg * (1 - lostalpha);
    lostalpha   = lostalpha / (0.99f + lostalpha);
  }
  lasttimestamp = timestamp;
  lastupdate    = now;
};

// update absolute latency based on LagPing messages
int PlayerInfo::updatePingLag(void *buf, float threshold, float max,
			      bool &warn, bool &kick) {
  uint16_t _pingseqno;
  int lag = 0;
  nboUnpackUShort(buf, _pingseqno);
  if (pingseqno == _pingseqno) {
    float timepassed = TimeKeeper::getCurrent() - lastping;
    // time is smoothed exponentially using a dynamic smoothing factor
    lagavg   = lagavg * (1 - lagalpha) + lagalpha * timepassed;
    lagalpha = lagalpha / (0.9f + lagalpha);
    lag      = int(lagavg * 1000);
    lagcount++;

    // warn players from time to time whose lag is > threshold (-lagwarn)
    if ((team != ObserverTeam) && (threshold > 0) && lagavg > threshold
	&& lagcount - laglastwarn > 2 * lagwarncount) {
      laglastwarn = lagcount;
      lagwarncount++;
      warn = true;
      kick = (lagwarncount++ > max);
    } else {
      warn = false;
      kick = false;
    }
    lostavg     = lostavg * (1 - lostalpha);
    lostalpha   = lostalpha / (0.99f + lostalpha);
    pingpending = false;
  } else {
    warn = false;
    kick = false;
  }
  return lag;
};

bool PlayerInfo::nextPing(float &waitTime) {
  TimeKeeper tm = TimeKeeper::getCurrent();
  bool shouldPing = false;
  if ((state > PlayerInLimbo) && (type == TankPlayer)
      && (nextping - tm < waitTime)) {
    waitTime   = nextping - tm;
    shouldPing = true;
  };
  return shouldPing;
};

int PlayerInfo::getNextPingSeqno() {
  TimeKeeper tm = TimeKeeper::getCurrent();
  if ((state <= PlayerInLimbo) || (type != TankPlayer) || nextping - tm >= 0)
    // no time for pinging
    return -1;

 pingseqno = (pingseqno + 1) % 10000;
  if (pingpending) {
    // ping lost
    lostavg   = lostavg * (1 - lostalpha) + lostalpha;
    lostalpha = lostalpha / (0.99f + lostalpha);
  }

  pingpending = true;
  lastping    = tm;
  nextping    = tm;
  nextping   += 10.0f;
  pingssent++;
  return pingseqno;
};

void PlayerInfo::hasSent(char message[]) {
  lastmsg = TimeKeeper::getCurrent();
  DEBUG1("Player %s [%d]: %s\n", callSign, playerIndex, message);
};

void PlayerInfo::handleFlagHistory(char message[]) {
  message[0] = 0;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    char flag[MessageLen];
    sprintf(message,"%-16s : ", callSign);
    std::vector<FlagType*>::iterator fhIt = flagHistory.begin();

    while (fhIt != flagHistory.end()) {
      FlagType * fDesc = (FlagType*)(*fhIt);
      if (fDesc->endurance == FlagNormal)
	sprintf(flag, "(*%c) ", fDesc->flagName[0]);
      else
	sprintf(flag, "(%s) ", fDesc->flagAbbv);
      strcat(message, flag);
      fhIt++;
    }
  }
};

void PlayerInfo::addFlagToHistory(FlagType* type) {
  if (flagHistory.size() >= MAX_FLAG_HISTORY)
    flagHistory.erase(flagHistory.begin());
  flagHistory.push_back(type);
};

bool PlayerInfo::hasPlayedEarly() {
  bool returnValue = playedEarly;
  playedEarly      = false;
  return returnValue;
};

void PlayerInfo::setPlayedEarly() {
  playedEarly = true;
};

void        PlayerInfo::setReplayState(PlayerReplayState state) {
  replayState = state;
}

PlayerReplayState PlayerInfo::getReplayState()
{
  return replayState;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
