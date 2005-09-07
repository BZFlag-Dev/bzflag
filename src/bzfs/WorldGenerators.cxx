/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// 
// WorldGenerators
//   Routines to build Random and Team worlds
//

#include "common.h"

// implementation header
#include "WorldGenerators.h"

// system headers
#include <math.h>

// common headers
#include "WorldInfo.h"
#include "ObstacleMgr.h"
#include "StateDatabase.h" 
#include "BZDBCache.h" 

// local headers
#include "TeamBases.h" 
#include "CustomZone.h" 
#include "CmdLineOptions.h" 

// externs
extern CmdLineOptions *clOptions;
extern BasesList bases;


WorldInfo *defineRandomWorld()
{
  WorldInfo* world = new WorldInfo();
  if (!world)
    return NULL;

  // make walls
  float worldSize = BZDBCache::worldSize;
  float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  world->addWall(0.0f, 0.5f * worldSize, 0.0f, (float)(1.5 * M_PI), 0.5f * worldSize, wallHeight);
  world->addWall(0.5f * worldSize, 0.0f, 0.0f, (float)M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.0f, -0.5f * worldSize, 0.0f, (float)(0.5 * M_PI), 0.5f * worldSize, wallHeight);
  world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

  float worldfactor = worldSize / (float)DEFAULT_WORLD;
  int actCitySize = int(clOptions->citySize * worldfactor + 0.5f);
  int numTeleporters = 8 + int(8 * (float)bzfrand() * worldfactor);
  float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);
  // make boxes
  int i;
  float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
  float h = boxHeight;
  const int numBoxes = int((0.5f + 0.7f * bzfrand()) * actCitySize * actCitySize);
  for (i = 0; i < numBoxes; i++) {
    if (clOptions->randomHeights)
      h = boxHeight * ( 2.0f * (float)bzfrand() + 0.5f);
    world->addBox(worldSize * ((float)bzfrand() - 0.5f),
	worldSize * ((float)bzfrand() - 0.5f),
	0.0f, (float)(2.0 * M_PI * bzfrand()),
	boxBase, boxBase, h);
  }

  // make pyramids
  float pyrHeight = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
  float pyrBase = BZDB.eval(StateDatabase::BZDB_PYRBASE);
  h = pyrHeight;
  const int numPyrs = int((0.5f + 0.7f * bzfrand()) * actCitySize * actCitySize);
  for (i = 0; i < numPyrs; i++) {
    if (clOptions->randomHeights)
      h = pyrHeight * ( 2.0f * (float)bzfrand() + 0.5f);
    world->addPyramid(worldSize * ((float)bzfrand() - 0.5f),
	worldSize * ((float)bzfrand() - 0.5f),
	0.0f, (float)(2.0 * M_PI * bzfrand()),
	pyrBase, pyrBase, h);
  }

  if (clOptions->useTeleporters) {
    // make teleporters
    float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
    float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
    float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
    int (*linked)[2] = new int[numTeleporters][2];
    for (i = 0; i < numTeleporters;) {
      const float x = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
      const float y = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
      const float rotation = (float)(2.0 * M_PI * bzfrand());

      // if too close to building then try again
	  Obstacle* obs;
      if (NOT_IN_BUILDING != world->inCylinderNoOctree(&obs, x, y, 0,
					       1.75f * teleBreadth, 1.0f))
	continue;

      world->addTeleporter(x, y, 0.0f, rotation,
	  0.5f*teleWidth, teleBreadth, 2.0f*teleHeight, teleWidth, false);
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

  OBSTACLEMGR.makeWorld();
  world->finishWorld();

  return world;
}


WorldInfo *defineTeamWorld()
{
  WorldInfo* world = new WorldInfo();
  if (!world)
    return NULL;

  const float worldSize = BZDBCache::worldSize;
  const float worldfactor = worldSize / (float)DEFAULT_WORLD;
  const int actCitySize = int(clOptions->citySize * worldfactor + 0.5f);
  const float pyrBase = BZDB.eval(StateDatabase::BZDB_PYRBASE);

  // set team base and team flag safety positions
  int t;
  for (t = RedTeam; t <= PurpleTeam; t++)
    bases[t] = TeamBases((TeamColor)t, true);

  const bool haveRed    = clOptions->maxTeam[RedTeam] > 0;
  const bool haveGreen  = clOptions->maxTeam[GreenTeam] > 0;
  const bool haveBlue   = clOptions->maxTeam[BlueTeam] > 0;
  const bool havePurple = clOptions->maxTeam[PurpleTeam] > 0;

  // make walls
  const float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  world->addWall(0.0f, 0.5f * worldSize, 0.0f, (float)(1.5 * M_PI), 0.5f * worldSize, wallHeight);
  world->addWall(0.5f * worldSize, 0.0f, 0.0f, (float)M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.0f, -0.5f * worldSize, 0.0f, (float)(0.5 * M_PI), 0.5f * worldSize, wallHeight);
  world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

  const float pyrHeight = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
  const float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
  // make pyramids
  if (haveRed) {
    // around red base
    const float *pos = bases[RedTeam].getBasePosition(0);
    world->addPyramid(
	pos[0] + 0.5f * baseSize - pyrBase,
	pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize + pyrBase,
	pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize + pyrBase,
	pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize - pyrBase,
	pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
  }

  if (haveGreen) {
    // around green base
    const float *pos = bases[GreenTeam].getBasePosition(0);
    world->addPyramid(
	pos[0] - 0.5f * baseSize + pyrBase,
	pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] - 0.5f * baseSize - pyrBase,
	pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] - 0.5f * baseSize - pyrBase,
	pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] - 0.5f * baseSize + pyrBase,
	pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
  }

  if (haveBlue) {
    // around blue base
    const float *pos = bases[BlueTeam].getBasePosition(0);
    world->addPyramid(
	pos[0] - 0.5f * baseSize - pyrBase,
	pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] - 0.5f * baseSize + pyrBase,
	pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize - pyrBase,
	pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize + pyrBase,
	pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
  }

  if (havePurple) {
    // around purple base
    const float *pos = bases[PurpleTeam].getBasePosition(0);
    world->addPyramid(
	pos[0] - 0.5f * baseSize - pyrBase,
	pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] - 0.5f * baseSize + pyrBase,
	pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize - pyrBase,
	pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	pos[0] + 0.5f * baseSize + pyrBase,
	pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
  }

  // create symmetric map of random buildings for random CTF mode
  if (clOptions->randomCTF) {
    int i;
    float h = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
    const bool redGreen = haveRed || haveGreen;
    const bool bluePurple = haveBlue || havePurple;
    if (!redGreen && !bluePurple) {
      std::cerr << "need some teams, use -mp\n";
      exit(20);
    }
    const float *redPosition = bases[RedTeam].getBasePosition(0);
    const float *greenPosition = bases[GreenTeam].getBasePosition(0);
    const float *bluePosition = bases[BlueTeam].getBasePosition(0);
    const float *purplePosition = bases[PurpleTeam].getBasePosition(0);

    int numBoxes = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize);
    float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
    float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);

    for (i = 0; i < numBoxes;) {
      if (clOptions->randomHeights)
	h = boxHeight * (2.0f * (float)bzfrand() + 0.5f);
      float x = worldSize * ((float)bzfrand() - 0.5f);
      float y = worldSize * ((float)bzfrand() - 0.5f);
      // don't place near center and bases
      if ((redGreen &&
	   (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
	    boxBase * 4 ||
	    hypotf(fabs(-x - redPosition[0]),fabs(-y - redPosition[1])) <=
	    boxBase * 4)) ||
	  (bluePurple &&
	   (hypotf(fabs(y - bluePosition[0]), fabs(-x - bluePosition[1])) <=
	    boxBase * 4 ||
	    hypotf(fabs(-y - bluePosition[0]), fabs(x - bluePosition[1])) <=
	    boxBase * 4)) ||
	  (redGreen && bluePurple &&
	   (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
	    boxBase * 4 ||
	    hypotf(fabs(-x - bluePosition[0]), fabs(-y - bluePosition[1])) <=
	    boxBase * 4 ||
	    hypotf(fabs(y - redPosition[0]), fabs(-x - redPosition[1])) <=
	    boxBase * 4 ||
	    hypotf(fabs(-y - redPosition[0]), fabs(x - redPosition[1])) <=
	    boxBase * 4)) ||
	  (hypotf(fabs(x), fabs(y)) <= worldSize / 12))
	continue;

      float angle = (float)(2.0 * M_PI * bzfrand());
      if (redGreen) {
	world->addBox(x, y, 0.0f, angle, boxBase, boxBase, h);
	world->addBox(-x, -y, 0.0f, angle, boxBase, boxBase, h);
	i += 2;
      }
      if (bluePurple) {
	world->addBox(y, -x, 0.0f, angle, boxBase, boxBase, h);
	world->addBox(-y, x, 0.0f, angle, boxBase, boxBase, h);
	i += 2;
      }
    }

    // make pyramids
    h = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
    const int numPyrs = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize * 2);
    for (i = 0; i < numPyrs; i++) {
      if (clOptions->randomHeights)
	h = pyrHeight * (2.0f * (float)bzfrand() + 0.5f);
      float x = worldSize * ((float)bzfrand() - 0.5f);
      float y = worldSize * ((float)bzfrand() - 0.5f);
      // don't place near center or bases
      if ((redGreen &&
	   (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
	    pyrBase * 6 ||
	    hypotf(fabs(-x - redPosition[0]), fabs(-y - redPosition[1])) <=
	    pyrBase * 6)) ||
	  (bluePurple &&
	   (hypotf(fabs(y - bluePosition[0]), fabs(-x - bluePosition[1])) <=
	    pyrBase * 6 ||
	    hypotf(fabs(-y - bluePosition[0]), fabs(x - bluePosition[1])) <=
	    pyrBase * 6)) ||
	  (redGreen && bluePurple &&
	   (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
	    pyrBase * 6 ||
	    hypotf(fabs(-x - bluePosition[0]),fabs(-y - bluePosition[1])) <=
	    pyrBase * 6 ||
	    hypotf(fabs(y - redPosition[0]), fabs(-x - redPosition[1])) <=
	    pyrBase * 6 ||
	    hypotf(fabs(-y - redPosition[0]), fabs(x - redPosition[1])) <=
	    pyrBase * 6)) ||
	  (hypotf(fabs(x), fabs(y)) <= worldSize/12))
	continue;

      float angle = (float)(2.0 * M_PI * bzfrand());
      if (redGreen) {
	world->addPyramid(x, y, 0.0f, angle,pyrBase, pyrBase, h);
	world->addPyramid(-x, -y, 0.0f, angle,pyrBase, pyrBase, h);
	i += 2;
      }
      if (bluePurple) {
	world->addPyramid(y, -x,0.0f, angle, pyrBase, pyrBase, h);
	world->addPyramid(-y, x,0.0f, angle, pyrBase, pyrBase, h);
	i += 2;
      }
    }

    // make teleporters
    if (clOptions->useTeleporters) {
      const int teamFactor = redGreen && bluePurple ? 4 : 2;
      const int numTeleporters = (8 + int(8 * (float)bzfrand())) / teamFactor * teamFactor;
      const int numLinks = 2 * numTeleporters / teamFactor;
      float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
      float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
      float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
      int (*linked)[2] = new int[numLinks][2];
      for (i = 0; i < numTeleporters;) {
	const float x = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
	const float y = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
	const float rotation = (float)(2.0 * M_PI * bzfrand());

	// if too close to building then try again
	Obstacle* obs;
	if (NOT_IN_BUILDING != world->inCylinderNoOctree(&obs, x, y, 0,
						 1.75f * teleBreadth, 1.0f))
	  continue;
	// if to close to a base then try again
	if ((redGreen &&
	     (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
	      baseSize * 4 ||
	      hypotf(fabs(x - greenPosition[0]), fabs(y - greenPosition[1])) <=
	      baseSize * 4)) ||
	    (bluePurple &&
	     (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
	      baseSize * 4 ||
	      hypotf(fabs(x - purplePosition[0]), fabs(y - purplePosition[1])) <=
	      baseSize * 4)))
	  continue;

	linked[i / teamFactor][0] = linked[i / teamFactor][1] = 0;
	if (redGreen) {
	  world->addTeleporter(x, y, 0.0f, rotation, 0.5f * teleWidth,
	      teleBreadth, 2.0f * teleHeight, teleWidth, false);
	  world->addTeleporter(-x, -y, 0.0f, (float)(rotation + M_PI), 0.5f * teleWidth,
	      teleBreadth, 2.0f * teleHeight, teleWidth, false);
	  i += 2;
	}
	if (bluePurple) {
	  world->addTeleporter(y, -x, 0.0f, (float)(rotation + M_PI / 2.0),
			       0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
			       teleWidth, false);
	  world->addTeleporter(-y, x, 0.0f, (float)(rotation + M_PI * 3.0 / 2.0),
			       0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
			       teleWidth, false);
	  i += 2;
	}
      }

      // make teleporter links
      int numUnlinked = numLinks;
      for (i = 0; i < numLinks / 2; i++)
	for (int j = 0; j < 2; j++) {
	  int a = (int)(numUnlinked * (float)bzfrand());
	  if (linked[i][j])
	    continue;
	  for (int k = 0, i2 = i; i2 < numLinks / 2; ++i2) {
	    for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
	      if (linked[i2][j2])
		continue;
	      if (k++ == a) {
		world->addLink((2 * i + j) * teamFactor, (2 * i2 + j2) * teamFactor);
		world->addLink((2 * i + j) * teamFactor + 1, (2 * i2 + j2) * teamFactor + 1);
		if (redGreen && bluePurple) {
		  world->addLink((2 * i + j) * teamFactor + 2, (2 * i2 + j2) * teamFactor + 2);
		  world->addLink((2 * i + j) * teamFactor + 3, (2 * i2 + j2) * teamFactor + 3);
		}
		linked[i][j] = 1;
		numUnlinked--;
		if (i != i2 || j != j2) {
		  world->addLink((2 * i2 + j2) * teamFactor, (2 * i + j) * teamFactor);
		  world->addLink((2 * i2 + j2) * teamFactor + 1, (2 * i + j) * teamFactor + 1);
		  if (redGreen && bluePurple) {
		    world->addLink((2 * i2 + j2) * teamFactor + 2, (2 * i + j) * teamFactor + 2);
		    world->addLink((2 * i2 + j2) * teamFactor + 3, (2 * i + j) * teamFactor + 3);
		  }
		  linked[i2][j2] = 1;
		  numUnlinked--;
		}
	      }
	    }
	  }
	}
      delete[] linked;
    }

  } else {

    float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);
    float avenueSize = BZDB.eval(StateDatabase::BZDB_AVENUESIZE);
    // pyramids in center
    world->addPyramid(
	-(boxBase + 0.25f * avenueSize),
	-(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	(boxBase + 0.25f * avenueSize),
	-(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	-(boxBase + 0.25f * avenueSize),
	(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(
	(boxBase + 0.25f * avenueSize),
	(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(0.0f, -(boxBase + 0.5f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(0.0f,  (boxBase + 0.5f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(-(boxBase + 0.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid( (boxBase + 0.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);

    // halfway out from city center
    world->addPyramid(0.0f, -(3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(0.0f,  (3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid(-(3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    world->addPyramid( (3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	pyrBase, pyrBase, pyrHeight);
    // add boxes, four at once with same height so no team has an advantage
    const float xmin = -0.5f * ((2.0f * boxBase + avenueSize) * (actCitySize - 1));
    const float ymin = -0.5f * ((2.0f * boxBase + avenueSize) * (actCitySize - 1));
    const float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
    for (int j = 0; j <= actCitySize / 2; j++) {
      for (int i = 0; i < actCitySize / 2; i++) {
	if (i != actCitySize / 2 || j != actCitySize / 2) {
	  float h = boxHeight;
	  if (clOptions->randomHeights)
	    h *= 2.0f * (float)bzfrand() + 0.5f;
	  world->addBox(
	      xmin + float(i) * (2.0f * boxBase + avenueSize),
	      ymin + float(j) * (2.0f * boxBase + avenueSize), 0.0f,
	      clOptions->randomBoxes ? (float)(0.5 * M_PI * (bzfrand() - 0.5)) : 0.0f,
	      boxBase, boxBase, h);
	  world->addBox(
	      -1.0f * (xmin + float(i) * (2.0f * boxBase + avenueSize)),
	      -1.0f * (ymin + float(j) * (2.0f * boxBase + avenueSize)), 0.0f,
	      clOptions->randomBoxes ? (float)(0.5 * M_PI * (bzfrand() - 0.5)) : 0.0f,
	      boxBase, boxBase, h);
	  world->addBox(
	      -1.0f * (ymin + float(j) * (2.0f * boxBase + avenueSize)),
	      xmin + float(i) * (2.0f * boxBase + avenueSize), 0.0f,
	      clOptions->randomBoxes ? (float)(0.5 * M_PI * (bzfrand() - 0.5)) : 0.0f,
	      boxBase, boxBase, h);
	  world->addBox(
	      ymin + float(j) * (2.0f * boxBase + avenueSize),
	      -1.0f * (xmin + float(i) * (2.0f * boxBase + avenueSize)), 0.0f,
	      clOptions->randomBoxes ? (float)(0.5 * M_PI * (bzfrand() - 0.5)) : 0.0f,
	      boxBase, boxBase, h);
	}
      }
    }
    // add teleporters
    if (clOptions->useTeleporters) {
      float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
      float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
      float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
      const float xoff = boxBase + 0.5f * avenueSize;
      const float yoff = boxBase + 0.5f * avenueSize;
      world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, (float)(1.25 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, (float)(0.75 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, (float)(1.75 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, (float)(0.25 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, (float)(1.25 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, (float)(0.75 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( 3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, (float)(1.75 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( 3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, (float)(0.25 * M_PI),
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);

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
  }

  // generate the required bases
  for (t = RedTeam; t <= PurpleTeam; t++) {
    if (clOptions->maxTeam[t] == 0) {
      bases.erase(t);
    } else {
      CustomZone zone;
      float p[3] = {0.0f, 0.0f, 0.0f};
      const float size[3] = {baseSize * 0.5f, baseSize * 0.5f, 0.0f};
      const float safeOff = 0.5f * (baseSize + pyrBase);
      switch (t) {
	case RedTeam: {
	  p[0] = (-worldSize + baseSize) / 2.0f;
	  p[1] = 0.0f;
	  world->addBase(p, 0.0f, size, t, false, false);
	  zone.addFlagSafety(p[0] + safeOff, p[1] - safeOff, world);
	  zone.addFlagSafety(p[0] + safeOff, p[1] + safeOff, world);
	  break;
	}
	case GreenTeam: {
	  p[0] = (worldSize - baseSize) / 2.0f;
	  p[1] = 0.0f;
	  world->addBase(p, 0.0f, size, t, false, false);
	  zone.addFlagSafety(p[0] - safeOff, p[1] - safeOff, world);
	  zone.addFlagSafety(p[0] - safeOff, p[1] + safeOff, world);
	  break;
	}
	case BlueTeam: {
	  p[0] = 0.0f;
	  p[1] = (-worldSize + baseSize) / 2.0f;
	  world->addBase(p, 0.0f, size, t, false, false);
	  zone.addFlagSafety(p[0] - safeOff, p[1] + safeOff, world);
	  zone.addFlagSafety(p[0] + safeOff, p[1] + safeOff, world);
	  break;
	}
	case PurpleTeam: {
	  p[0] = 0.0f;
	  p[1] = (worldSize - baseSize) / 2.0f;
	  world->addBase(p, 0.0f, size, t, false, false);
	  zone.addFlagSafety(p[0] - safeOff, p[1] - safeOff, world);
	  zone.addFlagSafety(p[0] + safeOff, p[1] - safeOff, world);
	  break;
	}
      }
    }
  }

  OBSTACLEMGR.makeWorld();
  world->finishWorld();

  return world;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
