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

#ifndef __VOTINGPOLL_H__
#define __VOTINGPOLL_H__

#include <string>
#include <iostream>

#include "VotingBooth.h"
#include "TimeKeeper.h"


/** VotingPoll is a means to manage and enforce a poll.  The poll will
 * maintain a voting booth when a poll is started.  As the poll is updated,
 * it verifies if a vote was succesfully passed.  If it is, the action
 * specified (kick/ban/etc) is enforced.
 */
class VotingPoll
{
 private:
  VotingBooth *_votingBooth;

  unsigned short int _voteTime;
  unsigned short int _vetoTime;
  unsigned short int _votesRequired;
  unsigned short int _maxVotes;
  float _votePercentage;
  unsigned short int _voteRepeatTime;


 protected:

    /** performs the supported and requested actions of a voting poll
    * if the poll was successful
    */
    bool enforce(void);

  
 public:

  VotingPoll(unsigned short int voteTime=60, unsigned short int vetoTime=20,
	     unsigned short int votesRequired=2, float votePercentage=50.1,
	     unsigned short int voteRepeatTime=300)
    : _voteTime(voteTime),
      _vetoTime(vetoTime),
      _votesRequired(votesRequired),
      _maxVotes(votesRequired+1),
      _votePercentage(votePercentage),
      _voteRepeatTime(voteRepeatTime)
  {
    _votingBooth = NULL;
    return;
  }

  ~VotingPoll(void)
  {
    if (_votingBooth != NULL) {
      delete _votingBooth;
    }
    return;
  }


  /** is the poll accepting votes?
   */
  inline bool isOpen(void) const;

  /** is the poll not accepting votes?
   */
  inline bool isClosed(void) const;

  /** open the poll if it is closed
   */
  bool open(void);

  /** close the poll if it is open
   */
  bool close(void);

  /** set the number of available voters
    */
  bool setAvailableVoters(unsigned short int count);
  
  /** returns whether truthfully whether a certain player is permitted
   * to vote; a player should check their right to vote before voting.
   */
  bool allowSuffrage() const;

  /** apply a yes vote; returns true if the vote could be made
    */
  bool voteYes();
  /** apply a no vote; returns true if the vote could be made
    */
  bool voteNo();

  /** returns truthfully if the poll has reached a passable tally.
    * i.e. enough votes have been received that the vote is successful
    */
  bool isSuccessful(void) const;


};


#else
class VotingPoll;
#endif
// ex: shiftwidth=2 tabstop=8
