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

void PlayerInfo::initPlayer(const struct sockaddr_in& clientAddr, int _fd,
			    int _playerIndex) {
  playerIndex      = _playerIndex;
  AddrLen addr_len = sizeof(clientAddr);

  // store address information for player
  memcpy(&taddr, &clientAddr, addr_len);
  memcpy(&uaddr, &clientAddr, addr_len);

  // update player state
  time = TimeKeeper::getCurrent();
  fd = _fd;
  state = PlayerInLimbo;
  peer = Address(taddr);
  tcplen = 0;
  assert(outmsg == NULL);
  outmsgSize = 0;
  outmsgOffset = 0;
  outmsgCapacity = 0;
  paused = false;
#ifdef HAVE_ADNS_H
  if (adnsQuery) {
    adns_cancel(adnsQuery);
    adnsQuery = NULL;
  }
#endif
};

void PlayerInfo::resetPlayer(bool ctf) {
  wasRabbit = false;
  toBeKicked = false;
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
  lastRecvPacketNo = 0;
  lastSendPacketNo = 0;

  uqueue = NULL;
  dqueue = NULL;

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

bool PlayerInfo::removePlayer(const char *reason) {

  // check if we are called again for a dropped player!
  if (fd == -1)
    return false;

  bool wasPlaying = state > PlayerInLimbo;

  // status message
  DEBUG1("Player %s [%d] on %d removed: %s\n", callSign, playerIndex, fd,
	 reason);

  // shutdown TCP socket
  shutdown(fd, 2);
  close(fd);

  fd = -1;
  accessInfo.verified      = false;
  accessInfo.loginAttempts = 0;
  regName.empty();

  uqueue = NULL;
  dqueue = NULL;
  delayq.dequeuePackets();
  lastRecvPacketNo = 0;
  lastSendPacketNo = 0;
  // shutdown the UDP socket
  memset(&uaddr, 0, sizeof(uaddr));

  // no UDP connection anymore
  udpin = false;
  udpout = false;
  toBeKicked = false;

  tcplen = 0;

  callSign[0] = 0;

  if (outmsg != NULL) {
    delete[] outmsg;
    outmsg = NULL;
  }
  outmsgSize = 0;

  flagHistory.clear();

#ifdef HAVE_ADNS_H
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
#endif

  state = PlayerNoExist;

#ifdef NETWORK_STATS
  if (wasPlaying) {
    dumpMessageStats();
  }
#endif

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
  return fd != -1 && userExists(regName);
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

void PlayerInfo::initStatistics() {

  pausedSince = TimeKeeper::getNullTime();
#ifdef NETWORK_STATS
  int i;
  struct MessageCount *statMsg;
  int direction;

  for (direction = 0; direction <= 1; direction++) {
    statMsg = msg[direction];
    for (i = 0; i < MessageTypes && statMsg[i].code != 0; i++) {
      statMsg[i].count = 0;
      statMsg[i].code = 0;
    }
    msgBytes[direction] = 0;
    perSecondTime[direction] = time;
    perSecondCurrentMsg[direction] = 0;
    perSecondMaxMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
    perSecondMaxBytes[direction] = 0;
  }
#endif
};

#ifdef NETWORK_STATS
void PlayerInfo::dumpMessageStats() {
  int i;
  struct MessageCount *msgStats;
  int total;
  int direction;

  DEBUG1("Player connect time: %f\n", TimeKeeper::getCurrent() - time);
  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    DEBUG1("Player messages %s:", direction ? "out" : "in");
    msgStats = msg[direction];
    for (i = 0; i < MessageTypes && msgStats[i].code != 0; i++) {
      DEBUG1(" %c%c:%u(%u)", msgStats[i].code >> 8, msgStats[i].code & 0xff,
	     msgStats[i].count, msgStats[i].maxSize);
      total += msgStats[i].count;
    }
    DEBUG1(" total:%u(%u) ", total, msgBytes[direction]);
    DEBUG1("max msgs/bytes per second: %u/%u\n",
	perSecondMaxMsg[direction],
	perSecondMaxBytes[direction]);
  }
  fflush(stdout);
}
#endif

bool PlayerInfo::isConnected() {
  return fd != -1;
};

int PlayerInfo::send(const void *buffer, size_t length) {
  int n;

  n = ::send(fd, (const char *)buffer, length, 0);
  if (n >= 0) 
    return n;

  // get error code
  const int err = getErrno();

  // if socket is closed then give up
  if (err == ECONNRESET || err == EPIPE) {
    return -1;
  }

  // just try again later if it's one of these errors
  if (err != EAGAIN && err != EINTR) {
    // dump other errors and remove the player
    nerror("error on write");
    toBeKicked = true;
    toBeKickedReason = "Write error";
  }
  return 0;
};

int PlayerInfo::bufferedSend(int playerIndex,
			     const void *buffer,
			     size_t length) {
  if (fd == -1)
    return 0;

  // try flushing buffered data
  if (outmsgSize != 0) {
    const int n = send(outmsg + outmsgOffset, outmsgSize);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      outmsgOffset += n;
      outmsgSize   -= n;
    }
  }
  // if the buffer is empty try writing the data immediately
  if ((outmsgSize == 0) && length > 0) {
    const int n = send(buffer, length);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      buffer  = (void*)(((const char*)buffer) + n);
      length -= n;
    }
  }
  // write leftover data to the buffer
  if (fd != -1 && length > 0) {
    // is there enough room in buffer?
    if (outmsgCapacity < outmsgSize + (int)length) {
      // double capacity until it's big enough
      int newCapacity = (outmsgCapacity == 0) ? 512 : outmsgCapacity;
      while (newCapacity < outmsgSize + (int)length)
	newCapacity <<= 1;

      // if the buffer is getting too big then drop the player.  chances
      // are the network is down or too unreliable to that player.
      // FIXME -- is 20kB too big?  too small?
      if (newCapacity >= 20 * 1024) {
	DEBUG2("Player %s [%d] drop, unresponsive with %d bytes queued\n",
	       callSign, playerIndex, outmsgSize + length);
	toBeKicked = true;
	toBeKickedReason = "send queue too big";
	return 0;
      }

      // allocate memory
      char *newbuf = new char[newCapacity];

      // copy old data over
      memmove(newbuf, outmsg + outmsgOffset, outmsgSize);

      // cutover
      delete[] outmsg;
      outmsg	       = newbuf;
      outmsgOffset   = 0;
      outmsgCapacity = newCapacity;
    }

    // if we can't fit new data at the end of the buffer then move existing
    // data to head of buffer
    // FIXME -- use a ring buffer to avoid moving memory
    if (outmsgOffset + outmsgSize + (int)length > outmsgCapacity) {
      memmove(outmsg, outmsg + outmsgOffset, outmsgSize);
      outmsgOffset = 0;
    }

    // append data
    memmove(outmsg + outmsgOffset + outmsgSize, buffer, length);
    outmsgSize += length;
  }
  return 0;
};

void PlayerInfo::udpSend(int udpSocket, const void *b, size_t l) {
#ifdef TESTLINK
  if ((random()%LINKQUALITY) == 0) {
    DEBUG1("Drop Packet due to Test\n");
    return;
  }
#endif
  sendto(udpSocket, (const char *)b, l, 0, (struct sockaddr*)&uaddr,
	 sizeof(uaddr));
};

void PlayerInfo::setUdpOut() {
  udpout = true;
  DEBUG2("Player %s [%d] outbound UDP up\n", callSign, playerIndex);
};

bool PlayerInfo::isMyUdpAddrPort(struct sockaddr_in &_uaddr) {
  return udpin && (uaddr.sin_port == _uaddr.sin_port) &&
    (memcmp(&uaddr.sin_addr, &_uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0);
};

bool PlayerInfo::setUdpIn(struct sockaddr_in &_uaddr) {
  if (udpin)
    return false;

  bool same = !memcmp(&uaddr.sin_addr,
		      &_uaddr.sin_addr,
		      sizeof(uaddr.sin_addr));
  if (same) {
    DEBUG2("Player %s [%d] inbound UDP up %s:%d actual %d\n",
	   callSign, playerIndex, inet_ntoa(uaddr.sin_addr),
	   ntohs(uaddr.sin_port), ntohs(_uaddr.sin_port));
    if (_uaddr.sin_port) {
      uaddr.sin_port = _uaddr.sin_port;
      udpin = true;

      // init the queues
      uqueue = dqueue = NULL;
      lastRecvPacketNo = lastSendPacketNo = 0;
    }
  } else {
    DEBUG2
      ("Player %s [%d] inbound UDP rejected %s:%d different IP than %s:%d\n",
       callSign, playerIndex, inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port),
       inet_ntoa(_uaddr.sin_addr), ntohs(_uaddr.sin_port));
  }
  return same;
};

RxStatus PlayerInfo::receive(size_t length) {
  RxStatus returnValue;
  if (fd == -1)
    return ReadPart;
  if ((int)length <= tcplen)
    return ReadAll;
  int size = recv(fd, tcpmsg + tcplen, length - tcplen, 0);
  if (size > 0) {
    tcplen += size;
    if (tcplen == (int)length)
      returnValue = ReadAll;
    else
      returnValue = ReadPart;
  } else if (size < 0) {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      returnValue = ReadPart;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      returnValue = ReadReset;
    } else {
      returnValue = ReadError;
    }
  } else { // if (size == 0)
    returnValue = ReadDiscon;
  }
  return returnValue;
};

void PlayerInfo::resetComm() {
    fd = -1;
    state = PlayerNoExist;
    delayq.init();
    outmsg = NULL;
    outmsgSize = 0;
    outmsgOffset = 0;
    outmsgCapacity = 0;
#ifdef HAVE_ADNS_H
    hostname = NULL;
    adnsQuery = NULL;
#endif  
};

void PlayerInfo::closeComm() {
  if (fd != -1) {
    shutdown(fd, 2);
    close(fd);
    delete [] outmsg;
  }
};

void PlayerInfo::dropUnconnected() {
  if (fd == -1)
    state = PlayerNoExist;
};

void PlayerInfo::debugUdpInfo() {
  if (fd != -1) {
    DEBUG3(" %d(%d-%d) %s:%d", playerIndex, udpin, udpout,
	   inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
  }
};

void PlayerInfo::debugUdpRead(int n,
			      struct sockaddr_in &_uaddr, int udpSocket) {
  DEBUG4("Player %s [%d] uread() %s:%d len %d from %s:%d on %i\n",
     callSign, playerIndex, inet_ntoa(uaddr.sin_addr),
      ntohs(uaddr.sin_port), n, inet_ntoa(_uaddr.sin_addr),
      ntohs(_uaddr.sin_port), udpSocket);
};

void PlayerInfo::debugAdd() {
  DEBUG1("Player %s [%d] has joined from %s:%d on %i\n",
	 callSign, playerIndex, inet_ntoa(taddr.sin_addr),
	 ntohs(taddr.sin_port), fd);
};

void PlayerInfo::fdSet(fd_set *read_set, fd_set *write_set, int &maxFile) {
  if (fd != -1) {
    _FD_SET(fd, read_set);

    if (outmsgSize > 0)
      _FD_SET(fd, write_set);
    if (fd > maxFile)
      maxFile = fd;
  }
};

int PlayerInfo::fdIsSet(fd_set *set) {
  return FD_ISSET(fd, set);
}

void PlayerInfo::getPlayerList(char *list) {
  sprintf(list, "[%d]%-16s: %s%s%s%s%s%s", playerIndex, callSign,
	  peer.getDotNotation().c_str(),
#ifdef HAVE_ADNS_H
	  hostname ? " (" : "",
	  hostname ? hostname : "",
	  hostname ? ")" : "",
#else
	  "", "", "",
#endif
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
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

void PlayerInfo::debugHugePacket(int length) {
  DEBUG1("Player [%d] sent huge packet length (len=%d), \
possible attack from %s\n",
	 playerIndex, length, peer.getDotNotation().c_str());
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

void PlayerInfo::udpFillRead(void *buf, int len) {
  if (len >= 1024)
    len = 1023;
  memcpy(udpmsg, buf, len);
};

void *PlayerInfo::getUdpBuffer() {
  return udpmsg;
};

void *PlayerInfo::getTcpBuffer() {
  return tcpmsg;
};

void PlayerInfo::cleanTcp() {
  tcplen = 0;
};

in_addr PlayerInfo::getIPAddress() {
  return taddr.sin_addr;
};

int PlayerInfo::pwrite(const void *b, int l, int udpSocket) {

  if (fd == -1 || l == 0) {
    return 0;
  }
  
  void *buf = (void *)b;
  uint16_t len, code;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
#ifdef NETWORK_STATS
  countMessage(code, len, 1);
#endif

  // Check if UDP Link is used instead of TCP, if so jump into udpSend
  if (udpout) {
    // only send bulk messages by UDP
    switch (code) {
    case MsgShotBegin:
    case MsgShotEnd:
    case MsgPlayerUpdate:
    case MsgGMUpdate:
    case MsgLagPing:
      udpSend(udpSocket, b, l);
      return 0;
    }
  }

  // always sent MsgUDPLinkRequest over udp with udpSend
  if (code == MsgUDPLinkRequest) {
    udpSend(udpSocket, b, l);
    return 0;
  }

  return bufferedSend(playerIndex, b, l);
};

int PlayerInfo::pflush(fd_set *set) {
  if ((fd != -1) && FD_ISSET(fd, set))
    return bufferedSend(playerIndex, NULL, 0);
  else
    return 0;
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

std::string PlayerInfo::reasonToKick() {
  std::string reason;
  if (toBeKicked) {
    reason = toBeKickedReason;
  }
  toBeKicked = false;
  return reason;
};

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

#ifdef NETWORK_STATS
void PlayerInfo::countMessage(uint16_t code, int len, int direction) {
  // add length of type and length
  len += 4;

  msgBytes[direction] += len;

  int i;
  struct MessageCount *msg1;
  msg1 = msg[direction];
  for (i = 0; i < MessageTypes && msg1[i].code != 0; i++)
    if (msg1[i].code == code)
      break;
  msg1[i].code = code;
  if (msg1[i].maxSize < len)
    msg1[i].maxSize = len;
  msg1[i].count++;

  TimeKeeper now = TimeKeeper::getCurrent();
  if (now - perSecondTime[direction] < 1.0f) {
    perSecondCurrentMsg[direction]++;
    perSecondCurrentBytes[direction] += len;
  } else {
    perSecondTime[direction] = now;
    if (perSecondMaxMsg[direction] < perSecondCurrentMsg[direction])
      perSecondMaxMsg[direction] = perSecondCurrentMsg[direction];
    if (perSecondMaxBytes[direction] < perSecondCurrentBytes[direction])
      perSecondMaxBytes[direction] = perSecondCurrentBytes[direction];
    perSecondCurrentMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
  }
};
#endif

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
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
