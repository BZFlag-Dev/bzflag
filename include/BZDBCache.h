/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZDBCACHE_H
#define	BZDBCACHE_H

#include <string>

#include "StateDatabase.h"

class BZDBCache
{
public:
	static void init();

	static bool  displayMainFlags;

	static float maxLOD;
	static float tankHeight;


private:
	static void clientCallback(const std::string &name, void *);
	static void serverCallback(const std::string &name, void *);
};

#endif
