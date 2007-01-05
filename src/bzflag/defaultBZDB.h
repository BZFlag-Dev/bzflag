/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* defaultBZDB.h:
*	defaults for the BZDB
*/

#ifndef	__DEFAULT_BZDB_H__
#define	__DEFAULT_BZDB_H__

#include "StateDatabase.h"

// default database entries
struct DefaultDBItem {
	const char*			name;
	const char*			value;
	bool				persistent;
	StateDatabase::Permission	permission;
	StateDatabase::Callback	callback;
};

extern DefaultDBItem	defaultDBItems[];

void loadBZDBDefaults ( void );

#endif // __DEFAULT_BZDB_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
