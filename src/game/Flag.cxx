/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// interface header
#include "Flag.h"

// system headers
#include <math.h>
#include <string>
#include <assert.h>
#include <string.h>

// common headers
#include "NetMessage.h"
#include "Pack.h"
#include "Team.h"
#include "TextUtils.h"


FlagSet *FlagType::flagSets = NULL;
const int FlagType::packSize = FlagPackSize;
FlagSet FlagType::customFlags;


/**
 * Initialize flag description singletons in a Flags namespace.  Why
 * we do this isn't exactly clear, but it does encapsulate the names
 * so that it's obvious they're flags when used (e.g. Flags::Agility).
 */
namespace Flags {
  /* alphabetical order */
  FlagType *Agility;
  FlagType *Blindness;
  FlagType *BlueTeam;
  FlagType *Bouncy;
  FlagType *Burrow;
  FlagType *CloakedBullet;
  FlagType *Cloaking;
  FlagType *Colorblindness;
  FlagType *ForwardOnly;
  FlagType *Genocide;
  FlagType *GreenTeam;
  FlagType *GuidedMissile;
  FlagType *Identify;
  FlagType *InvisibleBullet;
  FlagType *Jamming;
  FlagType *Jumping;
  FlagType *Laser;
  FlagType *LeftTurnOnly;
  FlagType *LowGravity;
  FlagType *MachineGun;
  FlagType *Masquerade;
  FlagType *Momentum;
  FlagType *Narrow;
  FlagType *NoJumping;
  FlagType *Obesity;
  FlagType *OscillationOverthruster;
  FlagType *PhantomZone;
  FlagType *PurpleTeam;
  FlagType *QuickTurn;
  FlagType *RapidFire;
  FlagType *RedTeam;
  FlagType *ReverseControls;
  FlagType *ReverseOnly;
  FlagType *Ricochet;
  FlagType *RightTurnOnly;
  FlagType *Seer;
  FlagType *Shield;
  FlagType *ShockWave;
  FlagType *Stealth;
  FlagType *Steamroller;
  FlagType *SuperBullet;
  FlagType *Thief;
  FlagType *Tiny;
  FlagType *TriggerHappy;
  FlagType *Useless;
  FlagType *Velocity;
  FlagType *WideAngle;
  FlagType *Wings;
  FlagType *Null; // leave Null at the end


  /**
   * initialize all flags.  this is oddly called during initialization
   * of MsgStrings instead of as part of some flag manager.
   */
  void init()
  {
    Null	= new FlagType("", "", FlagNormal, StandardShot, FlagGood, NoTeam,
			       "");
    RedTeam	= new FlagType("Red Team", "R*", FlagNormal, StandardShot, FlagGood, ::RedTeam,
			       "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!");
    GreenTeam	= new FlagType("Green Team", "G*", FlagNormal, StandardShot, FlagGood, ::GreenTeam,
			       "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!");
    BlueTeam	= new FlagType("Blue Team", "B*", FlagNormal, StandardShot, FlagGood, ::BlueTeam,
			       "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!");
    PurpleTeam	= new FlagType("Purple Team", "P*", FlagNormal, StandardShot, FlagGood, ::PurpleTeam,
			       "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!");
    Velocity	= new FlagType("High Speed", "V", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank moves faster.  Outrun bad guys.");
    QuickTurn	= new FlagType("Quick Turn", "QT", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank turns faster.  Good for dodging.");
    OscillationOverthruster	= new FlagType("Oscillation Overthruster", "OO", FlagUnstable, StandardShot, FlagGood, NoTeam,
					       "Can drive through buildings.  Can't backup or shoot while inside.");
    RapidFire	= new FlagType("Rapid Fire", "F", FlagUnstable, RapidFireShot, FlagGood, NoTeam,
			       "Shoots more often.  Shells go faster but not as far.");
    MachineGun	= new FlagType("Machine Gun", "MG", FlagUnstable, MachineGunShot, FlagGood, NoTeam,
			       "Very fast reload and very short range.");
    GuidedMissile	= new FlagType("Guided Missile", "GM", FlagUnstable, GMShot, FlagGood, NoTeam,
				       "Shots track a target.  Lock on with right button.  Can lock on or retarget after firing.");
    Laser	= new FlagType("Laser", "L", FlagUnstable, LaserShot, FlagGood, NoTeam,
			       "Shoots a laser.  Infinite speed, long range, but long reload time.");
    Ricochet	= new FlagType("Ricochet", "R", FlagUnstable, RicoShot, FlagGood, NoTeam,
			       "Shots bounce off walls.  Don't shoot yourself!");
    SuperBullet	= new FlagType("Super Bullet", "SB", FlagUnstable, SuperShot, FlagGood, NoTeam,
			       "Shoots through buildings.  Can kill Phantom Zone.");
    InvisibleBullet	= new FlagType("Invisible Bullet", "IB", FlagUnstable, InvisibleShot, FlagGood, NoTeam,
				       "Your shots don't appear on other radars.  Can still see them out window.");
    CloakedBullet	= new FlagType("Cloaked Shot", "CS", FlagUnstable, CloakedShot, FlagGood, NoTeam,
				       "Shots are invisible out-the-window. Still visible on radar. Glow is still visible out-the-window. Watch out for ricochet!");
    Stealth	= new FlagType("Stealth", "ST", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!");
    Tiny	= new FlagType("Tiny", "T", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank is small and can get through small openings.  Very hard to hit.");
    Narrow	= new FlagType("Narrow", "N", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings.");
    Shield	= new FlagType("Shield", "SH", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Getting hit only drops flag.  Flag flies an extra-long time.");
    Steamroller	= new FlagType("Steamroller", "SR", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Destroys tanks you touch but you have to get really close.");
    ShockWave	= new FlagType("Shock Wave", "SW", FlagUnstable, ShockWaveShot, FlagGood, NoTeam,
			       "Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings.");
    PhantomZone	= new FlagType("Phantom Zone", "PZ", FlagUnstable, PhantomShot, FlagGood, NoTeam,
			       "Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank shoots Zoned bullets and can't be shot (except by superbullet, shock wave, and other Zoned tanks).");
    Genocide	= new FlagType("Genocide", "G", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Killing one tank kills that tank's whole team.");
    Jumping	= new FlagType("Jumping", "JP", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank can jump.  Use Tab key.  Can't steer in the air.");
    Identify	= new FlagType("Identify", "ID", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Identifies type of nearest flag.");
    Cloaking	= new FlagType("Cloaking", "CL", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Makes your tank invisible out-the-window.  Still visible on radar.");
    Useless	= new FlagType("Useless", "US", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "You have found the useless flag. Use it wisely.");
    Masquerade	= new FlagType("Masquerade", "MQ", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "In opponent's hud, you appear as a teammate.");
    Seer	= new FlagType("Seer", "SE", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "See stealthed, cloaked and masquerading tanks as normal.  See invisible bullets and cloaked shots.");
    Thief	= new FlagType("Thief", "TH", FlagUnstable, ThiefShot, FlagGood, NoTeam,
			       "Steal flags.  Small and fast but can't kill.");
    Burrow	= new FlagType("Burrow", "BU", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank burrows underground, impervious to normal shots, but can be steamrolled by anyone!");
    Wings	= new FlagType("Wings", "WG", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank can drive in air.");
    Agility	= new FlagType("Agility", "A", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "Tank is quick and nimble making it easier to dodge.");
    LowGravity	= new FlagType("Low Gravity", "LG", FlagUnstable, StandardShot, FlagGood, NoTeam,
			       "The gravity is reduced. Tank jumps higher, falls slower.");
    ReverseControls	= new FlagType("ReverseControls", "RC", FlagSticky, StandardShot, FlagBad, NoTeam,
				       "Tank driving controls are reversed.");
    Colorblindness	= new FlagType("Colorblindness", "CB", FlagSticky, StandardShot, FlagBad, NoTeam,
				       "Can't tell team colors.  Don't shoot teammates!");
    Obesity	= new FlagType("Obesity", "O", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Tank becomes very large.  Can't fit through teleporters.");
    LeftTurnOnly	= new FlagType("Left Turn Only", "LT", FlagSticky, StandardShot, FlagBad, NoTeam,
				       "Can't turn right.");
    RightTurnOnly	= new FlagType("Right Turn Only", "RT", FlagSticky, StandardShot, FlagBad, NoTeam,
				       "Can't turn left.");
    ForwardOnly	= new FlagType("Forward Only", "FO", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Can't drive in reverse.");
    ReverseOnly	= new FlagType("ReverseOnly", "RO", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Can't drive forward.");
    Momentum	= new FlagType("Momentum", "M", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Tank has inertia.  Acceleration is limited.");
    Blindness	= new FlagType("Blindness", "B", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Can't see out window.  Radar still works.");
    Jamming	= new FlagType("Jamming", "JM", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Radar doesn't work.  Can still see.");
    WideAngle	= new FlagType("Wide Angle", "WA", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Fish-eye lens distorts view.");
    NoJumping	= new FlagType("No Jumping", "NJ", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Tank can't jump.");
    TriggerHappy	= new FlagType("Trigger Happy", "TR", FlagSticky, StandardShot, FlagBad, NoTeam,
				       "Tank can't stop firing.");
    Bouncy	= new FlagType("Bouncy", "BY", FlagSticky, StandardShot, FlagBad, NoTeam,
			       "Tank can't stop bouncing.");
  }


  /**
   * release the dynamic memory for all flags allocated during init()
   */
  void kill()
  {
    clearCustomFlags();

    /* alphabetical order */
    delete Agility;
    delete Blindness;
    delete BlueTeam;
    delete Bouncy;
    delete Burrow;
    delete CloakedBullet;
    delete Cloaking;
    delete Colorblindness;
    delete ForwardOnly;
    delete Genocide;
    delete GreenTeam;
    delete GuidedMissile;
    delete Identify;
    delete InvisibleBullet;
    delete Jamming;
    delete Jumping;
    delete Laser;
    delete LeftTurnOnly;
    delete LowGravity;
    delete MachineGun;
    delete Masquerade;
    delete Momentum;
    delete Narrow;
    delete NoJumping;
    delete Obesity;
    delete OscillationOverthruster;
    delete PhantomZone;
    delete PurpleTeam;
    delete QuickTurn;
    delete RapidFire;
    delete RedTeam;
    delete ReverseControls;
    delete ReverseOnly;
    delete Ricochet;
    delete RightTurnOnly;
    delete Seer;
    delete Shield;
    delete ShockWave;
    delete Stealth;
    delete Steamroller;
    delete SuperBullet;
    delete Thief;
    delete Tiny;
    delete TriggerHappy;
    delete Useless;
    delete Velocity;
    delete WideAngle;
    delete Wings;
    delete Null; // leave Null at end

    delete[] FlagType::flagSets;
  }

  void clearCustomFlags()
  {
    FlagSet::iterator itr, nitr;
    for (int q = 0; q < (int)NumQualities; ++q) {
      for (itr = FlagType::flagSets[q].begin(); itr != FlagType::flagSets[q].end(); ++itr) {
	if ((*itr)->custom) {
	  FlagType::getFlagMap().erase((*itr)->flagAbbv);
	  nitr = itr;
	  ++nitr;
	  delete (*itr);
	  FlagType::flagSets[q].erase(itr);
	  itr = nitr;
	  if (itr == FlagType::flagSets[q].end()) break;
	}
      }
    }
    FlagType::customFlags.clear();
  }
}


void* FlagType::pack(void* buf) const
{
  buf = nboPackUInt8(buf, (flagAbbv.size() > 0) ? flagAbbv[0] : 0);
  buf = nboPackUInt8(buf, (flagAbbv.size() > 1) ? flagAbbv[1] : 0);
  return buf;
}


void* FlagType::fakePack(void* buf) const
{
  buf = nboPackUInt8(buf, 'P');
  buf = nboPackUInt8(buf, 'Z');
  return buf;
}


size_t FlagType::pack(NetMessage& netMsg) const
{
  unsigned char buf[2] = {0, 0};

  pack((void*)buf);

  netMsg.packUInt8(buf[0]);
  netMsg.packUInt8(buf[1]);

  return 2;
}


size_t FlagType::fakePack(NetMessage& netMsg) const
{
  netMsg.packUInt8('P');
  netMsg.packUInt8('Z');
  return 2;
}


size_t FlagType::packCustom(NetMessage& netMsg) const
{
  const size_t s = netMsg.getSize();
  pack(netMsg);
  netMsg.packUInt8(uint8_t(flagQuality));
  netMsg.packUInt8(uint8_t(flagShot));
  netMsg.packStdString(flagName);
  netMsg.packStdString(flagHelp);
  return  netMsg.getSize() - s;
}


void* FlagType::unpack(void* buf, FlagType* &type)
{
  unsigned char abbv[3] = {0, 0, 0};
  buf = nboUnpackUInt8(buf, abbv[0]);
  buf = nboUnpackUInt8(buf, abbv[1]);
  type = Flag::getDescFromAbbreviation((const char *)abbv);
  return buf;
}


void* FlagType::unpackCustom(void* buf, FlagType* &type)
{
  unsigned char abbv[3] = {0, 0, 0};
  buf = nboUnpackUInt8(buf, abbv[0]);
  buf = nboUnpackUInt8(buf, abbv[1]);

  uint8_t quality, shot;
  buf = nboUnpackUInt8(buf, quality);
  buf = nboUnpackUInt8(buf, shot);

  std::string sName, sHelp;
  buf = nboUnpackStdString(buf, sName);
  buf = nboUnpackStdString(buf, sHelp);

  FlagEndurance e = FlagUnstable;
  switch((FlagQuality)quality) {
    case FlagGood: e = FlagUnstable; break;
    case FlagBad: e = FlagSticky; break;
    default: assert(false); // shouldn't happen
  }

  type = new FlagType(sName, reinterpret_cast<const char*>(&abbv[0]),
		      e, (ShotType)shot, (FlagQuality)quality, NoTeam, sHelp, true);
  return buf;
}


FlagTypeMap& FlagType::getFlagMap() {
  static FlagTypeMap flagMap;
  return flagMap;
}


void* Flag::pack(void* buf) const
{
  buf = type->pack(buf);
  buf = nboPackUInt16(buf, uint16_t(status));
  buf = nboPackUInt16(buf, uint16_t(endurance));
  buf = nboPackUInt8(buf, owner);
  buf = nboPackFVec3(buf, position);
  buf = nboPackFVec3(buf, launchPosition);
  buf = nboPackFVec3(buf, landingPosition);
  buf = nboPackFloat(buf, flightTime);
  buf = nboPackFloat(buf, flightEnd);
  buf = nboPackFloat(buf, initialVelocity);
  return buf;
}


size_t Flag::pack(NetMessage& netMsg) const
{
  const size_t s = netMsg.getSize();
  type->pack(netMsg);
  netMsg.packUInt16(uint16_t(status));
  netMsg.packUInt16(uint16_t(endurance));
  netMsg.packUInt8(owner);
  netMsg.packFVec3(position);
  netMsg.packFVec3(launchPosition);
  netMsg.packFVec3(landingPosition);
  netMsg.packFloat(flightTime);
  netMsg.packFloat(flightEnd);
  netMsg.packFloat(initialVelocity);
  return netMsg.getSize() - s;
}


size_t Flag::fakePack(NetMessage& netMsg) const
{
  const size_t s = netMsg.getSize();
  type->fakePack(netMsg);
  netMsg.packUInt16(uint16_t(status));
  netMsg.packUInt16(uint16_t(endurance));
  netMsg.packUInt8(owner);
  netMsg.packFVec3(position);
  netMsg.packFVec3(launchPosition);
  netMsg.packFVec3(landingPosition);
  netMsg.packFloat(flightTime);
  netMsg.packFloat(flightEnd);
  netMsg.packFloat(initialVelocity);
  return netMsg.getSize() - s;
}


void* Flag::fakePack(void* buf) const
{
  buf = type->fakePack(buf);
  buf = nboPackUInt16(buf, uint16_t(status));
  buf = nboPackUInt16(buf, uint16_t(endurance));
  buf = nboPackUInt8(buf, owner);
  buf = nboPackFVec3(buf, position);
  buf = nboPackFVec3(buf, launchPosition);
  buf = nboPackFVec3(buf, landingPosition);
  buf = nboPackFloat(buf, flightTime);
  buf = nboPackFloat(buf, flightEnd);
  buf = nboPackFloat(buf, initialVelocity);
  return buf;
}


void* Flag::unpack(void* buf)
{
  uint16_t data;
  // bytes
  buf = FlagType::unpack(buf, type);				     // 2
  buf = nboUnpackUInt16(buf, data); status = FlagStatus(data);	     // 2
  buf = nboUnpackUInt16(buf, data); endurance = FlagEndurance(data); // 2
  buf = nboUnpackUInt8(buf, owner);				     // 1
  buf = nboUnpackFVec3(buf, position);                               // 12 (3x4)
  buf = nboUnpackFVec3(buf, launchPosition);                         // 12 (3x4)
  buf = nboUnpackFVec3(buf, landingPosition);                        // 12 (3x4)
  buf = nboUnpackFloat(buf, flightTime);			     // 4
  buf = nboUnpackFloat(buf, flightEnd);				     // 4
  buf = nboUnpackFloat(buf, initialVelocity);			     // 4
  return buf;						// total        55
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


FlagSet& Flag::getGoodFlags()
{
  return FlagType::flagSets[FlagGood];
}


FlagSet& Flag::getBadFlags()
{
  return FlagType::flagSets[FlagBad];
}


const fvec4& FlagType::getColor() const
{
  static const fvec4 superColor(1.0f, 1.0f, 1.0f, 1.0f);

  if (flagTeam == NoTeam)
    return superColor;
  else
    return Team::getTankColor(flagTeam);
}



const std::string FlagType::label() const
{
  size_t i;

  /* convert to lowercase so we can uppercase the abbreviation later */
  std::string caseName = "";
  for (i = 0; i < flagName.size(); i++) {
    caseName += tolower(flagName.c_str()[i]);
  }

  /* modify the flag name to exemplify the abbreviation */
  size_t charPosition = caseName.size();
  for (i = 0; i < flagAbbv.size(); i++) {

    charPosition = caseName.find_first_of(tolower(flagAbbv[i]), 0);

    if (0 < charPosition && charPosition < caseName.size()) {
      /* if we can match an abbreviation on a word boundary -- prefer it */
      size_t alternateCharPosition = 1;
      while (0 < alternateCharPosition && alternateCharPosition < caseName.size()) {
	if (caseName[alternateCharPosition - 1] == ' ') {
	  charPosition = alternateCharPosition;
	  break;
	}
	alternateCharPosition = caseName.find_first_of(tolower(flagAbbv[i]), alternateCharPosition+1);
      }
    }

    if (charPosition < caseName.size()) {
      caseName[charPosition] = toupper(caseName[charPosition]);
    }
  }

  if (flagTeam != NoTeam) {
    /* team flag info is more simple than non-team flag info */
    caseName += " flag";
  } else {
    /* non-team flags */
    caseName += TextUtils::format(" (%c%s)",
				  flagQuality==FlagGood?'+':'-',
				  flagAbbv.c_str());
  }

  return caseName;
}


const std::string FlagType::information() const
{
  return TextUtils::format("%s: %s",
			   label().c_str(),
			   flagHelp.c_str());
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
