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

/** @file
 * Flags add some spice to the game.  There are two kinds of flags:
 * team flags and super flags.  Super flags come in two types: good
 * and bad.
 *
 *   When playing a "capture the flag" style game, each team with at
 * least one player has a team flag which has the same color as the
 * team.  A team flag will remain in the game as long as there is a
 * player on that team.  A team flag may be picked up and freely
 * dropped at any time.  It may be captured, which causes it to go
 * back to it's home position (centered in the team base).  If a
 * flag is dropped by a hostile player in a third team's base, the
 * flag will go to the third team's flag safety position.  For example,
 * if a Green Team player dropped the Red Flag on Blue's Base, the
 * Red Flag would go to the Blue Team's safety position.  This is
 * because if it stayed in the Blue Base, any Red Team member who
 * picked it up would instantly have brought his team flag into
 * enemy territory and so blow up his whole team.
 *
 *   A super flag causes the characteristics of the tank that possesses
 * it to change.  A good super flag generally makes the tank more
 * powerful or deadly.  A bad super flag generally does the opposite.
 * A good super flag may always be dropped.  A bad super flag is
 * "sticky" which means that it can't be freely dropped.  The server
 * may have some means of getting rid of a bad super flag (perhaps
 * by destroying an enemy or two or after waiting 20 seconds).
 * The creation and destruction of super flags is under the server's
 * control so super flags may appear and disappear seemingly at
 * random.
 */

#ifndef	BZF_FLAG_H
#define	BZF_FLAG_H

#include "common.h"

/* system interface headers */
#include <set>
#include <map>
#include <string>

/* common interface headers */
#include "global.h"
#include "Address.h"
#include "BufferedNetworkMessage.h"


/** This enum says where a flag is. */
enum FlagStatus {
  /// the flag is not present in the world
  FlagNoExist = 0,
  /// the flag is sitting on the ground and can be picked up
  FlagOnGround,
  /// the flag is being carried by a tank
  FlagOnTank,
  /// the flag is falling through the air
  FlagInAir,
  /// the flag is entering the world
  FlagComing,
  /// the flag is leaving the world
  FlagGoing
};

/** This enum tells us if the flag type is droppable, and what happens to it
    when it's droppped. */
enum FlagEndurance {
  /// permanent flag
  FlagNormal = 0,
  /// disappears after use
  FlagUnstable = 1,
  /// can't be dropped normally
  FlagSticky = 2
};

/** This enum tells the "quality" of the flag type, i.e. whether it's good
    or bad */
enum FlagQuality {
  FlagGood = 0,
  FlagBad = 1,
  NumQualities
};

/** This enum says if the flag type gives the carrier a special shooting
    ability. */
enum ShotType {
	NoShot = 0,
	StandardShot,
	GMShot,
	LaserShot,
	ThiefShot,
	SuperShot,
	PhantomShot,
	ShockWaveShot,
	RicoShot,
	MachineGunShot,
	InvisibleShot,
	RapidFireShot
};


const int		FlagPLen = 6 + PlayerIdPLen + 48;

class FlagType;
typedef std::map<std::string, FlagType*> FlagTypeMap;
typedef std::set<FlagType*> FlagSet;

#define FlagPackSize 2

/** This class represents a flagtype, like "GM" or "CL". */
class FlagType {
public:
  FlagType( const char* name, const char* abbv, FlagEndurance _endurance,
	    ShotType sType, FlagQuality quality, TeamColor team, const char* help,
	    bool _custom = false ) {
    flagName = name;
    flagAbbv = abbv;
    endurance = _endurance;
    flagShot = sType;
    flagQuality = quality;
    flagHelp = help;
    flagTeam = team;
    custom = _custom;

    /* allocate flagset array on first use to work around mipspro
     * std::set compiler bug of making flagSets a fixed array.
     */
    if (flagSets == NULL) {
      flagSets = new FlagSet[NumQualities];
    }

    if (custom)
      customFlags.insert(this);

    flagSets[flagQuality].insert(this);
    getFlagMap()[flagAbbv] = this;
  }

  /** returns a label of flag name and abbreviation with the flag name
   * excentuating the abbreviation if relevant.
   */
  const std::string label() const;

  /** returns information about a flag including the name, abbreviation, and
   * description.  format is "name ([+|-]abbrev): description" where +|-
   * indicates whether the flag is inherently good or bad by default.
   */
  const std::string information() const;

  /** returns the color of the flag */
  const float* getColor() const;

  /** network serialization */
  void* pack(void* buf) const;
  void* fakePack(void* buf) const;
  void* packCustom(void* buf) const;
  size_t pack(BufferedNetworkMessage *msg) const;
  size_t fakePack(BufferedNetworkMessage *msg) const;
  size_t packCustom(BufferedNetworkMessage *msg) const;

  /** network deserialization */
  static void* unpack(void* buf, FlagType* &desc);
  static void* unpackCustom(void* buf, FlagType* &desc);

  /** Static wrapper function that makes sure that the flag map is
   * initialized before it's used.
   */
  static FlagTypeMap& getFlagMap();

  const char* flagName;
  const char* flagAbbv;
  FlagEndurance	endurance;
  const char* flagHelp;
  FlagQuality flagQuality;
  ShotType flagShot;
  TeamColor flagTeam;
  bool custom;

  static FlagSet *flagSets;
  static FlagSet customFlags;
  static const int packSize;
};


/** This class represents an actual flag. It has functions for serialization
    and deserialization as well as static functions that returns sets of
    all good or bad flags, and maps flag abbreviations to FlagType objects. */
class Flag {
public:
  /** This function serializes this object into a @c void* buffer for network
      transfer. */
  void* pack(void*) const;
  size_t pack(BufferedNetworkMessage *msg) const;
 /** This function serializes this object into a @c void* buffer for network
      transfer. */
  void* fakePack(void*) const;
  size_t fakePack(BufferedNetworkMessage *msg) const;
 /** This function uses the given serialization to set the member variables
      of this object. This really hide the type of flag */
  void* unpack(void*);

  /** This function returns a set of all good flagtypes that are available in
      the game.
      @see FlagType
      @see FlagQuality
  */
  static FlagSet& getGoodFlags();

  /** This function returns a set of all bad flagtypes that are available in
      the game.
      @see FlagType
      @see FlagQuality
  */
  static FlagSet& getBadFlags();

  /** This function returns a pointer to the FlagType object that is associated
      with the given abbreviation. If there is no such FlagType object, NULL
      is returned. */
  static FlagType* getDescFromAbbreviation(const char* abbreviation);

  FlagType* type;
  FlagStatus status;
  FlagEndurance	endurance;
  PlayerId owner;		// who has flag
  float position[3];		// position on ground
  float launchPosition[3];	// position flag launched from
  float landingPosition[3];	// position flag will land
  float flightTime;		// flight time so far
  float flightEnd;		// total duration of flight
  float initialVelocity;	// initial launch velocity
};

/** Flags no longer use enumerated IDs. Over the wire, flags are all
    represented by their abbreviation, null-padded to two bytes. Internally,
    flags are now represented by pointers to singleton FlagType classes.

    For more information about these flags, see Flag.cxx where these FlagType
    instances are created.
*/
namespace Flags {
  extern FlagType
    /* alphabetical order */
    *Agility,
    *Blindness,
    *BlueTeam,
    *Bouncy,
    *Burrow,
    *Cloaking,
    *Colorblindness,
    *ForwardOnly,
    *Genocide,
    *GreenTeam,
    *GuidedMissile,
    *Identify,
    *InvisibleBullet,
    *Jamming,
    *Jumping,
    *Laser,
    *LeftTurnOnly,
    *LowGravity,
    *MachineGun,
    *Masquerade,
    *Momentum,
    *Narrow,
    *NoJumping,
    *Obesity,
    *OscillationOverthruster,
    *PhantomZone,
    *PurpleTeam,
    *QuickTurn,
    *RapidFire,
    *RedTeam,
    *ReverseControls,
    *ReverseOnly,
    *Ricochet,
    *RightTurnOnly,
    *Seer,
    *Shield,
    *ShockWave,
    *Stealth,
    *Steamroller,
    *SuperBullet,
    *Thief,
    *Tiny,
    *TriggerHappy,
    *Useless,
    *Velocity,
    *WideAngle,
    *Wings,
    *Null; // leave Null at end

  /** This function initializes all the FlagType objects in the Flags
      namespace. */
  void init();
  void kill();

  /** Clear all the custom flags (i.e. when switching servers) */
  void clearCustomFlags();
}

#endif // BZF_FLAG_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
