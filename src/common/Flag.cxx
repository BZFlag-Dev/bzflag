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

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include <math.h>
#include "common.h"
#include "Team.h"
#include "Flag.h"
#include "Pack.h"

std::map<std::string, FlagDesc*> FlagDesc::flagMap;
int FlagDesc::flagCount = 0;
FlagSet FlagDesc::flagSets[NumQualities];


// Initialize flag description singletons in our Flags namespace
namespace Flags {
  FlagDesc *
    Null                    = new FlagDesc( "", "", FlagNormal, NormalShot, FlagGood, NULL ),
    RedTeam                 = new FlagDesc( "Red Team", "R*", FlagNormal, NormalShot, FlagGood,
					    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
    GreenTeam               = new FlagDesc( "Green Team", "G*", FlagNormal, NormalShot, FlagGood,
					    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
    BlueTeam                = new FlagDesc( "Blue Team", "B*", FlagNormal, NormalShot, FlagGood,
					    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
    PurpleTeam              = new FlagDesc( "Purple Team", "P*", FlagNormal, NormalShot, FlagGood,
					    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
    HighSpeed               = new FlagDesc( "High Speed", "V", FlagUnstable, NormalShot, FlagGood,
					    "Velocity (+V):  Tank moves faster.  Outrun bad guys." ),
    QuickTurn               = new FlagDesc( "Quick Turn", "A", FlagUnstable, NormalShot, FlagGood,
					    "Angular velocity (+A):  Tank turns faster.  Dodge quicker." ),
    OscillationOverthruster = new FlagDesc( "Oscillation Overthruster", "OO", FlagUnstable, NormalShot, FlagGood,
					    "Oscillation Overthruster (+OO):  Can drive through buildings.  Can't backup or shoot while inside." ),
    RapidFire               = new FlagDesc( "Rapid Fire", "F", FlagUnstable, SpecialShot, FlagGood,
					    "rapid Fire (+F):  Shoots more often.  Shells go faster but not as far." ),
    MachineGun              = new FlagDesc( "Machine Gun", "MG", FlagUnstable, SpecialShot, FlagGood,
					    "Machine Gun (+MG):  Very fast reload and very short range." ),
    GuidedMissle            = new FlagDesc( "Guided Missile", "GM", FlagUnstable, SpecialShot, FlagGood,
					    "Guided Missile (+GM):  Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." ),
    Laser                   = new FlagDesc( "Laser", "L", FlagUnstable, SpecialShot, FlagGood,
					    "Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time."),
    Ricochet                = new FlagDesc( "Ricochet", "R", FlagUnstable, SpecialShot, FlagGood,
					    "Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!" ),
    SuperBullet             = new FlagDesc( "Super Bullet", "SB", FlagUnstable, SpecialShot, FlagGood,
					    "SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone." ),
    InvisibleBullet         = new FlagDesc( "Invisible Bullet", "IB", FlagUnstable, NormalShot, FlagGood,
					    "Invisible Bullet (+IB):  Your shots don't appear on other radars.  Can still see them out window."),
    Stealth                 = new FlagDesc( "Stealth", "ST", FlagUnstable, NormalShot, FlagGood,
					    "STealth (+ST):  Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!"),
    Tiny                    = new FlagDesc( "Tiny", "T", FlagUnstable, NormalShot, FlagGood,
					    "Tiny (+T):  Tank is small and can get through small openings.  Very hard to hit." ),
    Narrow                  = new FlagDesc( "Narrow", "N", FlagUnstable, NormalShot, FlagGood,
					    "Narrow (+N):  Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings."),
    Shield                  = new FlagDesc( "Shield", "SH", FlagUnstable, NormalShot, FlagGood,
					    "SHield (+SH):  Getting hit only drops flag.  Flag flies an extra-long time."),
    Steamroller             = new FlagDesc( "Steamroller", "SR", FlagUnstable, NormalShot, FlagGood,
					    "SteamRoller (+SR):  Destroys tanks you touch but you have to get really close."),
    ShockWave               = new FlagDesc( "Shock Wave", "SW", FlagUnstable, SpecialShot, FlagGood,
					    "Shock Wave (+SW):  Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings."),
    PhantomZone             = new FlagDesc( "Phantom Zone", "PZ", FlagUnstable, NormalShot, FlagGood,
					    "Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank can't shoot or be shot (except by superbullet and shock wave)."),
    Genocide                = new FlagDesc( "Genocide", "G", FlagUnstable, NormalShot, FlagGood,
					    "Genocide (+G):  Killing one tank kills that tank's whole team."),
    Jumping                 = new FlagDesc( "Jumping", "JP", FlagUnstable, NormalShot, FlagGood,
					    "JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air."),
    Identify                = new FlagDesc( "Identify", "ID", FlagUnstable, NormalShot, FlagGood,
					    "IDentify (+ID):  Identifies type of nearest flag."),
    Cloaking                = new FlagDesc( "Cloaking", "CL", FlagUnstable, NormalShot, FlagGood,
					    "CLoaking (+CL):  Makes your tank invisible out-the-window.  Still visible on radar."),
    Useless                 = new FlagDesc( "Useless", "US", FlagUnstable, NormalShot, FlagGood,
					    "USeless (+US):  You have found the useless flag. Use it wisely."),
    Masquerade              = new FlagDesc( "Masquerade", "MQ", FlagUnstable, NormalShot, FlagGood,
					    "MasQuerade (+MQ):  In opponent's hud, you appear as a teammate."),
    Seer                    = new FlagDesc( "Seer", "SE", FlagUnstable, NormalShot, FlagGood,
					    "SEer (+SE):  See stealthed, cloaked and masquerading tanks as normal."),
    Colorblindness          = new FlagDesc( "Colorblindness", "CB", FlagSticky, NormalShot, FlagBad,
					    "ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!"),
    Obesity                 = new FlagDesc( "Obesity", "O", FlagSticky, NormalShot, FlagBad,
					    "Obesity (-O):  Tank becomes very large.  Can't fit through teleporters."),
    LeftTurnOnly            = new FlagDesc( "Left Turn Only", "<-", FlagSticky, NormalShot, FlagBad,
					    "left turn only (- <-):  Can't turn right."),
    RightTurnOnly           = new FlagDesc( "Right Turn Only", "->", FlagSticky, NormalShot, FlagBad,
					    "right turn only (- ->):  Can't turn left."),
    Momentum                = new FlagDesc( "Momentum", "M", FlagSticky, NormalShot, FlagBad,
					    "Momentum (-M):  Tank has inertia.  Acceleration is limited."),
    Blindness               = new FlagDesc( "Blindness", "B", FlagSticky, NormalShot, FlagBad,
					    "Blindness (-B):  Can't see out window.  Radar still works."),
    Jamming                 = new FlagDesc( "Jamming", "JM", FlagSticky, NormalShot, FlagBad,
					    "JaMming (-JM):  Radar doesn't work.  Can still see."),
    WideAngle               = new FlagDesc( "Wide Angle", "WA", FlagSticky, NormalShot, FlagBad,
					    "Wide Angle (-WA):  Fish-eye lens distorts view.");
}

void*			Flag::pack(void* buf) const
{
  buf = nboPackUShort(buf, uint16_t(id));
  buf = nboPackUShort(buf, uint16_t(status));
  buf = nboPackUShort(buf, uint16_t(type));
  buf = nboPackUByte(buf, owner);
  buf = nboPackVector(buf, position);
  buf = nboPackVector(buf, launchPosition);
  buf = nboPackVector(buf, landingPosition);
  buf = nboPackFloat(buf, flightTime);
  buf = nboPackFloat(buf, flightEnd);
  buf = nboPackFloat(buf, initialVelocity);
  return buf;
}

void*			Flag::unpack(void* buf)
{
  uint16_t data;
  buf = nboUnpackUShort(buf, data); id = FlagId(data);
  buf = nboUnpackUShort(buf, data); status = FlagStatus(data);
  buf = nboUnpackUShort(buf, data); type = FlagType(data);
  buf = nboUnpackUByte(buf, owner);
  buf = nboUnpackVector(buf, position);
  buf = nboUnpackVector(buf, launchPosition);
  buf = nboUnpackVector(buf, landingPosition);
  buf = nboUnpackFloat(buf, flightTime);
  buf = nboUnpackFloat(buf, flightEnd);
  buf = nboUnpackFloat(buf, initialVelocity);
  return buf;
}

const char*		Flag::getName(FlagId id)
{
  return descriptions[id].flagName;
}

const char*		Flag::getAbbreviation(FlagId id)
{
  return descriptions[id].flagAbbv;
}

FlagId	Flag::getIDFromAbbreviation(const char* abbreviation)
{
  for (int q = 0; q < NumQualities; q++) {
    for (std::set<FlagId>::iterator it = Flag::Desc::flagSets[q].begin();
         it != Flag::Desc::flagSets[q].end(); ++it) {
	   const char* abbrev = Flag::getAbbreviation(*it);
           if (strcasecmp(abbreviation, abbrev) == 0)
	      return *it;
	 }
  }
  return NullFlag;
}

FlagType		Flag::getType(FlagId id)
{
  return descriptions[id].flagType;
}

const char*		Flag::getHelp(FlagId id )
{
  return descriptions[id].flagHelp;
}

ShotType		Flag::getShotType( FlagId id )
{
  return descriptions[id].flagShot;
}

FlagSet&		Flag::getGoodFlags()
{
  return Flag::Desc::flagSets[FlagGood];
}

FlagSet&		Flag::getBadFlags()
{
  return Flag::Desc::flagSets[FlagBad];
}

const float*		Flag::getColor(FlagId id)
{
  static const float superColor[3] = { 1.0, 1.0, 1.0 };
  switch (id) {
    case NoFlag:
    case RedFlag:
    case GreenFlag:
    case BlueFlag:
    case PurpleFlag:
      return Team::getTankColor(TeamColor(id));
    default:
      return superColor;
  }
}
// ex: shiftwidth=2 tabstop=8
