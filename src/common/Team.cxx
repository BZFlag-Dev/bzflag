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

#include "common.h"
#include "Team.h"
#include "Pack.h"

float			Team::tankColor[NumTeams][3] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.2f, 0.2f, 1.0f },
				{ 1.0f, 0.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f }
			};
float			Team::radarColor[NumTeams][3] = {
				{ 1.0f, 1.0f, 0.0f },	// rogue
				{ 1.0f, 0.15f, 0.15f }, // red
				{ 0.2f, 0.9f, 0.2f },	// green
				{ 0.08f, 0.25, 1.0f },	// blue
				{ 1.0f, 0.4f, 1.0f },	// purple
				{ 1.0f, 1.0f, 1.0f }	// white
			};
// taken from old console team messge colors
//static const GLfloat    teamMsgColor[][3] = {
//                                { 1.0, 1.0, 0.5 },    // broadcast
//                                { 1.0, 0.5, 0.5 },    // red
//                                { 0.5, 1.0, 0.5 },    // green
//                                { 0.5, 0.5, 1.0 },    // blue
//                                { 1.0, 0.5, 1.0 }     // purple
//                        };

Team::Team()
{
  size = 0;
  activeSize = 0;
  won = 0;
  lost = 0;
}

void*			Team::pack(void* buf) const
{
  buf = nboPackUShort(buf, uint16_t(size));
  buf = nboPackUShort(buf, uint16_t(activeSize));
  buf = nboPackUShort(buf, uint16_t(won));
  buf = nboPackUShort(buf, uint16_t(lost));
  return buf;
}

void*			Team::unpack(void* buf)
{
  uint16_t inSize, inActiveSize, inWon, inLost;
  buf = nboUnpackUShort(buf, inSize);
  buf = nboUnpackUShort(buf, inActiveSize);
  buf = nboUnpackUShort(buf, inWon);
  buf = nboUnpackUShort(buf, inLost);
  size = (unsigned short)inSize;
  activeSize = (unsigned short)inActiveSize;
  won = (unsigned short)inWon;
  lost = (unsigned short)inLost;
  return buf;
}

const char*		Team::getName(TeamColor team) // const
{
  switch (team) {
    case RogueTeam: return "Rogue";
    case RedTeam: return "Red Team";
    case GreenTeam: return "Green Team";
    case BlueTeam: return "Blue Team";
    case PurpleTeam: return "Purple Team";
    case RabbitTeam: return "Rabbit";
    default: return "Invalid team";
  }
}

const float*		Team::getTankColor(TeamColor team) // const
{
  return tankColor[int(team)];
}

const float*		Team::getRadarColor(TeamColor team) // const
{
  return radarColor[int(team)];
}

void			Team::setColors(TeamColor team,
				const float* tank,
				const float* radar)
{
  tankColor[int(team)][0] = tank[0];
  tankColor[int(team)][1] = tank[1];
  tankColor[int(team)][2] = tank[2];
  radarColor[int(team)][0] = radar[0];
  radarColor[int(team)][1] = radar[1];
  radarColor[int(team)][2] = radar[2];
}
// ex: shiftwidth=2 tabstop=8
