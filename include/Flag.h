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

// Flags add some spice to the game.  There are two kinds of flags:
// team flags and super flags.  Super flags come in two types: good
// and bad.
//
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
//
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

#ifndef	BZF_FLAG_H
#define	BZF_FLAG_H

#include <set>
#include <map>
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
enum FlagEndurance {
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

const int		FlagPLen = 6 + PlayerIdPLen + 48;

class FlagType;
typedef std::map<std::string, FlagType*> FlagTypeMap;

class FlagType {
  public:
    FlagType( const char* name, const char* abbv, FlagEndurance _endurance,
	      ShotType sType, FlagQuality quality, TeamColor team, const char* help ) {
      flagName = name;
      flagAbbv = abbv;
      endurance = _endurance;
      flagShot = sType;
      flagQuality = quality;
      flagHelp = help;
      flagTeam = team;

      flagSets[flagQuality].insert(this);
      getFlagMap()[flagAbbv] = this;
      flagCount++;
    }

    const float*	getColor();
    void* pack(void* buf) const;
    static void* unpack(void* buf, FlagType* &desc);
    /** Static wrapper function that makes sure that the flag map is
	initialized before it's used. */
    static FlagTypeMap& getFlagMap();

    const char*		flagName;
    const char*		flagAbbv;
    FlagEndurance	endurance;
    const char*		flagHelp;
    FlagQuality		flagQuality;
    ShotType		flagShot;
    TeamColor		flagTeam;

    typedef std::set<FlagType*> FlagSet;
    static int		flagCount;
    static FlagSet	flagSets[NumQualities];
};

typedef FlagType::FlagSet  FlagSet;

class Flag {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

    static FlagSet&	getGoodFlags();
    static FlagSet&	getBadFlags();
    static FlagType*	getDescFromAbbreviation(const char* abbreviation);

  public:
    FlagType*		type;
    FlagStatus		status;
    FlagEndurance	endurance;
    PlayerId		owner;			// who has flag
    float		position[3];		// position on ground
    float		launchPosition[3];	// position flag launched from
    float		landingPosition[3];	// position flag will land
    float		flightTime;		// flight time so far
    float		flightEnd;		// total duration of flight
    float		initialVelocity;	// initial launch velocity
};

// Flags no longer use enumerated IDs. Over the wire, flags are all represented
// by their abbreviation, null-padded to two bytes. Internally, flags are now
// represented by pointers to singleton Flag::Desc classes.
//
// For more information about these flags, see Flag.cxx where these Flag::Desc
// instances are created.
//
namespace Flags {
  extern FlagType
    *Null,
    *RedTeam, *GreenTeam, *BlueTeam, *PurpleTeam, *Velocity, *QuickTurn,
    *OscillationOverthruster, *RapidFire, *MachineGun, *GuidedMissile, *Laser,
    *Ricochet, *SuperBullet, *InvisibleBullet, *Stealth, *Tiny, *Narrow,
    *Shield, *Steamroller, *ShockWave, *PhantomZone, *Genocide, *Jumping,
    *Identify, *Cloaking, *Useless, *Masquerade, *Seer, *Thief, *Burrow,
    *Colorblindness, *Obesity, *LeftTurnOnly, *RightTurnOnly, *Momentum, *Blindness,
    *Jamming, *WideAngle, *NoJumping, *TriggerHappy;

  void init();
}

#endif // BZF_FLAG_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

