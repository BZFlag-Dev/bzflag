/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sstream>
#include <string>

#include <stdio.h>
#include <string.h>
#include "config.h"

// opaque version number increments on protocol incompatibility
#ifndef BZ_PROTO_VERSION
#define BZ_PROTO_VERSION	"0021"
#endif

#ifndef BZ_MAJOR_VERSION
#define BZ_MAJOR_VERSION	1
#endif

#ifndef BZ_MINOR_VERSION
#define BZ_MINOR_VERSION	11
#endif

#ifndef BZ_REV
#define BZ_REV			31
#endif

// DEVEL | STABLE | MAINT
#ifndef BZ_BUILD_TYPE
#define BZ_BUILD_TYPE		"DEVEL"
#endif

const char *bzfcopyright = "Copyright (c) 1993 - 2004 Tim Riker";

#ifndef BZ_BUILD_DATE
/* to get the version in the right format YYYYMMDD */
/* yes this is horible but it needs to be done to get it right */
/* windows should pull from a resouce */
/* *nix gets this from the passed from my the Makefile */
char buildDate[] = {__DATE__};

int getBuildDate()
{
  int year = 1900, month = 0, day = 0;
  char monthStr[512];
  sscanf(buildDate, "%s %d %d", monthStr, &day, &year);

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

  return (year*10000) + (month*100)+ day;
}
#endif
// down here so above gets created
#include "version.h"

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

const char*		getAppVersion()
{
  static std::string	appVersion = "";
  if (!appVersion.size()){
    std::ostringstream	appVersionStream;
    // TODO add current platform, release, cpu, etc
    appVersionStream << BZ_MAJOR_VERSION << "." << BZ_MINOR_VERSION << "." << BZ_REV << "." << BZ_BUILD_DATE
	<< "-" << BZ_BUILD_TYPE << "-" << BZ_BUILD_OS;
#ifdef HAVE_SDL
    appVersionStream << "-SDL";
#endif
    appVersion = appVersionStream.str();
  }
  return appVersion.c_str();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
