/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "TimeKeeper.h"

#if !defined(_WIN32)
#include <sys/time.h>
#include <sys/types.h>
static struct timeval	lastTime = { 0, 0 };
#else /* !defined(_WIN32) */
#include <mmsystem.h>
#include "common.h"
static unsigned int	lastTime = 0;
static LARGE_INTEGER	qpcLastTime;
static double		qpcFrequency = 0.0;
#endif /* !defined(_WIN32) */

TimeKeeper		TimeKeeper::currentTime;
TimeKeeper		TimeKeeper::tickTime;
TimeKeeper		TimeKeeper::sunExplodeTime;
TimeKeeper		TimeKeeper::sunGenesisTime;
TimeKeeper		TimeKeeper::nullTime;

const TimeKeeper&	TimeKeeper::getCurrent(void)
{
  // if not first call then update current time, else use default initial time
#if !defined(_WIN32)
  if (lastTime.tv_sec != 0) {
    struct timeval now;
    gettimeofday(&now, NULL);
    currentTime += float(now.tv_sec - lastTime.tv_sec) +
		1.0e-6f * float(now.tv_usec - lastTime.tv_usec);
    lastTime = now;
  }
  else {
    gettimeofday(&lastTime, NULL);
  }
#else /* !defined(_WIN32) */
  if (qpcFrequency != 0.0) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    currentTime += (float)(qpcFrequency *
			(double)(now.QuadPart - qpcLastTime.QuadPart));
    qpcLastTime = now;
  }
  else if (lastTime != 0) {
    unsigned int now = (unsigned int)timeGetTime();
    unsigned int diff;
    if (now <= lastTime) {
      diff = 0;
    } else {
      diff = now - lastTime;
      if (diff > 10000)
        diff = 10000;
    }
    currentTime += 1.0e-3f * (float)diff;
    lastTime = now;
  }
  else {
    LARGE_INTEGER freq;
    if (QueryPerformanceFrequency(&freq)) {
      qpcFrequency = 1.0 / (double)freq.QuadPart;
      QueryPerformanceCounter(&qpcLastTime);
    }
    else {
      lastTime = (unsigned int)timeGetTime();
    }
  }
#endif /* !defined(_WIN32) */
  return currentTime;
}

const TimeKeeper&	TimeKeeper::getTick(void) // const
{
  return tickTime;
}

void			TimeKeeper::setTick(void)
{
  tickTime = getCurrent();
}

const TimeKeeper& TimeKeeper::getSunExplodeTime(void)
{
  sunExplodeTime.seconds = 10000.0 * 365 * 24 * 60 * 60;
  return sunExplodeTime;
}

const TimeKeeper& TimeKeeper::getSunGenesisTime(void)
{
  sunGenesisTime.seconds = -10000.0 * 365 * 24 * 60 * 60;
  return sunGenesisTime;
}

const TimeKeeper& TimeKeeper::getNullTime(void)
{
  nullTime.seconds = 0;
  return nullTime;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

