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
