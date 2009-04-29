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
#include "SegmentedShotStrategy.h"

/* system implementation headers */
#include <assert.h>

/* common implementation headers */
#include "TextureManager.h"
#include "Intersect.h"
#include "BZDBCache.h"
#include "WallObstacle.h"
#include "LinkManager.h"
#include "MeshFace.h"

/* local implementation headers */
#include "sound.h"
#include "LocalPlayer.h"
#include "World.h"
#include "EffectsRenderer.h"
#include "playing.h"


static BZDB_int debugShotSegments("debugShotSegments");


SegmentedShotStrategy::SegmentedShotStrategy(ShotPath* _path,
                                             bool useSuperTexture, bool faint)
: ShotStrategy(_path)
{
  // initialize times
  prevTime = getPath().getStartTime();
  lastTime = currentTime = prevTime;

  // start at first segment
  lastSegment = segment = 0;

  // get team
  if (_path->getPlayer() == ServerPlayer) {
    TeamColor tmpTeam = _path->getFiringInfo().shot.team;
    team = (tmpTeam < RogueTeam) ? RogueTeam :
	   (tmpTeam > HunterTeam) ? RogueTeam : tmpTeam;
  } else {
    Player* p = lookupPlayer(_path->getPlayer());
    team = p ? p->getTeam() : RogueTeam;
  }

  // initialize scene nodes
  if (!headless) {
    boltSceneNode = new BoltSceneNode(_path->getPosition(),_path->getVelocity());

    const fvec4& color = Team::getRadarColor(team);
    if (faint) {
      boltSceneNode->setColor(color.r, color.g, color.b, 0.2f);
      boltSceneNode->setTextureColor(1.0f, 1.0f, 1.0f, 0.3f);
    } else {
      boltSceneNode->setColor(color.r, color.g, color.b, 1.0f);
    }
    if (_path->getShotType() == CloakedShot) {
      const Player* myTank = LocalPlayer::getMyTank();
      if ((myTank == NULL) ||
          ((myTank->getId() != _path->getPlayer()) &&
           (myTank->getFlag() != Flags::Seer))) {
        boltSceneNode->setInvisible(true);
      }
    }

    TextureManager &tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(team);
    if (useSuperTexture)
      imageName += BZDB.get("superPrefix");
    imageName += BZDB.get("boltTexture");

    boltSceneNode->phasingShot = useSuperTexture;

    int texture = tm.getTextureID(imageName);
    if (texture >= 0)
      boltSceneNode->setTexture(texture);
  }
}


SegmentedShotStrategy::~SegmentedShotStrategy()
{
  // free scene nodes
  if (!headless) {
    delete boltSceneNode;
  }
}


void SegmentedShotStrategy::update(float dt)
{
  prevTime = currentTime;
  currentTime += dt;

  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  if ((segment < numSegments) && (segments[segment].end <= currentTime)) {

    lastSegment = segment;

    while ((segment < numSegments) && (segments[segment].end <= currentTime)) {

      segment++;

      if (segment < numSegments) {

        const ShotPathSegment& segm = segments[segment];

	switch (segm.reason) {
	  case ShotPathSegment::Boundary: {
	    break;
          }
          case ShotPathSegment::Through: {
            break;
          }
          case ShotPathSegment::Initial: {
            break;
          }
	  case ShotPathSegment::Ricochet: {
            // play ricochet sound.  ricochet of local player's shots
            // are important, others are not.
            const PlayerId myTankId = LocalPlayer::getMyTank()->getId();
            const bool important = (getPath().getPlayer() == myTankId);
            const fvec3& pos = segm.ray.getOrigin();
            SOUNDSYSTEM.play(SFX_RICOCHET, pos, important, false);

            // this is fugly but it's what we do
            const fvec3& newDir = segm.ray.getDirection();
            const fvec3& oldDir = segments[segment - 1].ray.getDirection();
            const fvec3 normal = (newDir - oldDir).normalize();

            if (!segm.noEffect) {
              EFFECTS.addRicoEffect(pos, normal);
            }
            break;
          }
          case ShotPathSegment::Teleport: {
            if (!segm.noEffect) {
              const fvec4* clipPlane = &segm.dstFace->getPlane();
              EFFECTS.addShotTeleportEffect(segm.ray.getOrigin(),
                                            segm.ray.getDirection(),
                                            clipPlane);
            }
	    break;
          }
	}
      }
    }
  }

  // if ran out of segments then expire shot on next update
  if (segment == numSegments) {
    setExpiring();
    if (numSegments > 0) {
      const ShotPathSegment& segm = segments[numSegments - 1];
      fvec3 pos;
      segm.ray.getPoint(float(segm.end - segm.start), pos);
      addShotExplosion(pos);
    }
  }
  else {
    // otherwise update position and velocity
    fvec3 p;
    segments[segment].ray.getPoint(float(currentTime - segments[segment].start), p);
    setPosition(p);
    setVelocity(segments[segment].ray.getDirection());
  }
}


bool SegmentedShotStrategy::predictPosition(float dt, fvec3& p) const
{
  float ctime = (float)currentTime + dt;
  int cur=0;
  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  while (cur < numSegments && segments[cur].end < ctime) cur++;
  if (cur >= numSegments) return false;

  segments[segment].ray.getPoint(float(ctime - segments[segment].start), p);

  return true;
}


bool SegmentedShotStrategy::predictVelocity(float dt, fvec3& p) const
{
  float ctime = (float)currentTime + dt;
  int cur = 0;
  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  while (cur < numSegments && segments[cur].end < ctime) { cur++; }
  if (cur >= numSegments) { return false; }

  p = segments[segment].ray.getDirection();

  return true;
}


void SegmentedShotStrategy::setCurrentTime(const double _currentTime)
{
  currentTime = _currentTime;
}


double	SegmentedShotStrategy::getLastTime() const
{
  return lastTime;
}


void SegmentedShotStrategy::setCurrentSegment(int _segment)
{
  segment = _segment;
}


float SegmentedShotStrategy::checkHit(const ShotCollider& tank,
                                      fvec3& position) const
{
  float minTime = Infinity;
  // expired shot can't hit anything
  if (getPath().isExpired()) {
    return minTime;
  }

  // get tank radius
  const float radius2 = tank.radius * tank.radius;

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  const float tankHeight = tank.size.z;
  fvec3 lastTankPositionRaw = tank.motion.getOrigin();
  lastTankPositionRaw.z += 0.5f * tankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tank.motion.getDirection());

  // if bounding box of tank and entire shot doesn't overlap then no hit
  const Extents& tankBBox = tank.bbox;
  if (!bbox.touches(tankBBox)) {
    return minTime;
  }

  float shotRadius = BZDB.eval(StateDatabase::BZDB_SHOTRADIUS);

  // check each segment in interval (prevTime,currentTime]
  const float dt = float(currentTime - prevTime);
  const int numSegments = (const int)segments.size();
  for (int i = lastSegment; i <= segment && i < numSegments; i++) {
    // can never hit your own first laser segment
    if ((i == 0) && tank.testLastSegment &&
        (getPath().getShotType() == LaserShot)) {
      continue;
    }

/*
    // skip segments that don't overlap in time with current interval
    if (segments[i].end <= prevTime) continue;
    if (currentTime <= segments[i].start) break;
*/

    // if shot segment and tank bboxes don't overlap then no hit, or if it's a shot that is out of the world boundry
    const ShotPathSegment& s = segments[i];
    if (!s.bbox.touches(tankBBox) || (s.reason == ShotPathSegment::Boundary)) {
      continue;
    }

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(Intersect::rayMinusRay(s.ray, float(prevTime - s.start), tankLastMotion, 0.0f));

    // get hit time
    float t;
    if (tank.test2D) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static fvec3 tankBase(0.0f, 0.0f, -0.5f * tankHeight);
      t = Intersect::timeRayHitsBlock(relativeRay, tankBase, tank.angle,
			0.5f * tank.length, shotRadius, tankHeight);
    } else {
      // find time when shot hits sphere around tank
      t = Intersect::rayAtDistanceFromOrigin(relativeRay, 0.99f * tank.radius);
    }

    // short circuit if time is greater then smallest time so far
    if (t > minTime) continue;

    // make sure time falls within segment
    if ((t < 0.0f) || (t > dt)) continue;
    if (t > (s.end - prevTime)) continue;

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
      //printf("%u:%u %u:%u\n", tank->getId().port, tank->getId().number, getPath().getPlayer().port, getPath().getPlayer().number);
    }
  }

  return minTime;
}


void SegmentedShotStrategy::addShot(SceneDatabase* scene, bool colorblind)
{
  const ShotPath& shotPath = getPath();
  boltSceneNode->move(shotPath.getPosition(), shotPath.getVelocity());
  if (boltSceneNode->getColorblind() != colorblind) {
    boltSceneNode->setColorblind(colorblind);
    TeamColor currentTeam = colorblind ? RogueTeam : team;

    const fvec4& color = Team::getRadarColor(currentTeam);
    boltSceneNode->setColor(color.r, color.g, color.b);

    TextureManager &tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(currentTeam);
    imageName += BZDB.get("boltTexture");
    int texture = tm.getTextureID(imageName);
    if (texture >= 0)
      boltSceneNode->setTexture(texture);
  }
  scene->addDynamicNode(boltSceneNode);
}


void SegmentedShotStrategy::radarRender() const
{
  const fvec3& orig = getPath().getPosition();
  const int length  = BZDBCache::linedRadarShots;
  const int size    = BZDBCache::sizedRadarShots;

  float shotTailLength = BZDB.eval(StateDatabase::BZDB_SHOTTAILLENGTH);

  // Display leading lines
  if (length > 0) {
    const fvec3& vel = getPath().getVelocity();
    const fvec3  dir = vel.normalize() * shotTailLength * (float)length;
    glBegin(GL_LINES);
    glVertex2fv(orig);
    if (BZDBCache::leadingShotLine) {
      glVertex2fv(orig.xy() + dir.xy());
    } else {
      glVertex2fv(orig.xy() - dir.xy());
    }
    glEnd();

    // draw a "bright" bullet tip
    if (size > 0) {
      glColor3f(0.75, 0.75, 0.75);
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
      glPointSize(1.0f);
    }
  } else {
    if (size > 0) {
      // draw a sized bullet
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
      glPointSize(1.0f);

    } else {
      // draw the tiny little bullet
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
    }
  }

}


void SegmentedShotStrategy::makeSegments(ObstacleEffect e)
{
  segments.clear();

  World* world = World::getWorld();
  if (!world) {
    return; // no world, no shots
  }

  bool teleFail = false;

  // compute segments of shot until total length of segments exceeds the
  // lifetime of the shot.
  const ShotPath& shotPath = getPath();

  fvec3  vel       = shotPath.getVelocity();
  double startTime = shotPath.getStartTime();
  float  timeLeft  = shotPath.getLifetime();

  // minTime is used to move back to the tank's origin during the first for loop
  float minTime = 0.0f;
  const float speed = vel.length();
  if (speed > 0.0f) {
    minTime = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT) / speed;
  }

  // if all shots ricochet and obstacle effect is stop, then make it ricochet
  if ((e == Stop) && world->allShotsRicochet()) {
    e = Reflect;
  }

  // prepare first segment
  fvec3 orig = shotPath.getPosition();

  ShotPathSegment::Reason reason = ShotPathSegment::Initial;
  int linkSrcID = -1;
  int linkDstID = -1;
  const MeshFace* dstFace = NULL;
  bool noEffect = false;

  const int   maxSegment = 100;
  const float worldSize = (BZDBCache::worldSize * 0.5f) - 0.01f;

  for (int i = 0; (i < maxSegment) && (timeLeft > Epsilon); i++) {
    // construct ray and find the first building, teleporter, or outer wall
    fvec3 testOrig = orig - (minTime * vel);
    // Sometime shot start outside world
    if (testOrig.x <= -worldSize) { testOrig.x = -worldSize; }
    if (testOrig.x >= +worldSize) { testOrig.x = +worldSize; }
    if (testOrig.y <= -worldSize) { testOrig.y = -worldSize; }
    if (testOrig.y >= +worldSize) { testOrig.y = +worldSize; }

    Ray testRay(testOrig, vel);
    float t = timeLeft + minTime;
    const bool hitGround = getGround(testRay, Epsilon, t);
    const Obstacle* building =
      (e == Through) ? getFirstLinkSrc(testRay, Epsilon, t)
                     : getFirstBuilding(testRay, Epsilon, t);

    
    t -= minTime;
    minTime = 0.0f; // only used the first time around the loop

    // construct next shot segment and add it to list
    const double endTime = startTime + double((t < 0.0f) ? Epsilon : t);
    const Ray startRay(orig, vel);
    ShotPathSegment segm(startTime, endTime, startRay, reason);
    if (reason == ShotPathSegment::Teleport) {
      segm.linkSrcID = linkSrcID;
      segm.linkDstID = linkDstID;
      segm.dstFace   = dstFace;
      segm.noEffect  = noEffect;
    }
    segments.push_back(segm);
    startTime = endTime;
    linkSrcID = -1;
    linkDstID = -1;
    dstFace = NULL;
    noEffect = false;

    const fvec3 nextOrig = (orig + (t * vel));

    // if hit outer wall with ricochet and the hit is
    // above the top of the wall then ignore the hit.
    const bool ignoreHit = (building != NULL) && (e == Reflect) &&
                           (building->getTypeID() == wallType) &&
	                   (nextOrig.z > building->getHeight());

    // check for teleportation
    const MeshFace* linkSrc = MeshFace::getShotLinkSrc(building);
    const MeshFace* linkDst = NULL;
    const LinkPhysics* physics = NULL;;
    if (linkSrc != NULL) {
      const ShotPath& myPath = getPath();
      const FlagType* flagType = myPath.getFlag();
      const TeamColor teamNum = myPath.getTeam();
      const unsigned int seed = shotPath.getShotId() + i;
      linkDst = linkManager.getShotLinkDst(linkSrc, seed,
                                           linkSrcID, linkDstID, physics,
                                           nextOrig, vel, teamNum, flagType);
      if (linkDst == NULL) {
        if (!teleFail) {
          if ((startTime - shotPath.getStartTime() + (double)t) < 1.0) {
            const MeshFace::SpecialData* sd = linkSrc->getSpecialData();
            const std::string& failMsg = sd->linkSrcShotFail;
            if (!failMsg.empty()) {
              addMessage(NULL, failMsg);
              teleFail = true; // only one message per shot
            }
          }
        }
        linkSrc = NULL; // disable teleporting
      }
    }

    // used up this much time in segment
    timeLeft -= (t < 0.0f) ? Epsilon : t;

    // check in reverse order to see what we hit first
    reason = ShotPathSegment::Through;

    if (ignoreHit) {
      // uh...ignore this.  usually used if you shoot over the boundary wall.
      // just move the point of origin and build the next segment
      orig = nextOrig;
      reason = ShotPathSegment::Boundary;
      timeLeft = 0.0f;
    }
    else if (linkSrc) {
      // move origin to point of teleport
      orig = nextOrig;
      // entered teleporter -- teleport it
      linkSrc->teleportShot(*linkDst, *physics, orig, orig, vel, vel);
      reason = ShotPathSegment::Teleport;
      dstFace = linkDst;
      noEffect = linkSrc->linkSrcNoEffect();
    }
    else if (building) {
      // hit building -- can bounce off or stop, buildings ignored for Through
      switch (e) {
	case Stop: {
	  if (!building->canRicochet()) {
            timeLeft = 0.0f;
            break;
          } else {
            // pass-through to the Reflect case
          }
        }
        case Reflect: {
          // move origin to point of reflection
          orig = nextOrig;

          // reflect direction about normal to building
          fvec3 normal;
          building->get3DNormal(orig, normal);
          reflect(vel, normal);
          reason = ShotPathSegment::Ricochet;
          break;
        }
        case Through: {
          assert(0);
          break;
        }
      }
    }
    else if (hitGround)	{ // we hit the ground
      switch (e) {
	case Stop:
	case Through: {
	  timeLeft = 0.0f;
	  break;
        }
	case Reflect: {
	  // move origin to point of reflection
          orig = nextOrig;

	  // reflect direction about normal to ground
	  const fvec3 zPos(0.0f, 0.0f, 1.0f);
	  reflect(vel, zPos);
	  reason = ShotPathSegment::Ricochet;
	  break;
	}
      }
    }
  }

  lastTime = startTime;

  // make bounding box for entire path
  const size_t numSegments = segments.size();
  if (numSegments > 0) {
    bbox = segments[0].bbox;
    for (size_t j = 1; j < numSegments; ++j) {
      bbox.expandToBox(segments[j].bbox);
    }
  } else {
    bbox.reset();
  }

  if ((debugShotSegments >= 1) && !BZDBCache::forbidDebug) {
    logDebugMessage(0, "\n");
    logDebugMessage(0, "SegShotStrategy %i\n", (int)getPath().getFiringInfo().shot.id);
    for (size_t s = 0; s< segments.size(); s++) {
      const ShotPathSegment& sps = segments[s];
      const double segTime = sps.end - sps.start;
      const fvec3 endPos = sps.ray.getPoint(segTime);
      const std::string reasonStr = ShotPathSegment::getReasonString(sps.reason).c_str();
      logDebugMessage(0, "  segment %i\n", (int)s);
      logDebugMessage(0, "    start  %f\n", sps.start);
      logDebugMessage(0, "    end    %f\n", sps.end);
      logDebugMessage(0, "    orig   %s\n", sps.ray.getOrigin().tostring().c_str());
      logDebugMessage(0, "    endPos %s\n", endPos.tostring().c_str());
      logDebugMessage(0, "    dir    %s\n", sps.ray.getDirection().tostring().c_str());
      logDebugMessage(0, "    reason %s\n", reasonStr.c_str());
      logDebugMessage(0, "    mins   %s\n", sps.bbox.mins.tostring().c_str());
      logDebugMessage(0, "    maxs   %s\n", sps.bbox.maxs.tostring().c_str());
    }
    logDebugMessage(0, "  path mins: %s\n", bbox.mins.tostring().c_str());
    logDebugMessage(0, "  path maxs: %s\n", bbox.maxs.tostring().c_str());
  }
}


const std::vector<ShotPathSegment>& SegmentedShotStrategy::getSegments() const
{
  return segments;
}


//
// NormalShotStrategy
//

NormalShotStrategy::NormalShotStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false)
{
  // make segments
  makeSegments(Stop);
}


NormalShotStrategy::~NormalShotStrategy()
{
  // do nothing
}


//
// RapidFireStrategy
//

RapidFireStrategy::RapidFireStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_RFIREADLIFE);
  float fireAdVel = BZDB.eval(StateDatabase::BZDB_RFIREADVEL);
  f.shot.vel *= fireAdVel;
  setReloadTime(_path->getReloadTime()
		/ BZDB.eval(StateDatabase::BZDB_RFIREADRATE));

  // make segments
  makeSegments(Stop);
}


RapidFireStrategy::~RapidFireStrategy()
{
  // do nothing
}


//
// ThiefStrategy
//

ThiefStrategy::ThiefStrategy(ShotPath *_path) :
  SegmentedShotStrategy(_path, false),cumTime(0.0f)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_THIEFADLIFE);
  float thiefAdVel = BZDB.eval(StateDatabase::BZDB_THIEFADSHOTVEL);
  f.shot.vel *= thiefAdVel;
  setReloadTime(_path->getReloadTime()
		/ BZDB.eval(StateDatabase::BZDB_THIEFADRATE));

  // make segments
  makeSegments(Stop);
  setCurrentTime(getLastTime());
  endTime = f.lifetime;

  // make thief scene nodes
  const int numSegments = (const int)(getSegments().size());
  thiefNodes = new LaserSceneNode*[numSegments];

  TextureManager &tm = TextureManager::instance();
  int texture = tm.getTextureID("thief");

  for (int i = 0; i < numSegments; i++) {
    const ShotPathSegment& segm = getSegments()[i];
    const float t = float(segm.end - segm.start);
    const Ray& ray = segm.ray;
    const fvec3& rawdir = ray.getDirection();
    const fvec3 dir = t * rawdir;
    thiefNodes[i] = new LaserSceneNode(ray.getOrigin(), dir);
    if (texture >= 0) {
      thiefNodes[i]->setTexture(texture);
    }

    if (i == 0) {
      thiefNodes[i]->setFirst();
    }

    thiefNodes[i]->setColor(0,1,1);
    thiefNodes[i]->setCenterColor(0,0,0);
  }
  setCurrentSegment(numSegments - 1);
}


ThiefStrategy::~ThiefStrategy()
{
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++) {
    delete thiefNodes[i];
  }
  delete[] thiefNodes;
}


void ThiefStrategy::update(float dt)
{
  cumTime += dt;
  if (cumTime >= endTime) {
    setExpired();
  }
}


void ThiefStrategy::addShot(SceneDatabase* scene, bool)
{
  // thief is so fast we always show every segment
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++) {
    scene->addDynamicNode(thiefNodes[i]);
  }
}


void ThiefStrategy::radarRender() const
{
  // draw all segments
  const std::vector<ShotPathSegment>& segmts = getSegments();
  const size_t numSegments = segmts.size();
  glBegin(GL_LINES);
    for (size_t i = 0; i < numSegments; i++) {
      const ShotPathSegment& segm = segmts[i];
      const fvec3& origin = segm.ray.getOrigin();
      const fvec3& direction = segm.ray.getDirection();
      const float dt = float(segm.end - segm.start);
      glVertex2fv(origin);
      glVertex2fv(origin.xy() + (dt * direction.xy()));
    }
  glEnd();
}


bool ThiefStrategy::isStoppedByHit() const
{
  return false;
}


//
// MachineGunStrategy
//

MachineGunStrategy::MachineGunStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_MGUNADLIFE);
  float mgunAdVel = BZDB.eval(StateDatabase::BZDB_MGUNADVEL);
  f.shot.vel *= mgunAdVel;
  setReloadTime(_path->getReloadTime()
		/ BZDB.eval(StateDatabase::BZDB_MGUNADRATE));

  // make segments
  makeSegments(Stop);
}


MachineGunStrategy::~MachineGunStrategy()
{
  // do nothing
}


//
// RicochetStrategy
//

RicochetStrategy::RicochetStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false)
{
  // make segments that bounce
  makeSegments(Reflect);
}


RicochetStrategy::~RicochetStrategy()
{
  // do nothing
}


//
// SuperBulletStrategy
//

SuperBulletStrategy::SuperBulletStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, true)
{
  // make segments that go through buildings
  makeSegments(Through);
}

SuperBulletStrategy::~SuperBulletStrategy()
{
  // do nothing
}


//
// PhantomBulletStrategy
//

PhantomBulletStrategy::PhantomBulletStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false,true)
{
  // make segments that go through buildings
  makeSegments(Through);
}

PhantomBulletStrategy::~PhantomBulletStrategy()
{
  // do nothing
}


//
// LaserStrategy
//

LaserStrategy::LaserStrategy(ShotPath* _path) :
  SegmentedShotStrategy(_path, false), cumTime(0.0f)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(StateDatabase::BZDB_LASERADLIFE);
  float laserAdVel = BZDB.eval(StateDatabase::BZDB_LASERADVEL);
  f.shot.vel *= laserAdVel;
  setReloadTime(_path->getReloadTime()
		/ BZDB.eval(StateDatabase::BZDB_LASERADRATE));

  // make segments
  makeSegments(Stop);
  setCurrentTime(getLastTime());
  endTime = f.lifetime;

  // make laser scene nodes
  const int numSegments = (const int)(getSegments().size());
  laserNodes = new LaserSceneNode*[numSegments];
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  TeamColor tmpTeam = (myTank->getFlag() == Flags::Colorblindness) ? RogueTeam : team;

  TextureManager &tm = TextureManager::instance();
  std::string imageName = Team::getImagePrefix(tmpTeam);
  imageName += BZDB.get("laserTexture");
  int texture = tm.getTextureID(imageName);

  for (int i = 0; i < numSegments; i++) {
    const ShotPathSegment& segm = getSegments()[i];
    const float t = float(segm.end - segm.start);
    const Ray& ray = segm.ray;
    const fvec3& rawdir = ray.getDirection();
    const fvec3 dir = t * rawdir;
    laserNodes[i] = new LaserSceneNode(ray.getOrigin(), dir);
    if (texture >= 0)
      laserNodes[i]->setTexture(texture);

      const fvec4& color = Team::getRadarColor(tmpTeam);
      laserNodes[i]->setColor(color.r, color.g, color.b);

      if (i == 0) {
        laserNodes[i]->setFirst();
      }
  }
  setCurrentSegment(numSegments - 1);
}


LaserStrategy::~LaserStrategy()
{
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++) {
    delete laserNodes[i];
  }
  delete[] laserNodes;
}


void LaserStrategy::update(float dt)
{
  cumTime += dt;
  if (cumTime >= endTime) {
    setExpired();
  }
}


void LaserStrategy::addShot(SceneDatabase* scene, bool)
{
  // laser is so fast we always show every segment
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++) {
    scene->addDynamicNode(laserNodes[i]);
  }
}


void LaserStrategy::radarRender() const
{
  // draw all segments
  const std::vector<ShotPathSegment>& segmts = getSegments();
  const size_t numSegments = segmts.size();
  glBegin(GL_LINES);
  for (size_t i = 0; i < numSegments; i++) {
    const ShotPathSegment& segm = segmts[i];
    const fvec3& origin = segm.ray.getOrigin();
    const fvec3& direction = segm.ray.getDirection();
    const float dt = float(segm.end - segm.start);
    glVertex2fv(origin);
    glVertex2fv(origin.xy() + (dt * direction.xy()));
  }
  glEnd();
}


bool LaserStrategy::isStoppedByHit() const
{
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
