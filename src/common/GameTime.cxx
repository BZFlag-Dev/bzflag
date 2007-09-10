/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**
 * GameTime:
 *	Manages the network time.
 *      Time is stored as microseconds since the epoch.
 */

// the top dog
#include "common.h"

// interface header
#include "GameTime.h"

// system headers
#include <time.h>
#ifndef _WIN32
#  include <sys/time.h>
#endif
#include <list>

// common headers
#include "Pack.h"
#include "bzfio.h"


// type definitions
typedef uint16_t u16;
typedef uint32_t u32;
#ifndef WIN32
typedef int64_t s64;
#else
typedef __int64 s64;
#endif
typedef struct {
  s64 netTime;
  s64 localTime;
} TimeRecord;

// local constants
static const double filterTime = 10.0;
static const unsigned int maxRecords = 1024; // safety
static const unsigned int maxRecordAge = 120 * 1000000;
static const s64 maxTime = 2345678;
static const double minRate = 0.50;
static const double maxRate = 2.00;

// local variables
static std::list<TimeRecord> timeRecs;
static double stepSecs = 0.0;
static s64 stepTime = 0;
static double avgRate = 1.0;
static TimeRecord avgPoint = {0, 0};


/******************************************************************************/

static s64 getRawTime()
{
#ifndef _WIN32

  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((s64)tv.tv_sec * (s64)1000000) + (s64)tv.tv_usec;

#else //_WIN32

  FILETIME ft;
  LARGE_INTEGER li;

  GetSystemTimeAsFileTime(&ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;

  /* QuadPart is in 100-nanosecond intervals */
  return (li.QuadPart / 10);

#endif //_WIN32
}


/******************************************************************************/

static void calcAvgRate()
{
  // FIXME - this is weak
  const int count = timeRecs.size();
  if (count <= 0) {
    avgRate = 1.0;
    avgPoint.netTime = 0;
    avgPoint.localTime = 0;
    return;
  }
  else if (timeRecs.size() == 1) {
    const TimeRecord& tr = *timeRecs.begin();
    avgRate = 1.0;
    avgPoint = tr;
    return;
  }
  else {
    const TimeRecord& last = *timeRecs.begin();
    const TimeRecord& first = *timeRecs.rbegin();
    const s64 netDiff = last.netTime - first.netTime;
    const s64 locDiff = last.localTime - first.localTime;
    if (locDiff != 0.0) {
      avgRate = ((double)netDiff / (double)locDiff);
      avgPoint = last;
    } else {
      // don't update
    }
  }
  return;
}


/******************************************************************************/

void GameTime::reset()
{
  stepSecs = 0.0;
  stepTime = 0;
  avgRate = 1.0;
  avgPoint.netTime = 0;
  avgPoint.localTime = 0;
  timeRecs.clear();
  return;
}


static void resetToRecord(const TimeRecord& record)
{
  avgRate = 1.0;
  avgPoint = record;
  stepTime = record.netTime;
  TimeRecord copy = record;
  timeRecs.clear();
  timeRecs.push_front(copy);
  return;
}


void GameTime::update()
{
  std::list<TimeRecord>::iterator it;
  unsigned int count = timeRecs.size();
  if (count <= 0) {
    const TimeRecord tr = {0,0};
    resetToRecord(tr);
  }
  else if (count == 1) {
    const TimeRecord& tr = *timeRecs.begin();
    resetToRecord(tr);
  }
  else {
    calcAvgRate();
    const TimeRecord& tr = *timeRecs.begin();
    const s64 diffTime = stepTime - tr.netTime;
    if ((diffTime < -maxTime) || (diffTime > +maxTime) ||
	(avgRate < minRate) || (avgRate > maxRate)) {
      logDebugMessage(4,"GameTime: discontinuity: usecs = %lli, rate = %f\n",
	     diffTime, avgRate);
      resetToRecord(tr);
    }
  }

  logDebugMessage(4,"GameTime: count = %i, rate = %f\n", timeRecs.size(), avgRate);

  return;
}


/******************************************************************************/

void GameTime::setStepTime()
{
  static s64 lastStep = 0;
  const s64 thisStep = getRawTime();
  if (timeRecs.size() <= 0) {
    stepTime = thisStep;
  } else {
    // long term prediction
    const double diffLocal = (double)(thisStep - avgPoint.localTime);
    const double longPred = (double)avgPoint.netTime + (diffLocal * avgRate);
    // short term prediction
    const double skipTime = (double)(thisStep - lastStep);
    const double shortPred = (double)stepTime + (skipTime * avgRate);
    // filtering
    const double c = (skipTime * 1.0e-6) / filterTime;
    const double a = (c > 0.0) && (c < 1.0) ? c : 0.5;
    const double b = 1.0 - a;
    stepTime = (s64)((a * longPred) + (b * (double)shortPred));
  }
  stepSecs = (double)stepTime * 1.0e-6;
  lastStep = thisStep;

  return;
}


double GameTime::getStepTime()
{
  return stepSecs;
}


/******************************************************************************/

int GameTime::packSize()
{
  return (2 * sizeof(u32));
}


void* GameTime::pack(void *buf, float lag)
{
  double halfLag;
  if ((lag <= 0.0f) || (lag > 10.0f)) {
    // assume a 150ms delay
    halfLag = 0.075;
  } else {
    halfLag = (double)(lag * 0.5f);
  }
  const s64 nowTime = getRawTime() + (s64)(halfLag * 1.0e6);
  buf = nboPackUInt(buf, (u32)(nowTime >> 32));		// msb's
  buf = nboPackUInt(buf, (u32)(nowTime & 0xFFFFFFFF));	// lsb's
  return buf;
}


void* GameTime::unpack(void *buf)
{
  u32 msb, lsb;
  buf = nboUnpackUInt(buf, msb);
  buf = nboUnpackUInt(buf, lsb);
  const s64 netTime = ((s64)msb << 32) + (s64)lsb;

  // store the value
  const TimeRecord tr = { netTime, getRawTime() };
  timeRecs.push_front(tr);

  // clear oversize entries
  while (timeRecs.size() > maxRecords) {
    timeRecs.pop_back();
  }

  // clear the aged entries
  if (timeRecs.size() > 0) {
    s64 nowTime = getRawTime();
    while (timeRecs.size() > 0) {
      TimeRecord back = *timeRecs.rbegin();
      if ((nowTime - back.localTime) < maxRecordAge) {
	break;
      }
      timeRecs.pop_back();
    }
  }

  return buf;
}


/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
