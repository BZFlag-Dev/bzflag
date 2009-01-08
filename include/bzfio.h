/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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

void setDebugTimestamp (bool enable, bool doMicros);
void logDebugMessage(int level, const char* fmt, ...);
void logDebugMessage(int level, const std::string &text);

// multiple logging routines can be registered
typedef void (*LoggingProc)(int level, const std::string& msg, void* data);
bool registerLoggingProc(LoggingProc, void* data);
bool unregisterLoggingProc(LoggingProc, void* data);

/** global debug level used by libraries and applications, provided in bzfio.cxx */
extern int debugLevel;

#endif /* __BZFIO_H__ */

/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
