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

/* no header other than VotingPoll.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "VotingPoll.h"


/* private */

/* protected */

/* public: */

inline bool VotingPoll::isOpen(void) const {
  if (_votingBooth != NULL) {
    return true;
  }
  return false;
}

inline bool VotingPoll::isClosed(void) const {
  return (!this->isOpen());
}

bool VotingPoll::open(void) {
  if (this->isOpen()) {
    return true;
  }
  _votingBooth = new VotingBooth();
  return false;
}

bool VotingPoll::close(void) {
  if (this->isClosed()) {
    return true;
  }
  delete _votingBooth;
  _votingBooth=NULL;
  return true;
}

bool VotingPoll::setAvailableVoters(unsigned short int count) {
  if (this->isOpen()) {
    _maxVotes = count;
    return true;
  }
  return false;
}

bool VotingPoll::allowSuffrage() const{
  return (this->isOpen());
}

bool VotingPoll::voteYes() {
  if (this->isClosed()) {
    return false;
  }

  return false;
}

bool VotingPoll::voteNo() {
  if (this->isClosed()) {
    return false;
  }

  return false;
}

bool VotingPoll::isSuccessful(void) const {
  return false;
}

// ex: shiftwidth=2 tabstop=8
