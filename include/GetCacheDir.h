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

#ifndef __GET_CACHE_DIR_H__
#define __GET_CACHE_DIR_H__

#include "common.h"

/* interface system headers */
#include <string>

// FIXME - need to rename this file, and src/game/GetCacheDir.cxx
//         to DirectoryNames.<c|h>[xx] or something

extern std::string		getConfigDirName();
extern std::string		getCacheDirName();
extern std::string		getCaptureDirName();
extern std::string		getScreenShotDirName();

#endif  // __GET_CACHE_DIR_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
