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

#include "common.h"
#include "version.h"

/* system headers */
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>


// opaque version number increments on protocol incompatibility
// update the following files (and their protocol implementations) to match:
//  misc/bzfquery.php
//  misc/bzfquery.pl
//  misc/bzfquery.py
//  misc/bzls.lua
#ifndef BZ_PROTO_VERSION
#  define BZ_PROTO_VERSION	"0221"
#endif

// version numbers - also update as needed:
//  ChangeLog
//  MSVC/bzflag.rc
//  README
//  configure.ac
//  include/version.h
//  package/win32/nsis/BZFlag.nsi
#ifndef BZ_MAJOR_VERSION
#  define BZ_MAJOR_VERSION	2
#endif

#ifndef BZ_MINOR_VERSION
#  define BZ_MINOR_VERSION	4
#endif

#ifndef BZ_REV
#  define BZ_REV		14
#endif

// DEVEL | RC# | STABLE | MAINT
#ifndef BZ_BUILD_TYPE
#  define BZ_BUILD_TYPE		"MAINT"
#endif

const char *bzfcopyright = "Copyright (c) 1993-2018 Tim Riker";

static int getBuildDate()
{
  int year = 1900, month = 0, day = 0;
#ifdef BUILD_DATE
  // the BUILD_DATE define looks like "YYYY-MM-DD"
  sscanf(BUILD_DATE, "%d-%d-%d", &year, &month, &day);
#else
  char monthStr[512];
  // the __DATE__ macro looks like "Jun 15 2013" (*with* the quotes)
  sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);

  // we want it not as a name but a number
  if (strcmp(monthStr, "Jan") == 0)
    month = 1;
  else if (strcmp(monthStr, "Feb") == 0)
    month = 2;
  else if (strcmp(monthStr, "Mar") == 0)
    month = 3;
  else if (strcmp(monthStr, "Apr") == 0)
    month = 4;
  else if (strcmp(monthStr, "May") == 0)
    month = 5;
  else if (strcmp(monthStr, "Jun") == 0)
    month = 6;
  else if (strcmp(monthStr, "Jul") == 0)
    month = 7;
  else if (strcmp(monthStr, "Aug") == 0)
    month = 8;
  else if (strcmp(monthStr, "Sep") == 0)
    month = 9;
  else if (strcmp(monthStr, "Oct") == 0)
    month = 10;
  else if (strcmp(monthStr, "Nov") == 0)
    month = 11;
  else if (strcmp(monthStr, "Dec") == 0)
    month = 12;
#endif

  return (year*10000) + (month*100) + day;
}

const char*		getProtocolVersion()
{
  static std::string protVersion = BZ_PROTO_VERSION;
  return protVersion.c_str();
}

const char*		getServerVersion()
{
  static std::string serverVersion = std::string("BZFS") + getProtocolVersion();
  return serverVersion.c_str();
}

const char*		getMajorMinorVersion()
{
  static std::string	version = "";
  if (!version.size()) {
    std::ostringstream	versionStream;
    versionStream << BZ_MAJOR_VERSION << "." << BZ_MINOR_VERSION;
    version = versionStream.str();
  }
  return version.c_str();
}

const char*		getMajorMinorRevVersion()
{
  static std::string	version = "";
  if (!version.size()) {
    std::ostringstream	versionStream;
    versionStream << BZ_MAJOR_VERSION << "." << BZ_MINOR_VERSION << "." << BZ_REV;
    version = versionStream.str();
  }
  return version.c_str();
}

const char*		getAppVersion()
{
  static std::string	appVersion = "";
  if (!appVersion.size()) {
    std::ostringstream	appVersionStream;
    // TODO add current platform, release, cpu, etc
    appVersionStream << getMajorMinorRevVersion() << "." << getBuildDate()
	<< "-" << BZ_BUILD_TYPE << "-" << BZ_BUILD_OS;
#ifdef HAVE_SDL
    appVersionStream << "-SDL";
#ifdef HAVE_SDL2
    appVersionStream << "2";
#endif
#endif
    appVersion = appVersionStream.str();
  }
  return appVersion.c_str();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
