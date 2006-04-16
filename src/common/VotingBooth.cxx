/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

// interface header
#include "VotingBooth.h"

// system headers
#include <string>
#include <vector>

const short int VotingBooth::RETRACTED_VOTE=-2;

/* private */

/* protected */

/* public: */

VotingBooth::VotingBooth(std::string question)
  : _question(question)
{
  return;
}

VotingBooth::VotingBooth(const VotingBooth& booth)
  : _question(booth._question),
    _choice(booth._choice),
    _vote(booth._vote)
{
  return;
}

VotingBooth::~VotingBooth(void)
{
  return;
}


/* convenience func that sets up and returns a default poll */
VotingBooth *YesNoVotingBooth(std::string question)
{
  VotingBooth *poll = new VotingBooth(question);

  poll->addResponse("no");
  poll->addResponse("yes");

  return poll;
}


bool VotingBooth::addResponse(const std::string response)
{
  if (response.size() == 0) {
    return false;
  }

  _choice.push_back(response);

  return true;
}


bool VotingBooth::hasVoted(const std::string voterName) const
{
  if (voterName.size() <= 0) {
    return false;
  }
  if (_vote.find(voterName) != _vote.end()) {
    return true;
  }
  return false;
}


bool VotingBooth::vote(std::string voterName, std::string response)
{
  if (this->hasVoted(voterName)) {
    /* voters are not allowed to vote multiple times */
    return false;
  }

  /* make sure the response is valid */
  std::vector<std::string>::iterator i = std::find(_choice.begin(), _choice.end(), response);
  if (i == _choice.end()) {
    return false;
  }

  /* record the dang vote */
  _vote[voterName] = i - _choice.begin();

  return true;
}

bool VotingBooth::retractVote(const std::string voterName)
{
  VoterResponseMap::iterator i = _vote.find(voterName);

  /* if not found, then nothing to retract */
  if (i == _vote.end()) {
    return false;
  }

  _vote[voterName] = RETRACTED_VOTE;

  return false;
}

unsigned long int VotingBooth::getVoteCount(const std::string response) const
{
  unsigned long int total=0;

  for (VoterResponseMap::const_iterator i = _vote.begin();
       i != _vote.end(); ++i) {
    /* negative indices indicate an uncounted vote (perhaps retracted) */
    if ( (i->second >= 0) && (_choice[i->second] == response) ) {
      total++;
    }
  }
  return total;
}


unsigned long int VotingBooth::getTotalVotes(void) const
{
  unsigned long int total=0;

  for (std::vector<std::string>::const_iterator i = _choice.begin();
       i != _choice.end(); ++i) {
    total += this->getVoteCount(*i);
  }
  return total;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
