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

#ifndef __VOTINGBOOTH_H__
#define __VOTINGBOOTH_H__

#include <string>
#include <vector>
#include <set>

// work around an ugly STL bug in BeOS
// FIXME someone test whether it is still needed
#ifdef __BEOS__
#define private public
#endif
#include <bitset>
#ifdef __BEOS__
#undef private
#endif

#include <fstream>
#include <iostream>

static const unsigned short int MAX_VOTE_ANSWERS=255;

/** VotingBooth is a means to create and track a vote.  A single booth
 * will track and allow voting on a poll.
 *
 */
class VotingBooth
{
 private:

  /** results array contains integer counts for each vote response
   */
  int voteResults[MAX_VOTE_ANSWERS];

  /** question that is voted upon
   */
  std::string question;

  /** array of potential answers
   */
  std::string answer[MAX_VOTE_ANSWERS];

 protected:

 public:
  
  VotingBooth(void);
  ~VotingBooth(void);
  
};


#else
class VotingBooth;
#endif
// ex: shiftwidth=2 tabstop=8
