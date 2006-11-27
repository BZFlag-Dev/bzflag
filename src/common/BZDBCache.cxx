/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning(4: 4786)
#endif
#include "common.h"
#include <cmath>

// interface header
#include "BZDBCache.h"

bool  BZDBCache::displayMainFlags;
bool  BZDBCache::blend;
bool  BZDBCache::texture;
bool  BZDBCache::shadows;
bool  BZDBCache::stencilShadows;
bool  BZDBCache::zbuffer;
bool  BZDBCache::tesselation;
bool  BZDBCache::lighting;
bool  BZDBCache::smooth;
bool  BZDBCache::colorful;
bool  BZDBCache::animatedTreads;
bool  BZDBCache::leadingShotLine;
bool  BZDBCache::showShotGuide;
int   BZDBCache::radarStyle;
float BZDBCache::radarTankPixels;
int   BZDBCache::linedRadarShots;
int   BZDBCache::sizedRadarShots;
int   BZDBCache::flagChunks;
float BZDBCache::pulseRate;
float BZDBCache::pulseDepth;
bool  BZDBCache::showCollisionGrid;
bool  BZDBCache::showCullingGrid;
int   BZDBCache::maxFlagLOD;

bool BZDBCache::drawCelestial;
bool BZDBCache::drawClouds;
bool BZDBCache::drawGround;
bool BZDBCache::drawGroundLights;
bool BZDBCache::drawMountains;
bool BZDBCache::drawSky;

float BZDBCache::worldSize;
float BZDBCache::radarLimit;
float BZDBCache::gravity;
float BZDBCache::tankWidth;
float BZDBCache::tankLength;
float BZDBCache::tankHeight;
float BZDBCache::tankSpeed;
float BZDBCache::tankRadius;
float BZDBCache::flagRadius;
float BZDBCache::flagPoleSize;
float BZDBCache::flagPoleWidth;
float BZDBCache::maxLOD;

float BZDBCache::freezeTagRadius;
float BZDBCache::collisionLimit;
float BZDBCache::dmzWidth;

float BZDBCache::hudGUIBorderOpacityFactor;

static float getGoodPosValue (float oldVal, const std::string var )
{
	float newVal = BZDB.eval(var);
	if (isnan(newVal) || newVal <= 0.0f)	// it's bad
	{
		BZDB.setFloat(var,oldVal,BZDB.getPermission(var));
		return oldVal;
	}
	return newVal;
}

static float getGoodNonZeroValue (float oldVal, const std::string var )
{
	float newVal = BZDB.eval(var);
	if (isnan(newVal) || newVal == 0.0f)	// it's bad
	{
		BZDB.setFloat(var,oldVal,BZDB.getPermission(var));
			return oldVal;
	}
	return newVal;
}

void BZDBCache::init()
{
  // Client-side variables
  BZDB.addCallback("displayMainFlags", clientCallback, NULL);
  BZDB.addCallback("radarStyle", clientCallback, NULL);
  BZDB.addCallback("radarTankPixels", clientCallback, NULL);
  BZDB.addCallback("blend", clientCallback, NULL);
  BZDB.addCallback("texture", clientCallback, NULL);
  BZDB.addCallback("shadows", clientCallback, NULL);
  BZDB.addCallback("stencilShadows", clientCallback, NULL);
  BZDB.addCallback("zbuffer", clientCallback, NULL);
  BZDB.addCallback("tesselation", clientCallback, NULL);
  BZDB.addCallback("lighting", clientCallback, NULL);
  BZDB.addCallback("smooth", clientCallback, NULL);
  BZDB.addCallback("colorful", clientCallback, NULL);
  BZDB.addCallback("animatedTreads", clientCallback, NULL);
  BZDB.addCallback("leadingShotLine", clientCallback, NULL);
  BZDB.addCallback("showShotGuide", clientCallback, NULL);
  BZDB.addCallback("flagChunks", clientCallback, NULL);
  BZDB.addCallback("pulseRate", clientCallback, NULL);
  BZDB.addCallback("pulseDepth", clientCallback, NULL);
  BZDB.addCallback("showCollisionGrid", clientCallback, NULL);
  BZDB.addCallback("showCullingGrid", clientCallback, NULL);
  BZDB.addCallback("hudGUIBorderOpacityFactor", clientCallback, NULL);
  BZDB.addCallback("maxFlagLOD", clientCallback, NULL);

  // Server-side variables
  BZDB.addCallback(StateDatabase::BZDB_DRAWCELESTIAL, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_DRAWCLOUDS, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_DRAWGROUND, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_DRAWGROUNDLIGHTS, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_DRAWMOUNTAINS, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_DRAWSKY, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_MAXLOD, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_WORLDSIZE, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_RADARLIMIT, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_GRAVITY, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_TANKWIDTH, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_TANKLENGTH, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_TANKHEIGHT, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_TANKSPEED, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_FLAGRADIUS, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_FLAGPOLESIZE, serverCallback, NULL);
  BZDB.addCallback(StateDatabase::BZDB_FLAGPOLEWIDTH, serverCallback, NULL);

  drawCelestial = BZDB.isTrue(StateDatabase::BZDB_DRAWCELESTIAL);
  drawClouds = BZDB.isTrue(StateDatabase::BZDB_DRAWCLOUDS);
  drawGround = BZDB.isTrue(StateDatabase::BZDB_DRAWGROUND);
  drawGroundLights = BZDB.isTrue(StateDatabase::BZDB_DRAWGROUNDLIGHTS);
  drawMountains = BZDB.isTrue(StateDatabase::BZDB_DRAWMOUNTAINS);
  drawSky = BZDB.isTrue(StateDatabase::BZDB_DRAWSKY);

  maxLOD = BZDB.eval(StateDatabase::BZDB_MAXLOD);
  worldSize = getGoodPosValue(worldSize,StateDatabase::BZDB_WORLDSIZE);
  radarLimit = BZDB.eval(StateDatabase::BZDB_RADARLIMIT);
  gravity = getGoodNonZeroValue(gravity,StateDatabase::BZDB_GRAVITY);
  tankWidth = getGoodPosValue(tankWidth,StateDatabase::BZDB_TANKWIDTH);
  tankLength = getGoodPosValue(tankLength,StateDatabase::BZDB_TANKLENGTH);
  tankHeight = getGoodPosValue(tankHeight,StateDatabase::BZDB_TANKHEIGHT);
  tankSpeed = getGoodPosValue(tankSpeed,StateDatabase::BZDB_TANKSPEED);
  tankRadius = getGoodPosValue(tankRadius,StateDatabase::BZDB_TANKRADIUS);
  flagRadius = getGoodPosValue(flagRadius,StateDatabase::BZDB_FLAGRADIUS);
  flagPoleSize = getGoodPosValue(flagPoleSize,StateDatabase::BZDB_FLAGPOLESIZE);
  flagPoleWidth = getGoodPosValue(flagPoleWidth,StateDatabase::BZDB_FLAGPOLEWIDTH);

  freezeTagRadius = getGoodPosValue(freezeTagRadius,StateDatabase::BZDB_FREEZETAGRADIUS);
  collisionLimit = getGoodPosValue(freezeTagRadius,StateDatabase::BZDB_COLLISIONLIMIT);
  dmzWidth = getGoodPosValue(dmzWidth,StateDatabase::BZDB_DMZWIDTH);

  update();
}

void BZDBCache::clientCallback(const std::string& name, void *)
{
  if (name == "blend")
    blend = BZDB.isTrue("blend");
  else if (name == "displayMainFlags")
    displayMainFlags = BZDB.isTrue("displayMainFlags");
  else if (name == "texture")
    texture = BZDB.isTrue("texture");
  else if (name == "shadows")
    shadows = BZDB.isTrue("shadows");
  else if (name == "stencilShadows")
    stencilShadows = BZDB.isTrue("stencilShadows");
  else if (name == "zbuffer")
    zbuffer = BZDB.isTrue("zbuffer");
  else if (name == "tesselation")
    tesselation = BZDB.isTrue("tesselation");
  else if (name == "lighting")
    lighting = BZDB.isTrue("lighting");
  else if (name == "smooth")
    smooth = BZDB.isTrue("smooth");
  else if (name == "colorful")
    colorful = BZDB.isTrue("colorful");
  else if (name == "radarStyle")
    radarStyle = BZDB.evalInt("radarStyle");
  else if (name == "radarTankPixels")
    radarTankPixels = BZDB.eval("radarTankPixels");
  else if (name == "animatedTreads")
    animatedTreads = BZDB.isTrue("animatedTreads");
  else if (name == "leadingShotLine")
    leadingShotLine = BZDB.isTrue("leadingShotLine");
  else if (name == "showShotGuide")
    showShotGuide = BZDB.isTrue("showShotGuide");
  else if (name == "flagChunks")
    flagChunks = BZDB.evalInt("flagChunks");
  else if (name == "pulseRate")
    pulseRate = BZDB.eval("pulseRate");
  else if (name == "pulseDepth")
    pulseDepth = BZDB.eval("pulseDepth");
  else if (name == "showCollisionGrid")
    showCollisionGrid = BZDB.isTrue("showCollisionGrid");
  else if (name == "showCullingGrid")
    showCullingGrid = BZDB.isTrue("showCullingGrid");
  else if (name == "hudGUIBorderOpacityFactor")
    hudGUIBorderOpacityFactor = BZDB.eval("hudGUIBorderOpacityFactor");
  else if (name == "maxFlagLOD")
    maxFlagLOD = BZDB.evalInt("maxFlagLOD");
}

void BZDBCache::serverCallback(const std::string& name, void *)
{
  if (name == StateDatabase::BZDB_DRAWCELESTIAL) {
    drawCelestial = BZDB.isTrue(StateDatabase::BZDB_DRAWCELESTIAL);
  }
  else if (name == StateDatabase::BZDB_DRAWCLOUDS) {
    drawClouds = BZDB.isTrue(StateDatabase::BZDB_DRAWCLOUDS);
  }
  else if (name == StateDatabase::BZDB_DRAWGROUND) {
    drawGround = BZDB.isTrue(StateDatabase::BZDB_DRAWGROUND);
  }
  else if (name == StateDatabase::BZDB_DRAWGROUNDLIGHTS) {
    drawGroundLights = BZDB.isTrue(StateDatabase::BZDB_DRAWGROUNDLIGHTS);
  }
  else if (name == StateDatabase::BZDB_DRAWMOUNTAINS) {
    drawMountains = BZDB.isTrue(StateDatabase::BZDB_DRAWMOUNTAINS);
  }
  else if (name == StateDatabase::BZDB_DRAWSKY) {
    drawSky = BZDB.isTrue(StateDatabase::BZDB_DRAWSKY);
  }
  else if (name == StateDatabase::BZDB_MAXLOD) {
    maxLOD = BZDB.eval(StateDatabase::BZDB_MAXLOD);
  }
  else if (name == StateDatabase::BZDB_WORLDSIZE) {
    worldSize = getGoodPosValue(worldSize,StateDatabase::BZDB_WORLDSIZE);
  }
  else if (name == StateDatabase::BZDB_RADARLIMIT) {
    radarLimit = BZDB.eval(StateDatabase::BZDB_RADARLIMIT);
  }
  else if (name == StateDatabase::BZDB_GRAVITY) {
    gravity = getGoodNonZeroValue(gravity,StateDatabase::BZDB_GRAVITY);
  }
  else if (name == StateDatabase::BZDB_TANKWIDTH) {
    tankWidth = getGoodPosValue(tankWidth,StateDatabase::BZDB_TANKWIDTH);
  }
  else if (name == StateDatabase::BZDB_TANKLENGTH) {
    tankLength = getGoodPosValue(tankLength,StateDatabase::BZDB_TANKLENGTH);
  }
  else if (name == StateDatabase::BZDB_TANKHEIGHT) {
    tankHeight = getGoodPosValue(tankHeight,StateDatabase::BZDB_TANKHEIGHT);
  }
  else if (name == StateDatabase::BZDB_TANKSPEED) {
    tankSpeed = getGoodPosValue(tankSpeed,StateDatabase::BZDB_TANKSPEED);
  }
// Why only in update() ?
//  else if (name == StateDatabase::BZDB_FLAGRADIUS) {
//    flagRadius = BZDB.eval(StateDatabase::BZDB_FLAGRADIUS);
//  }
  else if (name == StateDatabase::BZDB_FLAGPOLESIZE) {
    flagPoleSize = getGoodPosValue(flagPoleSize,StateDatabase::BZDB_FLAGPOLESIZE);
  }
  else if (name == StateDatabase::BZDB_FLAGPOLEWIDTH) {
    flagPoleWidth = getGoodPosValue(flagPoleWidth,StateDatabase::BZDB_FLAGPOLEWIDTH);
  }
}

void BZDBCache::update() {
  tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  linedRadarShots = static_cast<int>(BZDB.eval("linedradarshots"));
  sizedRadarShots = static_cast<int>(BZDB.eval("sizedradarshots"));
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
