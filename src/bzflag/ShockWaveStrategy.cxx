/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ShockWaveStrategy.h"

/* common implementation headers */
#include "SceneRenderer.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "playing.h"
#include "World.h"


ShockWaveStrategy::ShockWaveStrategy(ShotPath *_path) :
  ShotStrategy(_path),
  radius(BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS)),
  radius2(radius * radius)
{
  // setup shot
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_SHOCKADLIFE);

  // make scene node
  const float* pos = _path->getPosition();
  if (RENDERER.useQuality() >= 2) {
    shockNode = new SphereLodSceneNode(pos, radius);
    shockNode->setShockWave(true);
  } else {
    shockNode = new SphereBspSceneNode(pos, radius);
  }

  // get team
  if (_path->getPlayer() == ServerPlayer) {
    TeamColor tmpTeam = _path->getFiringInfo().shot.team;
    team = (tmpTeam < RogueTeam) ? RogueTeam :
	   (tmpTeam > HunterTeam) ? RogueTeam : tmpTeam;
  } else {
    Player* p = lookupPlayer(_path->getPlayer());
    team = p ? p->getTeam() : RogueTeam;
  }

  const float* c = Team::getRadarColor(team);
  if (RENDERER.useQuality() >= 2) {
    shockNode->setColor(c[0], c[1], c[2], 0.5f);
  } else {
    shockNode->setColor(c[0], c[1], c[2], 0.75f);
  }
}


ShockWaveStrategy::~ShockWaveStrategy()
{
  delete shockNode;
}


void ShockWaveStrategy::update(float dt)
{
  radius += dt * (BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) - BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS)) / getPath().getLifetime();
  radius2 = radius * radius;

  // update shock wave scene node
  shockNode->move(getPath().getPosition(), radius);

  // team color
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  TeamColor currentTeam;
  if ((myTank->getFlag() == Flags::Colorblindness) &&
      (getPath().getPlayer() != ServerPlayer)) {
    currentTeam = RogueTeam;
  } else {
    currentTeam = team;
  }

  const float* c = Team::getRadarColor(currentTeam);

  // fade old-style shockwaves
  if (RENDERER.useQuality() >= 2) {
    shockNode->setColor(c[0], c[1], c[2], 0.5f);
  } else {
    const float shockIn = BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS);
    const float shockOut = BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS);
    const GLfloat frac = (radius - shockIn) / (shockOut - shockIn);
    shockNode->setColor(c[0], c[1], c[2], 0.75f - (0.5f * frac));
  }

  // expire when full size
  if (radius >= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS)) setExpired();
}


float ShockWaveStrategy::checkHit(const BaseLocalPlayer* tank, float position[3]) const
{
  // return if player is inside radius of destruction -- note that a
  // shock wave can kill anything inside the radius, be it behind or
  // in a building or even zoned.
  const float* playerPos = tank->getPosition();
  const float* shotPos = getPath().getPosition();
  const float dx = playerPos[0] - shotPos[0];
  const float dy = playerPos[1] - shotPos[1];
  const float dz = playerPos[2] - shotPos[2];
  if (dx * dx + dy * dy + dz * dz <= radius2) {
    position[0] = playerPos[0];
    position[1] = playerPos[1];
    position[2] = playerPos[2];
    return 0.5f;
  } else {
    return Infinity;
  }
}

bool ShockWaveStrategy::isStoppedByHit() const
{
  return false;
}

void ShockWaveStrategy::addShot(SceneDatabase* scene, bool)
{
  scene->addDynamicSphere(shockNode);
}

void ShockWaveStrategy::radarRender() const
{
  // draw circle of current radius
  static const int sides = 20;
  const float* shotPos = getPath().getPosition();
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < sides; i++) {
    const float angle = (float)(2.0 * M_PI * double(i) / double(sides));
    glVertex2f(shotPos[0] + radius * cosf(angle),
	       shotPos[1] + radius * sinf(angle));
  }
  glEnd();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
