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
 * flag enums and class
 */

#ifndef	BZF_FLAG_H
#define	BZF_FLAG_H

#include <set>
#include "common.h"
#include "global.h"
#include "Address.h"

enum FlagStatus {
			FlagNoExist = 0,
			FlagOnGround,
			FlagOnTank,
			FlagInAir,
			FlagComing,
			FlagGoing
};
enum FlagType {
			FlagNormal = 0,		// permanent flag
			FlagUnstable = 1,	// disappears after use
			FlagSticky = 2		// can't be dropped normally
};

enum FlagQuality {
			FlagGood = 0,
			FlagBad = 1,
			NumQualities
};

enum ShotType {
			NormalShot = 0,
			SpecialShot = 1
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
			UselessFlag,		// yay, its useless
			MasqueradeFlag,		// appear in the hud as a teammate
			SeerFlag,		// see cloak, steath, masquerade as normal

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

			// special flags for size of team and super-flag sets
			FirstFlag = RedFlag,
			LastFlag = WideAngleFlag,
			FirstTeamFlag =	RedFlag,
			LastTeamFlag =	PurpleFlag,
			FirstSuperFlag = VelocityFlag,
			LastSuperFlag =	LastFlag
};

const int		FlagPLen = 6 + PlayerIdPLen + 48;

typedef std::set<FlagId> FlagSet;

class Flag {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

    static FlagSet&	getGoodFlags();
    static FlagSet&	getBadFlags();
    static const char*	getName(FlagId);
    static const char*	getAbbreviation(FlagId);
    static FlagId	getIDFromAbbreviation(const char* abbreviation);
    static FlagType	getType(FlagId);
    static const char*	getHelp(FlagId);
    static ShotType	getShotType(FlagId);
    static const float*	getColor(FlagId);

  public:
    FlagId		id;
    FlagStatus		status;
    FlagType		type;
    PlayerId		owner;			// who has flag
    float		position[3];		// position on ground
    float		launchPosition[3];	// position flag launched from
    float		landingPosition[3];	// position flag will land
    float		flightTime;		// flight time so far
    float		flightEnd;		// total duration of flight
    float		initialVelocity;	// initial launch velocity

  private:
    class Desc
    {
    public:
	Desc( const char* name, const char *abbv, FlagType fType, ShotType sType, FlagQuality quality, const char *help ) {
		flagName = name;
		flagAbbv = abbv;
		flagType = fType;
		flagShot = sType;
		flagQuality = quality;
		flagHelp = help;

		flagSets[flagQuality].insert((FlagId) flagCount++);
	}
	const char*	flagName;
	const char*	flagAbbv;
	FlagType	flagType;
	const char*	flagHelp;
	FlagQuality	flagQuality;
	ShotType	flagShot;

	static int	flagCount;
	static FlagSet	flagSets[NumQualities];
    };

    static Desc descriptions[];
};

#endif // BZF_FLAG_H
// ex: shiftwidth=2 tabstop=8
