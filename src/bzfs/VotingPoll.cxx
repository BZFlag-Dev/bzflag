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

bool VotingPoll::isOpen(void) const{
  return false;
}

bool VotingPoll::allowSuffrage() const{
  return (this->isOpen());
}

bool VotingPoll::voteYes() {
  return false;
}
bool VotingPoll::voteNo() {
  return false;
}

bool VotingPoll::isSuccessful(void) const{
  return false;
}

// ex: shiftwidth=2 tabstop=8
