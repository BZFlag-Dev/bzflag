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

int FlagDesc::flagCount = 0;
FlagSet FlagDesc::flagSets[NumQualities];


// Initialize flag description singletons in our Flags namespace
namespace Flags {

  FlagDesc *Null;
  FlagDesc *RedTeam;
  FlagDesc *GreenTeam;
  FlagDesc *BlueTeam;
  FlagDesc *PurpleTeam;
  FlagDesc *Velocity;
  FlagDesc *QuickTurn;
  FlagDesc *OscillationOverthruster;
  FlagDesc *RapidFire;
  FlagDesc *MachineGun;
  FlagDesc *GuidedMissile;
  FlagDesc *Laser;
  FlagDesc *Ricochet;
  FlagDesc *SuperBullet;
  FlagDesc *InvisibleBullet;
  FlagDesc *Stealth;
  FlagDesc *Tiny;
  FlagDesc *Narrow;
  FlagDesc *Shield;
  FlagDesc *Steamroller;
  FlagDesc *ShockWave;
  FlagDesc *PhantomZone;
  FlagDesc *Genocide;
  FlagDesc *Jumping;
  FlagDesc *Identify;
  FlagDesc *Cloaking;
  FlagDesc *Useless;
  FlagDesc *Masquerade;
  FlagDesc *Seer;
  FlagDesc *Thief;
  FlagDesc *Burrow;
  FlagDesc *Colorblindness;
  FlagDesc *Obesity;
  FlagDesc *LeftTurnOnly;
  FlagDesc *RightTurnOnly;
  FlagDesc *Momentum;
  FlagDesc *Blindness;
  FlagDesc *Jamming;
  FlagDesc *WideAngle;

  void init()
  {
    Null		    = new FlagDesc( "", "", FlagNormal, NormalShot, FlagGood, NoTeam, NULL );
    RedTeam                 = new FlagDesc( "Red Team", "R*", FlagNormal, NormalShot, FlagGood, ::RedTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    GreenTeam               = new FlagDesc( "Green Team", "G*", FlagNormal, NormalShot, FlagGood, ::GreenTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    BlueTeam                = new FlagDesc( "Blue Team", "B*", FlagNormal, NormalShot, FlagGood, ::BlueTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    PurpleTeam              = new FlagDesc( "Purple Team", "P*", FlagNormal, NormalShot, FlagGood, ::PurpleTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    Velocity                = new FlagDesc( "High Speed", "V", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "High Speed (+V):  Tank moves faster.  Outrun bad guys." );
    QuickTurn               = new FlagDesc( "Quick Turn", "A", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Angular velocity (+A):  Tank turns faster.  Dodge quicker." );
    OscillationOverthruster = new FlagDesc( "Oscillation Overthruster", "OO", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Oscillation Overthruster (+OO):  Can drive through buildings.  Can't backup or shoot while inside." );
    RapidFire               = new FlagDesc( "Rapid Fire", "F", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "rapid Fire (+F):  Shoots more often.  Shells go faster but not as far." );
    MachineGun              = new FlagDesc( "Machine Gun", "MG", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Machine Gun (+MG):  Very fast reload and very short range." );
    GuidedMissile           = new FlagDesc( "Guided Missile", "GM", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Guided Missile (+GM):  Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." );
    Laser                   = new FlagDesc( "Laser", "L", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time.");
    Ricochet                = new FlagDesc( "Ricochet", "R", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!" );
    SuperBullet             = new FlagDesc( "Super Bullet", "SB", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone." );
    InvisibleBullet         = new FlagDesc( "Invisible Bullet", "IB", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Invisible Bullet (+IB):  Your shots don't appear on other radars.  Can still see them out window.");
    Stealth                 = new FlagDesc( "Stealth", "ST", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "STealth (+ST):  Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!");
    Tiny                    = new FlagDesc( "Tiny", "T", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Tiny (+T):  Tank is small and can get through small openings.  Very hard to hit." );
    Narrow                  = new FlagDesc( "Narrow", "N", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Narrow (+N):  Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings.");
    Shield                  = new FlagDesc( "Shield", "SH", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SHield (+SH):  Getting hit only drops flag.  Flag flies an extra-long time.");
    Steamroller             = new FlagDesc( "Steamroller", "SR", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SteamRoller (+SR):  Destroys tanks you touch but you have to get really close.");
    ShockWave               = new FlagDesc( "Shock Wave", "SW", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Shock Wave (+SW):  Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings.");
    PhantomZone             = new FlagDesc( "Phantom Zone", "PZ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank can't shoot or be shot (except by superbullet and shock wave).");
    Genocide                = new FlagDesc( "Genocide", "G", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Genocide (+G):  Killing one tank kills that tank's whole team.");
    Jumping                 = new FlagDesc( "Jumping", "JP", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air.");
    Identify                = new FlagDesc( "Identify", "ID", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "IDentify (+ID):  Identifies type of nearest flag.");
    Cloaking                = new FlagDesc( "Cloaking", "CL", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "CLoaking (+CL):  Makes your tank invisible out-the-window.  Still visible on radar.");
    Useless                 = new FlagDesc( "Useless", "US", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "USeless (+US):  You have found the useless flag. Use it wisely.");
    Masquerade              = new FlagDesc( "Masquerade", "MQ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "MasQuerade (+MQ):  In opponent's hud, you appear as a teammate.");
    Seer                    = new FlagDesc( "Seer", "SE", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SEer (+SE):  See stealthed, cloaked and masquerading tanks as normal.");
    Thief                   = new FlagDesc( "Thief", "TH", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "THief (+TH):  Steal flags.  Small and fast but can't kill.");
    Burrow                  = new FlagDesc( "Burrow", "BU", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Burrow (+BU):  Tank burrows underground, impervious to normal shots, but can be steamrolled by anyone!");
    Colorblindness          = new FlagDesc( "Colorblindness", "CB", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!");
    Obesity                 = new FlagDesc( "Obesity", "O", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Obesity (-O):  Tank becomes very large.  Can't fit through teleporters.");
    LeftTurnOnly            = new FlagDesc( "Left Turn Only", "LT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "left turn only (-LT):  Can't turn right.");
    RightTurnOnly           = new FlagDesc( "Right Turn Only", "RT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "right turn only (-RT):  Can't turn left.");
    Momentum                = new FlagDesc( "Momentum", "M", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Momentum (-M):  Tank has inertia.  Acceleration is limited.");
    Blindness               = new FlagDesc( "Blindness", "B", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Blindness (-B):  Can't see out window.  Radar still works.");
    Jamming                 = new FlagDesc( "Jamming", "JM", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "JaMming (-JM):  Radar doesn't work.  Can still see.");
    WideAngle               = new FlagDesc( "Wide Angle", "WA", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Wide Angle (-WA):  Fish-eye lens distorts view.");
  }

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

std::map<std::string, FlagDesc*>& FlagDesc::getFlagMap() {
  static std::map<std::string, FlagDesc*> flagMap;
  return flagMap;
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
  std::string abbvString;

  /* Uppercase the abbreviation */
  while (*abbreviation) {
    abbvString += toupper(*abbreviation);
    abbreviation++;
  }

  i = FlagDesc::getFlagMap().find(abbvString);
  if (i == FlagDesc::getFlagMap().end())
    /* Not found, return the Null flag */
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

/* ex: shiftwidth=2 tabstop=8
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 */

