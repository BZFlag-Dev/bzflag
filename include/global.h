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
			JAFOPlayer,
			ComputerPlayer
};

// team info
const int		NumTeams = 7;
const int		CtfTeams = 5;
enum TeamColor {
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
const float		Epsilon =	1.0e-5f;	// arbitrary
const float		Infinity =	MAXFLOAT;	// arbitrary

#define	DEFAULT_GRAVITY	-9.81f
#define DEFAULT_WORLD	800
// universe info
extern float	WorldSize;							// meters
const float		BaseSize =	60.0f;					// meters

// rough tank geometry
const float		TankHeight =	2.05f;		// meters
const float		TankRadius =	4.32f;// meters
const float		MuzzleHeight =	1.57f;		// meters
const float		MuzzleFront =	TankRadius+0.1f;// meters

// rough shot geometry
const float		ShotRadius =	0.5f;		// meters
const float		ShotLength =	0.5f;		// meters
const float		ShotTailLength=	4.0f;		// meters

// outer wall geometry
const float		WallHeight =	3.0f*TankHeight;// meters

// pyramid geometry
const float		PyrBase =	4.0f*TankHeight;// meters
const float		PyrHeight =	5.0f*TankHeight;// meters

// box geometry
const float		BoxBase =	30.0f;// meters
const float		BoxHeight =	6.0f*MuzzleHeight;// meters

// teleporter geometry (My God, it's full of stars...)
const float		TeleUnit =	1.12f;// meters
const float		TeleWidth =	1.0f * TeleUnit;// meters
const float		TeleBreadth =	4.0f * TeleUnit;// meters
const float		TeleHeight =	9.0f * TeleUnit;// meters

// tank performance info
const float		TankAngVel =	M_PI / 4.0f;	// radians/sec
const float		ShotSpeed =	100.0f;		// meters/sec
const float		ShotRange =	350.0f;		// meters
const float		ReloadTime =	ShotRange / ShotSpeed;	// seconds

// city geometry
const int		CitySize =	5;
const float		AvenueSize =	2.0f * BoxBase;	// meters

// other game info
const float		TeleportTime =	1.0f;		// seconds
const float		FlagAltitude =	11.0f;		// meters
const float		FlagRadius =	2.5f;		// meters

// readout stuff
const int		MaxMessages =	20;		// msg. history length
const float		FlashOnTime =	1.0f;		// seconds
const float		FlashOffTime =	0.2f;		// seconds
const int		MinX = 256;
const int		MinY = 192;
const int		NoMotionSize =	10;		// no motion zone size
const int		MaxMotionSize = 37;		// motion zone size
const float		RadarLowRangeFactor =	0.25f;// low radar range
const float		RadarMedRangeFactor =	0.5f;	// medium radar range
const float		RadarHiRangeFactor =	1.0f;	// high radar range

// game styles
enum GameStyle {
			PlainGameStyle =		0x0000,
			TeamFlagGameStyle =		0x0001,	// capture the flag
			SuperFlagGameStyle =		0x0002,	// superflags allowed
			RoguesGameStyle =		0x0004,	// rogues allowed
			JumpingGameStyle =		0x0008,	// jumping allowed
			InertiaGameStyle =		0x0010,	// momentum for all
			RicochetGameStyle =		0x0020,	// all shots ricochet
			ShakableGameStyle =		0x0040,	// can drop bad flags
			AntidoteGameStyle =		0x0080,	// anti-bad flags
			TimeSyncGameStyle =		0x0100,	// time sync'd to srvr
			RabbitChaseGameStyle =	0x0200  // rabbit chase
};

// map object flags
#define _DRIVE_THRU	0x01
#define _SHOOT_THRU 0x02
#define _FLIP_Z		0x04

const int mapVersion = 1;

struct GlobalDBItem {
  public:
    const char*			name;
    const char*			value;
    bool			persistent;
    StateDatabase::Permission	permission;
};
extern GlobalDBItem		globalDBItems[51];

#endif // BZF_GLOBAL_H
// ex: shiftwidth=2 tabstop=8
