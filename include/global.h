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

/*
 * Global constants
 */

#ifndef	BZF_GLOBAL_H
#define	BZF_GLOBAL_H

#include <math.h>
#include "common.h"
#include "StateDatabase.h"


// values affecting struct and class layout
const int		CallSignLen = 32;	// including terminating NUL
const int		EmailLen = 128;		// including terminating NUL
const int		MessageLen = 128;	// including terminating NUL

// types of things we can be
enum PlayerType {
			TankPlayer,
			ComputerPlayer
};

// team info
const int		NumTeams = 7;
const int		CtfTeams = 5;
enum TeamColor {
			AutomaticTeam = -2,
			NoTeam = -1,
			RogueTeam = 0,
			RedTeam = 1,
			GreenTeam = 2,
			BlueTeam = 3,
			PurpleTeam = 4,
			ObserverTeam = 5,
			RabbitTeam = 6
};

#ifdef ROBOT
// robots
#define MAX_ROBOTS 100
#endif

// epsilon and very far for ray intersections
const float		Epsilon =	ZERO_TOLERANCE;	// arbitrary
const float		Infinity =	MAXFLOAT;	// arbitrary

#define DEFAULT_WORLD	800

// readout stuff
const int		MaxMessages =	20;		// msg. history length
const int		MinX = 256;
const int		MinY = 192;
const int		NoMotionSize =	10;		// no motion zone size
const int		MaxMotionSize = 37;		// motion zone size

// game styles
enum GameStyle {
	PlainGameStyle =		0x0000,
	TeamFlagGameStyle =		0x0001,	// capture the flag
	SuperFlagGameStyle =		0x0002,	// superflags allowed
	RunFlagGameStyle =		0x0004,	// run the flag (formerly hold-the-flag)

	JumpingGameStyle =		0x0008,	// jumping allowed
	InertiaGameStyle =		0x0010,	// momentum for all
	RicochetGameStyle =		0x0020,	// all shots ricochet
	ShakableGameStyle =		0x0040,	// can drop bad flags
	AntidoteGameStyle =		0x0080,	// anti-bad flags
	HandicapGameStyle =		0x0100, // hadicap players based on ability

	RabbitChaseGameStyle =		0x0200	// rabbit chase
};

// map object flags
#define _DRIVE_THRU	0x01
#define _SHOOT_THRU	0x02
#define _FLIP_Z		0x04

const int mapVersion = 1;

struct GlobalDBItem {
  public:
    const char*			name;
    const char*			value;
    bool			persistent;
    StateDatabase::Permission	permission;
};
extern const unsigned int numGlobalDBItems;
extern const struct GlobalDBItem globalDBItems[];

#endif // BZF_GLOBAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
