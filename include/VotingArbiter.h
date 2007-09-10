/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __VOTINGARBITER_H__
#define __VOTINGARBITER_H__

#include "common.h"
#include <iostream>
#include <deque>
#include <string>

#include "VotingBooth.h"
#include "TimeKeeper.h"


/** VotingArbiter is a means to manage and enforce a poll.  The poll will
 * maintain a voting booth when a poll is started.  As the poll is
 * updated, it verifies if a vote was succesfully passed.
 *
 * voteTime is how long a poll will allow players to vote
 * vetoTime is how long after a poll is successful that a veto is possible
 * votesRequired is a minimum number of votes required
 * votePercentage is a minimum percentage (0-100) required to succeed
 * voteRepeatTime is how frequently a player may request a poll
 */
class VotingArbiter
{
  // book-keeping to minimize abuse
  typedef struct poller {
    std::string name;
    TimeKeeper lastRequest;
  } poller_t;


private:
  VotingBooth *_votingBooth;

  TimeKeeper _startTime;

  // this is the number of players capable of participating in the poll.
  // it is a dynamic count of how many players are allowed to vote.
  unsigned short int _maxVotes;

  // server-configurable poll voting behavior variables
  unsigned short int _voteTime;
  unsigned short int _vetoTime;
  unsigned short int _votesRequired;
  float _votePercentage;
  unsigned short int _voteRepeatTime;

  /* queue of people who have recently successfully initiated a poll.
   * people on the poller list cannot initiate a poll.
   */
  std::deque<poller_t> _pollers;

  /* who and what are being voted on, and who asked for the poll */
  std::string _pollee;
  std::string _polleeIP;
  std::string _action;
  std::string _pollRequestor;

  /* names of players who are allowed to vote */
  std::deque<std::string> _suffraged;

protected:

  /** remove people in the pollers vector that are able to poll again
   */
  void updatePollers(void);

  /** see if a person is in the list of people waiting for a repoll
   * timeout.
   */
  bool isPollerWaiting(const std::string &name) const;


public:

  /* default constructor */
  VotingArbiter(unsigned short int voteTime, unsigned short int vetoTime,
		unsigned short int votesRequired, float votePercentage,
		unsigned short int voteRepeatTime);
  /* default copy constructor */
  VotingArbiter(const VotingArbiter& arbiter);
  ~VotingArbiter(void);

  /** is there a poll that this arbiter knows about?
   */
  inline bool knowsPoll(void) const;

  /** reset/forget the poll if one is known
   */
  bool forgetPoll(void);

  /** is there a poll that is accepting votes?
   */
  inline bool isPollOpen(void) const;

  /** is there a poll that is not accepting votes?
   */
  inline bool isPollClosed(void) const;

  /** is there a poll that has completed (poll time + veto time)
   */
  inline bool isPollExpired(void) const;

  /** return a string representing the action that is presently set
   */
  inline std::string getPollAction(void) const;

  /** return a string representing the target (player or setting) being voted upon
   */
  inline std::string getPollTarget(void) const;

  /** return a string representing the IP of the player being voted
   * upon, this is mostly useful for placing a ban.
   */
  inline std::string getPollTargetIP(void) const;

  /** return a string representing the player who requested the poll
   */
  inline std::string getPollRequestor(void) const;


  /** attempt to activate/open a poll
   */
  bool poll(const std::string &player, const std::string &playerRequesting, std::string action, std::string playerIP="");

  /** convenience method to attempt to start a kick poll
   */
  bool pollToKick(const std::string &player, const std::string &playerRequesting, const std::string &playerIP);

  /** convenience method to attempt to start a kill poll
   */
  bool pollToKill(const std::string &player, const std::string &playerRequesting, const std::string &playerIP);

  /** convenience method to attempt to start a ban poll
   */
  bool pollToBan(const std::string &player, const std::string &playerRequesting, const std::string &playerIP);
  /** convenience method to attempt to start a set poll
   */
  bool pollToSet(const std::string &setting, const std::string &playerRequesting);

  /** convenience method to attempt to reset flags that, at the time, are unused
   */
  bool pollToResetFlags(const std::string &playerRequesting);

  /** halt/close the poll if it is open
   */
  bool closePoll(void);


  /** set the number of available voters
   */
  bool setAvailableVoters(unsigned short int count);

  /** grant a player the right to vote
   */
  bool grantSuffrage(const std::string &player);

  /** returns whether truthfully whether a certain player is permitted
   * to vote; a player should check their right to vote before voting.
   */
  bool hasSuffrage(const std::string &player) const;

  /**
   * returns whether or not a user has voted
   */
  bool hasVoted(const std::string &player) const;

  /** apply a yes vote; returns true if the vote could be made
   */
  bool voteYes(const std::string &player);
  /** apply a no vote; returns true if the vote could be made
   */
  bool voteNo(const std::string &player);

  /** returns the number of yes votes received
   */
  unsigned long int getYesCount(void) const;

  /** returns the number of no votes received
   */
  unsigned long int getNoCount(void) const;

  /** returns the number of abstentions
   */
  unsigned long int getAbstentionCount(void) const;

  /** returns the vote duration length of the poll
   */
  unsigned short int getVoteTime(void) const;

  /** returns the veto duration length of the poll
   */
  unsigned short int getVetoTime(void) const;

  /** returns the time the poll was started
   */
  TimeKeeper getStartTime(void) const;

  /** returns truthfully if the poll has reached a passable tally.
   * i.e. enough positive votes have been received that the vote is
   * successful.
   */
  bool isPollSuccessful(void) const;

  /** returns truthfully if the poll is bound to fail given the
   * known votes and required vote percentage
   */
  bool isPollSureToFail(void) const;

  /** returns how much time (in seconds) is remaining on a poll, if one is
   * active
   */
  unsigned short int timeRemaining(void) const;

  /** allow a player vote to be retracted (perhaps if a player leaves
   */
  bool retractVote(const std::string &player);

};


inline VotingArbiter::VotingArbiter(unsigned short int voteTime=60,
				    unsigned short int vetoTime=20,
				    unsigned short int votesRequired=2,
				    float votePercentage=50.1,
				    unsigned short int voteRepeatTime=300)
  : _votingBooth(NULL),
    _maxVotes(votesRequired+1),
    _voteTime(voteTime),
    _vetoTime(vetoTime),
    _votesRequired(votesRequired),
    _votePercentage(votePercentage),
    _voteRepeatTime(voteRepeatTime)
{
  _startTime = TimeKeeper::getNullTime();
  _pollee = "nobody";
  _polleeIP = "";
  _action = "";
  _pollRequestor = "nobody";
  return;
}

inline VotingArbiter::VotingArbiter(const VotingArbiter& arbiter)
  : _votingBooth(arbiter._votingBooth),
    _startTime(arbiter._startTime),
    _maxVotes(arbiter._maxVotes),
    _voteTime(arbiter._voteTime),
    _vetoTime(arbiter._vetoTime),
    _votesRequired(arbiter._votesRequired),
    _votePercentage(arbiter._votePercentage),
    _voteRepeatTime(arbiter._voteRepeatTime),
    _pollers(arbiter._pollers),
    _pollee(arbiter._pollee),
    _polleeIP(arbiter._polleeIP),
    _action(arbiter._action),
    _pollRequestor(arbiter._pollRequestor),
    _suffraged(arbiter._suffraged)
{
  return;
}

inline VotingArbiter::~VotingArbiter(void)
{
  if (_votingBooth != NULL) {
    delete _votingBooth;
    _votingBooth = NULL;
  }
  return;
}


inline bool VotingArbiter::knowsPoll(void) const
{
  return (_votingBooth != NULL);
}

inline bool VotingArbiter::isPollClosed(void) const
{
  if (!this->knowsPoll()) {
    return false;
  }
  // check timer
  if ((TimeKeeper::getCurrent() - _startTime) >= _voteTime) {
    return true;
  }
  return false;
}

inline bool VotingArbiter::isPollOpen(void) const
{
  if (!this->knowsPoll()) {
    return false;
  }
  if ((TimeKeeper::getCurrent() - _startTime) < _voteTime) {
    return true;
  }
  return false;
}

inline bool VotingArbiter::isPollExpired(void) const
{
  // if the poll is not at least closed, then it's not expired
  if (!this->knowsPoll()) {
    return false;
  }
  // check timer
  if ((TimeKeeper::getCurrent() - _startTime) > _voteTime + _vetoTime) {
    return true;
  }
  return false;
}

inline std::string VotingArbiter::getPollAction(void) const
{
  if (_action.size() <= 0) {
    return "do something unknown to";
  }
  return _action;
}

inline std::string VotingArbiter::getPollTarget(void) const
{
  return _pollee.size() == 0 ? "nobody" : _pollee;
}

inline std::string VotingArbiter::getPollTargetIP(void) const
{
  return _polleeIP.size() == 0 ? "" : _polleeIP;
}

inline std::string VotingArbiter::getPollRequestor(void) const
{
  return _pollRequestor.size() == 0 ? "nobody" : _pollRequestor;
}

inline unsigned short int VotingArbiter::getVoteTime(void) const
{
  return _voteTime;
}

inline unsigned short int VotingArbiter::getVetoTime(void) const
{
  return _vetoTime;
}

inline TimeKeeper VotingArbiter::getStartTime(void) const
{
  if (!this->knowsPoll()) {
    return TimeKeeper::getNullTime();
  }
  return _startTime;
}

#else
class VotingArbiter;
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
