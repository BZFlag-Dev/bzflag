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

#include "common.h"
#include <math.h>
#include <string>
#include <sstream>
#include "WorldRegions.h"
#include "WorldInfo.h"
#include "global.h"
#include "Flag.h"
#include "Team.h"
#include "mathr.h"

extern float basePos[NumTeams][3];
extern float baseRotation[NumTeams];
extern float baseSize[NumTeams][3];
extern float safetyBasePos[NumTeams][3];

static std::string defineWalls(const WorldInfo::ObstacleList& list)
{
	std::string xml;
	for (unsigned int i = 0; i < list.size(); ++i) {
		const WorldInfo::ObstacleLocation& o = list[i];
		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0] - cosf(o.rotation),
				o.pos[1] - sinf(o.rotation),
				o.pos[2] + 0.5f * o.size[2],
				rad2deg(o.rotation),
				2.0f, o.size[0], o.size[2]);
	}
	return xml;
}

static std::string defineBoxes(const WorldInfo::ObstacleList& list)
{
	std::string xml;
	for (unsigned int i = 0; i < list.size(); ++i) {
		const WorldInfo::ObstacleLocation& o = list[i];
		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0], o.pos[1], o.pos[2] + 0.5f * o.size[2],
				rad2deg(o.rotation),
				o.size[0], o.size[1], 0.5f * o.size[2]);
	}
	return xml;
}

static std::string definePyramids(const WorldInfo::ObstacleList& list)
{
	std::string xml;
	for (unsigned int i = 0; i < list.size(); ++i) {
		const WorldInfo::ObstacleLocation& o = list[i];
		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<pyramid>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</pyramid>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0], o.pos[1], o.pos[2] + 0.25f * o.size[2],
				rad2deg(o.rotation),
				o.size[0], o.size[1], o.size[2]);
	}
	return xml;
}

static std::string defineTeleporters(const WorldInfo::TeleporterList& list)
{
	std::string xml;
	for (unsigned int i = 0; i < list.size(); ++i) {
		const WorldInfo::Teleporter& o = list[i];

		const float c = cosf(o.rotation), s = sinf(o.rotation);
		const float d = o.size[1] + 0.5f * o.border;

		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0] - s * d,
				o.pos[1] + c * d,
				o.pos[2] + 0.5f * o.size[2],
				rad2deg(o.rotation),
				o.border, o.border, o.size[2]);

		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0] + s * d,
				o.pos[1] - c * d,
				o.pos[2] + 0.5f * o.size[2],
				rad2deg(o.rotation),
				o.border, o.border, o.size[2]);

		xml += string_util::format(
				"<obstacle>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</obstacle>\n",
				o.pos[0], o.pos[1], o.pos[2] + o.size[2] + 0.5f * o.border,
				rad2deg(o.rotation),
				o.size[0], o.size[1] + 2.0 * o.border, o.border);
	}
	return xml;
}

static std::string defineBases()
{
	std::string xml;
	for (unsigned int i = 0; i < NumTeams; ++i) {
		xml += string_util::format(
				"<base team=\"%s\">\n"
					"<shape base=\"true\" spawn=\"true\">\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
					"<safety x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"</base>\n",
				Team::getEnumName(static_cast<TeamColor>(i)).c_str(),
				basePos[i][0], basePos[i][1], basePos[i][2],
				rad2deg(baseRotation[i]),
				baseSize[i][0], baseSize[i][1], 0.5f,
				safetyBasePos[i][0], safetyBasePos[i][1], safetyBasePos[i][2]);

		if (i != RogueTeam)
			xml += string_util::format(
				"<flag-spawn flags=\"%s\">\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<rotate x=\"0\" y=\"0\" z=\"1\" a=\"%f\"/>\n"
							"<size x=\"0\" y=\"0\" z=\"0\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</flag-spawn>\n",
				Flag::getName(static_cast<FlagId>(i)),
				basePos[i][0], basePos[i][1], basePos[i][2],
				rad2deg(baseRotation[i]));
	}
	return xml;
}

static std::string defineFlagSpawn()
{
	std::string xml;
	xml += string_util::format(
				"<flag-spawn>\n"
					"<shape>\n"
						"<box>\n"
							"<translate x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
							"<size x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
						"</box>\n"
					"</shape>\n"
				"</flag-spawn>\n",
				0.0, 0.0, 0.5 * WorldSize,
				0.5 * WorldSize - BaseSize,
				0.5 * WorldSize - BaseSize,
				0.5 * WorldSize);
	return xml;
}

#include "ConfigFileManager.h"
#include "RegionReaderBase.h"
#include "RegionReaderFlagSpawn.h"
#include "RegionReaderObstacle.h"
#include <stdio.h>
static void parseWorld(std::istream& stream)
{
	// prep reader
	ConfigFileManager reader;
	reader.add("base", new RegionReaderBase);
	reader.add("flag-spawn", new RegionReaderFlagSpawn);
	reader.add("obstacle", new RegionReaderObstacle);

	// read
	try {
		reader.read(stream);
	}
	catch (XMLIOException& e) {
		fprintf(stderr, "{world config} (%d,%d): %s\n",
								e.position.line,
								e.position.column,
								e.what());
	}
}

#include "RegionManagerBase.h"
#include "RegionManagerFlagSpawn.h"
#include "RegionManagerObstacle.h"
void defineWorldRegions(const WorldInfo* world)
{
	std::string xml;
	xml += defineWalls(world->getWalls());
	xml += defineBoxes(world->getBoxes());
	xml += definePyramids(world->getPyramids());
	xml += defineTeleporters(world->getTeleporters());
	xml += defineBases();
	xml += defineFlagSpawn();

	// clear old stuff
	RGNMGR_BASE->clear();
	RGNMGR_FLAG_SPAWN->clear();
	RGNMGR_OBSTACLE->clear();

	// read world
	std::istringstream stream(xml.c_str());
	parseWorld(stream);
}
// ex: shiftwidth=4 tabstop=4
