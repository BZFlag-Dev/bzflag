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

#include "Common.h"
#include "Log.h"

INSTANTIATE_SINGLETON(Log);

Log::Log()
{
  logFile = stdout;
}

void Log::outLog(const char *format, ...)
{
  fprintf(logFile, "LOG: ");
  va_list args;
  va_start (args, format);
  vfprintf (logFile, format, args);
  va_end (args);
  fprintf(logFile, "\n");
}

void Log::outDebug(const char *format, ...)
{
  fprintf(logFile, "DEB: ");
  va_list args;
  va_start (args, format);
  vfprintf (logFile, format, args);
  va_end (args);
  fprintf(logFile, "\n");
}

void Log::outError(const char *format, ...)
{
  fprintf(logFile, "ERR: ");
  va_list args;
  va_start (args, format);
  vfprintf (logFile, format, args);
  va_end (args);
  fprintf(logFile, "\n");
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8