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

/* vote 0 is no; vote 1 is yes; */
typedef unsigned short int vote_t;

/* max number of potential poll options */
static const unsigned short int MAX_VOTE_ANSWERS=255;

/** VotingBooth is a means to create and track a vote.  A single booth
 * will track and allow voting on a poll.
 *
 * By default, false and true (i.e. no and yes) are the respective first
 * two default results of any poll if unspecified.
 */
class VotingBooth
{
 private:

  /** question that is voted upon (optionally provided)
   */
  std::string _question;

  /** array of potential poll responses; first two entries are "no" and
   * "yes" respectively; additionally added options follow.
   */
  std::string _option[MAX_VOTE_ANSWERS];

  /** how many options have been manually added
   */
  unsigned short int _optionsAdded;

 protected:
  
 public:
  
  VotingBooth(void);
  ~VotingBooth(void);

  /** add an option to vote upon
   */
  vote_t addOption(std::string option);

  /** lookup the id of a vote option
   */
  vote_t getOptionIDFromString(std::string name);
  std::string getStringFromOptionID(vote_t id);

  /** a given user id/name responds and votes to a particular poll
   * option.
   */
  bool vote(std::string name, vote_t id);
};


#else
class VotingBooth;
#endif
// ex: shiftwidth=2 tabstop=8
