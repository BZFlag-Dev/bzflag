/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "bzfio.h"
#include "common.h"

#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

static bool doTimestamp = false;
static bool doMicros = false;

void setDebugTimestamp (bool enable, bool micros) 
{ 
  doTimestamp = enable;
  doMicros = micros;
}

static char *timestamp (char *buf){
  struct tm *tm;
  if (doMicros) {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    tm = localtime (&tv.tv_sec);
    sprintf (buf, "%04d-%02d-%02d %02d:%02d:%02d.%06ld: ", tm->tm_year+1900, tm->tm_mon+1,
             tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec );
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
      W32_DEBUG_TRACE(buffer);
    #else
      if (doTimestamp) {
        std::cout << timestamp (tsbuf);
      }
      std::cout << buffer;
    #endif
  }
}

