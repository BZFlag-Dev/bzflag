/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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

#ifndef	BZF_GLOBAL_H
#define	BZF_GLOBAL_H

#include <math.h>
#include "common.h"

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
const int		NumTeams = 5;
enum TeamColor {
			NoTeam = -1,
			RogueTeam = 0,
			RedTeam = 1,
			GreenTeam = 2,
			BlueTeam = 3,
			PurpleTeam = 4
};

// epsilon and very far for ray intersections
const float		Epsilon =	1.0e-5f;	// arbitrary
const float		Infinity =	MAXFLOAT;	// arbitrary

// universe info
const float		Gravity =	-9.8f;		// meters/sec/sec
const float		WorldSize =	800.0f;		// meters
const float		BaseSize =	60.0f;		// meters

// rough tank geometry
const float		TankLength =	6.0f;		// meters
const float		TankWidth =	2.8f;		// meters
const float		TankHeight =	2.05f;		// meters
const float		TankRadius =	0.72f*TankLength;// meters
const float		MuzzleHeight =	1.57f;		// meters
const float		MuzzleFront =	TankRadius+0.1f;// meters

// rough shot geometry
const float		ShotRadius =	0.5f;		// meters
const float		ShotLength =	0.5f;		// meters
const float		ShotTailLength=	10.0f;		// meters

// outer wall geometry
const float		WallPosition =	0.5f*WorldSize +// meters
						0.75f * TankLength;
const float		WallHeight =	3.0f*TankHeight;// meters

// pyramid geometry
const float		PyrBase =	4.0f*TankHeight;// meters
const float		PyrHeight =	5.0f*TankHeight;// meters

// box geometry
const float		BoxBase = 	5.0f*TankLength;// meters
const float		BoxHeight = 	6.0f*MuzzleHeight;// meters

// teleporter geometry (My God, it's full of stars...)
const float		TeleUnit =	0.4f * TankWidth;// meters
const float		TeleWidth =	1.0f * TeleUnit;// meters
const float		TeleBreadth =	4.0f * TeleUnit;// meters
const float		TeleHeight = 	9.0f * TeleUnit;// meters

// tank performance info
const float		TankSpeed =	25.0f;		// meters/sec
const float		TankAngVel =	M_PI / 4.0f;	// radians/sec
const float		ShotSpeed =	100.0f;		// meters/sec
const float		ShotRange = 	350.0f;		// meters
const float		ReloadTime =	ShotRange / ShotSpeed;	// seconds

// city geometry
const int		CitySize = 	5;
const float		AvenueSize = 	2.0f * BoxBase;	// meters

// other game info
const float		ExplodeTime =	5.0f;		// seconds
const float		TeleportTime =	1.0f;		// seconds
const float		FlagAltitude =	11.0f;		// meters
const float		FlagRadius =	2.5f;		// meters

// readout stuff
const int		MaxMessages =	20;		// msg. history length
const int		MaxMessageLen =	80;		// length of message
const float		FlashOnTime = 	1.0f;		// seconds
const float		FlashOffTime =	0.2f;		// seconds
const int		NoMotionSize =	10;		// no motion zone size
const int		MaxMotionSize = 200;		// motion zone size
const float		RadarLowRange =	0.25f*WorldSize;// low radar range
const float		RadarMedRange =	0.5f*WorldSize;	// medium radar range
const float		RadarHiRange =	WorldSize;	// high radar range

// game styles
enum GameStyle {
			PlainGameStyle =	0x0000,
			TeamFlagGameStyle =	0x0001,	// capture the flag
			SuperFlagGameStyle =	0x0002,	// superflags allowed
			RoguesGameStyle =	0x0004,	// rogues allowed
			JumpingGameStyle =	0x0008,	// jumping allowed
			InertiaGameStyle =	0x0010,	// momentum for all
			RicochetGameStyle =	0x0020,	// all shots ricochet
			ShakableGameStyle =	0x0040,	// can drop bad flags
			AntidoteGameStyle =	0x0080,	// anti-bad flags
			TimeSyncGameStyle =	0x0100	// time sync'd to srvr
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
//	This enumerates all the types of flags.  The integer values
//	of the enumerations come it two sets:  the team flags then
//	the super-flags.  The good super flags are listed first and
//	the bad second;  this is just for clarity and isn't necessary.
enum FlagId {
			NullFlag = 0,		// the null flag (or rogue
			NoFlag = RogueTeam,	// team's non-existent flag)

			// team flags:
			RedFlag = RedTeam,	// red team's flag
			GreenFlag = GreenTeam,	// green team's flag
			BlueFlag = BlueTeam,	// blue team's flag
			PurpleFlag = PurpleTeam,// purple teams flag

			// the good super flags:
			VelocityFlag,		// drive faster
			QuickTurnFlag,		// turn faster
			OscOverthrusterFlag,	// go rent Buckaroo Banzai
			RapidFireFlag,		// shoot faster
			MachineGunFlag,		// shoot very fast, close range
			GuidedMissileFlag,	// seeks the 'locked on' target
			LaserFlag,		// infinite speed & range shot
			RicochetFlag,		// shots bounce off walls
			SuperBulletFlag,	// shoots through schools
			InvisibleBulletFlag,	// your shots are invisible
			StealthFlag,		// makes you invisible on radar
			TinyFlag,		// makes you very small
			NarrowFlag,		// makes you infinitely thin
			ShieldFlag,		// can get shot once harmlessly
			SteamrollerFlag,	// kill by touching
			ShockWaveFlag,		// kills all within small range
			PhantomZoneFlag,	// teleporters phantomize you
			GenocideFlag,		// kill one kills whole team
			JumpingFlag,		// lets your tank jump
			IdentifyFlag,		// lets you identify other flags
			CloakingFlag,		// make you invisible out window

			// the bad super flags
			ColorblindnessFlag,	// can't see team colors
			ObesityFlag,		// makes you really fat
			LeftTurnOnlyFlag,	// can't turn right
			RightTurnOnlyFlag,	// can't turn left
			MomentumFlag,		// gives you lots of momentum
			BlindnessFlag,		// can't see out window
			JammingFlag,		// radar doesn't work
			WideAngleFlag,		// fish eye view (sounds good
						// but isn't)

			// the rest are not yet implemented
			MagnetFlag,		// draws shots toward you
			KamikaziFlag,		// self destruct big time
			HeatSeekerFlag,		// aims at strongest heat source
			DeathFlag,		// die the instant you grab it
			PossessionFlag,		// control another tank's motion
			ThiefFlag,		// can steal another's flag

Kamikazi,	// large destruct radius when dropped
Suicide,	// high speed, fast shots, die at first kill
ECM,		// disable radars of nearby tanks (not teammates)
InterdimensionalTeleport,	// teleport (to random teleporter) when dropped
SmartBomb,	// drop, gets to apex, large radius of destruction
		// (including player who dropped it)
TurboBoost,	// quick burst of acceleration


			// special flags for size of team and super-flag sets
			FirstFlag = RedFlag,
			LastFlag = WideAngleFlag,
			FirstTeamFlag =	RedFlag,
			LastTeamFlag =	PurpleFlag,
			FirstSuperFlag = VelocityFlag,
			LastSuperFlag =	LastFlag
};

// Super flag characteristic modifiers
const float		VelocityAd =	1.5f;		// 50% faster
const float		AngularAd =	1.5f;		// 50% faster turns
const float		RFireAdVel =	1.5f;		// 50% faster shots
const float		RFireAdRate =	2.0f;		// 2x faster reload
const float		RFireAdLife =	1.0f/RFireAdRate;// 1/2 normal lifetime
const float		MGunAdVel =	1.5f;		// 50% faster shots
const float		MGunAdRate =	10.0f;		// 10x faster reload
const float		MGunAdLife =	1.0f/MGunAdRate;// 1/10 normal lifetime
const float		LaserAdVel =	1000.0f;		// 1000x faster shots
const float		LaserAdRate =	0.5f;		// 1/2x faster reload
const float		LaserAdLife =	0.1f;		// 1/10 normal lifetime
const float		GMissileAng =	M_PI / 5.0f;	// max turn rate (rad/s)
const float		TinyFactor =	0.4f;		// 40% normal size
const float		ShieldFlight =	2.7f;		// flag goes 170% higher
const float		SRRadiusMult =	2.0;		// 200% normal radius
const float		ShockAdLife =	0.20f;		// 20% normal lifetime
const float		ShockInRadius =	TankLength;	// size of tank
const float		ShockOutRadius=	2.0f * BoxBase;	// size of building
const float		JumpVelocity =	19.0f;		// m/s
const float		IdentityRange =	50.0f;		// meters
const float		ObeseFactor =	2.5f;		// 250% normal size
const float		WideAngleAng =	M_PI / 1.8f;	// 100 degree fov
const float		MomentumLinAcc=	1.0f;		// 
const float		MomentumAngAcc=	1.0f;		//
const float		MagnetPower =	0.0f;		// off

#endif // BZF_GLOBAL_H
