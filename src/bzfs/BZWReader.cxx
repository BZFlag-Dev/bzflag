/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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
#include <fstream>
#include "BZWReader.h"
#include "CmdLineOptions.h"
#include "WorldFileObject.h"
#include "CustomBox.h"
#include "CustomPyramid.h"
#include "CustomGate.h"
#include "CustomLink.h"
#include "CustomBase.h"
#include "CustomWeapon.h"
#include "CustomWorld.h"
#include "CustomZone.h"

extern CmdLineOptions *clOptions;
extern BasesList bases;

std::istream &readToken(std::istream& input, char *buffer, int n)
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


bool readWorldStream(std::istream& input, const char *location, std::vector<WorldFileObject*>& wlist)
{
  int line = 1;
  char buffer[1024];
  WorldFileObject *object    = NULL;
  WorldFileObject *newObject = NULL;
  bool gotWorld = false;

  while (!input.eof())
  {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	std::cout << location << '(' << line << ") : discarding incomplete object\n";
	delete object;
      }
      object = newObject;
      newObject = NULL;
    }

    // read first token but do not skip newlines
    readToken(input, buffer, sizeof(buffer));
    if (strcmp(buffer, "") == 0) {
      // ignore blank line
    } else if (buffer[0] == '#') {
      // ignore comment
    } else if (strcasecmp(buffer, "end") == 0) {
      if (object) {
	wlist.push_back(object);
	object = NULL;
      } else {
	std::cout << location << '(' << line << ") : unexpected \"end\" token\n";
	return false;
      }
    } else if (strcasecmp(buffer, "box") == 0) {
      newObject = new CustomBox;
    } else if (strcasecmp(buffer, "pyramid") == 0) {
      newObject = new CustomPyramid();
    } else if (strcasecmp(buffer, "teleporter") == 0) {
      newObject = new CustomGate();
    } else if (strcasecmp(buffer, "link") == 0) {
      newObject = new CustomLink();
    } else if (strcasecmp(buffer, "base") == 0) {
      newObject = new CustomBase;
    } else if (strcasecmp(buffer, "weapon") == 0) {
      newObject = new CustomWeapon;
    } else if (strcasecmp(buffer, "zone") == 0) {
      newObject = new CustomZone;
    } else if (strcasecmp(buffer, "world") == 0) {
      if (!gotWorld) {
	newObject = new CustomWorld();
	gotWorld = true;
      }
    } else if (object) {
      if (!object->read(buffer, input)) {
	// unknown token
	std::cout << location << '(' << line << ") : unknown object parameter \"" << buffer << "\" - skipping\n";
	// delete object;
	// return false;
      }
    } else { // filling the current object
      // unknown token
      std::cout << location << '(' << line << ") : invalid object type \"" << buffer << "\" - skipping\n";
      delete object;
     // return false;
    }

    // discard remainder of line
    while (input.good() && input.peek() != '\n')
      input.get(buffer, sizeof(buffer));
    input.getline(buffer, sizeof(buffer));
    ++line;
  }

  if (object) {
    std::cout << location << '(' << line << ") : missing \"end\" token\n";
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
    std::cout << "could not find bzflag world file : " << filename << std::endl;
    return NULL;
  }

  // create world object
  WorldInfo *world = new WorldInfo;
  if (!world)
    return NULL;

  // read file
  std::vector<WorldFileObject*> list;
  if (!readWorldStream(input, filename, list)) {
    emptyWorldFileObjectList(list);
    delete world;
    return NULL;
  }

  // make walls
  float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  world->addWall(0.0f, 0.5f * worldSize, 0.0f, 1.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.5f * worldSize, 0.0f, 0.0f, M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.0f, -0.5f * worldSize, 0.0f, 0.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

  // add objects
  const int n = list.size();
  for (int i = 0; i < n; ++i)
    list[i]->write(world);

  if (clOptions->gameStyle & TeamFlagGameStyle) {
    for (int i = RedTeam; i <= PurpleTeam; i++) {
      if ((clOptions->maxTeam[i] > 0) && bases.find(i) == bases.end()) {
	std::cout << "base was not defined for team " << i << ", capture the flag game style removed.\n";
	clOptions->gameStyle &= (~TeamFlagGameStyle);
	break;
      }
    }
  }

  // clean up
  emptyWorldFileObjectList(list);
  world->finishWorld();
  return world;
}
