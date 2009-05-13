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

// interface header
#include "GuidedMissleStrategy.h"

// common headers
#include "BZDBCache.h"
#include "TextureManager.h"
#include "LinkManager.h"
#include "Intersect.h"
#include "MeshFace.h"

// local headers
#include "LocalPlayer.h"
#include "World.h"
#include "playing.h"


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

  // setup shot
  speed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED) *
          BZDB.eval(StateDatabase::BZDB_GMADSPEED);
  const fvec3 dir = getPath().getVelocity().normalize();
  f.shot.vel = speed * dir;
  const fvec3& vel = getPath().getVelocity();

  // initialize segments
  currentTime = getPath().getStartTime();
  Ray ray = Ray(f.shot.pos, vel);
  ShotPathSegment segment(currentTime, currentTime, ray);
  segments.push_back(segment);
  segments.push_back(segment);
  segments.push_back(segment);
  segments.push_back(segment);

  // set next position to starting position
  nextPos = f.shot.pos;
  nextVel = f.shot.vel;

  // check that first segment doesn't start inside a building
  float muzzleFront = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  const fvec3 startPos = f.shot.pos - (muzzleFront * dir);

  Ray firstRay = Ray(startPos, vel);
  prevTime = currentTime;
  prevTime -= (muzzleFront / speed);
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
  if (isRemote && (dt < 0.0f)) {
    return;
  }

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
    if (lastTarget != NoPlayer) {
      target = lookupPlayer(lastTarget);
    }
  }
  else {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank) {
      target = myTank->getTarget();
    }
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

  if ((target != NULL) &&
      ((target->getFlag() == Flags::Stealth) ||
       ((target->getStatus() & short(PlayerState::Alive)) == 0))) {
    target = NULL;
    lastTarget = NoPlayer;
    needUpdate = true;
  }

  nextVel = getPath().getVelocity();

  // compute next segment's ray
  if (target) {
    // turn towards target
    fvec3 targetPos = target->getPosition();
    targetPos.z += target->getMuzzleHeight(); // right between the eyes
    const fvec3 desiredDir = (targetPos - nextPos).normalize();
    const fvec3 currentDir = nextVel.normalize();
    fvec3 cross = fvec3::cross(currentDir, desiredDir);
    float crossLen = cross.length();
    if (crossLen > 0.0f) {
      cross *= (1.0f / crossLen); // normalize
      const float gmissileAng = BZDB.eval(StateDatabase::BZDB_GMTURNANGLE);
      const float radDiff = asinf(crossLen);
      float radians = (gmissileAng * dt);
      if (radians > radDiff) {
        if (fvec3::dot(currentDir, desiredDir) > 0.0f) {
          radians = radDiff;
        }
      }
      nextVel = nextVel.rotate(radians, cross).normalize() * speed;
    }
  }

  Ray ray(nextPos, nextVel);

  renderTimes++;

  // Changed: GM smoke trail, leave it every seconds, none of this per frame crap
  if (currentTime - lastPuff > puffTime ) {
    lastPuff = currentTime;
    addShotPuff(nextPos, nextVel);
  }

  // get next position
  nextPos = ray.getPoint(dt);

  // see if we hit something
  double segmentEndTime = currentTime;

  if (nextPos.z <= 0.0f) {
    // hit ground -- expire it and shorten life of segment to time of impact
    setExpiring();
    float t = ray.getOrigin().z / (ray.getOrigin().z - nextPos.z);
    segmentEndTime = prevTime;
    segmentEndTime += t * (currentTime - prevTime);
    nextPos = ray.getPoint(t);
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
  setPosition(nextPos);
  setVelocity(nextVel);
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
  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  const bool isRemote =
    (getPath().getPlayer() != LocalPlayer::getMyTank()->getId());

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

  if ((target != NULL) &&
      ((target->getFlag() == Flags::Stealth) ||
       ((target->getStatus() & short(PlayerState::Alive)) == 0))) {
    target = NULL;
  }

  fvec3 tmpVel = getPath().getVelocity();

  // compute next segment's ray
  if (target) {
    // turn towards target
    fvec3 targetPos = target->getPosition();
    targetPos.z += target->getMuzzleHeight(); // right between the eyes
    const fvec3 desiredDir = (targetPos - nextPos).normalize();
    const fvec3 currentDir = tmpVel.normalize();
    fvec3 cross = fvec3::cross(currentDir, desiredDir);
    float crossLen = cross.length();
    if (crossLen > 0.0f) {
      cross *= (1.0f / crossLen); // normalize
      const float gmissileAng = BZDB.eval(StateDatabase::BZDB_GMTURNANGLE);
      const float radDiff = asinf(crossLen);
      float radians = (gmissileAng * dt);
      if (radians > radDiff) {
        if (fvec3::dot(currentDir, desiredDir) > 0.0f) {
          radians = radDiff;
        }
      }
      tmpVel = tmpVel.rotate(radians, cross).normalize() * speed;
    }
  }

  Ray ray = Ray(nextPos, tmpVel);

  // get next position
  p = ray.getPoint(dt);

  // see if we hit something
  if (p.z <= 0.0f) {
    return false;
  }

  // see if we hit a building
  float t = float(currentTime - prevTime);

  const Obstacle* building = getFirstBuilding(ray, Epsilon, t);

  // check for teleportation
  const MeshFace* linkSrc = MeshFace::getShotLinkSrc(building);
  const MeshFace* linkDst = NULL;
  const LinkPhysics* physics = NULL;;
  int linkSrcID, linkDstID;
  if (linkSrc != NULL) {
    const ShotPath& myPath = getPath();
    const FlagType* flagType = myPath.getFlag();
    const TeamColor teamNum = myPath.getTeam();
    const unsigned int seed = getPath().getShotId();
    linkDst = linkManager.getShotLinkDst(linkSrc, seed,
                                         linkSrcID, linkDstID, physics,
                                         p, tmpVel, teamNum, flagType);
    if (linkDst == NULL) {
      linkSrc = NULL;
    }
  }

  // check in reverse order to see what we hit first
  if (linkSrc) {
    // entered teleporter -- teleport it
    linkSrc->teleportShot(*linkDst, *physics, p, p, tmpVel, tmpVel);
  }
  else if (building) {
    // expire on next update
    return false;
  }

  // update shot
  v = tmpVel;

  return true;
}


float GuidedMissileStrategy::checkBuildings(const Ray& ray)
{
  World* world = World::getWorld();
  if (!world) {
    return -1.0f;
  }

  float t = float(currentTime - prevTime);
  const Obstacle* building = getFirstBuilding(ray, Epsilon, t);

  // check for teleportation
  const MeshFace* linkSrc = MeshFace::getShotLinkSrc(building);
  const MeshFace* linkDst = NULL;
  const LinkPhysics* physics = NULL;;
  int linkSrcID, linkDstID;
  if (linkSrc != NULL) {
    const ShotPath& myPath = getPath();
    const FlagType* flagType = myPath.getFlag();
    const TeamColor teamNum = myPath.getTeam();
    const unsigned int seed = getPath().getShotId();
    linkDst = linkManager.getShotLinkDst(linkSrc, seed,
                                         linkSrcID, linkDstID, physics,
                                         nextPos, nextVel, teamNum, flagType);
    if (linkDst == NULL) {
      if ((currentTime - getPath().getStartTime() + (double)t) < 1.0) {
        const MeshFace::SpecialData* sd = linkSrc->getSpecialData();
        const std::string& failMsg = sd->linkSrcShotFail;
        if (!failMsg.empty()) {
          addMessage(NULL, failMsg);
        }
      }
      linkSrc = NULL; // disable teleporting
    }
  }

  // check in reverse order to see what we hit first
  if (linkSrc) {
    // entered teleporter -- teleport it
    fvec3 vel = getPath().getVelocity();
    linkSrc->teleportShot(*linkDst, *physics, nextPos, nextPos,
                                              nextVel, nextVel);
    return t;
  }
  else if (building) {
    // expire on next update
    setExpiring();
    addShotExplosion(ray.getPoint(t));
    return t;
  }
  return -1.0f;
}


float GuidedMissileStrategy::checkHit(const ShotCollider& tank, fvec3& position) const
{
  float minTime = Infinity;
  if (getPath().isExpired()) {
    return minTime;
  }

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

  fvec3 lastTankPositionRaw = tank.motion.getOrigin();
  lastTankPositionRaw.z += (0.5f * tankHeight);

  Ray tankLastMotion(lastTankPositionRaw, tank.motion.getDirection());

  // check each segment
  const size_t numSegments = segments.size();
  size_t i = 0;
  // only test most recent segment if shot is from my tank
  if ((numSegments > 1) && tank.testLastSegment) {
    i = numSegments - 1;
  }
  for (; i < numSegments; i++) {
    const Ray& ray = segments[i].ray;

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(Intersect::rayMinusRay(ray, 0.0, tankLastMotion, 0.0));

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

    if (t > minTime) {
      continue;
    }

    // if not in shot segment times then no hit
    if ((t < 0.0f) || (t > (segments[i].end - segments[i].start))) {
      continue;
    }

    // check if shot hits tank -- get position at time t, see if in radius
    fvec3 closestPos = relativeRay.getPoint(t);
    if (closestPos.lengthSq() < radius2) {
      // save best time so far
      minTime = t;

      // compute location of tank at time of hit
      fvec3 tankPos = tank.motion.getPoint(t);

      // compute position of intersection
      position = tankPos + closestPos;
    }
  }

  return minTime;
}


void GuidedMissileStrategy::sendUpdate(const FiringInfo& firingInfo) const
{
  // only send an update when needed
  if (!needUpdate) {
    return;
  }
  ((GuidedMissileStrategy*)this)->needUpdate = false;

  // construct and send packet
  char packet[ShotUpdatePLen + PlayerIdPLen];
  void *buf = (void*)packet;
  buf = firingInfo.shot.pack(buf);
  buf = nboPackUInt8(buf, lastTarget);
  ServerLink::getServer()->send(MsgGMUpdate, sizeof(packet), packet);
}


void GuidedMissileStrategy::readUpdate(void* msg)
{
  // position and velocity have been replaced by the remote system's
  // concept of the position and velocity.  this may cause a discontinuity
  // in the shot's position but it's probably better to have the shot in
  // the right place than to maintain smooth motion.  these updates ought
  // to be rare anyway.

  // read the lastTarget
  nboUnpackUInt8(msg, lastTarget);

  nextPos = getPath().getPosition();
  nextVel = getPath().getVelocity();

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
    const fvec3 dir = vel.normalize() * shotTailLength * (float)length;
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
