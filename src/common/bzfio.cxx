/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "common.h"
#include "bzfio.h"

/* system implementation headers */
#include <iostream>
#include <vector>
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


/** global used to control logging level across all applications */
int debugLevel = 0;

static bool doMicros = false;
static bool doTimestamp = false;

static int callProcDepth = 0;


/******************************************************************************/
/******************************************************************************/

struct LoggingProcPair {
  LoggingProcPair(LoggingProc p, void* d)
  : proc(p)
  , data(d)
  {}
  bool operator==(const LoggingProcPair& lpp) const {
    return (proc == lpp.proc) && (data == lpp.data);
  }
  LoggingProc proc;
  void*       data;
};
typedef std::vector<LoggingProcPair> LoggingProcVec;
static LoggingProcVec loggingProcs;


bool registerLoggingProc(LoggingProc proc, void* data)
{
  if (proc == NULL) {
    return false;
  }
  LoggingProcPair lpp(proc, data);
  for (size_t i = 0; i < loggingProcs.size(); i++) {
    if (lpp == loggingProcs[i]) {
      return false; // already registered
    }
  }
  loggingProcs.push_back(lpp);
  return true;
}

  
bool unregisterLoggingProc(LoggingProc proc, void* data)
{
  if (callProcDepth != 0) {
    logDebugMessage(0, "error: unregisterLoggingProc() used in a LoggingProc");
    return false;
  }
  LoggingProcPair lpp(proc, data);
  for (size_t i = 0; i < loggingProcs.size(); i++) {
    if (lpp == loggingProcs[i]) {
      loggingProcs.erase(loggingProcs.begin() + i);
      return true;
    }
  }
  return false;
}


static void callProcs(int level, const std::string& msg)
{
  callProcDepth++;
  for (size_t i = 0; i < loggingProcs.size(); i++) {
    const LoggingProcPair& lpp = loggingProcs[i];
    lpp.proc(level, msg, lpp.data);
  }
  callProcDepth--;
}


/******************************************************************************/
/******************************************************************************/

void setDebugTimestamp(bool enable, bool micros)
{
#ifdef _WIN32
  micros = false;
#endif
  doTimestamp = enable;
  doMicros = micros;
}


static const int tsBufferSize = 26;

static char *timestamp(char *buf, bool micros)
{
  struct tm *tm;
  if (micros) {
#if !defined(_WIN32)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tm = localtime((const time_t *)&tv.tv_sec);
    snprintf(buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02ld.%06ld: ",
             tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, (long)tm->tm_sec, (long)tv.tv_usec);
#endif
  } else {
    time_t tt;
    time(&tt);
    tm = localtime(&tt);
    snprintf(buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02d: ",
             tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
  }
  return buf;
}


/******************************************************************************/
/******************************************************************************/

void logDebugMessage(int level, const char* fmt, va_list ap)
{
  char buffer[8192] = { 0 };
  char tsbuf[tsBufferSize] = { 0 };

  if (!fmt) {
    return;
  }

  vsnprintf(buffer, sizeof(buffer), fmt, ap);

  if ((debugLevel >= level) || (level == 0)) {
#if defined(_MSC_VER)
    if (doTimestamp) {
      W32_DEBUG_TRACE(timestamp (tsbuf, false));
    }
    W32_DEBUG_TRACE(buffer);
#else
    if (doTimestamp) {
      std::cout << timestamp (tsbuf, doMicros);
    }
    std::cout << buffer;
    fflush(stdout);
#endif
  }

  callProcs(level, buffer);
}


void logDebugMessage(int level, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  logDebugMessage(level, fmt, ap);
  va_end(ap);
}


void logDebugMessage(int level, const std::string &text)
{
  if (!text.size())
    return;

  if ((debugLevel >= level) || (level == 0)) {
    char tsbuf[26];
#if defined(_MSC_VER)
    if (doTimestamp) {
      W32_DEBUG_TRACE(timestamp(tsbuf, false));
    }
    W32_DEBUG_TRACE(text.c_str());
#else
    if (doTimestamp) {
      std::cout << timestamp(tsbuf, doMicros);
    }
    std::cout << text;
    fflush(stdout);
#endif
  }

  callProcs(level, text);
}


/******************************************************************************/
/******************************************************************************/
