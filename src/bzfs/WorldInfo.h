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

#ifndef BZF_WORLD_INFO_H
#define BZF_WORLD_INFO_H

#include "common.h"
#include <vector>

class WorldInfo {
public:
	WorldInfo();
	~WorldInfo();

	void setFlagHeight(float);
	void addWall(float x, float y, float z, float r, float w, float h);
	void addBox(float x, float y, float z, float r, float w, float d, float h);
	void addPyramid(float x, float y, float z, float r, float w, float d, float h);
	void addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b);
	void addBase(float x, float y, float z, float r, float w, float d, float h);
	void addLink(int from, int to);
	int packDatabase();
	void *getDatabase() const;
	int getDatabaseSize() const;

	struct ObstacleLocation {
		public:
			float pos[3];
			float rotation;
			float size[3];
	};

	struct Teleporter : public ObstacleLocation {
		public:
			float border;
			int to[2];
	};

public:
	typedef std::vector<ObstacleLocation> ObstacleList;
	typedef std::vector<Teleporter> TeleporterList;
	const ObstacleList&	getWalls() const { return walls; }
	const ObstacleList& getBoxes() const { return boxes; }
	const ObstacleList& getBases() const { return bases; }
	const ObstacleList& getPyramids() const { return pyramids; }
	const TeleporterList& getTeleporters() const { return teleporters; }

	int inBuilding(const ObstacleLocation **location, float x, float y, float z, float radius) const;
private:
	void addObstacle(ObstacleList&, float x, float y, float z,
									float r, float w, float d, float h);
	bool inRect(const float *p1, float angle, const float *size, float x, float y, float radius) const;
	bool rectHitCirc(float dx, float dy, const float *p, float r) const;

private:
	ObstacleList	walls;
	ObstacleList	boxes;
	ObstacleList	bases;
	ObstacleList	pyramids;
	TeleporterList	teleporters;
	char *database;
	int databaseSize;
	float			flagHeight;
};

#endif
// ex: shiftwidth=4 tabstop=4
