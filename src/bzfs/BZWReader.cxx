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

/* bzflag special common - 1st one */
#include "common.h"

// interface header
#include "BZWReader.h"

// implementation-specific system headers
#include <fstream>
#include <string>
#include <vector>

// implementation-specific bzflag headers
#include "Team.h"
#include "URLManager.h"

// implementation-specific bzfs-specific headers
#include "BZWError.h"
#include "CmdLineOptions.h"
#include "TeamBases.h"
#include "WorldFileObject.h"
#include "CustomBox.h"
#include "CustomPyramid.h"
#include "CustomGate.h"
#include "CustomLink.h"
#include "CustomBase.h"
#include "CustomWeapon.h"
#include "CustomWorld.h"
#include "CustomZone.h"
#include "CustomTetra.h"
#include "CustomDynamicColor.h"
#include "CustomTextureMatrix.h"
#include "CustomMesh.h"
#include "CustomArc.h"
#include "CustomCone.h"
#include "CustomSphere.h"
#include "CustomWaterLevel.h"

extern CmdLineOptions *clOptions;
extern BasesList bases;

static const std::string urlProtocol("http://");

BZWReader::BZWReader(std::string filename) : location(filename), input(NULL)
{
  errorHandler = new BZWError(location);
  if (filename.compare(0, urlProtocol.size(), urlProtocol)) {
    input = new std::ifstream(filename.c_str(), std::ios::in);
  } else {
    URLManager::instance().getURL(location, httpData);
    input = new std::istringstream(httpData);
  }

  if (input->peek() == EOF) {
    errorHandler->fatalError(std::string("could not find bzflag world file"), 0);
  }
}

BZWReader::~BZWReader()
{
  // clean up
  delete errorHandler;
  delete input;
}

void BZWReader::readToken(char *buffer, int n)
{
  int c = -1;

  // skip whitespace
  while (input->good() && (c = input->get()) != -1 && isspace(c) && c != '\n')
    ;

  // read up to whitespace or n - 1 characters into buffer
  int i = 0;
  if (c != -1 && c != '\n') {
    buffer[i++] = c;
    while (input->good() && i < n - 1 && (c = input->get()) != -1 && !isspace(c))
      buffer[i++] = (char)c;
  }

  // terminate string
  buffer[i] = 0;

  // put back last character we didn't use
  if (c != -1 && isspace(c))
    input->putback(c);
}


bool BZWReader::readWorldStream(std::vector<WorldFileObject*>& wlist)
{
  int line = 1;
  char buffer[1024];
  WorldFileObject *object    = NULL;
  WorldFileObject *newObject = NULL;
  WorldFileObject * const fakeObject = (WorldFileObject*)((char*)NULL + 1); // for options

  bool gotWorld = false;

  while (!input->eof() && !input->fail() && input->good())
  {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	errorHandler->warning(std::string("discarding incomplete object"), line);
	if (object != fakeObject)
	  delete object;
      }
      object = newObject;
      newObject = NULL;
    }

    // read first token but do not skip newlines
    readToken(buffer, sizeof(buffer));
    if (strcmp(buffer, "") == 0) {
      // ignore blank line
    } else if (buffer[0] == '#') {
      // ignore comment
    } else if (strcasecmp(buffer, "end") == 0) {
      if (object) {
        if (object != fakeObject) {
	  wlist.push_back(object);
	}
	object = NULL;
      } else {
	errorHandler->fatalError(std::string("unexpected \"end\" token"), line);
	return false;
      }
    } else if (strcasecmp(buffer, "box") == 0) {
      newObject = new CustomBox;
    } else if (strcasecmp(buffer, "pyramid") == 0) {
      newObject = new CustomPyramid();
    } else if (strcasecmp(buffer, "tetra") == 0) {
      newObject = new CustomTetra();
    } else if (strcasecmp(buffer, "teleporter") == 0) {
      newObject = new CustomGate();
    } else if (strcasecmp(buffer, "link") == 0) {
      newObject = new CustomLink();
    } else if (strcasecmp(buffer, "base") == 0) {
      newObject = new CustomBase;
    } else if (strcasecmp(buffer, "mesh") == 0) {
      newObject = new CustomMesh;
    } else if (strcasecmp(buffer, "arc") == 0) {
      newObject = new CustomArc;
    } else if (strcasecmp(buffer, "cone") == 0) {
      newObject = new CustomCone;
    } else if (strcasecmp(buffer, "sphere") == 0) {
      newObject = new CustomSphere;
    } else if (strcasecmp(buffer, "weapon") == 0) {
      newObject = new CustomWeapon;
    } else if (strcasecmp(buffer, "zone") == 0) {
      newObject = new CustomZone;
    } else if (strcasecmp(buffer, "world") == 0) {
      if (!gotWorld) {
	newObject = new CustomWorld();
	gotWorld = true;
      } else {
	errorHandler->warning(std::string("multiple \"world\" sections found"), line);
      }
    } else if (strcasecmp(buffer, "waterLevel") == 0) {
      newObject = new CustomWaterLevel;
    } else if (strcasecmp(buffer, "dynamicColor") == 0) {
      newObject = new CustomDynamicColor;
    } else if (strcasecmp(buffer, "textureMatrix") == 0) {
      newObject = new CustomTextureMatrix;
    } else if (strcasecmp(buffer, "options") == 0) {
      newObject = fakeObject;
    } else if (object) {
      if (object != fakeObject) {
        if (!object->read(buffer, *input)) {
	  // unknown token
	  errorHandler->warning(std::string("unknown object parameter \"") + std::string(buffer) + std::string("\" - skipping"), line);
	  // delete object;
	  // return false;
	}
      }
    } else { // filling the current object
      // unknown token
      errorHandler->warning(std::string("invalid object type \"") + std::string(buffer) + std::string("\" - skipping"), line);
      if (object != fakeObject)
        delete object;
     // return false;
    }

    // discard remainder of line
    while (input->good() && input->peek() != '\n')
      input->get(buffer, sizeof(buffer));
    input->getline(buffer, sizeof(buffer));
    ++line;
  }

  if (object) {
    errorHandler->fatalError(std::string("missing \"end\" parameter"), line);
    if (object != fakeObject)
      delete object;
    return false;
  }

  return true;
}

WorldInfo* BZWReader::defineWorldFromFile()
{
  // make sure input is valid
  if (input->peek() == EOF) {
    errorHandler->fatalError(std::string("unexpected EOF"), 0);
    return NULL;
  }

  // create world object
  WorldInfo *world = new WorldInfo;
  if (!world) {
    errorHandler->fatalError(std::string("WorldInfo failed to initialize"), 0);
    return NULL;
  }

  // read file
  std::vector<WorldFileObject*> list;
  if (!readWorldStream(list)) {
    emptyWorldFileObjectList(list);
    errorHandler->fatalError(std::string("world file failed to load."), 0);
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
	errorHandler->warning(std::string("base was not defined for ") + Team::getName((TeamColor)i) + std::string(", capture the flag game style removed."), 0);
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

