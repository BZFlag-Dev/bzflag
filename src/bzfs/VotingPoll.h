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
  float _votePercentage;
  unsigned short int _voteRepeatTime;
   

 protected:
  
 public:
  
  VotingPoll(unsigned short int voteTime=60, 
	     unsigned short int vetoTime=20,
	     unsigned short int votesRequired=3,
	     float votePercentage=50.1,
	     unsigned short int voteRepeatTime=300);
  ~VotingPoll(void);

  inline VotingBooth *getBooth(void) {
    return _votingBooth;
  }
  

};


#else
class VotingPoll;
#endif
// ex: shiftwidth=2 tabstop=8
