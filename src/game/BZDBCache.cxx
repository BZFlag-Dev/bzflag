/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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


//============================================================================//

BZDBCache::Bool  BZDBCache::displayMainFlags;
BZDBCache::Bool  BZDBCache::blend;
BZDBCache::Bool  BZDBCache::texture;
BZDBCache::Bool  BZDBCache::zbuffer;
BZDBCache::Bool  BZDBCache::tesselation;
BZDBCache::Bool  BZDBCache::lighting;
BZDBCache::Bool  BZDBCache::smooth;
BZDBCache::Bool  BZDBCache::colorful;
BZDBCache::Bool  BZDBCache::animatedTreads;
BZDBCache::Bool  BZDBCache::leadingShotLine;
BZDBCache::Bool  BZDBCache::showShotGuide;
BZDBCache::Int   BZDBCache::shadowMode;
BZDBCache::Float BZDBCache::shadowAlpha;
BZDBCache::Int   BZDBCache::radarStyle;
BZDBCache::Float BZDBCache::radarTankPixels;
BZDBCache::Int   BZDBCache::linedRadarShots;
BZDBCache::Int   BZDBCache::sizedRadarShots;
BZDBCache::Float BZDBCache::shotLength;
BZDBCache::Int   BZDBCache::flagChunks;
BZDBCache::Float BZDBCache::pulseRate;
BZDBCache::Float BZDBCache::pulseDepth;
BZDBCache::Bool  BZDBCache::showCollisionGrid;
BZDBCache::Bool  BZDBCache::showCullingGrid;
BZDBCache::Int   BZDBCache::maxFlagLOD;
BZDBCache::Int   BZDBCache::vsync;

BZDBCache::Bool  BZDBCache::forbidDebug;
BZDBCache::Bool  BZDBCache::drawCelestial;
BZDBCache::Bool  BZDBCache::drawClouds;
BZDBCache::Bool  BZDBCache::drawGround;
BZDBCache::Bool  BZDBCache::drawGroundLights;
BZDBCache::Bool  BZDBCache::drawMountains;
BZDBCache::Bool  BZDBCache::drawSky;

BZDBCache::Float BZDBCache::worldSize;
BZDBCache::Float BZDBCache::radarLimit;
BZDBCache::Float BZDBCache::gravity;
BZDBCache::Float BZDBCache::muzzleHeight;
BZDBCache::Float BZDBCache::tankWidth;
BZDBCache::Float BZDBCache::tankLength;
BZDBCache::Float BZDBCache::tankHeight;
BZDBCache::Float BZDBCache::tankSpeed;
BZDBCache::Float BZDBCache::tankAngVel;
BZDBCache::Float BZDBCache::tankRadius;
BZDBCache::Float BZDBCache::flagRadius;
BZDBCache::Float BZDBCache::flagPoleSize;
BZDBCache::Float BZDBCache::flagPoleWidth;
BZDBCache::Float BZDBCache::maxLOD;
BZDBCache::Float BZDBCache::minGameFrameTime;
BZDBCache::Float BZDBCache::maxGameFrameTime;

BZDBCache::Float BZDBCache::hudGUIBorderOpacityFactor;


//============================================================================//

static float getGoodPosValue(float oldVal, const std::string& var)
{
  float newVal = BZDB.eval(var);
  if (isnan(newVal) || newVal <= 0.0f) { // it's bad
    BZDB.setFloat(var, oldVal, BZDB.getPermission(var));
    return oldVal;
  }
  return newVal;
}


static float getGoodNonZeroValue(float oldVal, const std::string& var)
{
  float newVal = BZDB.eval(var);
  if (isnan(newVal) || newVal == 0.0f) { // it's bad
    BZDB.setFloat(var, oldVal, BZDB.getPermission(var));
      return oldVal;
  }
  return newVal;
}


//============================================================================//

static bool parseGameFrameTimes(const std::string& str, float& minTime,
                                                        float& maxTime)
{
  static const float minGameFPS = 10.0f;
  static const float maxGameFPS = 100.0f;
  static const float minFrameTime = (1.0f / maxGameFPS);
  static const float maxFrameTime = (1.0f / minGameFPS);

  minTime = minFrameTime;
  maxTime = maxFrameTime;

  float f0, f1;

  switch (sscanf(str.c_str(), "%f %f", &f0, &f1)) {
    case 1: {
      if (f0 < minGameFPS) { f0 = minGameFPS; }
      if (f0 > maxGameFPS) { f0 = maxGameFPS; }
      minTime = (1.0f / f0);
      maxTime = (1.0f / f0);
      break;
    }
    case 2: {
      if (f0 < minGameFPS) { f0 = minGameFPS; }
      if (f0 > maxGameFPS) { f0 = maxGameFPS; }
      if (f1 < minGameFPS) { f1 = minGameFPS; }
      if (f1 > maxGameFPS) { f1 = maxGameFPS; }
      if (f0 > f1) {
        return false;
      }
      minTime = (1.0f / f1);
      maxTime = (1.0f / f0);
      break;
    }
    default: { return false; }
  }

  return true;
}


//============================================================================//

void BZDBCache::init()
{
  // Client-side variables
  BZDB.addCallback("displayMainFlags",          clientCallback, NULL);
  BZDB.addCallback("radarStyle",                clientCallback, NULL);
  BZDB.addCallback("radarTankPixels",           clientCallback, NULL);
  BZDB.addCallback("shadowMode",                clientCallback, NULL);
  BZDB.addCallback("shadowAlpha",               clientCallback, NULL);
  BZDB.addCallback("blend",                     clientCallback, NULL);
  BZDB.addCallback("texture",                   clientCallback, NULL);
  BZDB.addCallback("zbuffer",                   clientCallback, NULL);
  BZDB.addCallback("tesselation",               clientCallback, NULL);
  BZDB.addCallback("lighting",                  clientCallback, NULL);
  BZDB.addCallback("smooth",                    clientCallback, NULL);
  BZDB.addCallback("colorful",                  clientCallback, NULL);
  BZDB.addCallback("animatedTreads",            clientCallback, NULL);
  BZDB.addCallback("shotLength",                clientCallback, NULL);
  BZDB.addCallback("leadingShotLine",           clientCallback, NULL);
  BZDB.addCallback("showShotGuide",             clientCallback, NULL);
  BZDB.addCallback("flagChunks",                clientCallback, NULL);
  BZDB.addCallback("pulseRate",                 clientCallback, NULL);
  BZDB.addCallback("pulseDepth",                clientCallback, NULL);
  BZDB.addCallback("showCollisionGrid",         clientCallback, NULL);
  BZDB.addCallback("showCullingGrid",           clientCallback, NULL);
  BZDB.addCallback("hudGUIBorderOpacityFactor", clientCallback, NULL);
  BZDB.addCallback("maxFlagLOD",                clientCallback, NULL);
  BZDB.addCallback("vsync",                     clientCallback, NULL);

  // Server-side variables
  BZDB.addCallback(BZDBNAMES.DRAWCELESTIAL,    serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.DRAWCLOUDS,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.DRAWGROUND,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.DRAWGROUNDLIGHTS, serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.DRAWMOUNTAINS,    serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.DRAWSKY,          serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.FLAGRADIUS,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.FLAGPOLESIZE,     serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.FLAGPOLEWIDTH,    serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.FORBIDDEBUG,      serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.GRAVITY,          serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.MAXLOD,           serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.WORLDSIZE,        serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.RADARLIMIT,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.TANKWIDTH,        serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.TANKLENGTH,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.TANKHEIGHT,       serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.TANKSPEED,        serverCallback, NULL);
  BZDB.addCallback(BZDBNAMES.TANKANGVEL,       serverCallback, NULL);
  BZDB.addCallback("_gameFPS",                 serverCallback, NULL);

  forbidDebug      = BZDB.isTrue(BZDBNAMES.FORBIDDEBUG);
  drawCelestial    = BZDB.isTrue(BZDBNAMES.DRAWCELESTIAL);
  drawClouds       = BZDB.isTrue(BZDBNAMES.DRAWCLOUDS);
  drawGround       = BZDB.isTrue(BZDBNAMES.DRAWGROUND);
  drawGroundLights = BZDB.isTrue(BZDBNAMES.DRAWGROUNDLIGHTS);
  drawMountains    = BZDB.isTrue(BZDBNAMES.DRAWMOUNTAINS);
  drawSky          = BZDB.isTrue(BZDBNAMES.DRAWSKY);

  maxLOD           = BZDB.eval(BZDBNAMES.MAXLOD);
  radarLimit       = BZDB.eval(BZDBNAMES.RADARLIMIT);
  flagPoleSize     = getGoodPosValue(flagPoleSize,    BZDBNAMES.FLAGPOLESIZE);
  flagPoleWidth    = getGoodPosValue(flagPoleWidth,   BZDBNAMES.FLAGPOLEWIDTH);
  flagRadius       = getGoodPosValue(flagRadius,      BZDBNAMES.FLAGRADIUS);
  gravity          = getGoodNonZeroValue(gravity,     BZDBNAMES.GRAVITY);
  muzzleHeight     = getGoodPosValue(muzzleHeight,    BZDBNAMES.MUZZLEHEIGHT);
  tankHeight       = getGoodPosValue(tankHeight,      BZDBNAMES.TANKHEIGHT);
  tankLength       = getGoodPosValue(tankLength,      BZDBNAMES.TANKLENGTH);
  tankRadius       = getGoodPosValue(tankRadius,      BZDBNAMES.TANKRADIUS);
  tankSpeed        = getGoodPosValue(tankSpeed,       BZDBNAMES.TANKSPEED);
  tankAngVel       = getGoodPosValue(tankAngVel,      BZDBNAMES.TANKANGVEL);
  tankWidth        = getGoodPosValue(tankWidth,       BZDBNAMES.TANKWIDTH);
  worldSize        = getGoodPosValue(worldSize,       BZDBNAMES.WORLDSIZE);

  parseGameFrameTimes(BZDB.get("_gameFPS"), (float&)minGameFrameTime,
                                            (float&)maxGameFrameTime);

  update();
}


//============================================================================//

void BZDBCache::clientCallback(const std::string& name, void *)
{
  if (name == "blend")
    blend = BZDB.isTrue("blend");
  else if (name == "displayMainFlags")
    displayMainFlags = BZDB.isTrue("displayMainFlags");
  else if (name == "texture")
    texture = BZDB.isTrue("texture");
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
  else if (name == "shadowMode")
    shadowMode = BZDB.evalInt("shadowMode");
  else if (name == "shadowAlpha")
    shadowAlpha = BZDB.eval("shadowAlpha");
  else if (name == "animatedTreads")
    animatedTreads = BZDB.isTrue("animatedTreads");
  else if (name == "shotLength")
    shotLength = BZDB.eval("shotLength");
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
  else if (name == "vsync")
    vsync = BZDB.evalInt("vsync");
}


//============================================================================//

void BZDBCache::serverCallback(const std::string& name, void *)
{
  if (name == BZDBNAMES.FORBIDDEBUG) {
    forbidDebug = BZDB.isTrue(BZDBNAMES.FORBIDDEBUG);
  }
  else if (name == BZDBNAMES.DRAWCELESTIAL) {
    drawCelestial = BZDB.isTrue(BZDBNAMES.DRAWCELESTIAL);
  }
  else if (name == BZDBNAMES.DRAWCLOUDS) {
    drawClouds = BZDB.isTrue(BZDBNAMES.DRAWCLOUDS);
  }
  else if (name == BZDBNAMES.DRAWGROUND) {
    drawGround = BZDB.isTrue(BZDBNAMES.DRAWGROUND);
  }
  else if (name == BZDBNAMES.DRAWGROUNDLIGHTS) {
    drawGroundLights = BZDB.isTrue(BZDBNAMES.DRAWGROUNDLIGHTS);
  }
  else if (name == BZDBNAMES.DRAWMOUNTAINS) {
    drawMountains = BZDB.isTrue(BZDBNAMES.DRAWMOUNTAINS);
  }
  else if (name == BZDBNAMES.DRAWSKY) {
    drawSky = BZDB.isTrue(BZDBNAMES.DRAWSKY);
  }
  else if (name == BZDBNAMES.MAXLOD) {
    maxLOD = BZDB.eval(BZDBNAMES.MAXLOD);
  }
  else if (name == BZDBNAMES.WORLDSIZE) {
    worldSize = getGoodPosValue(worldSize,BZDBNAMES.WORLDSIZE);
  }
  else if (name == BZDBNAMES.RADARLIMIT) {
    radarLimit = BZDB.eval(BZDBNAMES.RADARLIMIT);
  }
  else if (name == BZDBNAMES.GRAVITY) {
    gravity = getGoodNonZeroValue(gravity,BZDBNAMES.GRAVITY);
  }
  else if (name == BZDBNAMES.MUZZLEHEIGHT) {
    muzzleHeight = getGoodNonZeroValue(gravity,BZDBNAMES.MUZZLEHEIGHT);
  }
  else if (name == BZDBNAMES.TANKWIDTH) {
    tankWidth = getGoodPosValue(tankWidth,BZDBNAMES.TANKWIDTH);
  }
  else if (name == BZDBNAMES.TANKLENGTH) {
    tankLength = getGoodPosValue(tankLength,BZDBNAMES.TANKLENGTH);
  }
  else if (name == BZDBNAMES.TANKHEIGHT) {
    tankHeight = getGoodPosValue(tankHeight,BZDBNAMES.TANKHEIGHT);
  }
  else if (name == BZDBNAMES.TANKSPEED) {
    tankSpeed = getGoodPosValue(tankSpeed,BZDBNAMES.TANKSPEED);
  }
// Why only in update() ?   FIXME
//  else if (name == BZDBNAMES.FLAGRADIUS) {
//    flagRadius = BZDB.eval(BZDBNAMES.FLAGRADIUS);
//  }
  else if (name == BZDBNAMES.FLAGPOLESIZE) {
    flagPoleSize = getGoodPosValue(flagPoleSize,BZDBNAMES.FLAGPOLESIZE);
  }
  else if (name == BZDBNAMES.FLAGPOLEWIDTH) {
    flagPoleWidth = getGoodPosValue(flagPoleWidth,BZDBNAMES.FLAGPOLEWIDTH);
  }
  else if (name == "_gameFPS") {
    parseGameFrameTimes(BZDB.get("_gameFPS"), (float&)minGameFrameTime,
                                              (float&)maxGameFrameTime);
  }
}


//============================================================================//

void BZDBCache::update() {
  tankRadius = BZDB.eval(BZDBNAMES.TANKRADIUS);
  linedRadarShots = BZDB.evalInt("linedradarshots");
  sizedRadarShots = BZDB.evalInt("sizedradarshots");
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
