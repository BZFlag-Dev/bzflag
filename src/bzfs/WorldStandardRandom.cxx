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

extern bool randomHeights;
extern bool useTeleporters;

WorldInfo *defineRandomWorld()
{
	const int numTeleporters = 8 + int(8 * (float)bzfrand());
	WorldInfo* world = new WorldInfo();
	if (!world)
		return NULL;

	// make walls
	world->addWall(0.0f, 0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 1.5f * M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.0f, M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(0.0f, -0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.5f * M_PI, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);
	world->addWall(-0.5f * atof(BZDB->get("_worldSize").c_str()), 0.0f, 0.0f, 0.0f, 0.5f * atof(BZDB->get("_worldSize").c_str()), WallHeight);

	// make boxes
	int i;
	float h = BoxHeight;
	for (i = 0; i < CitySize * CitySize; i++) {
		if (randomHeights)
			h = BoxHeight * ( 2.0f * (float)bzfrand() + 0.5f );
		world->addBox(atof(BZDB->get("_worldSize").c_str()) * ((float)bzfrand() - 0.5f),
				atof(BZDB->get("_worldSize").c_str()) * ((float)bzfrand() - 0.5f),
				0.0f, 2.0f * M_PI * (float)bzfrand(),
				BoxBase, BoxBase, h);
	}

	// make pyramids
	h = PyrHeight;
	for (i = 0; i < CitySize * CitySize; i++) {
		if (randomHeights)
			h = PyrHeight * ( 2.0f * (float)bzfrand() + 0.5f);
		world->addPyramid(atof(BZDB->get("_worldSize").c_str()) * ((float)bzfrand() - 0.5f),
				atof(BZDB->get("_worldSize").c_str()) * ((float)bzfrand() - 0.5f),
				0.0f, 2.0f * M_PI * (float)bzfrand(),
				PyrBase, PyrBase, h);
	}

	if (useTeleporters) {
		// make teleporters
		int (*linked)[2] = (int(*)[2])new int[2 * numTeleporters];
		for (i = 0; i < numTeleporters;) {
			const float x = (atof(BZDB->get("_worldSize").c_str()) - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
			const float y = (atof(BZDB->get("_worldSize").c_str()) - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
			const float rotation = 2.0f * M_PI * (float)bzfrand();

			// if too close to building then try again
			if (world->inBuilding(NULL, x, y, 0, 1.75f * TeleBreadth))
				continue;

			world->addTeleporter(x, y, 0.0f, rotation,
					0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight, TeleWidth);
			linked[i][0] = linked[i][1] = 0;
			i++;
		}

		// make teleporter links
		int numUnlinked = 2 * numTeleporters;
		for (i = 0; i < numTeleporters; i++)
			for (int j = 0; j < 2; j++) {
				int a = (int)(numUnlinked * (float)bzfrand());
				if (linked[i][j])
					continue;
				for (int k = 0, i2 = i; i2 < numTeleporters; ++i2)
					for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
						if (linked[i2][j2])
							continue;
						if (k++ == a) {
							world->addLink(2 * i + j, 2 * i2 + j2);
							linked[i][j] = 1;
							numUnlinked--;
							if (i != i2 || j != j2) {
								world->addLink(2 * i2 + j2, 2 * i + j);
								linked[i2][j2] = 1;
								numUnlinked--;
							}
						}
					}
			}
		delete[] linked;
	}

	return world;
}
// ex: shiftwidth=4 tabstop=4
