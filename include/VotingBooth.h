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

#include <ctype.h>
#include <string>

// work around an ugly STL bug in BeOS
// FIXME someone test whether it is still needed
#ifdef __BEOS__
#define private public
#endif
#include <bitset>
#ifdef __BEOS__
#undef private
#endif

#include <iostream>

/* positive vote is an index; non-positive is an error */
typedef long int vote_t;

/* max number of potential poll responses */
static const unsigned short int MAX_VOTE_RESPONSES=255;

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

  /** array of potential poll responses (a response is a poll
   * option/choice -- not a vote itself)
   */
  std::string _response[MAX_VOTE_RESPONSES];

  /** how many responses have been manually added
   */
  unsigned short int _responseCount;

  /** counts of the vote responses 
   */
  unsigned long int _vote[MAX_VOTE_RESPONSES];

  /** lists of who has voted
   */
  std::string _voter[MAX_VOTE_RESPONSES];

  /** how many voters have been manually added
   */
  unsigned short int _voterCount;

  /* require unique voters */
  bool _requireUnique;

 protected:

  /* strait from stroustrup; with added  */
  inline static int compare_nocase(const std::string& s1, const std::string &s2, int maxlength=4096)
  {
    std::string::const_iterator p1 = s1.begin();
    std::string::const_iterator p2 = s2.begin();
    int i=0;
    while (p1 != s1.end() && p2 != s2.end()) {
      if (i >= maxlength) {
	return 0;
      }
      if (tolower(*p1) != tolower(*p2)) {
	return (tolower(*p1) < tolower(*p2)) ? -1 : 1;
      }
      ++p1;
      ++p2;
      ++i;
    }
    return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1; // size is unsigned
  }
  
 public:
  
  VotingBooth(std::string question = "", bool requireUnique = true);
  ~VotingBooth(void);

  /** add an response to vote upon
   */
  vote_t addResponse(const std::string response);

  /** lookup the id of a vote response
   */
  vote_t getResponseIDFromString(const std::string name) const;
  const std::string getStringFromResponseID(vote_t id) const;

  /** a given user id/name responds and votes to a particular poll
   * response.  
   */
  bool vote(const std::string name, vote_t id);

  /** return how many votes have been placed for a particular response.
   * lookup votes by vote identifier.
   */
  unsigned long int getVoteCount(vote_t id) const;

  /** return how many votes have been placed for a particular response.
   * lookup votes by response.
   */
  unsigned long int getVoteCount(const std::string response) const;

  /** return total number of votes received
   */
  unsigned long int VotingBooth::getTotalVotes(void) const;

  /** returns the number of voters
   */
  inline unsigned long int getVoterCount(void) const {
    return _voterCount;
  }

  /** returns the number of responses available
   */
  inline unsigned long int getResponseCount(void) const {
    return _responseCount;
  }

  inline const std::string getPollName(void) const {
    return _question;
  }

};

/* convenience func that sets up and returns a default poll */
VotingBooth *YesNoVotingBooth(std::string question = "");

#else
class VotingBooth;
#endif
// ex: shiftwidth=2 tabstop=8
