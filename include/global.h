/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_GLOBAL_H
#define	BZF_GLOBAL_H

/*
 * Global constants
 */

#include "common.h"

/* system headers */
#include <math.h>

/* common headers */
#include "StateDatabase.h"

#include "bzfsAPI.h"

// values affecting struct and class layout
const int CallSignLen = 32;	// including terminating NUL
const int PasswordLen = 32;	// including terminating NUL
const int TokenLen = 22;		// opaque string (now int(10)) and terminating NUL
const int VersionLen = 60;	// including terminating NUL
const int MessageLen = 128;	// including terminating NUL
const int ServerNameLen = 80;
const int ReferrerLen = 256;	// including terminating NUL

// types of things we can be
enum PlayerType {
  TankPlayer,
  ComputerPlayer,
  ChatPlayer
};

// team info
const int NumTeams = 8;
const int CtfTeams = 5;

enum TeamColor {
  AutomaticTeam = -2,
  NoTeam        = -1,
  RogueTeam     = 0,
  RedTeam       = 1,
  GreenTeam     = 2,
  BlueTeam      = 3,
  PurpleTeam    = 4,
  ObserverTeam  = 5,
  RabbitTeam    = 6,
  HunterTeam    = 7
};

// Player allow attributes for MsgAllow
enum PlayerAllow {
  AllowNone = 0x00,
  AllowShoot = 0x01,
  AllowJump = 0x02,
  AllowTurnLeft = 0x04,
  AllowTurnRight = 0x08,
  AllowMoveForward = 0x10,
  AllowMoveBackward = 0x20,
  AllowAll = 0xFF
};

#ifdef ROBOT
// robots
#define MAX_ROBOTS 100
#endif

// epsilon and very far for ray intersections
const float Epsilon   =	ZERO_TOLERANCE;	// arbitrary
const float Infinity  =	MAXFLOAT;	// arbitrary

#define DEFAULT_WORLD	800

// readout stuff
const int MaxMessages =	20;		// msg. history length
const int MinX = 256;
const int MinY = 192;
const int NoMotionSize =	10;		// no motion zone size
const int MaxMotionSize = 37;		// motion zone size

enum GameType
{
  TeamFFA,	  // normal teamed FFA
  ClassicCTF,	  // your normal CTF
  OpenFFA,	  // teamless FFA
  RabbitChase	  // hunt the rabbit mode
};
// game styles
enum GameOptions {
  Unused              =  0x0001,
  SuperFlagGameStyle  =	 0x0002, // superflags allowed
  NoTeamKills	      =  0x0004, // teams can't kill each other
  JumpingGameStyle    =	 0x0008, // jumping allowed
  InertiaGameStyle    =	 0x0010, // momentum for all
  RicochetGameStyle   =	 0x0020, // all shots ricochet
  ShakableGameStyle   =	 0x0040, // can drop bad flags
  AntidoteGameStyle   =	 0x0080, // anti-bad flags
  HandicapGameStyle   =	 0x0100, // handicap players based on score (eek! was TimeSyncGameStyle)
  FreezeTagGameStyle  =  0x0200 // collisions freeze player farther from base
  // add here before reusing old ones above
};

// map object flags
#define _DRIVE_THRU  (1 << 0)
#define _SHOOT_THRU  (1 << 1)
#define _FLIP_Z      (1 << 2)
#define _RICOCHET    (1 << 3)

const int mapVersion = 1;

struct GlobalDBItem {
public:
	const char*		  name;
	const char*		  value;
	bool			  persistent;
	StateDatabase::Permission permission;
};
extern const unsigned int numGlobalDBItems;
extern const struct GlobalDBItem globalDBItems[];

bz_eTeamType convertTeam(TeamColor team);
TeamColor convertTeam(bz_eTeamType team);

#endif // BZF_GLOBAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
