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

#include "WorldInfo.h"
#include "global.h"

extern float basePos[NumTeams][3];
extern float baseRotation[NumTeams];
extern float baseSize[NumTeams][3];
extern float safetyBasePos[NumTeams][3];
extern bool randomBoxes;
extern bool randomHeights;
extern bool useTeleporters;

WorldInfo *defineTeamWorld()
{
	WorldInfo* world = new WorldInfo();
	if (!world)
		return NULL;

	// set team base and team flag safety positions
	basePos[0][0] = 0.0f;
	basePos[0][1] = 0.0f;
	basePos[0][2] = 0.0f;
	baseRotation[0] = 0.0f;
	baseSize[0][0] = 0.0f;
	baseSize[0][1] = 0.0f;
	basePos[1][0] = (-atof(BZDB->get("_worldSize").c_str()) + BaseSize) / 2.0f;
	basePos[1][1] = 0.0f;
	basePos[1][2] = 0.0f;
	baseRotation[1] = 0.0f;
	baseSize[1][0] = BaseSize / 2.0f;
	baseSize[1][1] = BaseSize / 2.0f;
	basePos[2][0] = (atof(BZDB->get("_worldSize").c_str()) - BaseSize) / 2.0f;
	basePos[2][1] = 0.0f;
	basePos[2][2] = 0.0f;
	baseRotation[2] = 0.0f;
	baseSize[2][0] = BaseSize / 2.0f;
	baseSize[2][1] = BaseSize / 2.0f;
	basePos[3][0] = 0.0f;
	basePos[3][1] = (-atof(BZDB->get("_worldSize").c_str()) + BaseSize) / 2.0f;
	basePos[3][2] = 0.0f;
	baseRotation[3] = 0.0f;
	baseSize[3][0] = BaseSize / 2.0f;
	baseSize[3][1] = BaseSize / 2.0f;
	basePos[4][0] = 0.0f;
	basePos[4][1] = (atof(BZDB->get("_worldSize").c_str()) - BaseSize) / 2.0f;
	basePos[4][2] = 0.0f;
	baseRotation[4] = 0.0f;
	baseSize[4][0] = BaseSize / 2.0f;
	baseSize[4][1] = BaseSize / 2.0f;
	safetyBasePos[0][0] = basePos[0][0];
	safetyBasePos[0][1] = basePos[0][1];
	safetyBasePos[0][2] = basePos[0][2];
	safetyBasePos[1][0] = basePos[1][0] + 0.5f * BaseSize + PyrBase;
	safetyBasePos[1][1] = basePos[1][1] + 0.5f * BaseSize + PyrBase;
	safetyBasePos[1][2] = basePos[1][2];
	safetyBasePos[2][0] = basePos[2][0] - 0.5f * BaseSize - PyrBase;
	safetyBasePos[2][1] = basePos[2][1] - 0.5f * BaseSize - PyrBase;
	safetyBasePos[2][2] = basePos[2][2];
	safetyBasePos[3][0] = basePos[3][0] - 0.5f * BaseSize - PyrBase;
	safetyBasePos[3][1] = basePos[3][1] + 0.5f * BaseSize + PyrBase;
	safetyBasePos[3][2] = basePos[3][2];
	safetyBasePos[4][0] = basePos[4][0] + 0.5f * BaseSize + PyrBase;
	safetyBasePos[4][1] = basePos[4][1] - 0.5f * BaseSize - PyrBase;
	safetyBasePos[4][2] = basePos[4][2];

	// make walls
	world->addWall(0.0f, 0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 1.5f * M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.0f, M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(0.0f, -0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.5f * M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(-0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.0f, 0.0f, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);

	// make pyramids
	// around red base
	world->addPyramid(
	basePos[1][0] + 0.5f * BaseSize - PyrBase,
	basePos[1][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[1][0] + 0.5f * BaseSize + PyrBase,
	basePos[1][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[1][0] + 0.5f * BaseSize + PyrBase,
	basePos[1][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[1][0] + 0.5f * BaseSize - PyrBase,
	basePos[1][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// around green base
	world->addPyramid(
	basePos[2][0] - 0.5f * BaseSize + PyrBase,
	basePos[2][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[2][0] - 0.5f * BaseSize - PyrBase,
	basePos[2][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[2][0] - 0.5f * BaseSize - PyrBase,
	basePos[2][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[2][0] - 0.5f * BaseSize + PyrBase,
	basePos[2][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// around blue base
	world->addPyramid(
	basePos[3][0] - 0.5f * BaseSize - PyrBase,
	basePos[3][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[3][0] - 0.5f * BaseSize + PyrBase,
	basePos[3][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[3][0] + 0.5f * BaseSize - PyrBase,
	basePos[3][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[3][0] + 0.5f * BaseSize + PyrBase,
	basePos[3][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// around purple base
	world->addPyramid(
	basePos[4][0] - 0.5f * BaseSize - PyrBase,
	basePos[4][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[4][0] - 0.5f * BaseSize + PyrBase,
	basePos[4][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[4][0] + 0.5f * BaseSize - PyrBase,
	basePos[4][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	basePos[4][0] + 0.5f * BaseSize + PyrBase,
	basePos[4][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// in center
	world->addPyramid(
	-(BoxBase + 0.25f * AvenueSize),
	-(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	(BoxBase + 0.25f * AvenueSize),
	-(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	-(BoxBase + 0.25f * AvenueSize),
	(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(
	(BoxBase + 0.25f * AvenueSize),
	(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(0.0f, -(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(0.0f,  (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(-(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid( (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// halfway out from city center
	world->addPyramid(0.0f, -(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(0.0f,  (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid(-(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);
	world->addPyramid( (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	PyrBase, PyrBase, PyrHeight);

	// add boxes, four at once with same height so no team has an advantage
	const float xmin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (CitySize - 1));
	const float ymin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (CitySize - 1));
	for (int j = 0; j <= CitySize/2; j++)
		for (int i = 0; i < CitySize/2; i++)
			if (i != CitySize/2 || j != CitySize/2) {
				float h = BoxHeight;
				if (randomHeights)
					h *= 2.0f * (float)bzfrand() + 0.5f;
				world->addBox(
						xmin + float(i) * (2.0f * BoxBase + AvenueSize),
						ymin + float(j) * (2.0f * BoxBase + AvenueSize), 0.0f,
						randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
						BoxBase, BoxBase, h);
				world->addBox(
						-1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)),
						-1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)), 0.0f,
						randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
						BoxBase, BoxBase, h);
				world->addBox(
						-1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)),
						xmin + float(i) * (2.0f * BoxBase + AvenueSize), 0.0f,
						randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
						BoxBase, BoxBase, h);
				world->addBox(
						ymin + float(j) * (2.0f * BoxBase + AvenueSize),
						-1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)), 0.0f,
						randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
						BoxBase, BoxBase, h);
			}

	// add teleporters
	if (useTeleporters) {
		const float xoff = BoxBase + 0.5f * AvenueSize;
		const float yoff = BoxBase + 0.5f * AvenueSize;
		world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, 1.25f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, 0.75f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, 1.75f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, 0.25f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter(-3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.25f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter(-3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.75f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter( 3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.75f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
		world->addTeleporter( 3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.25f * M_PI,
				0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);

		world->addLink(0, 14);
		world->addLink(1, 7);
		world->addLink(2, 12);
		world->addLink(3, 5);
		world->addLink(4, 10);
		world->addLink(5, 3);
		world->addLink(6, 8);
		world->addLink(7, 1);
		world->addLink(8, 6);
		world->addLink(9, 0);
		world->addLink(10, 4);
		world->addLink(11, 2);
		world->addLink(12, 2);
		world->addLink(13, 4);
		world->addLink(14, 0);
		world->addLink(15, 6);
	}

	return world;
}
// ex: shiftwidth=4 tabstop=4
