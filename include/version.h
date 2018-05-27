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

#include "config.h"

#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef BZ_CONFIG_DIR_VERSION
#define BZ_CONFIG_DIR_VERSION   "2.4"
#endif

#ifndef BZ_CONFIG_FILE_NAME
#define BZ_CONFIG_FILE_NAME "config.cfg"
#endif

#ifndef BZ_CONFIG_FILE_VERSION
#define BZ_CONFIG_FILE_VERSION  5
#endif

#define BZ_CONNECT_HEADER   "BZFLAG\r\n\r\n"

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
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
