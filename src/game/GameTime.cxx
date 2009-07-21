/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
 *	Manages network timing.
 *      Time is stored as doubles representing seconds
 */

// the top dog
#include "common.h"

// interface header
#include "GameTime.h"

// system headers
#include <list>

// common headers
#include "Pack.h"
#include "TimeKeeper.h"
#include "BZDBCache.h"
#include "bzfio.h"


// type definitions
struct TimeRecord {
  double netTime;
  double localTime;
};

// local constants
static const double filterTime   = 10.0;
static const double maxRecordAge = 120.0;
static const double maxTime      = 5.00;
static const double minRate      = 0.50;
static const double maxRate      = 2.00;
static const size_t maxRecords   = 1024; // safety

// local variables
static std::list<TimeRecord> timeRecs;
static double     stepTime = 0;
static double     avgRate  = 1.0;
static TimeRecord avgPoint = { 0, 0 };


static BZDB_int debugGameTime("debugGameTime");


//============================================================================//

static double getRawTime()
{
  return TimeKeeper::getCurrent().getSeconds();
}


//============================================================================//

static void calcAvgRate()
{
  // FIXME - this is weak
  const size_t count = timeRecs.size();
  if (count == 0) {
    avgRate = 1.0;
    avgPoint.netTime = 0;
    avgPoint.localTime = 0;
    return;
  }
  else if (count == 1) {
    const TimeRecord& tr = *timeRecs.begin();
    avgRate = 1.0;
    avgPoint = tr;
    return;
  }
  else {
    const TimeRecord& last = *timeRecs.begin();
    const TimeRecord& first = *timeRecs.rbegin();
    const double netDiff = last.netTime - first.netTime;
    const double locDiff = last.localTime - first.localTime;
    if (locDiff != 0.0) {
      avgRate = ((double)netDiff / (double)locDiff);
      avgPoint = last;
    } else {
      // don't update
    }
  }
  return;
}


//============================================================================//

void GameTime::reset()
{
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
  const TimeRecord trCopy = record; // make a copy, could reference timeRecs
  timeRecs.clear();
  timeRecs.push_front(trCopy);
  return;
}


void GameTime::update()
{
  std::list<TimeRecord>::iterator it;
  const size_t count = timeRecs.size();
  if (count == 0) {
    const TimeRecord tr = {0, 0};
    resetToRecord(tr);
  }
  else if (count == 1) {
    const TimeRecord& tr = *timeRecs.begin();
    resetToRecord(tr);
  }
  else {
    calcAvgRate();
    const TimeRecord& tr = *timeRecs.begin();
    const double diffTime = stepTime - tr.netTime;
    if ((diffTime < -maxTime) || (diffTime > +maxTime) ||
	(avgRate < minRate) || (avgRate > maxRate)) {
      logDebugMessage(4, "GameTime: discontinuity: usecs = %f, rate = %f\n",
		      diffTime, avgRate);
      resetToRecord(tr);
    }
  }

  logDebugMessage(4, "GameTime: count = %i, rate = %f\n", timeRecs.size(), avgRate);

  return;
}


//============================================================================//

void GameTime::setStepTime()
{
  static double lastStep = 0;
  const double thisStep = getRawTime();
  if (timeRecs.size() <= 0) {
    stepTime = thisStep;
  }
  else {
    // long term prediction
    const double diffLocal = (double)(thisStep - avgPoint.localTime);
    const double longPred = (double)avgPoint.netTime + (diffLocal * avgRate);
    // short term prediction
    const double skipTime = (double)(thisStep - lastStep);
    const double shortPred = (double)stepTime + (skipTime * avgRate);
    // filtering
    const double c = skipTime / filterTime;
    const double a = (c > 0.0) && (c < 1.0) ? c : 0.5;
    const double b = 1.0 - a;
    stepTime = (a * longPred) + (b * shortPred);
  }
  lastStep = thisStep;

  return;
}


double GameTime::getStepTime()
{
  return stepTime;
}


//============================================================================//

int GameTime::packSize()
{
  return sizeof(double) + sizeof(float);
}


void* GameTime::pack(void *buf, float lag)
{
  float halfLag;
  if ((lag < 0.0f) || (lag > 10.0f)) {
    halfLag = 0.075f; // assume a 150ms delay
  } else {
    halfLag = (lag * 0.5f);
  }
  buf = nboPackDouble(buf, getRawTime());
  buf = nboPackFloat(buf, halfLag);
  return buf;
}


void GameTime::pack(BufferedNetworkMessage *msg, float lag)
{
  float halfLag;
  if ((lag < 0.0f) || (lag > 10.0f)) {
    halfLag = 0.075f; // assume a 150ms delay
  } else {
    halfLag = (lag * 0.5f);
  }
  msg->packDouble(getRawTime());
  msg->packFloat(halfLag);
}


void* GameTime::unpack(void *buf)
{
  double netTime;
  float halfLag;
  buf = nboUnpackDouble(buf, netTime);
  buf = nboUnpackFloat(buf, halfLag);
  netTime += halfLag;

  // store the value
  const double localTime = getRawTime();
  const TimeRecord tr = { netTime, localTime };
  timeRecs.push_front(tr);

  // clear oversize entries
  while (timeRecs.size() >= maxRecords) {
    timeRecs.pop_back();
  }

  // clear old entries
  if (timeRecs.size() > 0) {
    while (timeRecs.size() > 0) {
      TimeRecord back = *timeRecs.rbegin();
      if ((localTime - back.localTime) < maxRecordAge) {
	break;
      }
      timeRecs.pop_back();
    }
  }

  update();

  if (debugGameTime >= 2) {
    int i = 0;
    std::list<TimeRecord>::const_iterator it;
    for (it = timeRecs.begin(); it != timeRecs.end(); ++it, i++) {
      logDebugMessage(0, "GameTime record %i: netTime=%f, localTime=%f, diff=%f\n",
                      i, it->netTime, it->localTime, it->netTime - it->localTime);
    }
  }
  if (debugGameTime >= 1) {
    logDebugMessage(0,
      "GameTime:unpack()"
      " net:%.3f local:%.3f diff:%.3f step:%.3f halfLag:%.3f rate:%.6f\n",
      netTime, localTime, netTime - localTime, stepTime, halfLag, avgRate);
  }

  return buf;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
