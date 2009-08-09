/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

#ifdef _WIN32
#include <windows.h>
#else
#ifdef __APPLE__
#include <sstream>
#include <CoreServices/CoreServices.h>
#include <sys/sysctl.h>
#else
#include <sys/utsname.h>
#endif // __APPLE__
#endif // _WIN32

// common headers
#include "TextUtils.h"


// opaque version number increments on protocol incompatibility
#ifndef BZ_PROTO_VERSION
#  define BZ_PROTO_VERSION	"0108"
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
#  define BZ_REV		50
#endif

// DEVEL | STABLE | MAINT
#ifndef BZ_BUILD_TYPE
#  define BZ_BUILD_TYPE		"DEVEL"
#endif

const char *bzfcopyright = "Copyright (c) 1993 - 2009 Tim Riker";


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
/* yes this is horrible but it needs to be done to get it right */
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

std::string		getOSString()
{
  std::string versionString = "unknown";
#ifdef _WIN32
  // build up version string
  OSVERSIONINFOEX versionInfo;
  SYSTEM_INFO systemInfo;

  versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  GetVersionEx((LPOSVERSIONINFO)&versionInfo);
  GetNativeSystemInfo(&systemInfo);

  std::string platform = "Win32";
  if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    platform = "Win64";
  if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    platform = "WinIA64";
  versionString = TextUtils::format("%s%d.%d.%d sp%d.$d",platform.c_str(),versionInfo.dwMajorVersion,versionInfo.dwMinorVersion,versionInfo.dwBuildNumber, versionInfo.wServicePackMajor,versionInfo.wServicePackMinor);
#else
#ifdef __APPLE__
  OSErr err = noErr;
  
  long systemMajor, systemMinor, systemBugFix = 0;
  err = Gestalt(gestaltSystemVersionMajor, &systemMajor);
  if (err == noErr) {
    err = Gestalt(gestaltSystemVersionMinor, &systemMinor);
    if (err == noErr) {
      err = Gestalt(gestaltSystemVersionBugFix, &systemBugFix);
    }
  }
  
  std::stringstream reply;
  if (err == noErr) {
    reply << "MacOS ";
    reply << systemMajor;
    reply << ".";
    reply << systemMinor;
    reply << ".";
    reply << systemBugFix;
  } else {
    reply << "unknown system version (Gestalt error)";
  }
  
  long systemArchitecture = 0;
  err = Gestalt(gestaltSysArchitecture, &systemArchitecture);
  if (err == noErr) {
    switch (systemArchitecture) {
      case gestalt68k: {reply << " 68k"; break;}
      case gestaltPowerPC: {reply << " PPC"; break;}
      case gestaltIntel: {reply << " i386"; break;}
      default: {reply << " unknown CPU architecture (Gestalt reply is ";
	reply << systemArchitecture; reply << ")";}
    }
  } else {
    reply << " unknown CPU architecture (Gestalt error)";
  }
  
  int value = 0;
  size_t length = sizeof(value);
  if (sysctlbyname("hw.cpu64bit_capable", &value, &length, NULL, 0) == 0) {
    if (value) {
      reply << "; CPU 64 bit capable";
    }
  }
  
  // "MacOS 10.4.11 i386" for example
  versionString = reply.str();
#else
  struct utsname buf;
  if (uname(&buf) == 0) {
    std::vector<std::string> rtok = TextUtils::tokenize(buf.release, ".", 4);
    std::string rel;
    unsigned int i;
    // use up to three period separated elements of the release string
    for (i = 0; i < 3 && i < rtok.size(); i++) {
      if (rel.size() > 0)
	rel += ".";
      rel += rtok[i];
    }
    // "Linux 2.6.27 x86_64" for example
    versionString = TextUtils::format("%s %s %s", buf.sysname, rel.c_str(), buf.machine);
  }
  else {
    perror("uname");
    versionString = "unix unknown";
  }
#endif // __APPLE__
#endif // _WIN32
  return versionString;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
