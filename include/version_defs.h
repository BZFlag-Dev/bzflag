/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// This file contains the protocol and version definitions, and some utility
// macros used for assembling version strings. This file is parsed by NSIS and
// scripts, so the formatting/whitespace around the definitions should not be
// changed.

#pragma once

// opaque version number increments on protocol incompatibility
// update the following files (and their protocol implementations) to match:
//  misc/bzfquery.php
//  misc/bzfquery.pl
//  misc/bzfquery.py
//  misc/bzls.lua
#ifndef BZ_PROTO_VERSION
#  define BZ_PROTO_VERSION  "0221"
#endif

// version numbers - also update as needed:
//  ChangeLog
//  README
//  configure.ac
//  Xcode/BZFlag.xcodeproj/project.pbxproj
//  Xcode/BZFlag-Info.plist
#ifndef BZ_MAJOR_VERSION
#  define BZ_MAJOR_VERSION 2
#endif

#ifndef BZ_MINOR_VERSION
#  define BZ_MINOR_VERSION 4
#endif

#ifndef BZ_REV
#  define BZ_REV 31
#endif

// DEVEL | ALPHA | BETA | RC | STABLE
// DEVEL is used for typical unreleased or unfinished builds, like major dev versions (2.5.*)
//   or odd revision numbers (2.4.33)
// STABLE is for official stable releases, like 2.4.32, 2.4.34, etc
// ALPHA, BETA, and RC are for dev versions approaching a release
#ifndef BZ_BUILD_TYPE
#  define BZ_BUILD_TYPE "DEVEL"
#endif

// Useful for ALPHA, BETA, and RC
// For DEVEL and STABLE, this should be set to 0
#ifndef BZ_BUILD_TYPE_REVISION
#  define BZ_BUILD_TYPE_REVISION 0
#endif


/////////////////////////////////////////////////////////////////////////////////////////
// No need to modify the macros below. These are used inside of Version.rc on Windows. //
/////////////////////////////////////////////////////////////////////////////////////////

// Helper macros to convert numeric values to strings
#define BZ_STR_HELPER(x) #x
#define BZ_STR(x) BZ_STR_HELPER(x)

// Automatically construct the version string: "MAJOR.MINOR.REV.0"
#ifdef _WIN32
#ifndef BZ_VERSION_STR
#define BZ_VERSION_STR BZ_STR(BZ_MAJOR_VERSION) "." BZ_STR(BZ_MINOR_VERSION) "." BZ_STR(BZ_REV) ".0"
#endif
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
