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
#include <string.h>
#include <iostream>
#include <fstream>
#include "WorldFile.h"
#include "WorldInfo.h"
#include "global.h"

extern float basePos[NumTeams][3];
extern float baseRotation[NumTeams];
extern float baseSize[NumTeams][3];
extern float safetyBasePos[NumTeams][3];

//
// types for reading world files
//

class WorldFileObject {
	public:
		WorldFileObject() { }
		virtual ~WorldFileObject() { }

		virtual bool read(const char *cmd, std::istream&) = 0;
		virtual void write(WorldInfo*) const = 0;
};

class WorldFileObstacle : public WorldFileObject {
	public:
		WorldFileObstacle();
		virtual bool read(const char *cmd, std::istream&);

	protected:
		float pos[3];
		float rotation;
		float size[3];
};

WorldFileObstacle::WorldFileObstacle()
{
	pos[0] = pos[1] = pos[2] = 0.0f;
	rotation = 0.0f;
	size[0] = size[1] = size[2] = 1.0f;
}

bool WorldFileObstacle::read(const char *cmd, std::istream& input)
{
	if (strcmp(cmd, "position") == 0)
		input >> pos[0] >> pos[1] >> pos[2];
	else if (strcmp(cmd, "rotation") == 0) {
		input >> rotation;
		rotation = rotation * M_PI / 180.0f;
	} else if (strcmp(cmd, "size") == 0)
		input >> size[0] >> size[1] >> size[2];
	else
		return false;
	return true;
}

class CustomBox : public WorldFileObstacle {
	public:
		CustomBox();
		virtual void write(WorldInfo*) const;
};

CustomBox::CustomBox()
{
	size[0] = size[1] = BoxBase;
	size[2] = BoxHeight;
}

void CustomBox::write(WorldInfo *world) const
{
	world->addBox(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2]);
}

class CustomPyramid : public WorldFileObstacle {
	public:
		CustomPyramid();
		virtual void write(WorldInfo*) const;
};

CustomPyramid::CustomPyramid()
{
	size[0] = size[1] = PyrBase;
	size[2] = PyrHeight;
}

void CustomPyramid::write(WorldInfo *world) const
{
	world->addPyramid(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2]);
}

class CustomGate : public WorldFileObstacle {
	public:
		CustomGate();
		virtual bool read(const char *cmd, std::istream&);
		virtual void write(WorldInfo*) const;

	protected:
		float border;
};

CustomGate::CustomGate()
{
	size[0] = 0.5f * TeleWidth;
	size[1] = TeleBreadth;
	size[2] = 2.0f * TeleHeight;
	border = TeleWidth;
}

bool CustomGate::read(const char *cmd, std::istream& input)
{
	if (strcmp(cmd, "border") == 0)
		input >> border;
	else
		return WorldFileObstacle::read(cmd, input);
	return true;
}

void CustomGate::write(WorldInfo *world) const
{
	world->addTeleporter(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2], border);
}

class CustomLink : public WorldFileObject {
	public:
		CustomLink();
		virtual bool read(const char *cmd, std::istream&);
		virtual void write(WorldInfo*) const;

	protected:
		int from;
		int to;
};

CustomLink::CustomLink()
{
	from = 0;
	to = 0;
}

bool CustomLink::read(const char *cmd, std::istream& input)
{
	if (strcmp(cmd, "from") == 0)
		input >> from;
	else if (strcmp(cmd, "to") == 0)
		input >> to;
	else
		return false;
	return true;
}

void CustomLink::write(WorldInfo *world) const
{
	world->addLink(from, to);
}

class CustomBase : public WorldFileObstacle {
	public:
		CustomBase();
		virtual bool read(const char *cmd, std::istream&);
		virtual void write(WorldInfo*) const;

	protected:
		int color;
};

CustomBase::CustomBase()
{
	pos[0] = pos[1] = pos[2] = 0.0f;
	rotation = 0.0f;
	size[0] = size[1] = BaseSize;
}

bool CustomBase::read(const char *cmd, std::istream& input)
{
	if (strcmp(cmd, "color") == 0)
		input >> color;
	else {
		WorldFileObstacle::read(cmd, input);
	}
	return true;
}

void CustomBase::write(WorldInfo* world) const
{
	basePos[color][0] = pos[0];
	basePos[color][1] = pos[1];
	basePos[color][2] = pos[2];
	baseRotation[color] = rotation;
	baseSize[color][0] = size[0];
	baseSize[color][1] = size[1];
	baseSize[color][2] = size[2];
	safetyBasePos[color][0] = 0;
	safetyBasePos[color][1] = 0;
	safetyBasePos[color][2] = 0;
	world->addBase(pos[0], pos[1], pos[2], rotation, size[0], size[1], (pos[2] > 0.0) ? 1.0f : 0.0f);
}

class CustomWorld : public WorldFileObject {
	public:
		CustomWorld();
		virtual bool read(const char *cmd, std::istream&);
		virtual void write(WorldInfo*) const;

	protected:
		int size;
		int fHeight;
};

CustomWorld::CustomWorld()
{
	size = 800;
	fHeight = 0;
}

bool CustomWorld::read(const char *cmd, std::istream& input)
{
	if (strcmp(cmd, "size") == 0)
		input >> size;
	else if (strcmp(cmd, "flagHeight") == 0)
		input >> fHeight;
	else
		return false;
	return true;
}

void CustomWorld::write(WorldInfo * world) const
{
	world->setFlagHeight(fHeight);
	//world->addLink(from, to);
}

// list of world file objects
typedef std::vector<WorldFileObject*> WorldFileObjectList;

//
// global functions
//

static void emptyWorldFileObjectList(WorldFileObjectList& list)
{
	const int n = (int)list.size();
	for (int i = 0; i < n; ++i)
		delete list[i];
	list.clear();
}

static std::istream &readToken(std::istream& input, char *buffer, int n)
{
	int c = -1;

	// skip whitespace
	while (input.good() && (c = input.get()) != -1 && isspace(c) && c != '\n')
		;

	// read up to whitespace or n - 1 characters into buffer
	int i = 0;
	if (c != -1 && c != '\n') {
		buffer[i++] = c;
		while (input.good() && i < n - 1 && (c = input.get()) != -1 && !isspace(c))
			buffer[i++] = (char)c;
	}

	// terminate string
	buffer[i] = 0;

	// put back last character we didn't use
	if (c != -1 && isspace(c))
		input.putback(c);

	return input;
}

static bool readWorldStream(std::istream& input, const char *location, WorldFileObjectList& list)
{
	int line = 1;
	char buffer[1024];
	WorldFileObject *object    = NULL;
	WorldFileObject *newObject = NULL;
	while (!input.eof())
	{
		// watch out for starting a new object when one is already in progress
		if (newObject) {
			if (object) {
				std::cerr << location << "(" << line << ") : " << "discarding incomplete object" << std::endl;
				delete object;
			}
			object = newObject;
			newObject = NULL;
		}

		// read first token but do not skip newlines
		readToken(input, buffer, sizeof(buffer));
		if (strcmp(buffer, "") == 0) {
			// ignore blank line
		}

		else if (buffer[0] == '#') {
			// ignore comment
		}

		else if (strcmp(buffer, "end") == 0) {
			if (object) {
				list.push_back(object);
				object = NULL;
			}
			else {
				std::cerr << location << "(" << line << ") : " << "unexpected \"end\" token" << std::endl;
				return false;
			}
		}

		else if (strcmp(buffer, "box") == 0)
			newObject = new CustomBox;

		else if (strcmp(buffer, "pyramid") == 0)
			newObject = new CustomPyramid();

		else if (strcmp(buffer, "teleporter") == 0)
			newObject = new CustomGate();

		else if (strcmp(buffer, "link") == 0)
			newObject = new CustomLink();

		else if (strcmp(buffer, "base") == 0)
			newObject = new CustomBase;

		// FIXME - only load one object of the type CustomWorld!
		else if (strcmp(buffer, "world") == 0)
			newObject = new CustomWorld();

		else if (object) {
			if (!object->read(buffer, input)) {
				// unknown token
				std::cerr << location << "(" << line << ") : " <<
						"invalid object parameter \"" << buffer << "\"" << std::endl;
				delete object;
				return false;
			}
		}

		// filling the current object
		else {
			// unknown token
			std::cerr << location << "(" << line << ") : " << "invalid object type \"" << buffer << "\"" << std::endl;
			delete object;
			return false;
		}

		// discard remainder of line
		while (input.good() && input.peek() != '\n')
			input.get(buffer, sizeof(buffer));
		input.getline(buffer, sizeof(buffer));
		++line;
	}

	if (object) {
		std::cerr << location << "(" << line << ") : " << "missing \"end\" token" << std::endl;
		delete object;
		return false;
	}

	return true;
}

WorldInfo *defineWorldFromFile(const char *filename)
{
	// open file
	std::ifstream input(filename, std::ios::in);
	if (!input) {
		std::cerr << "could not find bzflag world file : " << filename << std::endl;
		return NULL;
	}

	// create world object
	WorldInfo* world = new WorldInfo;
	if (!world)
		return NULL;

	// read file
	WorldFileObjectList list;
	if (!readWorldStream(input, filename, list)) {
		emptyWorldFileObjectList(list);
		delete world;
		return NULL;
	}

	// make walls
	world->addWall(0.0f, 0.5f * atof(BZDB->get("worldSize").c_str()), 0.0f, 1.5f * M_PI, 0.5f * atof(BZDB->get("worldSize").c_str()), WallHeight);
	world->addWall(0.5f * atof(BZDB->get("worldSize").c_str()), 0.0f, 0.0f, M_PI, 0.5f * atof(BZDB->get("worldSize").c_str()), WallHeight);
	world->addWall(0.0f, -0.5f * atof(BZDB->get("worldSize").c_str()), 0.0f, 0.5f * M_PI, 0.5f * atof(BZDB->get("worldSize").c_str()), WallHeight);
	world->addWall(-0.5f * atof(BZDB->get("worldSize").c_str()), 0.0f, 0.0f, 0.0f, 0.5f * atof(BZDB->get("worldSize").c_str()), WallHeight);

	// add objects
	const int n = (int)list.size();
	for (int i = 0; i < n; ++i)
		list[i]->write(world);

	// clean up
	emptyWorldFileObjectList(list);
	return world;
}
// ex: shiftwidth=4 tabstop=4
