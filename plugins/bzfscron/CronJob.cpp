/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <vector>
#include <string>
#include <iostream>

#include "CronJob.h"
#include "TextUtils.h"
#include "bzfsAPI.h"
#include "bzfio.h"


// debug util func
static std::string vector_dump(std::vector<int> &iv) {
  std::string tmp = "<";
  for (std::vector<int>::iterator itr = iv.begin(); itr != iv.end(); ++itr)
    tmp += TextUtils::format(" %d", *itr);
  tmp += " >";
  return tmp;
}

CronJob::CronJob() {
  // do nothing
}

CronJob::CronJob(std::string job) {
  setJob(job);
}

CronJob::~CronJob() {
  // do nothing
}

void CronJob::setJob(std::string job) {
  if (job.size() == 0) return;
  if (TextUtils::no_whitespace(job).size() == 0) return;

  // parse the string we're given into five vectors of n ints and a command
  // note: this is rather expensive
  inputJob = job;

  // first bust it up into tokens based on whitespace.
  // the first five are the timing values and the 'sixth through nth' is the command.
  std::vector<std::string> toks = TextUtils::tokenize(job, " \t", 6);

  // hokey dokey.  now we have six strings and we need five arrays of ints and one string out of them.
  minutes = parseTimeList(toks[0], 0, 59);
  hours = parseTimeList(toks[1], 0, 23);
  days = parseTimeList(toks[2], 1, 31);
  months = parseTimeList(toks[3], 1, 12);
  weekdays = parseTimeList(toks[4], 0, 7);
  command = toks[5];

  // sunday is both 7 and 0, make sure we have both or neither
  if (isInVector(weekdays, 0) && !isInVector(weekdays, 7)) weekdays.push_back(7);
  else if (isInVector(weekdays, 7) && !isInVector(weekdays, 0)) weekdays.push_back(0);

  // dump the list if we're debuggering
  if (debugLevel >= 4) {
    std::cout << "bzfscron: read job: " << inputJob << std::endl;
    std::cout << "bzfscron: job minutes: " << vector_dump(minutes) << std::endl;
    std::cout << "bzfscron: job hours: " << vector_dump(hours) << std::endl;
    std::cout << "bzfscron: job days: " << vector_dump(days) << std::endl;
    std::cout << "bzfscron: job months: " << vector_dump(months) << std::endl;
    std::cout << "bzfscron: job weekdays: " << vector_dump(weekdays) << std::endl;
    std::cout << "bzfscron: job command: " << command << std::endl;
  }
}

std::vector<int> CronJob::parseTimeList(const std::string in, const int min, const int max) {
  std::vector<int> vi;
  std::string list = in;

  // First things first.  Find out if there's a periodicity and trim it off.
  int pos = (int)in.find("/");
  int period = 1;
  if (pos != std::string::npos) {
    period = atoi(in.substr(pos + 1).c_str());
    list = in.substr(0, pos);
  }

  // Now tokenize on ","
  std::vector<std::string> stage1 = TextUtils::tokenize(list, ",");
  // No tokens?  That's cool too.
  if (stage1.size() == 0) stage1.push_back(list);

  // And for each token, blow up any "-" ranges and "*" ranges.
  for (std::vector<std::string>::iterator itr = stage1.begin(); itr != stage1.end(); ++itr) {
    if ((*itr).find("*") != std::string::npos) {
      bz_debugMessage(4, "bzfscron: exploding * range");
      for (int i = min; i <= max; ++i)
	vi.push_back(i);
    } else if ((pos = (int)(*itr).find("-")) != std::string::npos) {
      bz_debugMessage(4, "bzfscron: exploding x-y range");
      int rmin = 0, rmax = 0;
      rmin = atoi((*itr).substr(0, pos).c_str());
      rmax = atoi((*itr).substr(pos + 1).c_str());
      if (rmin < min) rmin = min;
      if (rmax > max) rmax = max;
      for (int i = rmin; i <= rmax; ++i)
	vi.push_back(i);
    } else {
      bz_debugMessage(4, "bzfscron: using single int");
      vi.push_back(atoi((*itr).c_str()));
    }
  }

  // Remember that periodicity we got rid of earlier?  Now we need it.
  // Eliminate any elements which disagree with the periodicity.
  if (period > 1) {
    std::vector<int> vp;
    for (std::vector<int>::iterator itr2 = vi.begin(); itr2 != vi.end(); ++itr2) {
      if (((*itr2) == 0) || ((*itr2) % period == 0))
	vp.push_back(*itr2);
    }
    return vp;
  } else {
    return vi;
  }
}

bool CronJob::matches(int n, int h, int d, int m, int w) const {
  // if we are supposed to execute now, return true, otherwise return false
  return (isInVector(minutes, n) &&
	  isInVector(hours, h) &&
	  isInVector(days, d) &&
	  isInVector(months, m) &&
	  isInVector(weekdays, w));
}

bool CronJob::isInVector(const std::vector<int> &iv, const int x) {
  for (std::vector<int>::const_iterator itr = iv.begin(); itr != iv.end(); ++itr) {
    if (*itr == x)
      return true;
  }
  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
