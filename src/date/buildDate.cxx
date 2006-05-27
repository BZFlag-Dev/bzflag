/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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

/* system headers */
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>


// opaque version number increments on protocol incompatibility
#ifndef BZ_PROTO_VERSION
#define BZ_PROTO_VERSION	"0039"
#endif

// version numbers - also update:
//  configure.ac
//  ChangeLog
//  README
//  package/win32/nsis/*.nsi
//  package/win32/*.nsi
//  tools/TextTool-W32/TextTool.rc
//  win32/VC6/installer.dsp
#ifndef BZ_MAJOR_VERSION
#  define BZ_MAJOR_VERSION	2
#endif

#ifndef BZ_MINOR_VERSION
#  define BZ_MINOR_VERSION	1
#endif

#ifndef BZ_REV
#  define BZ_REV		7
#endif

// DEVEL | STABLE | MAINT
#ifndef BZ_BUILD_TYPE
#  define BZ_BUILD_TYPE		"DEVEL"
#endif

const char *bzfcopyright = "Copyright (c) 1993 - 2006 Tim Riker";


//
//  Although the ./configure process will generate
//  -DBZ_BUILD_DATE for the build, here it's voided.
//
//  Could someone explain the reason for the
//  inconience caused by the ./configure method? This
//  way is simple, touch the *.cxx to get a new time
//  stamp (no big recompiles). If this file is updated,
//  you are also forced to get a new timestamp.
//
//  Using __DATE__ for all OSes is more consistent.
//
#undef BZ_BUILD_DATE


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

const char*		getMajorMinorVersion()
{
  static std::string	version = "";
  if (!version.size()){
    std::ostringstream	versionStream;
    versionStream << BZ_MAJOR_VERSION << "." << BZ_MINOR_VERSION;
    version = versionStream.str();
  }
  return version.c_str();
}

const char*		getMajorMinorRevVersion()
{
  static std::string	version = "";
  if (!version.size()){
    std::ostringstream	versionStream;
    versionStream << BZ_MAJOR_VERSION << "." << BZ_MINOR_VERSION << "." << BZ_REV;
    version = versionStream.str();
  }
  return version.c_str();
}

const char*		getAppVersion()
{
  static std::string	appVersion = "";
  if (!appVersion.size()){
    std::ostringstream	appVersionStream;
    // TODO add current platform, release, cpu, etc
    appVersionStream << getMajorMinorRevVersion() << "." << BZ_BUILD_DATE
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
