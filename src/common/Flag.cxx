/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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

const char*		Flag::flagName[] = {
				"Rogue",		// should never be used
				"Red Team",
				"Green Team",
				"Blue Team",
				"Purple Team",
				"High Speed",
				"Quick Turn",
				"Oscillation Overthruster",
				"Rapid Fire",
				"Machine Gun",
				"Guided Missile",
				"Laser",
				"Ricochet",
				"Super Bullet",
				"Invisible Bullet",
				"Stealth",
				"Tiny",
				"Narrow",
				"Shield",
				"Steamroller",
				"Shock Wave",
				"Phantom Zone",
				"Genocide",
				"Jumping",
				"Identify",
				"Cloaking",
				"Colorblindness",
				"Obesity",
				"Left Turn Only",
				"Right Turn Only",
				"Momentum",
				"Blindness",
				"Jamming",
				"Wide Angle"
			};

const char*		Flag::flagAbbv[] = {
				"",			// rogue team
				"",			// red team
				"",			// green team
				"",			// blue team
				"",			// purple team
				"V",			// high speed
				"A",			// quick turn
				"OO",			// oscillation over...
				"F",			// rapid fire
				"MG",			// machine gun
				"GM",			// guided missile
				"L",			// laser
				"R",			// ricochet
				"SB",			// super bullet
				"IB",			// invisible bullet
				"ST",			// stealth
				"T",			// tiny
				"N",			// narrow
				"SH",			// shield
				"SR",			// steamroller
				"SW",			// shock wave
				"PZ",			// phantom zone
				"G",			// genocide
				"JP",			// jumping
				"ID",			// identify
				"CL",			// cloaking
				"CB",			// colorblindness
				"O",			// obesity
				"<-",			// left turn only
				"->",			// right turn only
				"M",			// momentum
				"B",			// blindness
				"JM",			// jamming
				"WA"			// wide angle
			};

void*			Flag::pack(void* buf) const
{
  buf = nboPackUShort(buf, uint16_t(id));
  buf = nboPackUShort(buf, uint16_t(status));
  buf = nboPackUShort(buf, uint16_t(type));
  buf = owner.pack(buf);
  buf = nboPackFloat(buf, position[0]);
  buf = nboPackFloat(buf, position[1]);
  buf = nboPackFloat(buf, position[2]);
  buf = nboPackFloat(buf, launchPosition[0]);
  buf = nboPackFloat(buf, launchPosition[1]);
  buf = nboPackFloat(buf, launchPosition[2]);
  buf = nboPackFloat(buf, landingPosition[0]);
  buf = nboPackFloat(buf, landingPosition[1]);
  buf = nboPackFloat(buf, landingPosition[2]);
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
  buf = owner.unpack(buf);
  buf = nboUnpackFloat(buf, position[0]);
  buf = nboUnpackFloat(buf, position[1]);
  buf = nboUnpackFloat(buf, position[2]);
  buf = nboUnpackFloat(buf, launchPosition[0]);
  buf = nboUnpackFloat(buf, launchPosition[1]);
  buf = nboUnpackFloat(buf, launchPosition[2]);
  buf = nboUnpackFloat(buf, landingPosition[0]);
  buf = nboUnpackFloat(buf, landingPosition[1]);
  buf = nboUnpackFloat(buf, landingPosition[2]);
  buf = nboUnpackFloat(buf, flightTime);
  buf = nboUnpackFloat(buf, flightEnd);
  buf = nboUnpackFloat(buf, initialVelocity);
  return buf;
}

const char*		Flag::getName(FlagId id)
{
  return flagName[int(id)];
}

const char*		Flag::getAbbreviation(FlagId id)
{
  return flagAbbv[int(id)];
}

FlagType		Flag::getType(FlagId id)
{
  switch (id) {
    case NoFlag:
    case RedFlag:
    case GreenFlag:
    case BlueFlag:
    case PurpleFlag:
	return FlagNormal;
    case ColorblindnessFlag:
    case ObesityFlag:
    case LeftTurnOnlyFlag:
    case RightTurnOnlyFlag:
    case MomentumFlag:
    case BlindnessFlag:
    case JammingFlag:
    case WideAngleFlag:
	return FlagSticky;
    default:
	return FlagUnstable;
  }
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
