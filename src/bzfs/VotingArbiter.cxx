/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* no header other than VotingArbiter.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "VotingArbiter.h"


/* private */

/* protected */

void VotingArbiter::updatePollers(void)
{
  while (!_pollers.empty()) {
    poller_t& p = _pollers.front();
    if ((TimeKeeper::getCurrent() - p.lastRequest) > _voteRepeatTime) {
      // remove pollers that expired their repoll timeout
      _pollers.pop_front();
    } else {
      break;
    }
  }
  return;
}

bool VotingArbiter::isPollerWaiting(std::string name) const
{
  for (unsigned int i = 0; i < _pollers.size(); i++) {
    if (_pollers[i].name == name) {
      return true;
    }
  }
  return false;
}


/* public */

bool VotingArbiter::forgetPoll(void) 
{
  if (_votingBooth != NULL) {
    delete _votingBooth;
    _votingBooth = NULL;
  }
  _startTime = TimeKeeper::getNullTime();
  _pollee = "nobody";
  _action = UNDEFINED;
  _pollRequestor = "nobody";
  return true;
}

bool VotingArbiter::poll(std::string player, std::string playerRequesting, pollAction_t action) 
{
  poller_t p;
  char message[256];
  bool tooSoon;
  
  // you have to forget the current poll before another can be spawned
  if (this->isPollOpen()) {
    std::cout << "/poll was called and a poll is already open?" << std::endl;
    return false;
  }

  // update the poller list (people on the pollers list cannot initiate a poll
  updatePollers();

  // see if the poller is in the list
  tooSoon = isPollerWaiting(playerRequesting);
  if (tooSoon) {
    std::cout << "/poll was called and the requesting poller already asked?? [" << playerRequesting << "]" << std::endl;
    return false;
  }
  
  // add this poller to the end list
  p.name = playerRequesting;
  p.lastRequest = TimeKeeper::getCurrent();
  _pollers.push_back(p);

  // create the booth to record votes
#ifdef _WIN32
  _snprintf(message, 256,  "%s %s", action == POLL_KICK_PLAYER ? "kick" : "ban", player.c_str());
#else
  snprintf(message, 256,  "%s %s", action == POLL_KICK_PLAYER ? "kick" : "ban", player.c_str());
#endif
  if (_votingBooth != NULL) {
    delete _votingBooth;
  }
  _votingBooth = YesNoVotingBooth(message);
  _pollee = player;
  _action = action;
  _pollRequestor = playerRequesting;

  // set timers
  _startTime = TimeKeeper::getCurrent();

  return true;
}

bool VotingArbiter::pollToKick(std::string player, std::string playerRequesting)
{
  return (this->poll(player, playerRequesting, POLL_KICK_PLAYER));
}

bool VotingArbiter::pollToBan(std::string player, std::string playerRequesting)
{
  return (this->poll(player, playerRequesting, POLL_BAN_PLAYER));
}

bool VotingArbiter::closePoll(void)
{
  if (this->isPollClosed()) {
    return true;
  }
  _startTime = TimeKeeper::getSunExplodeTime();
  return true;
}
      
bool VotingArbiter::setAvailableVoters(unsigned short int count) 
{
  _maxVotes = count;
  return true;
}

bool VotingArbiter::allowSuffrage(std::string player) const
{
  // is there a poll to vote on?
  if (!this->isPollOpen()) {
    return false;
  }

  // has this player already voted?
  if (_votingBooth->hasVoted(player)) {
    return false;
  }

  if (_votingBooth->getTotalVotes() >= _maxVotes) {
    return false;
  }
  
  return true;
}

bool VotingArbiter::voteYes(std::string player) 
{
  if (!this->knowsPoll() || this->isPollClosed()) {
    return false;
  }

  if (_votingBooth->getTotalVotes() >= _maxVotes) {
    return false;
  }

  return (_votingBooth->vote(player, 1));
}

bool VotingArbiter::voteNo(std::string player) 
{
  if (!this->knowsPoll() || this->isPollClosed()) {
    return false;
  }

  if (_votingBooth->getTotalVotes() >= _maxVotes) {
    return false;
  }

  return (_votingBooth->vote(player, 0));
}

bool VotingArbiter::isPollSuccessful(void) const 
{
  if (!this-knowsPoll()) {
    return false;
  }
  unsigned long int votes = _votingBooth->getVoteCount(1);

  //ensure minimum votage
  if (votes < _votesRequired) {
    return false;
  }

  // were there enough votes?
  if (((double)votes * (double)100.0 / (double)_maxVotes) > (double)_votePercentage) {
    return true;
  }
  
  return false;
}

unsigned long int VotingArbiter::timeRemaining(void) const
{
  if (_votingBooth == NULL) {
    return 0;
  }

  // if the poll is successful early, terminate the clock
  if (this->isPollSuccessful()) {
    return 0;
  }
  
  float remaining = _voteTime - (TimeKeeper::getCurrent() - _startTime);
  if (remaining < 0.0f) {
    return 0;
  }
  return (unsigned int)remaining;
}


// ex: shiftwidth=2 tabstop=8
