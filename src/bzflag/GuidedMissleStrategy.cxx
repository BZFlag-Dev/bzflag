/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "GuidedMissleStrategy.h"

/* common implementation headers */
#include "BZDBCache.h"
#include "TextureManager.h"
#include "Intersect.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "World.h"
#include "playing.h"


static float limitAngle(float a)
{
  if (a < -M_PI) a += (float)(2.0 * M_PI);
  else if (a >= M_PI) a -= (float)(2.0 * M_PI);
  return a;
}

GuidedMissileStrategy::GuidedMissileStrategy(ShotPath* _path) :
  ShotStrategy(_path),
  renderTimes(0),
  needUpdate(true)
{
  ptSceneNode = new BoltSceneNode(_path->getPosition(),_path->getVelocity());
  TextureManager &tm = TextureManager::instance();
  int texture = tm.getTextureID("missile");

  if (texture >= 0) {
    ptSceneNode->setTexture(texture);
    ptSceneNode->setTextureAnimation(4, 4);
    ptSceneNode->setColor(1.0f, 0.2f, 0.0f);
    ptSceneNode->setFlares(true);
  }

  // get initial shot info
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_GMADLIFE);
  const float* vel = getPath().getVelocity();
  const float d = 1.0f / hypotf(vel[0], hypotf(vel[1], vel[2]));
  float dir[3];
  dir[0] = vel[0] * d;
  dir[1] = vel[1] * d;
  dir[2] = vel[2] * d;
  azimuth = limitAngle(atan2f(dir[1], dir[0]));
  elevation = limitAngle(atan2f(dir[2], hypotf(dir[1], dir[0])));

  // initialize segments
  currentTime = getPath().getStartTime();
  Ray ray = Ray(f.shot.pos, dir);
  ShotPathSegment segment(currentTime, currentTime, ray);
  segments.push_back(segment);
  segments.push_back(segment);
  segments.push_back(segment);
  segments.push_back(segment);

  // setup shot
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  f.shot.vel[0] = shotSpeed * dir[0];
  f.shot.vel[1] = shotSpeed * dir[1];
  f.shot.vel[2] = shotSpeed * dir[2];

  // set next position to starting position
  nextPos[0] = f.shot.pos[0];
  nextPos[1] = f.shot.pos[1];
  nextPos[2] = f.shot.pos[2];

  // check that first segment doesn't start inside a building
  float startPos[3];
  float muzzleFront = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  startPos[0] = f.shot.pos[0] - muzzleFront * dir[0];
  startPos[1] = f.shot.pos[1] - muzzleFront * dir[1];
  startPos[2] = f.shot.pos[2] - muzzleFront * dir[2];
  Ray firstRay = Ray(startPos, dir);
  prevTime = currentTime;
  prevTime += -muzzleFront / BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  checkBuildings(firstRay);
  prevTime = currentTime;

  // no last target
  lastTarget = NoPlayer;

  lastPuff = currentTime;
  puffTime = BZDB.eval("gmPuffTime");
}

GuidedMissileStrategy::~GuidedMissileStrategy()
{
  delete ptSceneNode;
}

// NOTE -- ray is base of shot segment and normalized direction of flight.
//	distance traveled is ShotSpeed * dt.

void GuidedMissileStrategy::update(float dt)
{
  const bool isRemote = (getPath().getPlayer() !=
			 LocalPlayer::getMyTank()->getId());

  // ignore packets that arrive out of order
  if (isRemote && dt < 0.0f) return;

  // update time
  prevTime = currentTime;
  currentTime += dt;

  // if shot life ran out then send notification and expire shot.
  // only local shots are expired.
  if (!isRemote && currentTime - getPath().getStartTime() >= getPath().getLifetime())
  {
    /* NOTE -- comment out to not explode when shot expires */
    addShotExplosion(nextPos);
    setExpiring();
    return;
  }

  // get target
  const Player* target = NULL;
  if (isRemote)
  {
    if (lastTarget != NoPlayer)
      target = lookupPlayer(lastTarget);
  }
  else
  {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank)
      target = myTank->getTarget();

    // see if the target changed
    if (target)
    {
      if (lastTarget != target->getId())
      {
	needUpdate = true;
	lastTarget = target->getId();
      }
    }
    else
    {
      if (lastTarget != NoPlayer)
      {
	needUpdate = true;
	lastTarget = NoPlayer;
      }
    }
  }

  if ((target != NULL) && ((target->getFlag() == Flags::Stealth) || ((target->getStatus() & short(PlayerState::Alive)) == 0)))
  {
    target = NULL;
    lastTarget = NoPlayer;
    needUpdate = true;
  }

  // compute next segment's ray
  if (target)
  {
    // turn towards target
    // find desired direction
    const float* targetPos = target->getPosition();
    float desiredDir[3];
    desiredDir[0] = targetPos[0] - nextPos[0];
    desiredDir[1] = targetPos[1] - nextPos[1];
    desiredDir[2] = targetPos[2] - nextPos[2];
    desiredDir[2] += target->getMuzzleHeight(); // right between the eyes

    // compute desired angles
    float newAzimuth = atan2f(desiredDir[1], desiredDir[0]);
    float newElevation = atan2f(desiredDir[2], hypotf(desiredDir[1], desiredDir[0]));

    float gmissileAng = BZDB.eval(StateDatabase::BZDB_GMTURNANGLE);

    // compute new azimuth
    float deltaAzimuth = limitAngle(newAzimuth - azimuth);
    if (fabsf(deltaAzimuth) <= dt * gmissileAng)
      azimuth = limitAngle(newAzimuth);
    else if (deltaAzimuth > 0.0f)
      azimuth = limitAngle(azimuth + dt * gmissileAng);
    else
      azimuth = limitAngle(azimuth - dt * gmissileAng);

    // compute new elevation
    float deltaElevation = limitAngle(newElevation - elevation);
    if (fabsf(deltaElevation) <= dt * gmissileAng)
      elevation = limitAngle(newElevation);
    else if (deltaElevation > 0.0f)
      elevation = limitAngle(elevation + dt * gmissileAng);
    else
      elevation = limitAngle(elevation - dt * gmissileAng);
  }

  float newDirection[3];
  newDirection[0] = cosf(azimuth) * cosf(elevation);
  newDirection[1] = sinf(azimuth) * cosf(elevation);
  newDirection[2] = sinf(elevation);
  Ray ray = Ray(nextPos, newDirection);

  renderTimes++;

  // Changed: GM smoke trail, leave it every seconds, none of this per frame crap
  if (currentTime - lastPuff > puffTime )
  {
    lastPuff = currentTime;
    addShotPuff(nextPos,azimuth,elevation);
  }

  // get next position
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  ray.getPoint(dt * shotSpeed, nextPos);

  // see if we hit something
  double segmentEndTime = currentTime;

  if (nextPos[2] <= 0.0f)
  {
    // hit ground -- expire it and shorten life of segment to time of impact
    setExpiring();
    float t = ray.getOrigin()[2] / (ray.getOrigin()[2] - nextPos[2]);
    segmentEndTime = prevTime;
    segmentEndTime += t * (currentTime - prevTime);
    ray.getPoint(t / shotSpeed, nextPos);
    addShotExplosion(nextPos);
  }
  else
  {
    // see if we hit a building
    const float t = checkBuildings(ray);
    if (t >= 0.0f)
    {
      segmentEndTime = prevTime;
      segmentEndTime += t;
    }
  }


  // throw out old segment and add new one
  ShotPathSegment nextSegment(prevTime, segmentEndTime, ray);
  segments.insert(segments.begin(), nextSegment);
  segments.pop_back();

  // update shot
  newDirection[0] *= shotSpeed;
  newDirection[1] *= shotSpeed;
  newDirection[2] *= shotSpeed;
  setPosition(nextPos);
  setVelocity(newDirection);
}

bool GuidedMissileStrategy::predictPosition(float dt, float p[3]) const
{
  float v[3];
  return _predict(dt, p, v);
}

bool GuidedMissileStrategy::predictVelocity(float dt, float v[3]) const
{
  float p[3];
  return _predict(dt, p, v);
}

bool GuidedMissileStrategy::_predict(float dt, float p[3], float v[3]) const
{
  const bool isRemote = (getPath().getPlayer() !=
			 LocalPlayer::getMyTank()->getId());

  float ctime = (float)currentTime + dt;

  /*
   * If it expires there we'll return false.
   */
  if (ctime - getPath().getStartTime() >= getPath().getLifetime())
    return false;

  // get target
  const Player* target = NULL;
  if (isRemote)
  {
    if (lastTarget != NoPlayer)
      target = lookupPlayer(lastTarget);
  }
  else
  {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank)
      target = myTank->getTarget();
  }

  if ((target != NULL) && ((target->getFlag() == Flags::Stealth) || ((target->getStatus() & short(PlayerState::Alive)) == 0)))
    target = NULL;

  float tmpAzimuth = azimuth, tmpElevation = elevation;
  // compute next segment's ray
  if (target)
  {
    // turn towards target
    // find desired direction
    const float* targetPos = target->getPosition();
    float desiredDir[3];
    desiredDir[0] = targetPos[0] - nextPos[0];
    desiredDir[1] = targetPos[1] - nextPos[1];
    desiredDir[2] = targetPos[2] - nextPos[2];
    desiredDir[2] += target->getMuzzleHeight(); // right between the eyes

    // compute desired angles
    float newAzimuth = atan2f(desiredDir[1], desiredDir[0]);
    float newElevation = atan2f(desiredDir[2], hypotf(desiredDir[1], desiredDir[0]));

    float gmissileAng = BZDB.eval(StateDatabase::BZDB_GMTURNANGLE);

    // compute new azimuth
    float deltaAzimuth = limitAngle(newAzimuth - azimuth);
    if (fabsf(deltaAzimuth) <= dt * gmissileAng)
      tmpAzimuth = limitAngle(newAzimuth);
    else if (deltaAzimuth > 0.0f)
      tmpAzimuth = limitAngle(azimuth + dt * gmissileAng);
    else
      tmpAzimuth = limitAngle(azimuth - dt * gmissileAng);

    // compute new elevation
    float deltaElevation = limitAngle(newElevation - elevation);
    if (fabsf(deltaElevation) <= dt * gmissileAng)
      tmpElevation = limitAngle(newElevation);
    else if (deltaElevation > 0.0f)
      tmpElevation = limitAngle(elevation + dt * gmissileAng);
    else
      tmpElevation = limitAngle(elevation - dt * gmissileAng);
  }

  float newDirection[3];
  newDirection[0] = cosf(tmpAzimuth) * cosf(tmpElevation);
  newDirection[1] = sinf(tmpAzimuth) * cosf(tmpElevation);
  newDirection[2] = sinf(tmpElevation);
  Ray ray = Ray(nextPos, newDirection);

  // get next position
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  ray.getPoint(dt * shotSpeed, p);

  // see if we hit something
  if (p[2] <= 0.0f)
    return false;
  else
  {
    // see if we hit a building
    float t = float((currentTime - prevTime) * shotSpeed);
    int face;
    const Obstacle* building = getFirstBuilding(ray, Epsilon, t);
    const Teleporter* teleporter = getFirstTeleporter(ray, Epsilon, t, face);

    World *world = World::getWorld();
    if (!world) {
      return false;
    }

    // check in reverse order to see what we hit first
    if (teleporter) {
      // entered teleporter -- teleport it
      unsigned int seed = getPath().getShotId();
      int source = world->getTeleporter(teleporter, face);
      int teletarget = world->getTeleportTarget(source, seed);

      int outFace;
      const Teleporter* outTeleporter = world->getTeleporter(teletarget, outFace);
      teleporter->getPointWRT(*outTeleporter, face, outFace, p, NULL, tmpAzimuth, p, NULL, &tmpAzimuth);
    } else if (building) {
      // expire on next update
      return false;
    }
  }

  // update shot
  newDirection[0] *= shotSpeed;
  newDirection[1] *= shotSpeed;
  newDirection[2] *= shotSpeed;

  v[0] = newDirection[0];
  v[1] = newDirection[1];
  v[2] = newDirection[2];

  return true;
}


float GuidedMissileStrategy::checkBuildings(const Ray& ray)
{
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  float t = float((currentTime - prevTime) * shotSpeed);
  int face;
  const Obstacle* building = getFirstBuilding(ray, Epsilon, t);
  const Teleporter* teleporter = getFirstTeleporter(ray, Epsilon, t, face);

  World *world = World::getWorld();
  if (!world) {
    return -1.0f;
  }

  // check in reverse order to see what we hit first
  if (teleporter) {
    // entered teleporter -- teleport it
    unsigned int seed = getPath().getShotId();
    int source = world->getTeleporter(teleporter, face);
    int target = world->getTeleportTarget(source, seed);

    int outFace;
    const Teleporter* outTeleporter = world->getTeleporter(target, outFace);
    teleporter->getPointWRT(*outTeleporter, face, outFace, nextPos, NULL, azimuth, nextPos, NULL, &azimuth);
    return t / shotSpeed;
  } else if (building) {
    // expire on next update
    setExpiring();
    float pos[3];
    ray.getPoint(t / shotSpeed, pos);
    addShotExplosion(pos);
    return t / shotSpeed;
  }
  return -1.0f;
}

float GuidedMissileStrategy::checkHit(const ShotCollider& tank, float position[3]) const
{
  float minTime = Infinity;
  if (getPath().isExpired())
    return minTime;

  // GM is not active until activation time passes (for any tank)
  const float activationTime = BZDB.eval(StateDatabase::BZDB_GMACTIVATIONTIME);

  if ((getPath().getCurrentTime() - getPath().getStartTime()) < activationTime)
    return minTime;

  // get tank radius
  const float radius2 = tank.radius * tank.radius;

  float shotRadius = BZDB.eval(StateDatabase::BZDB_SHOTRADIUS);

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  const float tankHeight = tank.size[2];

  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tank.motion.getOrigin()[0];
  lastTankPositionRaw[1] = tank.motion.getOrigin()[1];
  lastTankPositionRaw[2] = tank.motion.getOrigin()[2] + 0.5f * tankHeight;

  Ray tankLastMotion(lastTankPositionRaw, tank.motion.getDirection());

  // check each segment
  const size_t numSegments = segments.size();
  size_t i = 0;
  // only test most recent segment if shot is from my tank
  if (numSegments > 1 && tank.testLastSegment )
    i = numSegments - 1;
  for (; i < numSegments; i++) {
    const Ray& ray = segments[i].ray;

    // construct ray with correct velocity
    float speed[3];
    const float* dir = ray.getDirection();
    float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
    speed[0] = shotSpeed * dir[0];
    speed[1] = shotSpeed * dir[1];
    speed[2] = shotSpeed * dir[2];
    Ray speedRay(ray.getOrigin(), speed);

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(rayMinusRay(speedRay, 0.0, tankLastMotion, 0.0));

    // get closest approach time
    float t;
    if (tank.test2D)
    {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float tankBase[3] = { 0.0f, 0.0f, -0.5f * tankHeight };
      t = timeRayHitsBlock(relativeRay, tankBase, tank.angle, 0.5f * tank.length, shotRadius, tankHeight);
    }
    else
    {
      // find time when shot hits sphere around tank
      t = rayAtDistanceFromOrigin(relativeRay, 0.99f * tank.radius);
    }

    if (t > minTime)
      continue;

    // if not in shot segment times then no hit
    if (t < 0.0f || t > segments[i].end - segments[i].start)
      continue;

    // check if shot hits tank -- get position at time t, see if in radius
    float closestPos[3];
    relativeRay.getPoint(t, closestPos);
    if (closestPos[0] * closestPos[0] +
	closestPos[1] * closestPos[1] +
	closestPos[2] * closestPos[2] < radius2) {
      // save best time so far
      minTime = t;

      // compute location of tank at time of hit
      float tankPos[3];
      tank.motion.getPoint(t, tankPos);

      // compute position of intersection
      position[0] = tankPos[0] + closestPos[0];
      position[1] = tankPos[1] + closestPos[1];
      position[2] = tankPos[2] + closestPos[2];
    }
  }

  return minTime;
}

void GuidedMissileStrategy::sendUpdate(const FiringInfo& firingInfo) const
{
  // only send an update when needed
  if (!needUpdate) return;
  ((GuidedMissileStrategy*)this)->needUpdate = false;

  // construct and send packet
  char packet[ShotUpdatePLen + PlayerIdPLen];
  void *buf = (void*)packet;
  buf = firingInfo.shot.pack(buf);
  buf = nboPackUByte(buf, lastTarget);
  ServerLink::getServer()->send(MsgGMUpdate, sizeof(packet), packet);
}

void GuidedMissileStrategy::readUpdate(uint16_t code, void* msg)
{
  // ignore non-guided missile messages (we shouldn't get them)
  if (code != MsgGMUpdate) return;

  // position and velocity have been replaced by the remote system's
  // concept of the position and velocity.  this may cause a discontinuity
  // in the shot's position but it's probably better to have the shot in
  // the right place than to maintain smooth motion.  these updates ought
  // to be rare anyway.

  // read the lastTarget
  nboUnpackUByte(msg, lastTarget);

  // fix up dependent variables
  const float* vel = getPath().getVelocity();
  const float d = 1.0f / hypotf(vel[0], hypotf(vel[1], vel[2]));
  float dir[3];
  dir[0] = vel[0] * d;
  dir[1] = vel[1] * d;
  dir[2] = vel[2] * d;
  azimuth = limitAngle(atan2f(dir[1], dir[0]));
  elevation = limitAngle(atan2f(dir[2], hypotf(dir[1], dir[0])));
  const float* pos = getPath().getPosition();
  nextPos[0] = pos[0];
  nextPos[1] = pos[1];
  nextPos[2] = pos[2];

  // note that we do not call update(float).  let that happen on the
  // next time step.
}

void GuidedMissileStrategy::addShot(SceneDatabase* scene, bool)
{
  ptSceneNode->move(getPath().getPosition(), getPath().getVelocity());
  scene->addDynamicNode(ptSceneNode);
}

void GuidedMissileStrategy::expire()
{
  if (getPath().getPlayer() == LocalPlayer::getMyTank()->getId()) {
    const ShotPath& shot = getPath();
    /* NOTE -- change 0 to 1 to not explode when shot expires (I think) */
    ServerLink::getServer()->sendEndShot(shot.getPlayer(), shot.getShotId(), 0);
  }
}

void GuidedMissileStrategy::radarRender() const
{
  const float *orig = getPath().getPosition();
  const int length = BZDBCache::linedRadarShots;
  const int size   = BZDBCache::sizedRadarShots;

  float shotTailLength = BZDB.eval(StateDatabase::BZDB_SHOTTAILLENGTH);
  // Display leading lines
  if (length > 0) {
    const float* vel = getPath().getVelocity();
    const float d = 1.0f / hypotf(vel[0], hypotf(vel[1], vel[2]));
    float dir[3];
    dir[0] = vel[0] * d * shotTailLength * length;
    dir[1] = vel[1] * d * shotTailLength * length;
    dir[2] = vel[2] * d * shotTailLength * length;
    glBegin(GL_LINES);
    glVertex2fv(orig);
    if (BZDBCache::leadingShotLine) {
      glVertex2f(orig[0] + dir[0], orig[1] + dir[1]);
    } else {
      glVertex2f(orig[0] - dir[0], orig[1] - dir[1]);
    }
    glEnd();

    // draw a "bright reddish" missle tip
    if (size > 0) {
      glColor3f(1.0f, 0.75f, 0.75f);
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2f(orig[0], orig[1]);
      glEnd();
      glPointSize(1.0f);
    }
  } else {
    if (size > 0) {
      // draw a sized missle
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
      glPointSize(1.0f);
    } else {
      // draw the tiny missle
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
