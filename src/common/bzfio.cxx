/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "bzfio.h"
#include "common.h"

#include <stdarg.h>
/* system implementation headers */
#include <time.h>
#include <string>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef __BEOS__
#  include <OS.h>
#endif
#if !defined(_WIN32)
#  include <sys/time.h>
#  include <sys/types.h>
#else /* !defined(_WIN32) */
#  include <mmsystem.h>
#endif

LoggingCallback *loggingCallback = NULL;

static bool doTimestamp = false;
static bool doMicros = false;
static bool doUTC = false;

void setDebugTimestamp (bool enable, bool micros, bool utc)
{
#ifdef _WIN32
  micros = false;
#endif
  doTimestamp = enable;
  doMicros = micros;
  doUTC = utc;
}

static const int tsBufferSize = 512;

static char *timestamp(char *buf, bool micros, bool utc)
{
  struct tm *tm;
  if (micros) {
#if !defined(_WIN32)
    struct timeval tv;
    gettimeofday (&tv, NULL);
    if (utc)
      tm = gmtime((const time_t *)&tv.tv_sec);
    else
      tm = localtime((const time_t *)&tv.tv_sec);
    snprintf (buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02ld.%06ld: ", tm->tm_year+1900,
	     tm->tm_mon+1,
	     tm->tm_mday, tm->tm_hour, tm->tm_min, (long)tm->tm_sec, (long)tv.tv_usec );
#endif
  } else {
    time_t tt;
    time (&tt);
    if (utc)
      tm = gmtime (&tt);
    else
      tm = localtime (&tt);

    snprintf (buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02d: ", tm->tm_year+1900, tm->tm_mon+1,
	     tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
  }
  return buf;
}


void logDebugMessage(int level, const char* fmt, ...)
{
  char buffer[8192] = { 0 };
  char tsbuf[tsBufferSize] = { 0 };
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  if (debugLevel >= level || level == 0) {
#if defined(_MSC_VER)
      if (doTimestamp)
	W32_DEBUG_TRACE(timestamp (tsbuf, false, doUTC));
      W32_DEBUG_TRACE(buffer);
#else
      if (doTimestamp)
	std::cout << timestamp (tsbuf, doMicros, doUTC);
      std::cout << buffer;
#endif
    }

  if (loggingCallback)
    loggingCallback->log(level,buffer);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
