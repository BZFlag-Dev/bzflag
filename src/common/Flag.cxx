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

int Flag::Desc::flagCount = 0;
FlagSet Flag::Desc::flagSets[NumQualities];

Flag::Desc	Flag::descriptions[] =
{
	Desc( "", "", FlagNormal, NormalShot, FlagGood, NULL ), //NullFlag
	Desc( "Red Team", "R*", FlagNormal, NormalShot, FlagGood,
		"Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
	Desc( "Green Team", "G*", FlagNormal, NormalShot, FlagGood,
		"Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
	Desc( "Blue Team", "B*", FlagNormal, NormalShot, FlagGood,
		"Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
	Desc( "Purple Team", "P*", FlagNormal, NormalShot, FlagGood,
		"Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ),
	Desc( "High Speed", "V", FlagUnstable, NormalShot, FlagGood,
		"Velocity (+V):  Tank moves faster.  Outrun bad guys." ),
	Desc( "Quick Turn", "A", FlagUnstable, NormalShot, FlagGood,
		"Angular velocity (+A):  Tank turns faster.  Dodge quicker." ),
	Desc( "Oscillation Overthruster", "OO", FlagUnstable, NormalShot, FlagGood,
		"Oscillation Overthruster (+OO):  Can drive through buildings.  Can't backup or shoot while inside." ),
	Desc( "Rapid Fire", "F", FlagUnstable, SpecialShot, FlagGood,
		"rapid Fire (+F):  Shoots more often.  Shells go faster but not as far." ),
	Desc( "Machine Gun", "MG", FlagUnstable, SpecialShot, FlagGood,
		"Machine Gun (+MG):  Very fast reload and very short range." ),
	Desc( "Guided Missile", "GM", FlagUnstable, SpecialShot, FlagGood,
		"Guided Missile (+GM):  Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." ),
	Desc( "Laser", "L", FlagUnstable, SpecialShot, FlagGood,
		"Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time."),
	Desc( "Ricochet", "R", FlagUnstable, SpecialShot, FlagGood,
		"Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!" ),
	Desc( "Super Bullet", "SB", FlagUnstable, SpecialShot, FlagGood,
		"SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone." ),
	Desc( "Invisible Bullet", "IB", FlagUnstable, NormalShot, FlagGood,
		"Invisible Bullet (+IB):  Your shots don't appear on other radars.  Can still see them out window."),
	Desc( "Stealth", "ST", FlagUnstable, NormalShot, FlagGood,
		"STealth (+ST):  Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!"),
	Desc( "Tiny", "T", FlagUnstable, NormalShot, FlagGood,
		"Tiny (+T):  Tank is small and can get through small openings.  Very hard to hit." ),
	Desc( "Narrow", "N", FlagUnstable, NormalShot, FlagGood,
		"Narrow (+N):  Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings."),
	Desc( "Shield", "SH", FlagUnstable, NormalShot, FlagGood,
		"SHield (+SH):  Getting hit only drops flag.  Flag flies an extra-long time."),
	Desc( "Steamroller", "SR", FlagUnstable, NormalShot, FlagGood,
		"SteamRoller (+SR):  Destroys tanks you touch but you have to get really close."),
	Desc( "Shock Wave", "SW", FlagUnstable, SpecialShot, FlagGood,
		"Shock Wave (+SW):  Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings."),
	Desc( "Phantom Zone", "PZ", FlagUnstable, NormalShot, FlagGood,
		"Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank can't shoot or be shot (except by superbullet and shock wave)."),
	Desc( "Genocide", "G", FlagUnstable, NormalShot, FlagGood,
		"Genocide (+G):  Killing one tank kills that tank's whole team."),
	Desc( "Jumping", "J", FlagUnstable, NormalShot, FlagGood,
		"JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air."),
	Desc( "Identify", "ID", FlagUnstable, NormalShot, FlagGood,
		"IDentify (+ID):  Identifies type of nearest flag."),
	Desc( "Cloaking", "CL", FlagUnstable, NormalShot, FlagGood,
		"CLoaking (+CL):  Makes your tank invisible out-the-window.  Still visible on radar."),
	Desc( "Useless", "US", FlagUnstable, NormalShot, FlagGood,
		"USeless (+US):  You have found the useless flag. Use it wisely."),
	Desc( "Masquerade", "MQ", FlagUnstable, NormalShot, FlagGood,
		"MasQuerade (+MQ):  In opponent's hud, you appear as a teammate."),
	Desc( "Colorblindness", "CB", FlagSticky, NormalShot, FlagBad,
		"ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!"),
	Desc( "Obesity", "O", FlagSticky, NormalShot, FlagBad,
		"Obesity (-O):  Tank becomes very large.  Can't fit through teleporters."),
	Desc( "Left Turn Only", "<-", FlagSticky, NormalShot, FlagBad,
		"left turn only (- <-):  Can't turn right."),
	Desc( "Right Turn Only", "->", FlagSticky, NormalShot, FlagBad,
		"right turn only (- ->):  Can't turn left."),
	Desc( "Momentum", "M", FlagSticky, NormalShot, FlagBad,
		"Momentum (-M):  Tank has inertia.  Acceleration is limited."),
	Desc( "Blindness", "B", FlagSticky, NormalShot, FlagBad,
		"Blindness (-B):  Can't see out window.  Radar still works."),
	Desc( "Jamming", "JM", FlagSticky, NormalShot, FlagBad,
		"JaMming (-JM):  Radar doesn't work.  Can still see."),
	Desc( "Wide Angle", "WA", FlagSticky, NormalShot, FlagBad,
		"Wide Angle (-WA):  Fish-eye lens distorts view."),
};

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
