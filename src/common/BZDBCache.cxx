/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

// interface header
#include "BZDBCache.h"

bool  BZDBCache::displayMainFlags;
bool  BZDBCache::blend;
bool  BZDBCache::texture;
bool  BZDBCache::shadows;
bool  BZDBCache::zbuffer;
bool  BZDBCache::tesselation;
bool  BZDBCache::lighting;
bool  BZDBCache::smooth;
bool  BZDBCache::colorful;
bool  BZDBCache::animatedTreads;
bool  BZDBCache::leadingShotLine;
int   BZDBCache::radarStyle;
float BZDBCache::radarTankPixels;
int   BZDBCache::linedRadarShots;
int   BZDBCache::sizedRadarShots;
int   BZDBCache::flagChunks;
float BZDBCache::pulseRate;
float BZDBCache::pulseDepth;
bool  BZDBCache::showCollisionGrid;
bool  BZDBCache::showCullingGrid;

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

float BZDBCache::hudGUIBorderOpacityFactor;

void BZDBCache::init()
{
  BZDB.addCallback("displayMainFlags", clientCallback, NULL);
  BZDB.addCallback("radarStyle", clientCallback, NULL);
  BZDB.addCallback("radarTankPixels", clientCallback, NULL);
  BZDB.addCallback("blend", clientCallback, NULL);
  BZDB.addCallback("texture", clientCallback, NULL);
  BZDB.addCallback("shadows", clientCallback, NULL);
  BZDB.addCallback("zbuffer", clientCallback, NULL);
  BZDB.addCallback("tesselation", clientCallback, NULL);
  BZDB.addCallback("lighting", clientCallback, NULL);
  BZDB.addCallback("smooth", clientCallback, NULL);
  BZDB.addCallback("colorful", clientCallback, NULL);
  BZDB.addCallback("animatedTreads", clientCallback, NULL);
  BZDB.addCallback("leadingShotLine", clientCallback, NULL);
  BZDB.addCallback("flagChunks", clientCallback, NULL);
  BZDB.addCallback("pulseRate", clientCallback, NULL);
  BZDB.addCallback("pulseDepth", clientCallback, NULL);
  BZDB.addCallback("showCollisionGrid", clientCallback, NULL);
  BZDB.addCallback("showCullingGrid", clientCallback, NULL);
  BZDB.addCallback("hudGUIBorderOpacityFactor", clientCallback, NULL);

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

  maxLOD = BZDB.eval(StateDatabase::BZDB_MAXLOD);
  worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  radarLimit = BZDB.eval(StateDatabase::BZDB_RADARLIMIT);
  gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);
  tankWidth = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
  tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  flagRadius = BZDB.eval(StateDatabase::BZDB_FLAGRADIUS);
  flagPoleSize = BZDB.eval(StateDatabase::BZDB_FLAGPOLESIZE);
  flagPoleWidth = BZDB.eval(StateDatabase::BZDB_FLAGPOLEWIDTH);

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
}

void BZDBCache::serverCallback(const std::string& name, void *)
{
  if (name == StateDatabase::BZDB_MAXLOD) {
    maxLOD = BZDB.eval(StateDatabase::BZDB_MAXLOD);
  }
  if (name == StateDatabase::BZDB_WORLDSIZE) {
    worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  }
  if (name == StateDatabase::BZDB_RADARLIMIT) {
    radarLimit = BZDB.eval(StateDatabase::BZDB_RADARLIMIT);
  }
  if (name == StateDatabase::BZDB_GRAVITY) {
    gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);
  }
  else if (name == StateDatabase::BZDB_TANKWIDTH) {
    tankWidth = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  }
  else if (name == StateDatabase::BZDB_TANKLENGTH) {
    tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  }
  else if (name == StateDatabase::BZDB_TANKHEIGHT) {
    tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
  }
  else if (name == StateDatabase::BZDB_TANKSPEED) {
    tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  }
// Why only in update() ?
//  else if (name == StateDatabase::BZDB_FLAGRADIUS) {
//    flagRadius = BZDB.eval(StateDatabase::BZDB_FLAGRADIUS);
//  }
  else if (name == StateDatabase::BZDB_FLAGPOLESIZE) {
    flagPoleSize = BZDB.eval(StateDatabase::BZDB_FLAGPOLESIZE);
  }
  else if (name == StateDatabase::BZDB_FLAGPOLEWIDTH) {
    flagPoleWidth = BZDB.eval(StateDatabase::BZDB_FLAGPOLEWIDTH);
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
