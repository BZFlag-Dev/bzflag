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

/** The Log class implements basic logging for the server
  * with three logging types, outLog, outDebug and outError.
  */

#ifndef __BZAUTHD_LOG_H__
#define __BZAUTHD_LOG_H__

#include <stdio.h>
#include <Singleton.h>

class Log : public Singleton<Log>
{
public:
  Log();
  void outLog(const char *format, ...);
  void outDebug(const char *format, ...);
  void outError(const char *format, ...);
private:
  FILE * logFile;
};

#define sLog Log::instance()

#endif // __BZAUTHD_LOG_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8