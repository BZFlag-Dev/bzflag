/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include "common.h"
#include "playing.h"
#include "ShotStrategy.h"
#include "World.h"
#include "Intersect.h"
#include "WallObstacle.h"
#include "LocalPlayer.h"
#include "SceneDatabase.h"
#include "BoltSceneNode.h"
#include "SphereSceneNode.h"
#include "LaserSceneNode.h"
#include "ServerLink.h"
#include "sound.h"
#include "OpenGLTexture.h"
#include "Team.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TextureManager.h"


ShotStrategy::ShotStrategy(ShotPath* _path) :
				path(_path)
{
  // do nothing
}

ShotStrategy::~ShotStrategy()
{
  // do nothing
}

bool			ShotStrategy::isStoppedByHit() const
{
  return true;
}

void			ShotStrategy::sendUpdate(const FiringInfo&) const
{
  // do nothing by default -- normal shots don't need updates
}

void			ShotStrategy::readUpdate(uint16_t, void*)
{
  // do nothing by default -- normal shots don't need updates
}

void			ShotStrategy::expire()
{
  // do nothing by default
}

void			ShotStrategy::setReloadTime(float t) const
{
  path->setReloadTime(t);
}

void			ShotStrategy::setPosition(const float* p) const
{
  path->setPosition(p);
}

void			ShotStrategy::setVelocity(const float* v) const
{
  path->setVelocity(v);
}

void			ShotStrategy::setExpiring() const
{
  path->setExpiring();
}

void			ShotStrategy::setExpired() const
{
  path->setExpired();
}

FiringInfo&		ShotStrategy::getFiringInfo(ShotPath* p) const
{
  return p->getFiringInfo();
}

const Obstacle*		ShotStrategy::getFirstBuilding(const Ray& ray,
						float min, float& t)
{
  const Obstacle* closestObstacle = NULL;

  // check walls
  {
    const std::vector<WallObstacle> &walls = World::getWorld()->getWalls();
    std::vector<WallObstacle>::const_iterator it = walls.begin();
    while (it != walls.end()) {
      const WallObstacle& wall = *it;
      if (!wall.isShootThrough()) {
	const float wallt = wall.intersect(ray);
	if (wallt > min && wallt < t) {
	  t = wallt;
	  closestObstacle = &wall;
	}
      }
      it++;
    }
  }

  // check teleporter borders
  {
    const std::vector<Teleporter> &teleporters = World::getWorld()->getTeleporters();
    std::vector<Teleporter>::const_iterator it = teleporters.begin();
    while (it != teleporters.end()) {
      const Teleporter& teleporter = *it;
	  if (!teleporter.isShootThrough()){
      const float telet = teleporter.intersect(ray);
      int face;
      if (telet > min && telet < t && teleporter.isTeleported(ray, face) < 0.0f) {
	t = telet;
	closestObstacle = &teleporter;
      }
	  }
      it++;
    }
  }

  // check boxes
  {
    const std::vector<BoxBuilding> &boxes = World::getWorld()->getBoxes();
    std::vector<BoxBuilding>::const_iterator it = boxes.begin();
    while (it != boxes.end()) {
      const BoxBuilding& box = *it;
	  if (!box.isShootThrough()){
      const float boxt = box.intersect(ray);
      if (boxt > min && boxt < t) {
	t = boxt;
	closestObstacle = &box;
      }
	  }
      it++;
    }
  }

  // check bases
  {
    const std::vector<BaseBuilding> &bases = World::getWorld()->getBases();
    std::vector<BaseBuilding>::const_iterator it = bases.begin();
    while (it != bases.end()) {
      const BaseBuilding& base = *it;
	  if (!base.isShootThrough()){
      const float baset = base.intersect(ray);
      if (baset > min && baset < t) {
	t = baset;
	closestObstacle = &base;
      }
	  }
      it++;
    }
  }

  // check pyramids
  {
    const std::vector<PyramidBuilding> &pyramids = World::getWorld()->getPyramids();
    std::vector<PyramidBuilding>::const_iterator it = pyramids.begin();
    while (it != pyramids.end()) {
      const PyramidBuilding& pyramid = *it;
	  if (!pyramid.isShootThrough()){
      const float pyramidt = pyramid.intersect(ray);
      if (pyramidt > min && pyramidt < t) {
	t = pyramidt;
	closestObstacle = &pyramid;
      }
	  }
      it++;
    }
  }

  return closestObstacle;
}

void			ShotStrategy::reflect(float* v,
							const float* n) // const
{
  // normal is assumed to be normalized, v needn't be
  const float d = -2.0f * (n[0] * v[0] + n[1] * v[1] + n[2] * v[2]);
  v[0] += d * n[0];
  v[1] += d * n[1];
  v[2] += d * n[2];
}


const Teleporter*	ShotStrategy::getFirstTeleporter(const Ray& ray,
					float min, float& t, int& f) const
{
  const Teleporter* closestTeleporter = NULL;
  int face;

  {
    const std::vector<Teleporter> &teleporters = World::getWorld()->getTeleporters();
    std::vector<Teleporter>::const_iterator it = teleporters.begin();
    while (it != teleporters.end()) {
      const Teleporter& teleporter = *it;
      const float telet = teleporter.isTeleported(ray, face);
      if (telet > min && telet < t) {
	t = telet;
	f = face;
	closestTeleporter = &teleporter;
      }
      it++;
    }
  }

  return closestTeleporter;
}

bool		ShotStrategy::getGround(const Ray& r, float min, float &t) const
{
  if (r.getDirection()[2] >= 0.0f)
    return false;

  float groundT = r.getOrigin()[2] / -r.getDirection()[2];
  if ((groundT > min) && (groundT < t))
  {
    t = groundT;
    return true;
  }
  return false;
}

//
// GuidedMissileStrategy
//

static float		limitAngle(float a)
{
  if (a < -M_PI) a += 2.0f * M_PI;
  else if (a >= M_PI) a -= 2.0f * M_PI;
  return a;
}

GuidedMissileStrategy::GuidedMissileStrategy(ShotPath* _path) :
				ShotStrategy(_path),
				renderTimes(0),
				needUpdate(true)
{
  ptSceneNode = new BoltSceneNode(_path->getPosition());
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
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_GMISSILEADLIFE);
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
}

GuidedMissileStrategy::~GuidedMissileStrategy()
{
  delete ptSceneNode;
}

// NOTE -- ray is base of shot segment and normalized direction of flight.
//	distance traveled is ShotSpeed * dt.

void			GuidedMissileStrategy::update(float dt)
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
  if (!isRemote &&
	currentTime - getPath().getStartTime() >= getPath().getLifetime()) {
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
  }
  else {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank) target = myTank->getTarget();

    // see if the target changed
    if (target) {
      if (lastTarget != target->getId()) {
	needUpdate = true;
	lastTarget = target->getId();
      }
    }
    else {
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
    const float* targetPos = target->getPosition();
    float desiredDir[3];
    desiredDir[0] = targetPos[0] - nextPos[0];
    desiredDir[1] = targetPos[1] - nextPos[1];
    desiredDir[2] = targetPos[2] - nextPos[2];
    desiredDir[2] += BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);	// aim for turret

    // compute desired angles
    float newAzimuth = atan2f(desiredDir[1], desiredDir[0]);
    float newElevation = atan2f(desiredDir[2],
				hypotf(desiredDir[1], desiredDir[0]));

    float gmissileAng = BZDB.eval(StateDatabase::BZDB_GMISSILEANG);

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

  // Changed: GM leave smoke trail, call add puff every 3 updates
  if ((++renderTimes % 3) == 0) addShotPuff(nextPos);

  // get next position
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  ray.getPoint(dt * shotSpeed, nextPos);

  // see if we hit something
  TimeKeeper segmentEndTime(currentTime);
  /* if (!isRemote) */ {
    if (nextPos[2] <= 0.0f) {
      // hit ground -- expire it and shorten life of segment to time of impact
      setExpiring();
      float t = ray.getOrigin()[2] / (ray.getOrigin()[2] - nextPos[2]);
      segmentEndTime = prevTime;
      segmentEndTime += t * (currentTime - prevTime);
      ray.getPoint(t / shotSpeed, nextPos);
      addShotExplosion(nextPos);
    }

    // see if we hit a building
    else {
      const float t = checkBuildings(ray);
      if (t >= 0.0f) {
	segmentEndTime = prevTime;
	segmentEndTime += t;
      }
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

float			GuidedMissileStrategy::checkBuildings(const Ray& ray)
{
  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  float t = (currentTime - prevTime) * shotSpeed;
  int face;
  const Obstacle* building = getFirstBuilding(ray, Epsilon, t);
  const Teleporter* teleporter = getFirstTeleporter(ray, Epsilon, t, face);

  // check in reverse order to see what we hit first
  if (teleporter) {
    // entered teleporter -- teleport it
    int source = World::getWorld()->getTeleporter(teleporter, face);
    int target = World::getWorld()->getTeleportTarget(source);
    int outFace;
    const Teleporter* outTeleporter =
			World::getWorld()->getTeleporter(target, outFace);
    teleporter->getPointWRT(*outTeleporter, face, outFace,
			nextPos, NULL, azimuth, nextPos, NULL, &azimuth);
    return t / shotSpeed;
  }

  else if (building) {
    // expire on next update
    setExpiring();
    float pos[3];
    ray.getPoint(t / shotSpeed, pos);
    addShotExplosion(pos);
    return t / shotSpeed;
  }
  return -1.0f;
}

float			GuidedMissileStrategy::checkHit(const BaseLocalPlayer* tank,
							float position[3]) const
{
  float minTime = Infinity;
  if (getPath().isExpired()) return minTime;

  // can't shoot myself for first 1/2 second (kludge!)
  if (((TimeKeeper::getCurrent() - getPath().getStartTime()) < 0.5) &&
      (tank->getId() == getPath().getPlayer()))
    return minTime;

  // get tank radius
  float radius = BZDBCache::tankRadius;
  if (tank->getFlag() == Flags::Obesity)   radius *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  else if (tank->getFlag() == Flags::Tiny) radius *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  else if (tank->getFlag() == Flags::Thief) radius *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  const float radius2 = radius * radius;

  float shotRadius = BZDB.eval(StateDatabase::BZDB_SHOTRADIUS);

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  Ray tankLastMotionRaw = tank->getLastMotion();
  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tankLastMotionRaw.getOrigin()[0];
  lastTankPositionRaw[1] = tankLastMotionRaw.getOrigin()[1];
  lastTankPositionRaw[2] = tankLastMotionRaw.getOrigin()[2] + 0.5f * BZDBCache::tankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tankLastMotionRaw.getDirection());

  // check each segment
  const int numSegments = segments.size();
  int i = 0;
  // only test most recent segment if shot is from my tank
  if (numSegments > 1 && tank->getId() == getPath().getPlayer())
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
    if (tank->getFlag() == Flags::Narrow) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float origin[3] = { 0.0f, 0.0f, 0.0f };
      t = timeRayHitsBlock(relativeRay, origin, tank->getAngle(),
		      0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH),
		      shotRadius,
		      BZDBCache::tankHeight);
    }
    else {
      // find time when shot hits sphere around tank
      t = rayAtDistanceFromOrigin(relativeRay, 0.99f * radius);
    }
    if (t > minTime) continue;

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
      tank->getLastMotion().getPoint(t, tankPos);

      // compute position of intersection
      position[0] = tankPos[0] + closestPos[0];
      position[1] = tankPos[1] + closestPos[1];
      position[2] = tankPos[2] + closestPos[2];
    }
  }

  return minTime;
}

void			GuidedMissileStrategy::sendUpdate(
				const FiringInfo& firingInfo) const
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

void			GuidedMissileStrategy::readUpdate(
				uint16_t code, void* msg)
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

void			GuidedMissileStrategy::addShot(
				SceneDatabase* scene, bool)
{
  ptSceneNode->move(getPath().getPosition(), getPath().getVelocity());
  scene->addDynamicNode(ptSceneNode);
}

void			GuidedMissileStrategy::expire()
{
  if (getPath().getPlayer() == LocalPlayer::getMyTank()->getId()) {
    const ShotPath& shot = getPath();
    /* NOTE -- change 0 to 1 to not explode when shot expires (I think) */
    ServerLink::getServer()->sendEndShot(shot.getPlayer(), shot.getShotId(), 0);
  }
}

void			GuidedMissileStrategy::radarRender() const
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
    glVertex2f(orig[0] + dir[0], orig[1] + dir[1]);
    glEnd();

    // draw a "bright reddish" missle tip
    if (size > 0) {
      glColor3f(1.0f, 0.75f, 0.75f);
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2f(orig[0] + dir[0], orig[1] + dir[1]);
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

//
// ShockWaveStrategy
//

ShockWaveStrategy::ShockWaveStrategy(ShotPath* path) :
				ShotStrategy(path),
				radius(BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS)),
				radius2(radius * radius)
{
  // setup shot
  FiringInfo& f = getFiringInfo(path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_SHOCKADLIFE);

  // make scene node
  shockNode = new SphereSceneNode(path->getPosition(), radius);
  Player* p = lookupPlayer(path->getPlayer());
  TeamColor team = p ? p->getTeam() : RogueTeam;
  const float* c = Team::getRadarColor(team);
  shockNode->setColor(c[0], c[1], c[2], 0.75f);
}

ShockWaveStrategy::~ShockWaveStrategy()
{
  delete shockNode;
}

void			ShockWaveStrategy::update(float dt)
{
  radius += dt * (BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) - BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS)) / getPath().getLifetime();
  radius2 = radius * radius;

  // update shock wave scene node
  const GLfloat frac = (radius - BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS)) /
			(BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) - BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS));
  shockNode->move(getPath().getPosition(), radius);
  Player* p = lookupPlayer(getPath().getPlayer());
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  TeamColor team = p && !(myTank->getFlag() == Flags::Colorblindness) ? p->getTeam() : RogueTeam;
  const float* c = Team::getRadarColor(team);
  shockNode->setColor(c[0], c[1], c[2], 0.75f - 0.5f * frac);

  // expire when full size
  if (radius >= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS)) setExpired();
}

float			ShockWaveStrategy::checkHit(const BaseLocalPlayer* tank,
							float position[3]) const
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
  }
  else
    return Infinity;
}

bool			ShockWaveStrategy::isStoppedByHit() const
{
  return false;
}

void			ShockWaveStrategy::addShot(
				SceneDatabase* scene, bool)
{
  scene->addDynamicSphere(shockNode);
}

void			ShockWaveStrategy::radarRender() const
{
  // draw circle of current radius
  static const int sides = 20;
  const float* shotPos = getPath().getPosition();
  glBegin(GL_LINE_LOOP);
    for (int i = 0; i < sides; i++) {
      const float angle = 2.0f * M_PI * float(i) / float(sides);
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
