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
#include "Pack.h"
#include "global.h"
#include "Protocol.h"

//
// WorldInfo
//

WorldInfo::WorldInfo() : database(NULL), flagHeight(FlagAltitude)
{
	// do nothing
}

WorldInfo::~WorldInfo()
{
	delete[] database;
}

void WorldInfo::setFlagHeight(float height)
{
	//The minimum height above ground an object must be in order
	//to have a flag appear beneath it
	flagHeight = height;
}

void WorldInfo::addObstacle(ObstacleList& list,
								float x, float y, float z,
								float r, float w, float d, float h)
{
	list.resize(list.size() + 1);
	ObstacleLocation& o = list.back();
	o.pos[0]   = x;
	o.pos[1]   = y;
	o.pos[2]   = z;
	o.rotation = r;
	o.size[0]  = w;
	o.size[1]  = d;
	o.size[2]  = h;
}

void WorldInfo::addWall(float x, float y, float z, float r, float w, float h)
{
	// no depth to walls
	addObstacle(walls, x, y, z, r, w, 0.0f, h);
}

void WorldInfo::addBox(float x, float y, float z, float r, float w, float d, float h)
{
	addObstacle(boxes, x, y, z, r, w, d, h);
}

void WorldInfo::addPyramid(float x, float y, float z, float r, float w, float d, float h)
{
	addObstacle(pyramids, x, y, z, r, w, d, h);
}

void WorldInfo::addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b)
{
	int n = (int)teleporters.size();
	teleporters.resize(teleporters.size() + 1);
	Teleporter& o = teleporters.back();
	o.pos[0]   = x;
	o.pos[1]   = y;
	o.pos[2]   = z;
	o.rotation = r;
	o.size[0]  = w;
	o.size[1]  = d;
	o.size[2]  = h;
	o.border = b;
	// default link through
	o.to[0] = n * 2 + 1;
	o.to[1] = n * 2;
}

void WorldInfo::addBase(float x, float y, float z, float r, float w, float d, float h)
{
	addObstacle(bases, x, y, z, r, w, d, h);
}

void WorldInfo::addLink(int from, int to)
{
	// silently discard links from teleporters that don't exist
	if (from <= (int)teleporters.size() * 2 + 1) {
		teleporters[from / 2].to[from % 2] = to;
	}
}

bool WorldInfo::rectHitCirc(float dx, float dy, const float *p, float r) const
{
	// Algorithm from Graphics Gems, pp51-53.
	const float rr = r * r, rx = -p[0], ry = -p[1];
	if (rx + dx < 0.0f) // west of rect
		if (ry + dy < 0.0f) //  sw corner
			return (rx + dx) * (rx + dx) + (ry + dy) * (ry + dy) < rr;
		else if (ry - dy > 0.0f) //  nw corner
			return (rx + dx) * (rx + dx) + (ry - dy) * (ry - dy) < rr;
		else //  due west
			return rx + dx > -r;

	else if (rx - dx > 0.0f) // east of rect
		if (ry + dy < 0.0f) //  se corner
			return (rx - dx) * (rx - dx) + (ry + dy) * (ry + dy) < rr;
		else if (ry - dy > 0.0f) //  ne corner
			return (rx - dx) * (rx - dx) + (ry - dy) * (ry - dy) < rr;
		else //  due east
			return rx - dx < r;

	else if (ry + dy < 0.0f) // due south
		return ry + dy > -r;

	else if (ry - dy > 0.0f) // due north
		return ry - dy < r;

	// circle origin in rect
	return true;
}

bool WorldInfo::inRect(const float *p1, float angle, const float *size, float x, float y, float r) const
{
	// translate origin
	float pa[2];
	pa[0] = x - p1[0];
	pa[1] = y - p1[1];

	// rotate
	float pb[2];
	const float c = cosf(-angle), s = sinf(-angle);
	pb[0] = c * pa[0] - s * pa[1];
	pb[1] = c * pa[1] + s * pa[0];

	// do test
	return rectHitCirc(size[0], size[1], pb, r);
}

int WorldInfo::inBuilding(const WorldInfo::ObstacleLocation **location, float x, float y, float z, float r) const
{
	unsigned int i;
	for (i = 0; i < bases.size(); i++) {
		if ((inRect(bases[i].pos, bases[i].rotation, bases[i].size, x, y, r) && bases[i].pos[2] <
		(z + flagHeight)) && (bases[i].pos[2] + bases[i].size[2]) > z) {
			if(location != NULL)
				*location = &bases[i];
			return 1;
		}
	}
	for (i = 0; i < boxes.size(); i++)
		if ((inRect(boxes[i].pos, boxes[i].rotation, boxes[i].size, x, y, r) && boxes[i].pos[2] <
		(z + flagHeight)) && (boxes[i].pos[2] + boxes[i].size[2]) > z) {
			if (location != NULL)
				*location = &boxes[i];
			return 2;
		}
	for (i = 0; i < pyramids.size(); i++) {
		if ((inRect(pyramids[i].pos, pyramids[i].rotation, pyramids[i].size,x,y,r)) &&
		pyramids[i].pos[2] < (z + flagHeight) && (pyramids[i].pos[2] + pyramids[i].size[2]) > z) {
			if (location != NULL)
				*location = &pyramids[i];
			return 3;
		}
	}
	for (i = 0; i < teleporters.size(); i++)
		if (inRect(teleporters[i].pos, teleporters[i].rotation, teleporters[i].size, x, y, r) &&
		teleporters[i].pos[2] < (z + flagHeight) &&
		(teleporters[i].pos[2] + teleporters[i].size[2]) > z) {
			if (location != NULL)
				*location = &teleporters[i];
			return 4;
		}
	if (location != NULL)
		*location = (ObstacleLocation *)NULL;
	return 0;
}

int WorldInfo::packDatabase()
{
	databaseSize = (2 + 6 * 4) * walls.size() +
				(2 + 7 * 4) * boxes.size() +
				(2 + 7 * 4) * pyramids.size() +
				(2 + 8 * 4) * teleporters.size() +
				(2 + 4) * 2 * teleporters.size();
	database = new char[databaseSize];
	void *databasePtr = database;

	// add walls
	ObstacleList::iterator index1;
	for (index1 = walls.begin(); index1 != walls.end(); ++index1) {
		databasePtr = nboPackUShort(databasePtr, WorldCodeWall);
		databasePtr = nboPackVector(databasePtr, index1->pos);
		databasePtr = nboPackFloat(databasePtr, index1->rotation);
		databasePtr = nboPackFloat(databasePtr, index1->size[0]);
		// walls have no depth
		// databasePtr = nboPackFloat(databasePtr, index1->size[1]);
		databasePtr = nboPackFloat(databasePtr, index1->size[2]);
	}

	// add boxes
	for (index1 = boxes.begin(); index1 != boxes.end(); ++index1) {
		databasePtr = nboPackUShort(databasePtr, WorldCodeBox);
		databasePtr = nboPackVector(databasePtr, index1->pos);
		databasePtr = nboPackFloat(databasePtr, index1->rotation);
		databasePtr = nboPackVector(databasePtr, index1->size);
	}

	// add pyramids
	for (index1 = pyramids.begin(); index1 != pyramids.end(); ++index1) {
		databasePtr = nboPackUShort(databasePtr, WorldCodePyramid);
		databasePtr = nboPackVector(databasePtr, index1->pos);
		databasePtr = nboPackFloat(databasePtr, index1->rotation);
		databasePtr = nboPackVector(databasePtr, index1->size);
	}

	// add teleporters
	TeleporterList::iterator index2;
	for (index2 = teleporters.begin(); index2 != teleporters.end(); ++index2) {
		databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporter);
		databasePtr = nboPackVector(databasePtr, index2->pos);
		databasePtr = nboPackFloat(databasePtr, index2->rotation);
		databasePtr = nboPackVector(databasePtr, index2->size);
		databasePtr = nboPackFloat(databasePtr, index2->border);
		// and each link
		unsigned int i = index2 - teleporters.begin();
		databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
		databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2));
		databasePtr = nboPackUShort(databasePtr, uint16_t(index2->to[0]));
		databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
		databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2 + 1));
		databasePtr = nboPackUShort(databasePtr, uint16_t(index2->to[1]));
	}
	return 1;
}

void *WorldInfo::getDatabase() const
{
	return database;
}

int WorldInfo::getDatabaseSize() const
{
	return databaseSize;
}
