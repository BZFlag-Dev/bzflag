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

#ifdef HAVE_ADNS_H
adns_state PlayerInfo::adnsState;
#endif

void PlayerInfo::initPlayer(const struct sockaddr_in& clientAddr,
			    int _playerIndex) {
  playerIndex      = _playerIndex;
  AddrLen addr_len = sizeof(clientAddr);

  // store address information for player
  memcpy(&taddr, &clientAddr, addr_len);

  state = PlayerInLimbo;
  peer = Address(taddr);
  paused = false;
#ifdef HAVE_ADNS_H
  if (adnsQuery) {
    adns_cancel(adnsQuery);
    adnsQuery = NULL;
  }
  // launch the asynchronous query to look up this hostname
  if (adns_submit_reverse
      (adnsState, (struct sockaddr *)&clientAddr,
       adns_r_ptr,
       (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose), 0,
       &adnsQuery) != 0) {
    DEBUG1("Player [%d] failed to submit reverse resolve query: errno %d\n",
	   playerIndex, getErrno());
    adnsQuery = NULL;
  } else {
    DEBUG2("Player [%d] submitted reverse resolve query\n", playerIndex);
  }
#endif

  pausedSince = TimeKeeper::getNullTime();
};

void PlayerInfo::resetPlayer(bool ctf) {
  wasRabbit = false;
  Admin = false;

  passwordAttempts = 0;

  regName = callSign;
  makeupper(regName);

  accessInfo.explicitAllows.reset();
  accessInfo.explicitDenys.reset();
  accessInfo.verified = false;
  accessInfo.loginTime = TimeKeeper::getCurrent();
  accessInfo.loginAttempts = 0;
  accessInfo.groups.clear();
  accessInfo.groups.push_back("DEFAULT");

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

#ifdef TIMELIMIT
  playedEarly = false;
#endif

  restartOnBase = ctf;
}

bool PlayerInfo::isAccessVerified() const {
  return accessInfo.verified;
}

bool PlayerInfo::removePlayer() {

  bool wasPlaying = state > PlayerInLimbo;

  accessInfo.verified      = false;
  accessInfo.loginAttempts = 0;
  regName.empty();

  delayq.dequeuePackets();

  callSign[0] = 0;

  flagHistory.clear();

#ifdef HAVE_ADNS_H
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
#endif

  state = PlayerNoExist;

  return wasPlaying;
}

bool PlayerInfo::gotAccessFailure() {
  return accessInfo.loginAttempts >= 5;
}

void PlayerInfo::setLoginFail() {
  accessInfo.loginAttempts++;
}

void PlayerInfo::setPermissionRights() {
  accessInfo.verified = true;
  // get their real info
  PlayerAccessInfo &info = getUserInfo(regName);
  accessInfo.explicitAllows = info.explicitAllows;
  accessInfo.explicitDenys = info.explicitDenys;
  accessInfo.groups = info.groups;
  DEBUG1("Identify %s\n", regName.c_str());
}

void PlayerInfo::reloadInfo() {
  if (accessInfo.verified && userExists(regName)) {
    accessInfo = getUserInfo(regName);
    accessInfo.verified = true;
  }
}

bool PlayerInfo::hasSetGroupPermission(const std::string& group) {
  return hasGroup(accessInfo, group);
}

bool PlayerInfo::hasPermission(PlayerAccessInfo::AccessPerm right) {
  return Admin || hasPerm(accessInfo, right);
}

void PlayerInfo::setGroup(const std::string& group) {
  addGroup(accessInfo, group);
}

void PlayerInfo::resetGroup(const std::string& group) {
  removeGroup(accessInfo, group);
}

void PlayerInfo::setAdmin() {
  passwordAttempts = 0;
  Admin = true;
}

void PlayerInfo::setRestartOnBase(bool on) {
  restartOnBase = on;
};

bool PlayerInfo::shouldRestartAtBase() {
  return restartOnBase;
};

std::string PlayerInfo::getName() {
  return regName;
};

bool PlayerInfo::isPasswordMatching(const char* pwd) {
  return verifyUserPassword(regName.c_str(), pwd);
};

bool PlayerInfo::isRegistered() const {
  return userExists(regName);
};

bool PlayerInfo::isExisting() {
  return exist() && userExists(regName);
};

bool PlayerInfo::isIdentifyRequired() {
  return hasPerm(getUserInfo(regName), PlayerAccessInfo::requireIdentify);
};

bool PlayerInfo::isAllowedToEnter() {
  return accessInfo.verified || !isRegistered() || !isIdentifyRequired();
};

void PlayerInfo::storeInfo(const char* pwd) {
  PlayerAccessInfo info;
  info.groups.push_back("DEFAULT");
  info.groups.push_back("REGISTERED");
  std::string pass = pwd;
  setUserPassword(regName.c_str(), pass.c_str());
  setUserInfo(regName, info);
  DEBUG1("Register %s %s\n",regName.c_str(), pwd);
  updateDatabases();
}

void PlayerInfo::setPassword(const std::string&  pwd) {
  setUserPassword(regName.c_str(), pwd.c_str());
  updateDatabases();
}

uint8_t PlayerInfo::getPlayerProperties() {
  uint8_t result = 0;
  if (isRegistered())
    result |= IsRegistered;
  if (accessInfo.verified)
    result |= IsIdentified;
  if (Admin)
    result |= IsAdmin;
  return result;
};

void PlayerInfo::resetComm() {
    state = PlayerNoExist;
    delayq.init();
#ifdef HAVE_ADNS_H
    hostname = NULL;
    adnsQuery = NULL;
#endif  
};

const char *PlayerInfo::getTargetIP() {
  return peer.getDotNotation().c_str();
};

int PlayerInfo::sizeOfIP() {
   return peer.getIPVersion() == 4 ? 8 : 20; // 8 for IPv4, 20 for IPv6
};

void *PlayerInfo::packAdminInfo(void *buf) {
  buf = nboPackUByte(buf, sizeOfIP());
  buf = nboPackUByte(buf, playerIndex);
  buf = nboPackUByte(buf, getPlayerProperties());
  buf = peer.pack(buf);
  return buf;
};

void PlayerInfo::debugUnknownPacket(int code) {
  DEBUG1("Player [%d] sent unknown packet type (%x), \
possible attack from %s\n",
	 playerIndex, code, peer.getDotNotation().c_str());
};

bool PlayerInfo::isAtIP(const std::string& IP) {
  return strcmp(peer.getDotNotation().c_str(), IP.c_str()) == 0;
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
    sprintf(msg,"%-16s : %3d +- %2dms %s", callSign,
	    int(lagavg * 1000),
	    int(jitteravg * 1000),
	    accessInfo.verified ? "(R)" : "");
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

in_addr PlayerInfo::getIPAddress() {
  return taddr.sin_addr;
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
      if (lagwarncount++ > max) {
	kick = true;
      } else {
	kick = true;
      }
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

bool PlayerInfo::passwordAttemptsMax() {
  bool maxAttempts = passwordAttempts >= 5;
  // see how many times they have tried, you only get 5
  if (maxAttempts) {
    DEBUG1("%s (%s) has attempted too many /password tries\n",
	   callSign, peer.getDotNotation().c_str());
  } else {
    passwordAttempts++;
  }
  return maxAttempts;
};

#ifdef HAVE_ADNS_H
// return true if host is resolved
bool PlayerInfo::checkDNSResolution() {
  if (!adnsQuery)
    return false;

  // check to see if query has completed
  adns_answer *answer;
  if (adns_check(adnsState, &adnsQuery, &answer, 0) != 0) {
    if (getErrno() != EAGAIN) {
      DEBUG1("Player [%d] failed to resolve: errno %d\n", playerIndex,
	     getErrno());
      adnsQuery = NULL;
    }
    return false;
  }

  // we got our reply.
  if (answer->status != adns_s_ok) {
    DEBUG1("Player [%d] got bad status from resolver: %s\n", playerIndex,
	   adns_strerror(answer->status));
    free(answer);
    adnsQuery = NULL;
    return false;
  }

  if (hostname)
    free(hostname); // shouldn't happen, but just in case
  hostname = strdup(*answer->rrs.str);
  DEBUG1("Player [%d] resolved to hostname: %s\n", playerIndex, hostname);
  free(answer);
  adnsQuery = NULL;
  return true;
}

const char *PlayerInfo::getHostname() {
  return hostname;
}

void PlayerInfo::startupResolver() {
  /* start up our resolver if we have ADNS */
  if (adns_init(&adnsState, adns_if_nosigpipe, 0) < 0) {
    perror("ADNS init failed");
    exit(1);
  }
}
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
