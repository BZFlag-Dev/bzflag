/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "TimeKeeper.h"

/* system implementation headers */
#include <errno.h>
#include <stdio.h>
#include <string>
#include <ctype.h>

/* implementation-specific common headers */
#include "TextUtils.h"

WordFilter PlayerInfo::serverSpoofingFilter;
TimeKeeper PlayerInfo::now = TimeKeeper::getCurrent();

bool PlayerInfo::callSignFiltering = false;
WordFilter *PlayerInfo::filterData = NULL;
bool PlayerInfo::simpleFiltering = true;


PlayerInfo::PlayerInfo(int _playerIndex) :
  state(PlayerInLimbo), playerIndex(_playerIndex), hasDoneEntering(false),
  type(TankPlayer), updates(AllUpdates), team(NoTeam), flag(-1), spamWarns(0), lastMsgTime(now),
  paused(false), pausedSince(TimeKeeper::getNullTime()), autopilot(false),
  tracker(0)
{
  notResponding = false;
  memset(callSign, 0, CallSignLen);
  memset(token, 0, TokenLen);
  memset(clientVersion, 0, VersionLen);
  memset(referrer, 0, ReferrerLen);
  pauseRequestLag = 0;
  pauseRequestTime = TimeKeeper::getNullTime();
  allowChangeTime = TimeKeeper::getNullTime();
  jumpStartPos = 0.0f;
  allowedHeightAtJumpStart = -1.0f;
}

void PlayerInfo::setFilterParameters(bool	_callSignFiltering,
				     WordFilter &_filterData,
				     bool	_simpleFiltering)
{
  callSignFiltering = _callSignFiltering;
  filterData	= &_filterData;
  simpleFiltering   = _simpleFiltering;
}

void PlayerInfo::resetPlayer(bool ctf)
{
  wasRabbit = false;

  lastupdate = now;
  lastmsg    = now;

  replayState = ReplayNone;

  playedEarly = false;

  restartOnBase = ctf;
}

void PlayerInfo::setRestartOnBase(bool on)
{
  restartOnBase = on;
}

bool PlayerInfo::shouldRestartAtBase()
{
  return restartOnBase;
}

void PlayerInfo::signingOn()
{
  state = PlayerDead;
}

void PlayerInfo::setAlive()
{
  state = PlayerAlive;
  paused = false;
  flag = -1;
}

void PlayerInfo::setDead()
{
  state = PlayerDead;
}

void *PlayerInfo::packUpdate(void *buf)
{
  buf = nboPackUInt16(buf, uint16_t(type));
  buf = nboPackUInt16(buf, uint16_t(team));
  return buf;
}

void PlayerInfo::packUpdate(BufferedNetworkMessage *msg)
{
  msg->packUInt16(uint16_t(type));
  msg->packUInt16(uint16_t(team));
}

void *PlayerInfo::packId(void *buf)
{
  buf = nboPackString(buf, callSign, CallSignLen);
  return buf;
}

void PlayerInfo::packId(BufferedNetworkMessage *msg)
{
  msg->packString(callSign, CallSignLen);
}

void PlayerInfo::setCallsign(const char *text)
{
  if (!text)
    return;

  memset(callSign, 0, CallSignLen);
  strncpy(callSign, text, CallSignLen - 1);
}

void PlayerInfo::setToken(const char *text)
{
  if (!text)
    return;

  memset(token, 0, TokenLen);
  strncpy(token, text, TokenLen - 1);
}

void PlayerInfo::setClientVersion(const char *text)
{
  if (!text)
    return;

  memset(clientVersion, 0, VersionLen);
  strncpy(clientVersion, text, VersionLen - 1);
}

void PlayerInfo::setType (PlayerType playerType)
{
  type = playerType;
}

void PlayerInfo::setUpdates (NetworkUpdates whichUpdates)
{
  updates = whichUpdates;
}

bool PlayerInfo::processEnter (uint16_t &rejectCode, char *rejectMsg)
{
  // terminate the strings
  callSign[CallSignLen - 1] = '\0';
  token[TokenLen - 1] = '\0';
  clientVersion[VersionLen - 1] = '\0';
  referrer[ReferrerLen - 1] = '\0';

  logDebugMessage(2, "Player %s [%d] sent version string: %s\n",
		  callSign, playerIndex, clientVersion);

  // spoof filter holds "SERVER" for robust name comparisons
  if (serverSpoofingFilter.wordCount() == 0) {
    serverSpoofingFilter.addToFilter("SERVER", "");
  }

  if (!isCallSignReadable()) {
    logDebugMessage(2, "rejecting unreadable callsign: %s\n", callSign);
    rejectCode   = RejectBadCallsign;
    strncpy(rejectMsg, errorString.c_str(), MessageLen);
    return false;
  }
  // no spoofing the server name
  if (serverSpoofingFilter.filter(callSign)) {
    rejectCode   = RejectRepeatCallsign;
    strncpy(rejectMsg, "The callsign specified is already in use.", MessageLen);
    return false;
  }

  // make sure the callsign is not obscene/filtered
  if (callSignFiltering) {
    logDebugMessage(2, "checking callsign: %s\n", callSign);

    char cs[CallSignLen];
    memcpy(cs, callSign, sizeof(char) * CallSignLen);
    if (filterData->filter(cs, simpleFiltering)) {
      rejectCode = RejectBadCallsign;
      strncpy(rejectMsg,
	      "The callsign was rejected. Try a different callsign.", MessageLen);
      return false;
    }
  }

  if (token[0] == 0) {
    strcpy(token, "NONE");
  }
  hasDoneEntering = true;

  return true;
}

bool PlayerInfo::unpackEnter(void *buf, uint16_t &rejectCode, char *rejectMsg)
{
  // data: type, team, name,
  uint16_t _type;
  uint16_t _updates;
  int16_t _team;
  buf = nboUnpackUInt16(buf, _type);
  buf = nboUnpackUInt16(buf, _updates);
  buf = nboUnpackInt16(buf, _team);
  type = PlayerType(_type);
  updates = NetworkUpdates(_updates);
  team = TeamColor(_team);
  buf = nboUnpackString(buf, callSign, CallSignLen);
  buf = nboUnpackString(buf, token, TokenLen);
  buf = nboUnpackString(buf, clientVersion, VersionLen);
  buf = nboUnpackString(buf, referrer, ReferrerLen);

  return processEnter(rejectCode, rejectMsg);
}

const char *PlayerInfo::getCallSign() const
{
  return callSign;
}

bool PlayerInfo::isCallSignReadable()
{
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

  // prevent spoofing global login indicators + and @ in the scoreboard,
  // reserve > for /msg >admin and /msg >team,
  // and reserve # for /kick or /ban #slot
  if (*callSign=='+' || *callSign=='@' || *callSign=='>' || *callSign=='#') {
    errorString = "Callsigns are not allowed to start with +, @, > or #.";
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

const char *PlayerInfo::getToken() const
{
  return token;
}

const char *PlayerInfo::getReferrer() const
{
  return referrer;
}

void PlayerInfo::clearToken()
{
  token[0] = '\0';
}

void PlayerInfo::clearReferrer()
{
  referrer[0] = '\0';
}

void *PlayerInfo::packVirtualFlagCapture(void *buf)
{
  buf = nboPackUInt16(buf, uint16_t(int(team) - 1));
  buf = nboPackUInt16(buf, uint16_t(1 + (int(team) % 4)));
  return buf;
}

bool PlayerInfo::isTeam(TeamColor _team) const
{
  return team == _team;
}

bool PlayerInfo::isObserver() const
{
  return team == ObserverTeam;
}

TeamColor PlayerInfo::getTeam() const
{
  return team;
}

void PlayerInfo::setTeam(TeamColor _team)
{
  team = _team;
}

void PlayerInfo::wasARabbit()
{
  team = HunterTeam;
  wasRabbit = true;
}

void PlayerInfo::wasNotARabbit()
{
  wasRabbit = false;
}

void PlayerInfo::resetFlag()
{
  flag = -1;
  lastFlagDropTime = now;
}

void PlayerInfo::setFlag(int _flag)
{
  flag = _flag;
}

bool PlayerInfo::isFlagTransitSafe()
{
  return now - lastFlagDropTime >= 2.0f;
}

const char *PlayerInfo::getClientVersion()
{
  return clientVersion;
}

std::string PlayerInfo::getIdleStat()
{
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

bool PlayerInfo::canBeRabbit(bool relaxing)
{
  if (paused || notResponding || (team == ObserverTeam))
    return false;
  return relaxing ? (state > PlayerInLimbo) : (state == PlayerAlive);
}

void PlayerInfo::setPaused(bool _paused)
{
  paused = _paused;
  pausedSince = now;
}

void PlayerInfo::setAutoPilot(bool _autopilot)
{
  autopilot = _autopilot;
}

bool PlayerInfo::isTooMuchIdling(float kickThresh)
{
  bool idling = false;
  if ((state > PlayerInLimbo) && (team != ObserverTeam)) {
    const float idletime = (float)(now - lastupdate);
    if (idletime > kickThresh) {
      idling = true;
    }
  }
  return idling;
}

bool PlayerInfo::hasStartedToNotRespond()
{
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

void PlayerInfo::hasSent()
{
  lastmsg = now;
}

bool PlayerInfo::hasPlayedEarly()
{
  bool returnValue = playedEarly;
  playedEarly      = false;
  return returnValue;
}

void PlayerInfo::setPlayedEarly(bool early)
{
  playedEarly = early;
}

void PlayerInfo::updateIdleTime()
{
  if (!paused && (state != PlayerDead)) {
    lastupdate = now;
  }
}

void	PlayerInfo::setReplayState(PlayerReplayState _state)
{
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
