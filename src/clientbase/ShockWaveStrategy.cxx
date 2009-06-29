/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "Roster.h"
#include "World.h"
#include "playing.h"


ShockWaveStrategy::ShockWaveStrategy(ShotPath *_path) :
  ShotStrategy(_path),
  radius(BZDB.eval(BZDBNAMES.SHOCKINRADIUS)),
  radius2(radius * radius)
{
  // setup shot
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(BZDBNAMES.SHOCKADLIFE);

  // make scene node
  const fvec3& pos = _path->getPosition();
  if (RENDERER.useQuality() >= _HIGH_QUALITY) {
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

  const fvec4& c = Team::getRadarColor(team);
  if (RENDERER.useQuality() >= _HIGH_QUALITY) {
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
  const float shockIn  = BZDB.eval(BZDBNAMES.SHOCKINRADIUS);
  const float shockOut = BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS);

  radius += dt * (shockOut - shockIn) / getPath().getLifetime();
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

  const fvec4& c = Team::getRadarColor(currentTeam);

  // fade old-style shockwaves
  if (RENDERER.useQuality() >= _HIGH_QUALITY) {
    shockNode->setColor(c[0], c[1], c[2], 0.5f);
  } else {
    const float frac = (radius - shockIn) / (shockOut - shockIn);
    shockNode->setColor(c[0], c[1], c[2], 0.75f - (0.5f * frac));
  }

  // expire when full size
  if (radius >= BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS)) setExpired();
}


bool ShockWaveStrategy::predictPosition(float dt, fvec3& p) const
{
  const float shockIn  = BZDB.eval(BZDBNAMES.SHOCKINRADIUS);
  const float shockOut = BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS);

  const float r = radius + dt * (shockOut - shockIn) / getPath().getLifetime();
  if (r >= shockOut) {
    return false;
  }

  const float *pos = getPath().getPosition();
  p[0] = pos[0];
  p[1] = pos[1];
  p[2] = pos[2];
  return true;
}

bool        ShockWaveStrategy::predictVelocity(float dt, fvec3& p) const
{
  const float shockIn  = BZDB.eval(BZDBNAMES.SHOCKINRADIUS);
  const float shockOut = BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS);

  const float r = radius + dt * (shockOut - shockIn) / getPath().getLifetime();
  if (r >= shockOut) {
    return false;
  }

  p.x = (shockOut - shockIn) / getPath().getLifetime();
  p.y = 0;
  p.z = 0;

  return true;
}


float ShockWaveStrategy::checkHit(const ShotCollider& tank, fvec3& position) const
{
  // return if player is inside radius of destruction -- note that a
  // shock wave can kill anything inside the radius, be it behind or
  // in a building or even zoned.
  const fvec3& playerPos = tank.position;
  const fvec3& shotPos = getPath().getPosition();
  if ((playerPos - shotPos).lengthSq() <= radius2) {
    position = playerPos;
    return 0.5f;
  }
  else {
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
  const fvec3& shotPos = getPath().getPosition();
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
