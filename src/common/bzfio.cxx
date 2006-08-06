/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "bzfio.h"

/* system implementation headers */
#include <iostream>
#include <stdarg.h>
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


static bool doTimestamp = false;
static bool doMicros = false;

void setDebugTimestamp (bool enable, bool micros)
{
#ifdef _WIN32
  micros = false;
#endif
  doTimestamp = enable;
  doMicros = micros;
}

static char *timestamp (char *buf, bool micros)
{
  struct tm *tm;
  if (micros) {
#if !defined(_WIN32)
    struct timeval tv;
    gettimeofday (&tv, NULL);
    tm = localtime((const time_t *)&tv.tv_sec);
    sprintf (buf, "%04d-%02d-%02d %02d:%02d:%02ld.%06ld: ", tm->tm_year+1900,
	     tm->tm_mon+1,
	     tm->tm_mday, tm->tm_hour, tm->tm_min, (long)tm->tm_sec, (long)tv.tv_usec );
#endif
  } else {
    time_t tt;
    time (&tt);
    tm = localtime (&tt);
    sprintf (buf, "%04d-%02d-%02d %02d:%02d:%02d: ", tm->tm_year+1900, tm->tm_mon+1,
	     tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
  }
  return buf;
}


void formatDebug(const char* fmt, ...)
{
  if (debugLevel >= 1) {
    char buffer[8192];
    char tsbuf[26];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 8192, fmt, args);
    va_end(args);
    #if defined(_MSC_VER)
      if (doTimestamp)
	W32_DEBUG_TRACE(timestamp (tsbuf, false));
      W32_DEBUG_TRACE(buffer);
    #else
      if (doTimestamp)
	std::cout << timestamp (tsbuf, doMicros);
      std::cout << buffer;
    #endif
  }
}

