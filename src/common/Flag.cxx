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

#ifdef _WIN32
#define strcasecmp _stricmp
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <math.h>
#include "common.h"
#include "Team.h"
#include "Flag.h"
#include "Pack.h"

int FlagType::flagCount = 0;
FlagSet FlagType::flagSets[NumQualities];


// Initialize flag description singletons in our Flags namespace
namespace Flags {

  FlagType *Null;
  FlagType *RedTeam;
  FlagType *GreenTeam;
  FlagType *BlueTeam;
  FlagType *PurpleTeam;
  FlagType *Velocity;
  FlagType *QuickTurn;
  FlagType *OscillationOverthruster;
  FlagType *RapidFire;
  FlagType *MachineGun;
  FlagType *GuidedMissile;
  FlagType *Laser;
  FlagType *Ricochet;
  FlagType *SuperBullet;
  FlagType *InvisibleBullet;
  FlagType *Stealth;
  FlagType *Tiny;
  FlagType *Narrow;
  FlagType *Shield;
  FlagType *Steamroller;
  FlagType *ShockWave;
  FlagType *PhantomZone;
  FlagType *Genocide;
  FlagType *Jumping;
  FlagType *Identify;
  FlagType *Cloaking;
  FlagType *Useless;
  FlagType *Masquerade;
  FlagType *Seer;
  FlagType *Thief;
  FlagType *Burrow;
  FlagType *Wings;
  FlagType *Colorblindness;
  FlagType *Obesity;
  FlagType *LeftTurnOnly;
  FlagType *RightTurnOnly;
  FlagType *ForwardOnly;
  FlagType *ReverseOnly;
  FlagType *Momentum;
  FlagType *Blindness;
  FlagType *Jamming;
  FlagType *WideAngle;
  FlagType *NoJumping;
  FlagType *TriggerHappy;
  FlagType *ReverseControls;

  void init()
  {
    Null		    = new FlagType( "", "", FlagNormal, NormalShot, FlagGood, NoTeam, NULL );
    RedTeam                 = new FlagType( "Red Team", "R*", FlagNormal, NormalShot, FlagGood, ::RedTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    GreenTeam               = new FlagType( "Green Team", "G*", FlagNormal, NormalShot, FlagGood, ::GreenTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    BlueTeam                = new FlagType( "Blue Team", "B*", FlagNormal, NormalShot, FlagGood, ::BlueTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    PurpleTeam              = new FlagType( "Purple Team", "P*", FlagNormal, NormalShot, FlagGood, ::PurpleTeam,
						    "Team flag:  If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" );
    Velocity                = new FlagType( "High Speed", "V", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "High Speed (+V):  Tank moves faster.  Outrun bad guys." );
    QuickTurn               = new FlagType( "Quick Turn", "A", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Angular velocity (+A):  Tank turns faster.  Good for dodging." );
    OscillationOverthruster = new FlagType( "Oscillation Overthruster", "OO", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Oscillation Overthruster (+OO):  Can drive through buildings.  Can't backup or shoot while inside." );
    RapidFire               = new FlagType( "Rapid Fire", "F", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "rapid Fire (+F):  Shoots more often.  Shells go faster but not as far." );
    MachineGun              = new FlagType( "Machine Gun", "MG", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Machine Gun (+MG):  Very fast reload and very short range." );
    GuidedMissile           = new FlagType( "Guided Missile", "GM", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Guided Missile (+GM):  Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." );
    Laser                   = new FlagType( "Laser", "L", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time.");
    Ricochet                = new FlagType( "Ricochet", "R", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!" );
    SuperBullet             = new FlagType( "Super Bullet", "SB", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone." );
    InvisibleBullet         = new FlagType( "Invisible Bullet", "IB", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Invisible Bullet (+IB):  Your shots don't appear on other radars.  Can still see them out window.");
    Stealth                 = new FlagType( "Stealth", "ST", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "STealth (+ST):  Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!");
    Tiny                    = new FlagType( "Tiny", "T", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Tiny (+T):  Tank is small and can get through small openings.  Very hard to hit." );
    Narrow                  = new FlagType( "Narrow", "N", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Narrow (+N):  Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings.");
    Shield                  = new FlagType( "Shield", "SH", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SHield (+SH):  Getting hit only drops flag.  Flag flies an extra-long time.");
    Steamroller             = new FlagType( "Steamroller", "SR", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SteamRoller (+SR):  Destroys tanks you touch but you have to get really close.");
    ShockWave               = new FlagType( "Shock Wave", "SW", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "Shock Wave (+SW):  Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings.");
    PhantomZone             = new FlagType( "Phantom Zone", "PZ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank can't shoot or be shot (except by superbullet and shock wave).");
    Genocide                = new FlagType( "Genocide", "G", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Genocide (+G):  Killing one tank kills that tank's whole team.");
    Jumping                 = new FlagType( "Jumping", "JP", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air.");
    Identify                = new FlagType( "Identify", "ID", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "IDentify (+ID):  Identifies type of nearest flag.");
    Cloaking                = new FlagType( "Cloaking", "CL", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "CLoaking (+CL):  Makes your tank invisible out-the-window.  Still visible on radar.");
    Useless                 = new FlagType( "Useless", "US", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "USeless (+US):  You have found the useless flag. Use it wisely.");
    Masquerade              = new FlagType( "Masquerade", "MQ", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "MasQuerade (+MQ):  In opponent's hud, you appear as a teammate.");
    Seer                    = new FlagType( "Seer", "SE", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "SEer (+SE):  See stealthed, cloaked and masquerading tanks as normal.");
    Thief                   = new FlagType( "Thief", "TH", FlagUnstable, SpecialShot, FlagGood, NoTeam,
						    "THief (+TH):  Steal flags.  Small and fast but can't kill.");
    Burrow                  = new FlagType( "Burrow", "BU", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Burrow (+BU):  Tank burrows underground, impervious to normal shots, but can be steamrolled by anyone!");
    Wings                   = new FlagType( "Wings", "WG", FlagUnstable, NormalShot, FlagGood, NoTeam,
						    "Wings (+WG):  Tank can drive in air.");
    ReverseControls             = new FlagType( "ReverseControls", "RC", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Reverse controls (-RC):  Tank driving controls are reversed.");
    Colorblindness          = new FlagType( "Colorblindness", "CB", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!");
    Obesity                 = new FlagType( "Obesity", "O", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Obesity (-O):  Tank becomes very large.  Can't fit through teleporters.");
    LeftTurnOnly            = new FlagType( "Left Turn Only", "LT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Left turn only (-LT):  Can't turn right.");
    RightTurnOnly           = new FlagType( "Right Turn Only", "RT", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Right turn only (-RT):  Can't turn left.");
    ForwardOnly             = new FlagType( "Forward Only", "FO", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Forward only (-FO):  Can't drive in reverse.");
    ReverseOnly             = new FlagType( "ReverseOnly", "RO", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Reverse only (-RO):  Can't drive forward.");
    Momentum                = new FlagType( "Momentum", "M", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Momentum (-M):  Tank has inertia.  Acceleration is limited.");
    Blindness               = new FlagType( "Blindness", "B", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Blindness (-B):  Can't see out window.  Radar still works.");
    Jamming                 = new FlagType( "Jamming", "JM", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "JaMming (-JM):  Radar doesn't work.  Can still see.");
    WideAngle               = new FlagType( "Wide Angle", "WA", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Wide Angle (-WA):  Fish-eye lens distorts view.");
    NoJumping               = new FlagType( "No Jumping", "NJ", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "No Jumping (-NJ):  Tank can't jump.");
    TriggerHappy            = new FlagType( "Trigger Happy", "TR", FlagSticky, NormalShot, FlagBad, NoTeam,
						    "Trigger Happy (-TR):  Tank can't stop firing.");
  }
}

void* FlagType::pack(void* buf) const
{
  buf = nboPackUByte(buf, flagAbbv[0]);
  buf = nboPackUByte(buf, flagAbbv[1]);
  return buf;
}

void* FlagType::unpack(void* buf, FlagType* &type)
{
  unsigned char abbv[3] = {0,0,0};
  buf = nboUnpackUByte(buf, abbv[0]);
  buf = nboUnpackUByte(buf, abbv[1]);
  type = Flag::getDescFromAbbreviation((const char *)abbv);
  return buf;
}

  FlagTypeMap& FlagType::getFlagMap() {
  static FlagTypeMap flagMap;
  return flagMap;
}

void*			Flag::pack(void* buf) const
{
  buf = type->pack(buf);
  buf = nboPackUShort(buf, uint16_t(status));
  buf = nboPackUShort(buf, uint16_t(endurance));
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

  buf = FlagType::unpack(buf, type);
  buf = nboUnpackUShort(buf, data); status = FlagStatus(data);
  buf = nboUnpackUShort(buf, data); endurance = FlagEndurance(data);
  buf = nboUnpackUByte(buf, owner);
  buf = nboUnpackVector(buf, position);
  buf = nboUnpackVector(buf, launchPosition);
  buf = nboUnpackVector(buf, landingPosition);
  buf = nboUnpackFloat(buf, flightTime);
  buf = nboUnpackFloat(buf, flightEnd);
  buf = nboUnpackFloat(buf, initialVelocity);
  return buf;
}

FlagType* Flag::getDescFromAbbreviation(const char* abbreviation)
{
  FlagTypeMap::iterator i;
  std::string abbvString;

  /* Uppercase the abbreviation */
  while (*abbreviation) {
    abbvString += toupper(*abbreviation);
    abbreviation++;
  }

  i = FlagType::getFlagMap().find(abbvString);
  if (i == FlagType::getFlagMap().end())
    /* Not found, return the Null flag */
    return Flags::Null;
  else
    return i->second;
}

FlagSet&		Flag::getGoodFlags()
{
  return FlagType::flagSets[FlagGood];
}

FlagSet&		Flag::getBadFlags()
{
  return FlagType::flagSets[FlagBad];
}

const float*		FlagType::getColor()
{
  static const float superColor[3] = { 1.0, 1.0, 1.0 };

  if (flagTeam == NoTeam)
    return superColor;
  else
    return Team::getTankColor(flagTeam);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

