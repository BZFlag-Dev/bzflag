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
  FlagDesc *Null		    = new FlagDesc( "", "", FlagNormal, NormalShot, FlagGood, NoTeam, NULL );
  FlagDesc *RedTeam                 = new FlagDesc( "Red Team", "R*", FlagNormal, NormalShot, FlagGood, ::RedTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
  FlagDesc *GreenTeam               = new FlagDesc( "Green Team", "G*", FlagNormal, NormalShot, FlagGood, ::GreenTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
  FlagDesc *BlueTeam                = new FlagDesc( "Blue Team", "B*", FlagNormal, NormalShot, FlagGood, ::BlueTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
  FlagDesc *PurpleTeam              = new FlagDesc( "Purple Team", "P*", FlagNormal, NormalShot, FlagGood, ::PurpleTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
  FlagDesc *Velocity                = new FlagDesc( "Velocity", "V", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Velocity (+V):  Tank moves faster.  Outrun bad guys." );
  FlagDesc *QuickTurn               = new FlagDesc( "Quick Turn", "A", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Angular velocity (+A):  Tank turns faster.  Dodge quicker." );
  FlagDesc *OscillationOverthruster = new FlagDesc( "Oscillation Overthruster", "OO", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Oscillation Overthruster (+OO):  Can drive through buildings.  Can't backup or shoot while inside." );
  FlagDesc *RapidFire               = new FlagDesc( "Rapid Fire", "F", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "rapid Fire (+F):  Shoots more often.  Shells go faster but not as far." );
  FlagDesc *MachineGun              = new FlagDesc( "Machine Gun", "MG", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Machine Gun (+MG):  Very fast reload and very short range." );
  FlagDesc *GuidedMissile           = new FlagDesc( "Guided Missile", "GM", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Guided Missile (+GM):  Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." );
  FlagDesc *Laser                   = new FlagDesc( "Laser", "L", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time.");
  FlagDesc *Ricochet                = new FlagDesc( "Ricochet", "R", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!" );
  FlagDesc *SuperBullet             = new FlagDesc( "Super Bullet", "SB", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone." );
  FlagDesc *InvisibleBullet         = new FlagDesc( "Invisible Bullet", "IB", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Invisible Bullet (+IB):  Your shots don't appear on other radars.  Can still see them out window.");
  FlagDesc *Stealth                 = new FlagDesc( "Stealth", "ST", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "STealth (+ST):  Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!");
  FlagDesc *Tiny                    = new FlagDesc( "Tiny", "T", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Tiny (+T):  Tank is small and can get through small openings.  Very hard to hit." );
  FlagDesc *Narrow                  = new FlagDesc( "Narrow", "N", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Narrow (+N):  Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings.");
  FlagDesc *Shield                  = new FlagDesc( "Shield", "SH", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SHield (+SH):  Getting hit only drops flag.  Flag flies an extra-long time.");
  FlagDesc *Steamroller             = new FlagDesc( "Steamroller", "SR", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SteamRoller (+SR):  Destroys tanks you touch but you have to get really close.");
  FlagDesc *ShockWave               = new FlagDesc( "Shock Wave", "SW", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Shock Wave (+SW):  Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings.");
  FlagDesc *PhantomZone             = new FlagDesc( "Phantom Zone", "PZ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank can't shoot or be shot (except by superbullet and shock wave).");
  FlagDesc *Genocide                = new FlagDesc( "Genocide", "G", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Genocide (+G):  Killing one tank kills that tank's whole team.");
  FlagDesc *Jumping                 = new FlagDesc( "Jumping", "JP", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air.");
  FlagDesc *Identify                = new FlagDesc( "Identify", "ID", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "IDentify (+ID):  Identifies type of nearest flag.");
  FlagDesc *Cloaking                = new FlagDesc( "Cloaking", "CL", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "CLoaking (+CL):  Makes your tank invisible out-the-window.  Still visible on radar.");
  FlagDesc *Useless                 = new FlagDesc( "Useless", "US", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "USeless (+US):  You have found the useless flag. Use it wisely.");
  FlagDesc *Masquerade              = new FlagDesc( "Masquerade", "MQ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "MasQuerade (+MQ):  In opponent's hud, you appear as a teammate.");
  FlagDesc *Seer                    = new FlagDesc( "Seer", "SE", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SEer (+SE):  See stealthed, cloaked and masquerading tanks as normal.");
  FlagDesc *Colorblindness          = new FlagDesc( "Colorblindness", "CB", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!");
  FlagDesc *Obesity                 = new FlagDesc( "Obesity", "O", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Obesity (-O):  Tank becomes very large.  Can't fit through teleporters.");
  FlagDesc *LeftTurnOnly            = new FlagDesc( "Left Turn Only", "LT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "left turn only (-LT):  Can't turn right.");
  FlagDesc *RightTurnOnly           = new FlagDesc( "Right Turn Only", "RT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "right turn only (-RT):  Can't turn left.");
  FlagDesc *Momentum                = new FlagDesc( "Momentum", "M", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Momentum (-M):  Tank has inertia.  Acceleration is limited.");
  FlagDesc *Blindness               = new FlagDesc( "Blindness", "B", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Blindness (-B):  Can't see out window.  Radar still works.");
  FlagDesc *Jamming                 = new FlagDesc( "Jamming", "JM", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "JaMming (-JM):  Radar doesn't work.  Can still see.");
  FlagDesc *WideAngle               = new FlagDesc( "Wide Angle", "WA", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Wide Angle (-WA):  Fish-eye lens distorts view.");
}

void* FlagDesc::pack(void* buf) const
{
  buf = nboPackUByte(buf, flagAbbv[0]);
  buf = nboPackUByte(buf, flagAbbv[1]);
  return buf;
}

void* FlagDesc::unpack(void* buf, FlagDesc* &desc)
{
  unsigned char abbv[3] = {0,0,0};
  buf = nboUnpackUByte(buf, abbv[0]);
  buf = nboUnpackUByte(buf, abbv[1]);
  desc = Flag::getDescFromAbbreviation((const char *)abbv);
  return buf;
}

void*			Flag::pack(void* buf) const
{
  buf = desc->pack(buf);
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

  buf = FlagDesc::unpack(buf, desc);
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

FlagDesc* Flag::getDescFromAbbreviation(const char* abbreviation)
{
  std::map<std::string, FlagDesc*>::iterator i;
  i = FlagDesc::flagMap.find(abbreviation);
  if (i == FlagDesc::flagMap.end())
    return Flags::Null;
  else
    return i->second;
}

FlagSet&		Flag::getGoodFlags()
{
  return FlagDesc::flagSets[FlagGood];
}

FlagSet&		Flag::getBadFlags()
{
  return FlagDesc::flagSets[FlagBad];
}

const float*		FlagDesc::getColor()
{
  static const float superColor[3] = { 1.0, 1.0, 1.0 };

  if (flagTeam == NoTeam)
    return superColor;
  else
    return Team::getTankColor(flagTeam);
}
// ex: shiftwidth=2 tabstop=8
