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

#ifndef	BZF_PLAYERSTATE_H
#define	BZF_PLAYERSTATE_H

#include "common.h"

class PlayerState
{
public:
    enum PStatus {				// bit masks
			DeadStatus =	0x0000,	// not alive, not paused, etc.
			Alive =		0x0001,	// player is alive
			Paused =	0x0002,	// player is paused
			Exploding =	0x0004,	// currently blowing up
			Teleporting =	0x0008,	// teleported recently
			FlagActive =	0x0010,	// flag special powers active
			CrossingWall =	0x0020,	// tank crossing building wall
			Falling =	0x0040	// tank accel'd by gravity
    };

    PlayerState();
    void*		pack(void*, uint16_t& code);
    void*		unpack(void*, uint16_t code);

    long		order;			// packet ordering
    short		status;			// see PStatus enum
    float		pos[3];			// position of tank
    float		velocity[3];		// velocity of tank
    float		azimuth;		// orientation of tank
    float		angVel;			// angular velocity of tank
    int			phydrv;			// physics driver
};



#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
