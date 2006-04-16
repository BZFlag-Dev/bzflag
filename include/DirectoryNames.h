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

#ifndef __DIRECTORY_NAMES_H__
	#define __DIRECTORY_NAMES_H__

	#include "common.h"

/* interface system headers */
	#include <string>


	#ifndef _WIN32
const char DirectorySeparator = '/';
	#else 
const char DirectorySeparator = '\\';
	#endif 

extern std::string getConfigDirName( const char *versionName = NULL );
extern std::string getCacheDirName();
extern std::string getRecordDirName();
extern std::string getScreenShotDirName();
extern std::string getTempDirName();
extern std::string getWorldDirName();

extern void setCustomConfigDir( const char *dir );
#endif // __DIRECTORY_NAMES_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
