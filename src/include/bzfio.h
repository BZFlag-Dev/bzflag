/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZFIO_H__
#define __BZFIO_H__

#include <string>
#include <stdarg.h>


//============================================================================//

/** global debug level used by libraries and applications, provided in bzfio.cpp */
extern int debugLevel;

void logDebugMessage(int level, const char* fmt, ...);
void logDebugMessage(int level, const std::string &text);
void logDebugMessageArgs(int level, const char* fmt, va_list ap);

void setDebugTimestamp (bool enable, bool doMicros, bool utc);

// multiple logging routines can be registered
typedef void (*LoggingProc)(int level, const std::string& msg, void* data);
bool registerLoggingProc(LoggingProc, void* data);
bool unregisterLoggingProc(LoggingProc, void* data);


//============================================================================//


#endif /* __BZFIO_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
