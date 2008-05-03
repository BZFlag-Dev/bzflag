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

#include "common.h"

/* system headers */
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>


// opaque version number increments on protocol incompatibility
#ifndef BZ_PROTO_VERSION
#  define BZ_PROTO_VERSION	"0061"
#endif
// ditto for bzrobots
#ifndef BZROBOTS_PROTO_VERSION
#  define BZROBOTS_PROTO_VERSION    "0001"
#endif

// version numbers - also update:
//  README
//  configure.ac
//  include/version.h
//  package/win32/nsis/BZFlag.nsi
#ifndef BZ_MAJOR_VERSION
#  define BZ_MAJOR_VERSION	2
#endif

#ifndef BZ_MINOR_VERSION
#  define BZ_MINOR_VERSION	99
#endif

#ifndef BZ_REV
#  define BZ_REV		03
#endif

// DEVEL | STABLE | MAINT
#ifndef BZ_BUILD_TYPE
#  define BZ_BUILD_TYPE		"DEVEL"
#endif

const char *bzfcopyright = "Copyright (c) 1993 - 2008 Tim Riker";


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
static const char buildDate[] = {__DATE__};

static const char monthsOfYear[][4] = {"Jan","Feb","Mar","Apr","May","Jun",
                                       "Jul","Aug","Sep","Oct","Nov","Dec"};

int getBuildDate()
{
  int year = 1900, month, day = 0;
  char monthStr[4];

  sscanf(buildDate, "%4s %d %d", monthStr, &day, &year);

  for (month = 12; month > 1; --month) {
    if (strcmp(monthStr, monthsOfYear[month-1]) == 0)
      break;
  }

  return 10000*year + 100*month + day;
}
#endif
// down here so above gets created
#include "version.h"

const char*		getProtocolVersion()
{
  static std::string protVersion = BZ_PROTO_VERSION;
  return protVersion.c_str();
}

const char*		getRobotsProtocolVersion()
{
  static std::string robotsProtVersion = BZROBOTS_PROTO_VERSION;
  return robotsProtVersion.c_str();
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
