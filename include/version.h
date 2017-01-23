/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef BZ_CONFIG_FILE_NAME
#define BZ_CONFIG_FILE_NAME	"config.cfg"
#endif

#define BZ_CONNECT_HEADER	"BZFLAG\r\n\r\n"

extern const char *bzfcopyright;

// various version functions in buildDate.cxx
extern const char* getProtocolVersion();
extern const char* getServerVersion();
extern const char* getMajorMinorVersion();
extern const char* getMajorMinorRevVersion();
extern const char* getAppVersion();

#endif //__VERSION_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
