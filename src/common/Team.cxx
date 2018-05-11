/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "Team.h"
#include "AnsiCodes.h"
#include "BZDBCache.h"
#include "Pack.h"

float			Team::tankColor[NumTeams][3] = {
				{ 1.0f, 1.0f, 0.0f },   // rogue
				{ 1.0f, 0.0f, 0.0f },   // red
				{ 0.0f, 1.0f, 0.0f },   // green
				{ 0.1f, 0.2f, 1.0f },   // blue
				{ 1.0f, 0.0f, 1.0f },   // purple
				{ 1.0f, 1.0f, 1.0f },   // observer
				{ 0.8f, 0.8f, 0.8f },   // rabbit
				{ 1.0f, 0.5f, 0.0f }	// hunter orange
			};
float			Team::radarColor[NumTeams][3] = {
				{ 1.0f, 1.0f, 0.0f },	// rogue
				{ 1.0f, 0.15f, 0.15f }, // red
				{ 0.2f, 0.9f, 0.2f },	// green
				{ 0.08f, 0.25, 1.0f},	// blue
				{ 1.0f, 0.4f, 1.0f },	// purple
				{ 1.0f, 1.0f, 1.0f },	// observer
				{ 1.0f, 1.0f, 1.0f },   // rabbit
				{ 1.0f, 0.5f, 0.0f }	// hunter orange
			};
float			Team::shotColor[NumTeams][3];


Team::Team()
{
  size = 0;
  won = 0;
  lost = 0;
}

void*			Team::pack(void* buf) const
{
  buf = nboPackUShort(buf, uint16_t(size));
  buf = nboPackUShort(buf, uint16_t(won));
  buf = nboPackUShort(buf, uint16_t(lost));
  return buf;
}

const void*		Team::unpack(const void* buf)
{
  uint16_t inSize, inWon, inLost;
  buf = nboUnpackUShort(buf, inSize);
  buf = nboUnpackUShort(buf, inWon);
  buf = nboUnpackUShort(buf, inLost);
  size = (unsigned short)inSize;
  won = (unsigned short)inWon;
  lost = (unsigned short)inLost;
  return buf;
}

const std::string  Team::getImagePrefix(TeamColor team)
{
  switch (team) {
  case RedTeam: return BZDB.get("redTeamPrefix");
  case GreenTeam: return BZDB.get("greenTeamPrefix");
  case BlueTeam: return BZDB.get("blueTeamPrefix");
  case PurpleTeam: return BZDB.get("purpleTeamPrefix");
  case RabbitTeam: return BZDB.get("rabbitTeamPrefix");
  case HunterTeam: return BZDB.get("hunterTeamPrefix");
  case ObserverTeam: return BZDB.get("observerTeamPrefix");
  default: return BZDB.get("rogueTeamPrefix");
  }
}

const char*		Team::getName(TeamColor team) // const
{
  switch (team) {
  case AutomaticTeam: return "Automatic";
  case RogueTeam: return "Rogue";
  case RedTeam: return "Red Team";
  case GreenTeam: return "Green Team";
  case BlueTeam: return "Blue Team";
  case PurpleTeam: return "Purple Team";
  case ObserverTeam: return "Observer";
  case RabbitTeam: return "Rabbit";
  case HunterTeam: return "Hunter";
  case NoTeam: return "No Team??";
  default: return "Invalid team";
  }
}

TeamColor	Team::getTeam(const std::string &name) // const
{
  if (name == Team::getName(AutomaticTeam)) {
    return AutomaticTeam;
  }
  for (int i = 0; i < NumTeams; i++) {
    if (name == Team::getName((TeamColor)i)) {
      return (TeamColor)i;
    }
  }
  return NoTeam;
}

const float*		Team::getTankColor(TeamColor team) // const
{
  if (int(team) < 0) {
    return tankColor[0];
  }
  return tankColor[int(team)];
}

const float*		Team::getRadarColor(TeamColor team) // const
{
  if (int(team) < 0) {
    return radarColor[0];
  }
  return radarColor[int(team)];
}

const float*		Team::getShotColor(TeamColor team) // const
{
  if (int(team) < 0) {
    return shotColor[0];
  }
  return shotColor[int(team)];
}

const std::string	Team::getAnsiCode(TeamColor team) // const
{
  return rgbToAnsi(getTankColor(team));
}

bool		Team::isColorTeam(TeamColor team) // const
{
  return team >= RedTeam  && team <= PurpleTeam;
}

void			Team::setColors(TeamColor team,
				const float* tank,
				const float* radar)
{
  const int teamIndex = int(team);
  // ignore bogus team color
  if (teamIndex < 0)
    return;

  for (int i = 0; i <= 2; i++) {
    tankColor[teamIndex][i] = tank[i];
    radarColor[teamIndex][i] = radar[i];
    shotColor[teamIndex][i] = addBrightness(tank[i]);
  }
}

void			Team::updateShotColors()
{
  for (int teamIndex = 0; teamIndex < NumTeams; teamIndex++) {
    for (int i = 0; i <= 2; i++) {
      shotColor[teamIndex][i] = addBrightness(tankColor[teamIndex][i]);
    }
  }
}

float Team::addBrightness(const float color)
{
  if (BZDBCache::shotBrightness == 0.0f) {
    return color;
  }

  float brightness = BZDBCache::shotBrightness;

  if (brightness > 0.0f) {
    brightness *= pow(1.0f - color, 4.0f);
  } else {
    brightness *= color;
  }

  // Make sure that the resulting color doesn't get below 0 or above 1
  if (brightness + color > 1.0f) {
    return 1.0f;
  } else if (brightness + color < 0.0f) {
    return 0.0f;
  }

  return brightness + color;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
