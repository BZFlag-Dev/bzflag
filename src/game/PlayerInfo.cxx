/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "PlayerInfo.h"

/* system implementation headers */
#include <errno.h>
#include <stdio.h>
#include <string>
#include <ctype.h>

/* implementation-specific common headers */
#include "TextUtils.h"

WordFilter PlayerInfo::serverSpoofingFilter;
TimeKeeper PlayerInfo::now = TimeKeeper::getCurrent();

bool	PlayerInfo::callSignFiltering = false;
WordFilter *PlayerInfo::filterData	= NULL;
bool	PlayerInfo::simpleFiltering   = true;

PlayerInfo::PlayerInfo(int _playerIndex) :
  playerIndex(_playerIndex),
  state(PlayerInLimbo),
  hasDoneEntering(false),
  team(NoTeam),
  flag(-1),
  allowedToSpawn(true),
  notifiedSpawn(false),
  spamWarns(0),
  lastMsgTime(now),
  paused(false),
  pausedSince(TimeKeeper::getNullTime()),
  autopilot(false),
  tracker(0)
{
  notResponding = false;
  memset(motto, 0, MottoLen);
  memset(callSign, 0, CallSignLen);
  memset(token, 0, TokenLen);
  memset(clientVersion, 0, VersionLen);
  clientVersionMajor = -1;
  clientVersionMinor = -1;
  clientVersionRevision = -1;
  completelyAdded = false;
  nextSpawnTime = TimeKeeper::getSunGenesisTime();
  wantsToSpawn = false;
  neverSpawned = true;
  type = TankPlayer;
}

void PlayerInfo::setFilterParameters(bool	_callSignFiltering,
				     WordFilter &_filterData,
				     bool	_simpleFiltering)
{
  callSignFiltering = _callSignFiltering;
  filterData	= &_filterData;
  simpleFiltering   = _simpleFiltering;
}

void PlayerInfo::resetPlayer(bool ctf) {
  wasRabbit = false;

  lastupdate = now;
  lastmsg    = now;

  replayState = ReplayNone;

  playedEarly = false;

  restartOnBase = ctf;
}

void PlayerInfo::setRestartOnBase(bool on) {
  restartOnBase = on;
}

bool PlayerInfo::shouldRestartAtBase() {
  return restartOnBase;
}

void PlayerInfo::signingOn() {
  state = PlayerDead;
}

void PlayerInfo::setAlive() {
  state = PlayerAlive;
  paused = false;
  wantsToSpawn = false;
  neverSpawned = false;
  flag = -1;
}

void PlayerInfo::setDead() {
  state = PlayerDead;
}

void *PlayerInfo::packUpdate(void *buf) {
  buf = nboPackUShort(buf, uint16_t(type));
  buf = nboPackUShort(buf, uint16_t(team));
  return buf;
}

void *PlayerInfo::packId(void *buf) {
  buf = nboPackString(buf, callSign, CallSignLen);
  buf = nboPackString(buf, motto, MottoLen);
  return buf;
}

bool PlayerInfo::unpackEnter(const void *buf, uint16_t &rejectCode, char *rejectMsg)
{
  // data: type, team, name, motto
  uint16_t _type;
  int16_t _team;
  buf = nboUnpackUShort(buf, _type);
  buf = nboUnpackShort(buf, _team);
  type = PlayerType(_type);
  team = TeamColor(_team);
  buf = nboUnpackString(buf, callSign, CallSignLen);
  buf = nboUnpackString(buf, motto, MottoLen);
  buf = nboUnpackString(buf, token, TokenLen);
  buf = nboUnpackString(buf, clientVersion, VersionLen);

  return processEnter(rejectCode, rejectMsg);
}

bool PlayerInfo::processEnter ( uint16_t &rejectCode, char *rejectMsg )
{
  // terminate the strings
  callSign[CallSignLen - 1] = '\0';
  motto[MottoLen - 1] = '\0';
  token[TokenLen - 1] = '\0';
  clientVersion[VersionLen - 1] = '\0';
  cleanMotto();

  logDebugMessage(2,"Player %s [%d] sent version string: %s\n",
    callSign, playerIndex, clientVersion);
  int major, minor, rev;
  if (sscanf(clientVersion, "%d.%d.%d", &major, &minor, &rev) == 3) {
    clientVersionMajor = major;
    clientVersionMinor = minor;
    clientVersionRevision = rev;
  }
  logDebugMessage(4,"Player %s version code parsed as:  %i.%i.%i\n", callSign,
    clientVersionMajor, clientVersionMinor, clientVersionRevision);

  // spoof filter holds "SERVER" for robust name comparisons
  if (serverSpoofingFilter.wordCount() == 0) {
    serverSpoofingFilter.addToFilter("SERVER", "");
  }

  if (isBot() && BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    logDebugMessage(2,"rejecting robot tank: %s\n", callSign);
    rejectCode   = RejectBadType;
    strcpy(rejectMsg, "Robot tanks are not allowed on this server.");
    return false;
  }

  if (!isCallSignReadable()) {
    logDebugMessage(2,"rejecting unreadable callsign: %s\n", callSign);
    rejectCode   = RejectBadCallsign;
    strcpy(rejectMsg, errorString.c_str());
    return false;
  }
  // no spoofing the server name
  if (serverSpoofingFilter.filter(callSign)) {
    rejectCode   = RejectRepeatCallsign;
    strcpy(rejectMsg, "The callsign specified is already in use.");
    return false;
  }
  if (!isMottoReadable()) {
    logDebugMessage(2,"ignoring unreadable player motto: %s (%s)\n", callSign, motto);
    // The sendMessage() function is not available in this scope.
    // TODO: reorganize the code to send this courtesy notice to the player.
    // sendMessage(ServerPlayer, playerIndex, "Ignoring your unreadable motto.");
    setMotto("");
  }

  // make sure the callsign is not obscene/filtered
  if (callSignFiltering) {
    logDebugMessage(2,"checking callsign: %s\n",callSign);

    char cs[CallSignLen];
    memcpy(cs, callSign, sizeof(char) * CallSignLen);
    if (filterData->filter(cs, simpleFiltering)) {
      rejectCode = RejectBadCallsign;
      strcpy(rejectMsg,
	"The callsign was rejected. Try a different callsign.");
      return false;
    }
  }

  // make sure the motto is not obscene/filtered
  if (callSignFiltering) {
    logDebugMessage(2,"checking motto: %s\n", motto);
    char em[MottoLen];
    memcpy(em, motto, sizeof(char) * MottoLen);
    if (filterData->filter(em, simpleFiltering)) {
      rejectCode = RejectBadMotto;
      strcpy(rejectMsg, "The motto was rejected. Try a different motto.");
      return false;
    }
  }

  if (token[0] == 0) {
    strcpy(token, "NONE");
  }
  hasDoneEntering = true;
  return true;
}

const char *PlayerInfo::getCallSign() const {
  return callSign;
}

void PlayerInfo::setCallSign(const char * c)
{
  if (c != NULL) {
    strncpy(callSign, c, CallSignLen);
    callSign[CallSignLen - 1] = '\0';	// ensure null termination
  }
}


bool PlayerInfo::isCallSignReadable() {
  // callsign readability filter, make sure there are more alphanum than non
  // keep a count of alpha-numerics

  int callsignlen = (int)strlen(callSign);
  // reject less than 2 characters
  if (callsignlen < 2) {
    errorString = "Callsigns must be at least 2 characters.";
    return false;
  }

  // reject trailing space
  if (isspace(callSign[strlen(callSign) - 1])) {
    errorString = "Trailing spaces are not allowed in callsigns.";
    return false;
  }

  // prevent spoofing global login indicators + and @ in the scoreboard
  // and reserve # for /kick or /ban #slot
  if (*callSign=='+' || *callSign=='@' || *callSign=='#' ) {
    errorString = "Callsigns are not allowed to start with +, @, or #.";
    return false;
  }

  // start with true to reject leading space
  bool lastWasSpace = true;
  int alnumCount = 0;
  const char *sp = callSign;
  do {
    // reject sequential spaces
    if (lastWasSpace && isspace(*sp)) {
      errorString = "Leading or consecutive spaces are not allowed in callsigns.";
      return false;
    }

    // reject ' and " and any nonprintable
    if ((*sp == '\'') || (*sp == '"') || ((unsigned)*sp > 0x7f) || !isprint(*sp)) {
      errorString = "Non-printable characters and quotes are not allowed in callsigns.";
      return false;
    }
    if (isspace(*sp)) {
      // only space is valid, not tab etc.
      if (*sp != ' ') {
	      errorString = "Invalid whitespace in callsign.";
	      return false;
      }
      lastWasSpace = true;
    } else {
      lastWasSpace = false;
      if (isalnum(*sp))
	      alnumCount++;
    }
  } while (*++sp);

  bool readable = ((float)alnumCount / (float)callsignlen) > 0.5f;
  if (!readable)
    errorString = "Callsign rejected. Please use mostly letters and numbers.";
  return readable;
}

const char *PlayerInfo::getMotto() const {
  return motto;
}

void PlayerInfo::cleanMotto() {
  // strip leading whitespace from motto
  char *sp = motto;
  char *tp = sp;
  while (isspace(*sp))
    sp++;

  // strip any non-printable characters and ' and " from motto
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"')) {
      *tp++ = *sp;
    }
  } while (*++sp);
  *tp = *sp;

  // strip trailing whitespace from motto
  while (isspace(*--tp)) {
    *tp=0;
  }
}

bool PlayerInfo::isMottoReadable() {
  // motto/"team" readability filter, make sure there are more
  // alphanum than non
  int mottoAlnumCount = 0;
  char *sp = motto;
  do {
    if (isalnum(*sp)) {
      mottoAlnumCount++;
    }
  } while (*++sp);
  int mottolen = (int)strlen(motto);
  return (mottolen <= 4) || (((float)mottoAlnumCount / (float)mottolen) > 0.5);
}

void PlayerInfo::setMotto(const char* _motto) {
  strncpy(motto, _motto, MottoLen);
  motto[MottoLen - 1] = '\0';	// ensure null termination
}

const char *PlayerInfo::getToken() const {
  return token;
}

void PlayerInfo::setToken(const char * c)
{
  if (c != NULL) {
    strncpy(token, c, TokenLen);
    token[TokenLen - 1] = '\0';	// ensure null termination
  }
}

void PlayerInfo::clearToken() {
  token[0] = '\0';
}

void *PlayerInfo::packVirtualFlagCapture(void *buf) {
  buf = nboPackUShort(buf, uint16_t(int(team) - 1));
  buf = nboPackUShort(buf, uint16_t(1 + (int(team) % 4)));
  return buf;
}

bool PlayerInfo::isTeam(TeamColor _team) const {
  return team == _team;
}

bool PlayerInfo::isObserver() const {
  return team == ObserverTeam;
}

TeamColor PlayerInfo::getTeam() const {
  return team;
}

void PlayerInfo::setTeam(TeamColor _team) {
  team = _team;
}

void PlayerInfo::wasARabbit() {
  team = HunterTeam;
  wasRabbit = true;
}

void PlayerInfo::wasNotARabbit() {
  wasRabbit = false;
}

void PlayerInfo::resetFlag() {
  flag = -1;
  lastFlagDropTime = now;
}

void PlayerInfo::setFlag(int _flag) {
  flag = _flag;
}

bool PlayerInfo::isFlagTransitSafe() {
  return now - lastFlagDropTime >= 2.0f;
}

double PlayerInfo::timeSinceLastFlagDrop() {
	return now - lastFlagDropTime;
}

const char *PlayerInfo::getClientVersion() {
  return clientVersion;
}

void PlayerInfo::setClientVersion(const char * c)
{
  if (c != NULL) {
    strncpy(clientVersion, c, VersionLen);
    clientVersion[VersionLen - 1] = '\0';	// ensure null termination
  }
}

void PlayerInfo::getClientVersionNumbers(int& major, int& minor, int& rev)
{
  major = clientVersionMajor;
  minor = clientVersionMinor;
  rev = clientVersionRevision;
  return;
}

double PlayerInfo::getPausedTime()
{
  if ((state > PlayerInLimbo) && (team != ObserverTeam) && (paused)) {
    return (now - pausedSince);
  }

  return 0;
}

double PlayerInfo::getIdleTime()
{
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    return (now - lastupdate);
  }
  else if (team == ObserverTeam) {
    return -1;
  }

  return 0;
}

std::string PlayerInfo::getIdleStat() {
  std::string reply;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    reply = TextUtils::format("%s\t: %4ds", callSign,
				int(now - lastupdate));
    if (paused) {
      reply += TextUtils::format("  paused %4ds", int(now - pausedSince));
    }
  }
  return reply;
}

bool PlayerInfo::canBeRabbit(bool relaxing) {
  if (paused || notResponding || (team == ObserverTeam))
    return false;
  return relaxing ? (state > PlayerInLimbo) : (state == PlayerAlive);
}

void PlayerInfo::setPaused(bool _paused) {
  paused = _paused;
  pausedSince = now;
}

void PlayerInfo::setAutoPilot(bool _autopilot) {
  autopilot = _autopilot;
}

bool PlayerInfo::isTooMuchIdling(float kickThresh) {
  bool idling = false;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    const float idletime = (float)(now - lastupdate);
    if (idletime > kickThresh) {
      idling = true;
    }
  }
  return idling;
}

bool PlayerInfo::hasStartedToNotRespond() {
  const float notRespondingTime =
    BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME);
  bool startingToNotRespond = false;
  if (state > PlayerInLimbo) {
    bool oldnr = notResponding;
    notResponding = (now - lastupdate) > notRespondingTime;
    if (!oldnr && notResponding) {
      startingToNotRespond = true;
    }
  }
  return startingToNotRespond;
}

void PlayerInfo::hasSent() {
  lastmsg = now;
}

bool PlayerInfo::hasPlayedEarly() {
  bool returnValue = playedEarly;
  playedEarly      = false;
  return returnValue;
}

void PlayerInfo::setPlayedEarly(bool early) {
  playedEarly = early;
}

void PlayerInfo::updateIdleTime() {
  if (!paused && (state != PlayerDead)) {
    lastupdate = now;
  }
}

void	PlayerInfo::setReplayState(PlayerReplayState _state) {
  replayState = _state;
}

PlayerReplayState PlayerInfo::getReplayState()
{
  return replayState;
}


void PlayerInfo::setTrackerID(unsigned short int t)
{
  tracker = t;
}


unsigned short int PlayerInfo::trackerID()
{
  return tracker;
}

void PlayerInfo::setCurrentTime(TimeKeeper tm)
{
  now = tm;
}

void PlayerInfo::killedBy(PlayerId killer)
{
	if (deathCountMap.find(killer) == deathCountMap.end())
		deathCountMap[killer] = 0;
	deathCountMap[killer]++;
}

void PlayerInfo::flushKiller(PlayerId killer)
{
	if (deathCountMap.find(killer) != deathCountMap.end())
		deathCountMap.erase(killer);
}

int PlayerInfo::howManyTimesKilledBy(PlayerId killer)
{
	if (deathCountMap.find(killer) == deathCountMap.end())
		return 0;
	return deathCountMap[killer];
}

void PlayerInfo::setAllowedToSpawn(bool canSpawn)
{
  allowedToSpawn = canSpawn;
}

void PlayerInfo::setNotifiedOfSpawnable(bool notified)
{
  notifiedSpawn = notified;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
