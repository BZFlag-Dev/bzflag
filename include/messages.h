/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* Communication protocol constants
*/

#ifndef	BZF_MESSAGES_H
#define	BZF_MESSAGES_H

#include "common.h"

/* system interface headers */
#include <string>


#define _CallSignLen		32		// including terminating NUL
#define _PasswordLen		32		// including terminating NUL
#define _EmailLen				128		// including terminating NUL
#define _TokenLen				22		// opaque string (now int(10)) and terminating NUL
#define _VersionLen			60		// including terminating NUL
#define _MessageLen			128		// including terminating NUL

class PlayerAddMessage
{
public:
	PlayerAddMessage();

	bool unpack ( void* buf );
	void* pack ( void* buf );

	int playerID;
	int team;
	int type;
	int wins;
	int losses;
	int	tks;
	std::string callsign;
	std::string email;
};

#endif // BZF_MESSAGES_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

