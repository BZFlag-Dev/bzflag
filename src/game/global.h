/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Global constants
 */

#ifndef BZF_GLOBAL_H
#define BZF_GLOBAL_H

#include <math.h>
#include "common.h"
#include "StateDatabase.h"

// values affecting struct and class layout
const int				CallSignLen = 32;		// including terminating NUL
const int				EmailLen = 128;			// including terminating NUL
const int				MessageLen = 128;		// including terminating NUL

// types of things we can be
enum PlayerType {
						TankPlayer,
						JAFOPlayer,
						ComputerPlayer
};

// team info
const unsigned int		NumTeams = 6;
enum TeamColor {
						NoTeam = -1,
						RogueTeam = 0,
						RedTeam = 1,
						GreenTeam = 2,
						BlueTeam = 3,
						PurpleTeam = 4,
						KingTeam = 5
};

// epsilon and very far for ray intersections
const float				Epsilon =		1.0e-5f;			// arbitrary
const float				Infinity =		MAXFLOAT;			// arbitrary

// universe info
const float				BaseSize =		60.0f;				// meters

// rough shot geometry
const float				ShotRadius =	0.5f;				// meters
const float				ShotLength =	0.5f;				// meters
const float				ShotTailLength=	10.0f;				// meters

// outer wall geometry
const float				WallPosition =	404.5f;				// meters
const float				WallHeight =	6.15f;				// meters

// pyramid geometry
const float				PyrBase =		8.2f;				// meters
const float				PyrHeight =		10.25f;				// meters

// box geometry
const float				BoxBase = 		30.0f;				// meters
const float				BoxHeight = 	9.42f;				// meters

// teleporter geometry (My God, it's full of stars...)
const float				TeleUnit =		1.12f;				// meters
const float				TeleWidth =		1.0f * TeleUnit;	// meters
const float				TeleBreadth =	4.0f * TeleUnit;	// meters
const float				TeleHeight = 	9.0f * TeleUnit;	// meters

// city geometry
const int				CitySize = 		5;
const float				AvenueSize = 	2.0f * BoxBase;		// meters

// readout stuff
const int				MaxMessages =	20;					// msg. history length
const int				MaxMessageLen =	80;					// length of message
const float				FlashOnTime = 	1.0f;				// seconds
const float				FlashOffTime =	0.2f;				// seconds

// game styles
enum GameStyle {
						PlainGameStyle =		0x0000,
						TeamFlagGameStyle =		0x0001,		// capture the flag
						SuperFlagGameStyle =	0x0002,		// superflags allowed
						RoguesGameStyle =		0x0004,		// rogues allowed
						JumpingGameStyle =		0x0008,		// jumping allowed
						InertiaGameStyle =		0x0010,		// momentum for all
						RicochetGameStyle =		0x0020,		// all shots ricochet
						ShakableGameStyle =		0x0040,		// can drop bad flags
						AntidoteGameStyle =		0x0080,		// anti-bad flags
						TimeSyncGameStyle =		0x0100,		// time sync'd to srvr
						KingoftheHillGameStyle= 0x0200		// king of the hill};
};

// flag stuff
// Flags add some spice to the game.  There are two kinds of flags:
// team flags and super flags.  Super flags come in two types: good
// and bad.
//   When playing a "capture the flag" style game, each team with at
// least one player has a team flag which has the same color as the
// team.  A team flag will remain in the game as long as there is a
// player on that team.  A team flag may be picked up and freely
// dropped at any time.  It may be captured, which causes it to go
// back to it's home position (centered in the team base).  If a
// flag is dropped by a hostile player in a third team's base, the
// flag will go to the third team's flag safety position.  For example,
// if a Green Team player dropped the Red Flag on Blue's Base, the
// Red Flag would go to the Blue Team's safety position.  This is
// because if it stayed in the Blue Base, any Red Team member who
// picked it up would instantly have brought his team flag into
// enemy territory and so blow up his whole team.
//   A super flag causes the characteristics of the tank that possesses
// it to change.  A good super flag generally makes the tank more
// powerful or deadly.  A bad super flag generally does the opposite.
// A good super flag may always be dropped.  A bad super flag is
// "sticky" which means that it can't be freely dropped.  The server
// may have some means of getting rid of a bad super flag (perhaps
// by destroying an enemy or two or after waiting 20 seconds).
// The creation and destruction of super flags is under the server's
// control so super flags may appear and disappear seemingly at
// random.
//
// FlagId
//		This enumerates all the types of flags.  The integer values
//		of the enumerations come it two sets:  the team flags then
//		the super-flags.  The good super flags are listed first and
//		the bad second;  this is just for clarity and isn't necessary.
enum FlagId {
						NullFlag = 0,			// the null flag (or rogue
						NoFlag = RogueTeam,		// team's non-existent flag)

						// team flags:
						RedFlag = RedTeam,		// red team's flag
						GreenFlag = GreenTeam,	// green team's flag
						BlueFlag = BlueTeam,	// blue team's flag
						PurpleFlag = PurpleTeam,// purple teams flag

						// the good super flags:
						VelocityFlag,			// drive faster
						QuickTurnFlag,			// turn faster
						OscOverthrusterFlag,	// go rent Buckaroo Banzai
						RapidFireFlag,			// shoot faster
						MachineGunFlag,			// shoot very fast, close range
						GuidedMissileFlag,		// seeks the 'locked on' target
						LaserFlag,				// infinite speed & range shot
						RicochetFlag,			// shots bounce off walls
						SuperBulletFlag,		// shoots through schools
						InvisibleBulletFlag,	// your shots are invisible
						StealthFlag,			// makes you invisible on radar
						TinyFlag,				// makes you very small
						NarrowFlag,				// makes you infinitely thin
						ShieldFlag,				// can get shot once harmlessly
						SteamrollerFlag,		// kill by touching
						ShockWaveFlag,			// kills all within small range
						PhantomZoneFlag,		// teleporters phantomize you
						GenocideFlag,			// kill one kills whole team
						JumpingFlag,			// lets your tank jump
						IdentifyFlag,			// lets you identify other flags
						CloakingFlag,			// make you invisible out window
						MasqueradeFlag,			// makes tank look like teammate
						ThiefFlag,				// can steal another's flag
						BurrowFlag,				// burrows tank into the ground
						SeerFlag,				// see cloak, stealth and masquerade as normal

						// the bad super flags
						ColorblindnessFlag,		// can't see team colors
						ObesityFlag,			// makes you really fat
						LeftTurnOnlyFlag,		// can't turn right
						RightTurnOnlyFlag,		// can't turn left
						MomentumFlag,			// gives you lots of momentum
						BlindnessFlag,			// can't see out window
						JammingFlag,			// radar doesn't work
						WideAngleFlag,			// fish eye view (sounds good
												// but isn't)
						NoJumpingFlag,			// tank cannot jump
						LongReloadFlag,			// tank takes twice as long to reload


						// special flags for size of team and super-flag sets
						FirstFlag =			RedFlag,
						LastFlag =			LongReloadFlag,
						FirstTeamFlag =		RedFlag,
						LastTeamFlag =		PurpleFlag,
						FirstSuperFlag =	VelocityFlag,
						LastSuperFlag =		LastFlag
};

struct GlobalDBItem {
public:
	const char*				name;
	const char*				value;
	bool					persistent;
	StateDatabase::Permission	permission;
};
extern GlobalDBItem			globalDBItems[49];

#endif // BZF_GLOBAL_H
// ex: shiftwidth=4 tabstop=4
