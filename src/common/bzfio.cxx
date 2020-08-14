/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// Interface header
#include "bzfio.h"

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

// Common header
#include "TimeKeeper.h"

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
    int year, month, day, hour, min, sec;
    long tv_usec;
    long *usecP;
    if (micros)
        usecP = &tv_usec;
    else
        usecP = nullptr;
    if (utc)
        TimeKeeper::UTCTime(&year, &month, &day, nullptr, &hour, &min, &sec, nullptr, usecP);
    else
        TimeKeeper::localTime(&year, &month, &day, &hour, &min, &sec, nullptr, usecP);
    if (micros)
        snprintf (buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02d.%06ld: ", year,
                  month, day, hour, min, sec, tv_usec);
    else
        snprintf (buf, tsBufferSize, "%04d-%02d-%02d %02d:%02d:%02d: ", year, month,
                  day, hour, min, sec);
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

    if (debugLevel >= level || level == 0)
    {
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
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
