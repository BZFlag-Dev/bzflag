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

#include "Team.h"
#include "Pack.h"
#include <string>
#include "StateDatabase.h"
#include <stdio.h>

float					Team::tankColor[NumTeams][3] = {
								{ 0.0f, 0.0f, 0.0f },
								{ 1.0f, 0.0f, 0.0f },
								{ 0.0f, 1.0f, 0.0f },
								{ 0.2f, 0.2f, 1.0f },
								{ 1.0f, 0.0f, 1.0f }
						};
float					Team::radarColor[NumTeams][3] = {
//								{ 1.0f, 1.0f, 0.0f },
//								{ 1.0f, 0.0f, 0.0f },
//								{ 0.0f, 1.0f, 0.0f },
//								{ 0.25f, 0.25, 1.0f },	// blue is hard to see
//								{ 1.0f, 0.0f, 1.0f }
								{ 1.0f, 1.0f, 0.0f },	// rogue
								{ 1.0f, 0.25f, 0.25f },	//red
								{ 0.5f, 1.0f, 0.5f },	//green
								{ 0.5f, 0.5, 1.0f },	// blue is hard to see
								{ 1.0f, 0.25f, 1.0f }	//purple
						};
// taken from old console team messge colors
//static const GLfloat    teamMsgColor[][3] = {
//                                { 1.0, 1.0, 0.5 },    // broadcast
//                                { 1.0, 0.5, 0.5 },    // red
//                                { 0.5, 1.0, 0.5 },    // green
//                                { 0.5, 0.5, 1.0 },    // blue
//                                { 1.0, 0.5, 1.0 }     // purple
//                        };


void*					Team::pack(void* buf) const
{
	buf = nboPackUShort(buf, uint16_t(size));
	buf = nboPackUShort(buf, uint16_t(activeSize));
	buf = nboPackUShort(buf, uint16_t(won));
	buf = nboPackUShort(buf, uint16_t(lost));
	return buf;
}

void*					Team::unpack(void* buf)
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

void					Team::init()
{
	BZDB->addCallback("colorTeamRogue",   onColorChange, NULL);
	BZDB->addCallback("colorTeamRed",     onColorChange, NULL);
	BZDB->addCallback("colorTeamGreen",   onColorChange, NULL);
	BZDB->addCallback("colorTeamBlue",    onColorChange, NULL);
	BZDB->addCallback("colorTeamPurple",  onColorChange, NULL);

	BZDB->addCallback("colorRadarRogue",  onColorChange, NULL);
	BZDB->addCallback("colorRadarRed",    onColorChange, NULL);
	BZDB->addCallback("colorRadarGreen",  onColorChange, NULL);
	BZDB->addCallback("colorRadarBlue",   onColorChange, NULL);
	BZDB->addCallback("colorRadarPurple", onColorChange, NULL);
}

std::string				Team::getName(TeamColor team) // const
{
	switch (team) {
		case RogueTeam: return "Rogue";
		case RedTeam: return "Red Team";
		case GreenTeam: return "Green Team";
		case BlueTeam: return "Blue Team";
		case PurpleTeam: return "Purple Team";
		default: return "Invalid team";
	}
}

std::string				Team::getEnumName(TeamColor team) // const
{
	switch (team) {
		case RogueTeam: return "rogue";
		case RedTeam: return "red";
		case GreenTeam: return "green";
		case BlueTeam: return "blue";
		case PurpleTeam: return "purple";
		default: return "invalid";
	}
}

TeamColor				Team::getEnum(const std::string& name)
{
	if (name == "rogue")
		return RogueTeam;
	else if (name == "red")
		return RedTeam;
	else if (name == "green")
		return GreenTeam;
	else if (name == "blue")
		return BlueTeam;
	else if (name == "purple")
		return PurpleTeam;
	else
		return NoTeam;
}

const float*				Team::getTankColor(TeamColor team) // const
{
	return tankColor[int(team)];
}

const float*				Team::getRadarColor(TeamColor team) // const
{
	return radarColor[int(team)];
}

void					Team::onColorChange(const std::string& name, void*)
{
	// choose color
	float* color;
	if (name == "colorTeamRogue")
		color = tankColor[RogueTeam];
	else if (name == "colorTeamRed")
		color = tankColor[RedTeam];
	else if (name == "colorTeamGreen")
		color = tankColor[GreenTeam];
	else if (name == "colorTeamBlue")
		color = tankColor[BlueTeam];
	else if (name == "colorTeamPurple")
		color = tankColor[PurpleTeam];
	else if (name == "colorRadarRogue")
		color = radarColor[RogueTeam];
	else if (name == "colorRadarRed")
		color = radarColor[RedTeam];
	else if (name == "colorRadarGreen")
		color = radarColor[GreenTeam];
	else if (name == "colorRadarBlue")
		color = radarColor[BlueTeam];
	else if (name == "colorRadarPurple")
		color = radarColor[PurpleTeam];
	else
		return;

	// parse value
	if (sscanf(BZDB->get(name).c_str(), "%f %f %f", color+0, color+1, color+2) != 3)
		return;

	// clamp if not in range
#define CLAMP(_x, _a, _b) (((_x) < (_a)) ? (_a) : (((_x) > (_b)) ? (_b) : (_x)))
	color[0] = CLAMP(color[0], 0.0f, 1.0f);
	color[1] = CLAMP(color[1], 0.0f, 1.0f);
	color[2] = CLAMP(color[2], 0.0f, 1.0f);
#undef CLAMP
}
