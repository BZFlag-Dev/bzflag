/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Flag.h"
#include "Pack.h"
#include "Team.h"


/*! \internal The format of this flag table is:
    flag name, flag name abbreviation, flag type, flag help

    Since the FlagTable class is defined within the Flag class, 
	any declaration requires the Flag:: qualifier
  */
const Flag::FlagTable Flag::flagTable[] = {
  Flag::FlagTable("Rogue", "X*", FlagNormal, "No flag for you!"),
  Flag::FlagTable("Red Team", "R*", FlagNormal, ""),
  Flag::FlagTable("Green Team", "G*", FlagNormal, ""),
  Flag::FlagTable("Blue Team", "B*", FlagNormal, ""),
  Flag::FlagTable("Purple Team", "P*", FlagNormal, ""),

  Flag::FlagTable("High Speed", "V", FlagUnstable, 
		  "Velocity (+V):  Tank moves faster.  Outrun bad guys."),
  Flag::FlagTable("Quick Turn", "A", FlagUnstable, 
		  "Angular velocity (+A):  Tank turns faster.  Dodge quicker."),
  Flag::FlagTable("Oscillation Overthruster", "OO", FlagUnstable, 
		  "Oscillation Overthruster (+OO):  Can drive through buildings.  "
		  "Can't backup or shoot while inside."),
  Flag::FlagTable("Rapid Fire", "F", FlagUnstable, 
		  "rapid Fire (+F):  Shoots more often.  Shells go faster but not as far."),
  Flag::FlagTable("Machine Gun", "MG", FlagUnstable, 
		  "Machine Gun (+MG):  Very fast reload and very short range."),
  Flag::FlagTable("Guided Missile", "GM", FlagUnstable, 
		  "Guided Missile (+GM):  Shots track a target.  "
		  "Lock on with right button.  "
		  "Can lock on or retarget after firing."),
  Flag::FlagTable("Laser", "L", FlagUnstable, 
		  "Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time."),
  Flag::FlagTable("Ricochet", "R", FlagUnstable, 
		  "Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!"),
  Flag::FlagTable("Super Bullet", "SB", FlagUnstable, 
		  "SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone."),
  Flag::FlagTable("Invisible Bullet", "IB", FlagUnstable, 
		  "Invisible Bullet (+IB):  Your shots don't appear on other radars.  "
		  "Can still see them out window."),
  Flag::FlagTable("Stealth", "ST", FlagUnstable, 
		  "STealth (+ST):  Tank is invisible on radar.  "
		  "Shots are still visible.  Sneak up behind enemies!"),
  Flag::FlagTable("Tiny", "T", FlagUnstable, 
		  "Tiny (+T):  Tank is small and can get through small openings.  "
		  "Very hard to hit."),
  Flag::FlagTable("Narrow", "N", FlagUnstable, 
		  "Narrow (+N):  Tank is super thin.  Very hard to hit from front but is "
		  "normal size from side.  Can get through small openings."),
  Flag::FlagTable("Shield", "SH", FlagUnstable, 
		  "SHield (+SH):  Getting hit only drops flag.  Flag flys an extra-long time."),
  Flag::FlagTable("Steamroller", "SR", FlagUnstable, 
		  "SteamRoller (+SR):  Destroys tanks you touch but you have to get really close."),
  Flag::FlagTable("Shock Wave", "SW", FlagUnstable, 
		  "Shock Wave (+SW):  Firing destroys all tanks nearby.  "
		  "Don't kill teammates!  Can kill tanks on/in buildings."),
  Flag::FlagTable("Phantom Zone", "PZ", FlagUnstable, 
		  "Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  "
		  "Zoned tank can drive through buildings.  "
		  "Zoned tank can't shoot or be shot (except by "
		  "superbullet and shock wave)."),
  Flag::FlagTable("Genocide", "G", FlagUnstable, 
		  "Genocide (+G):  Killing one tank kills that tank's whole team."),
  Flag::FlagTable("Jumping", "JP", FlagUnstable, 
		  "JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air."),
  Flag::FlagTable("Identify", "ID", FlagUnstable, 
		  "IDentify (+ID):  Identifies type of nearest flag."),
  Flag::FlagTable("Cloaking", "CL", FlagUnstable, 
		  "CLoaking (+CL):  Makes your tank invisible out-the-window.  "
		  "Still visible on radar."),
  Flag::FlagTable("Masquerade", "MQ", FlagUnstable, 
		  "MasQuerade (+MQ):  Makes your tank look like a teammate out-the-window.  "
		  "Normal team colors everywhere else."),
  Flag::FlagTable("Thief", "TH", FlagUnstable, 
		  "THief (+TH):  Steal flags.  Small and fast but can't kill."),
  Flag::FlagTable("Burrow", "BU", FlagUnstable, 
		  "BUrrow (+BU):  Drive partly underground. Can't be hit by regular shot."
		  "Don't get run over!"),
  Flag::FlagTable("Seer", "SE", FlagUnstable, 
		  "SEer (+SE):  See Cloaked, Stealthed and Masqueraded tanks as normal"),

  Flag::FlagTable("Colorblindness", "CB", FlagSticky, 
		  "ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!"),
  Flag::FlagTable("Obesity", "O", FlagSticky, 
		  "Obesity (-O):  Tank becomes very large.  Can't fit through teleporters."),
  Flag::FlagTable("Left Turn Only", "<-", FlagSticky, 
		  "left turn only (- <-):  Can't turn right."),
  Flag::FlagTable("Right Turn Only", "->", FlagSticky, 
		  "right turn only (- ->):  Can't turn left."),
  Flag::FlagTable("Momentum", "M", FlagSticky, 
		  "Momentum (-M):  Tank has inertia.  Acceleration is limited."),
  Flag::FlagTable("Blindness", "B", FlagSticky, 
		  "Blindness (-B):  Can't see out window.  Radar still works."),
  Flag::FlagTable("Jamming", "JM", FlagSticky, 
		  "JaMming (-JM):  Radar doesn't work.  Can still see."),
  Flag::FlagTable("Wide Angle", "WA", FlagSticky, 
		  "Wide Angle (-WA):  Fish-eye lens distorts view."),
  Flag::FlagTable("No Jumping", "NJ", FlagSticky, 
		  "No Jumping (-NJ):  Tank cannot jump."),
  Flag::FlagTable("Long Reload", "LR", FlagSticky, 
		  "Long Reload (-LR):  Tank takes twice as long to reload."),
  Flag::FlagTable("Antagonize", "AN", FlagSticky, 
		  "Antagonize (-AN): Tank sends taunts to other players involuntarily"),
  Flag::FlagTable("Trigger Happy", "TR", FlagSticky, 
		  "TRigger happy (-TR): Tank fires shots as soon as they are available"),
  Flag::FlagTable("USeless flag", "US", FlagSticky, 
		  "You have found the useless flag. Use it wisely"),
  Flag::FlagTable()
};



void*					Flag::pack(void* buf) const
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

void*					Flag::unpack(void* buf)
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

const char*				Flag::getName(FlagId id)
{
	return flagTable[int(id)].getname();
}

const char*				Flag::getAbbreviation(FlagId id)
{
	return flagTable[int(id)].getabbv();
}

const char*				Flag::getHelp(FlagId id)
{
	return flagTable[int(id)].gethelp();
}

FlagType				Flag::getType(FlagId id)
{
	return flagTable[int(id)].gettype();
}

const float*			Flag::getColor(FlagId id)
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

FlagId					Flag::getIDFromName(const char* name)
{
	// start at 1 because "Rogue" isn't a valid flag name
	for (unsigned int i = FirstFlag; i < countof(flagTable); ++i)
		if (strnocasecmp(flagTable[i].getname(), name) == 0)
			return FlagId(i);
	return NullFlag;
}

FlagId					Flag::getIDFromAbbreviation(const char* abbv)
{
	// start after teams because team flags don't have abbreviations
	for (unsigned int i = FirstFlag; i < countof(flagTable); ++i)
		if (strnocasecmp(flagTable[i].getabbv(), abbv) == 0)
			return FlagId(i);
	return NullFlag;
}
// ex: shiftwidth=4 tabstop=4
