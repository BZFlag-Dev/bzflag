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
#include "GuidedMissleStrategy.h"

/* common implementation headers */
#include "BZDBCache.h"
#include "TextureManager.h"
#include "Intersect.h"
#include "EventHandler.h"

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
  const fvec3& vel = getPath().getVelocity();
  const fvec3 dir = vel.normalize();
  azimuth   = limitAngle(atan2f(dir.y, dir.x));
  elevation = limitAngle(atan2f(dir.z, dir.xy().length()));

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
  f.shot.vel = shotSpeed * dir;

  // set next position to starting position
  nextPos = f.shot.pos;

  // check that first segment doesn't start inside a building
  float muzzleFront = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  const fvec3 startPos = f.shot.pos - (muzzleFront * dir);

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
  if (!isRemote && currentTime - getPath().getStartTime() >= getPath().getLifetime()) {
    /* NOTE -- comment out to not explode when shot expires */
    addShotExplosion(nextPos);
    setExpiring();
    return;
  }

  // get target
  const Player* target = NULL;
  if (isRemote) {
    if (lastTarget != NoPlayer)
      target = lookupPlayer(lastTarget);
  } else {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank)
      target = myTank->getTarget();

    // see if the target changed
    if (target) {
      if (lastTarget != target->getId()) {
	needUpdate = true;
	lastTarget = target->getId();
      }
    } else {
      if (lastTarget != NoPlayer) {
	needUpdate = true;
	lastTarget = NoPlayer;
      }
    }
  }

  if ((target != NULL) && ((target->getFlag() == Flags::Stealth) || ((target->getStatus() & short(PlayerState::Alive)) == 0))) {
    target = NULL;
    lastTarget = NoPlayer;
    needUpdate = true;
  }

  // compute next segment's ray
  if (target) {
    // turn towards target
    // find desired direction
    const fvec3& targetPos = target->getPosition();
    fvec3 desiredDir = targetPos - nextPos;
    desiredDir.z += target->getMuzzleHeight(); // right between the eyes

    // compute desired angles
    float newAzimuth   = atan2f(desiredDir.y, desiredDir.x);
    float newElevation = atan2f(desiredDir.z, desiredDir.xy().length());

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

  fvec3 newDirection;
  newDirection.x = cosf(azimuth) * cosf(elevation);
  newDirection.y = sinf(azimuth) * cosf(elevation);
  newDirection.z = sinf(elevation);
  Ray ray(nextPos, newDirection);

  renderTimes++;

  // Changed: GM smoke trail, leave it every seconds, none of this per frame crap
  if (currentTime - lastPuff > puffTime ) {
    lastPuff = currentTime;
    addShotPuff(nextPos,azimuth,elevation);
  }

  // get next position
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  ray.getPoint(dt * shotSpeed, nextPos);

  // see if we hit something
  double segmentEndTime = currentTime;

  if (nextPos[2] <= 0.0f) {
    // hit ground -- expire it and shorten life of segment to time of impact
    setExpiring();
    float t = ray.getOrigin()[2] / (ray.getOrigin()[2] - nextPos[2]);
    segmentEndTime = prevTime;
    segmentEndTime += t * (currentTime - prevTime);
    ray.getPoint(t / shotSpeed, nextPos);
    addShotExplosion(nextPos);
  } else {
    // see if we hit a building
    const float t = checkBuildings(ray);
    if (t >= 0.0f) {
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


bool GuidedMissileStrategy::predictPosition(float dt, fvec3& p) const
{
  fvec3 v;
  return _predict(dt, p, v);
}


bool GuidedMissileStrategy::predictVelocity(float dt, fvec3& v) const
{
  fvec3 p;
  return _predict(dt, p, v);
}


bool GuidedMissileStrategy::_predict(float dt, fvec3& p, fvec3& v) const
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
  if (isRemote) {
    if (lastTarget != NoPlayer)
      target = lookupPlayer(lastTarget);
  } else {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank)
      target = myTank->getTarget();
  }

  if ((target != NULL) && ((target->getFlag() == Flags::Stealth) || ((target->getStatus() & short(PlayerState::Alive)) == 0)))
    target = NULL;

  float tmpAzimuth = azimuth, tmpElevation = elevation;
  // compute next segment's ray
  if (target) {
    // turn towards target
    // find desired direction
    const fvec3& targetPos = target->getPosition();
    fvec3 desiredDir = targetPos - nextPos;
    desiredDir.z += target->getMuzzleHeight(); // right between the eyes

    // compute desired angles
    float newAzimuth   = atan2f(desiredDir.y, desiredDir.x);
    float newElevation = atan2f(desiredDir.z, desiredDir.xy().length());

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

  fvec3 newDirection;
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
  else {
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
      teleporter->getPointWRT(*outTeleporter, face, outFace,
                              p, NULL, tmpAzimuth, p, NULL, &tmpAzimuth);
      eventHandler.ShotTeleported(getPath(), face, outFace); // FIXME
    }
    else if (building) {
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
    fvec3 pos;
    ray.getPoint(t / shotSpeed, pos);
    addShotExplosion(pos);
    return t / shotSpeed;
  }
  return -1.0f;
}

float GuidedMissileStrategy::checkHit(const ShotCollider& tank, fvec3& position) const
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

  fvec3 lastTankPositionRaw;
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
    float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
    const fvec3& dir = ray.getDirection();
    fvec3 speed = shotSpeed * dir;
    Ray speedRay(ray.getOrigin(), speed);

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(Intersect::rayMinusRay(speedRay, 0.0, tankLastMotion, 0.0));

    // get closest approach time
    float t;
    if (tank.test2D) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static const fvec3 tankBase(0.0f, 0.0f, -0.5f * tankHeight);
      t = Intersect::timeRayHitsBlock(relativeRay, tankBase, tank.angle,
                                      0.5f * tank.length, shotRadius, tankHeight);
    } else {
      // find time when shot hits sphere around tank
      t = Intersect::rayAtDistanceFromOrigin(relativeRay, 0.99f * tank.radius);
    }

    if (t > minTime)
      continue;

    // if not in shot segment times then no hit
    if (t < 0.0f || t > segments[i].end - segments[i].start)
      continue;

    // check if shot hits tank -- get position at time t, see if in radius
    fvec3 closestPos;
    relativeRay.getPoint(t, closestPos);
    if (closestPos.lengthSq() < radius2) {
      // save best time so far
      minTime = t;

      // compute location of tank at time of hit
      fvec3 tankPos;
      tank.motion.getPoint(t, tankPos);

      // compute position of intersection
      position = tankPos + closestPos;
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
  buf = nboPackUInt8(buf, lastTarget);
  ServerLink::getServer()->send(MsgGMUpdate, sizeof(packet), packet);
}

void GuidedMissileStrategy::readUpdate( void* msg)
{
  // position and velocity have been replaced by the remote system's
  // concept of the position and velocity.  this may cause a discontinuity
  // in the shot's position but it's probably better to have the shot in
  // the right place than to maintain smooth motion.  these updates ought
  // to be rare anyway.

  // read the lastTarget
  nboUnpackUInt8(msg, lastTarget);

  // fix up dependent variables
  const fvec3& vel = getPath().getVelocity();
  const fvec3 dir = vel.normalize();
  azimuth   = limitAngle(atan2f(dir.y, dir.x));
  elevation = limitAngle(atan2f(dir.z, dir.xy().length()));
  const fvec3& pos = getPath().getPosition();
  nextPos = pos;

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
  const fvec3& orig = getPath().getPosition();
  const int length = BZDBCache::linedRadarShots;
  const int size   = BZDBCache::sizedRadarShots;

  const float shotTailLength = BZDB.eval(StateDatabase::BZDB_SHOTTAILLENGTH);
  // Display leading lines
  if (length > 0) {
    const fvec3& vel = getPath().getVelocity();
    const fvec3 dir = vel.normalize() * shotTailLength * length;
    glBegin(GL_LINES); {
      glVertex2fv(orig);
      if (BZDBCache::leadingShotLine) {
	glVertex2fv(orig.xy() + dir.xy());
      } else {
	glVertex2fv(orig.xy() - dir.xy());
      }
    } glEnd();

    // draw a "bright reddish" missle tip
    if (size > 0) {
      glColor3f(1.0f, 0.75f, 0.75f);
      glPointSize((float)size);
      glBegin(GL_POINTS); {
	glVertex2fv(orig);
      } glEnd();
      glPointSize(1.0f);
    }
  } else {
    if (size > 0) {
      // draw a sized missle
      glPointSize((float)size);
      glBegin(GL_POINTS); {
	glVertex2fv(orig);
      } glEnd();
      glPointSize(1.0f);
    } else {
      // draw the tiny missle
      glBegin(GL_POINTS); {
	glVertex2fv(orig);
      } glEnd();
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
