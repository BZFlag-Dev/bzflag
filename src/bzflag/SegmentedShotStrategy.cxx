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

/* interface header */
#include "SegmentedShotStrategy.h"

/* system implementation headers */
#include <assert.h>

/* common implementation headers */
#include "TextureManager.h"
#include "Intersect.h"
#include "BZDBCache.h"
#include "WallObstacle.h"

/* local implementation headers */
#include "sound.h"
#include "LocalPlayer.h"
#include "World.h"
#include "effectsRenderer.h"
#include "playing.h"

SegmentedShotStrategy::SegmentedShotStrategy(ShotPath* _path, bool useSuperTexture, bool faint) :
				ShotStrategy(_path)
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

    const float* c = Team::getRadarColor(team);
    if (faint) {
      boltSceneNode->setColor(c[0], c[1], c[2], 0.2f);
      boltSceneNode->setTextureColor(1.0f, 1.0f, 1.0f, 0.3f);
    } else {
      boltSceneNode->setColor(c[0], c[1], c[2], 1.0f);
    }

    TextureManager &tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(team);
    if (useSuperTexture)
      imageName += BZDB.get("superPrefix");
    imageName += BZDB.get("boltTexture");

    boltSceneNode->phasingShot = useSuperTexture;

    int texture = tm.getTextureID(imageName.c_str());
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

void			SegmentedShotStrategy::update(float dt)
{
  prevTime = currentTime;
  currentTime += dt;

  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  if (segment < numSegments && segments[segment].end <= currentTime) {
    lastSegment = segment;
    while (segment < numSegments && segments[segment].end <= currentTime) {
      if (++segment < numSegments) {
	switch (segments[segment].reason) {
	  case ShotPathSegment::Ricochet:
	    {
	      // play ricochet sound.  ricochet of local player's shots
	      // are important, others are not.
	      const PlayerId myTankId = LocalPlayer::getMyTank()->getId();
	      const bool important = (getPath().getPlayer() == myTankId);
	      const float* pos = segments[segment].ray.getOrigin();
	      playWorldSound(SFX_RICOCHET, pos, important);

	      // this is fugly but it's what we do
	      float dir[3];
	      const float* newDir = segments[segment].ray.getDirection();
	      const float* oldDir = segments[segment - 1].ray.getDirection();
	      dir[0] = newDir[0] - oldDir[0];
	      dir[1] = newDir[1] - oldDir[1];
	      dir[2] = newDir[2] - oldDir[2];

	      float rots[2];
	      const float horiz = sqrtf((dir[0]*dir[0]) + (dir[1]*dir[1]));
	      rots[0] = atan2f(dir[1], dir[0]);
	      rots[1] = atan2f(dir[2], horiz);

	      EFFECTS.addRicoEffect( pos, rots);
	      break;
	    }
	  case ShotPathSegment::Boundary:
	    break;
	  default:
	    {
	      // this is fugly but it's what we do
	      float dir[3];
	      dir[0] = segments[segment].ray.getDirection()[0];
	      dir[1] = segments[segment].ray.getDirection()[1];
	      dir[2] = segments[segment].ray.getDirection()[2];

	      float rots[2];
	      const float horiz = sqrtf((dir[0]*dir[0]) + (dir[1]*dir[1]));
	      rots[0] = atan2f(dir[1], dir[0]);
	      rots[1] = atan2f(dir[2], horiz);

	      const float* pos = segments[segment].ray.getOrigin();
	      EFFECTS.addShotTeleportEffect( pos, rots);
	    }
	    break;
	}
      }
    }
  }

  // if ran out of segments then expire shot on next update
  if (segment == numSegments) {
    setExpiring();

    if (numSegments > 0) {
      ShotPathSegment &segm = segments[numSegments - 1];
      const float     *dir  = segm.ray.getDirection();
      const float speed = hypotf(dir[0], hypotf(dir[1], dir[2]));
      float pos[3];
      segm.ray.getPoint(float(segm.end - segm.start - 1.0 / speed), pos);
      /* NOTE -- comment out to not explode when shot expires */
      addShotExplosion(pos);
    }
  }

  // otherwise update position and velocity
  else {
    float p[3];
    segments[segment].ray.getPoint(float(currentTime - segments[segment].start), p);
    setPosition(p);
    setVelocity(segments[segment].ray.getDirection());
  }
}

void			SegmentedShotStrategy::setCurrentTime(const
						TimeKeeper& _currentTime)
{
  currentTime = _currentTime;
}

const TimeKeeper&	SegmentedShotStrategy::getLastTime() const
{
  return lastTime;
}

bool			SegmentedShotStrategy::isOverlapping(
				const float (*bbox1)[3],
				const float (*bbox2)[3]) const
{
  if (bbox1[1][0] < bbox2[0][0]) return false;
  if (bbox1[0][0] > bbox2[1][0]) return false;
  if (bbox1[1][1] < bbox2[0][1]) return false;
  if (bbox1[0][1] > bbox2[1][1]) return false;
  if (bbox1[1][2] < bbox2[0][2]) return false;
  if (bbox1[0][2] > bbox2[1][2]) return false;
  return true;
}

void			SegmentedShotStrategy::setCurrentSegment(int _segment)
{
  segment = _segment;
}

float			SegmentedShotStrategy::checkHit(const BaseLocalPlayer* tank,
							float position[3]) const
{
  float minTime = Infinity;
  // expired shot can't hit anything
  if (getPath().isExpired()) return minTime;

  // get tank radius
  float radius = tank->getRadius();
  const float radius2 = radius * radius;

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  const float tankHeight = tank->getDimensions()[2];
  Ray tankLastMotionRaw = tank->getLastMotion();
  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tankLastMotionRaw.getOrigin()[0];
  lastTankPositionRaw[1] = tankLastMotionRaw.getOrigin()[1];
  lastTankPositionRaw[2] = tankLastMotionRaw.getOrigin()[2] + 0.5f * tankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tankLastMotionRaw.getDirection());

  // if bounding box of tank and entire shot doesn't overlap then no hit
  const float (*tankBBox)[3] = tank->getLastMotionBBox();
  if (!isOverlapping(bbox, tankBBox)) return minTime;

  float shotRadius = BZDB.eval(StateDatabase::BZDB_SHOTRADIUS);

  // check each segment in interval (prevTime,currentTime]
  const float dt = float(currentTime - prevTime);
  const int numSegments = (const int)segments.size();
  for (int i = lastSegment; i <= segment && i < numSegments; i++) {
    // can never hit your own first laser segment
    if (i == 0 && getPath().getFlag() == Flags::Laser &&
	getPath().getPlayer() == tank->getId())
      continue;

/*
    // skip segments that don't overlap in time with current interval
    if (segments[i].end <= prevTime) continue;
    if (currentTime <= segments[i].start) break;
*/

    // if shot segment and tank bboxes don't overlap then no hit
    const ShotPathSegment& s = segments[i];
    if (!isOverlapping(s.bbox, tankBBox)) continue;

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(rayMinusRay(s.ray, float(prevTime - s.start), tankLastMotion, 0.0f));

    // get hit time
    float t;
    if (tank->getFlag() == Flags::Narrow) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float tankBase[3] = { 0.0f, 0.0f, -0.5f * tankHeight };
      t = timeRayHitsBlock(relativeRay, tankBase, tank->getAngle(),
			0.5f * BZDBCache::tankLength, shotRadius, tankHeight);
    }
    else {
      // find time when shot hits sphere around tank
      t = rayAtDistanceFromOrigin(relativeRay, 0.99f * radius);
    }

    // short circuit if time is greater then smallest time so far
    if (t > minTime) continue;

    // make sure time falls within segment
    if (t < 0.0f || t > dt) continue;
    if (t > s.end - prevTime) continue;

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
      //printf("%u:%u %u:%u\n", tank->getId().port, tank->getId().number, getPath().getPlayer().port, getPath().getPlayer().number);
    }
  }
  return minTime;
}

void			SegmentedShotStrategy::addShot(
				SceneDatabase* scene, bool colorblind)
{
  const ShotPath& shotPath = getPath();
  boltSceneNode->move(shotPath.getPosition(), shotPath.getVelocity());
  if (boltSceneNode->getColorblind() != colorblind) {
    boltSceneNode->setColorblind(colorblind);
    TeamColor currentTeam = colorblind ? RogueTeam : team;

    const float* c = Team::getRadarColor(currentTeam);
    boltSceneNode->setColor(c[0], c[1], c[2]);

    TextureManager &tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(currentTeam);
    imageName += BZDB.get("boltTexture");
    int texture = tm.getTextureID(imageName.c_str());
    if (texture >= 0)
      boltSceneNode->setTexture(texture);
  }
  scene->addDynamicNode(boltSceneNode);
}

void			SegmentedShotStrategy::radarRender() const
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

    // draw a "bright" bullet tip
    if (size > 0) {
      glColor3f(0.75, 0.75, 0.75);
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2f(orig[0], orig[1]);
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

void			SegmentedShotStrategy::makeSegments(ObstacleEffect e)
{
  // compute segments of shot until total length of segments exceeds the
  // lifetime of the shot.
  const ShotPath &shotPath  = getPath();
  const float    *v	 = shotPath.getVelocity();
  TimeKeeper      startTime = shotPath.getStartTime();
  float timeLeft	    = shotPath.getLifetime();
  float minTime = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT) / hypotf(v[0], hypotf(v[1], v[2]));
  World *world = World::getWorld();
  if (!world) {
    return; /* no world, no shots */
  }

  // if all shots ricochet and obstacle effect is stop, then make it ricochet
  if (e == Stop && world->allShotsRicochet()) {
    e = Reflect;
  }

  // prepare first segment
  float o[3], d[3];
  d[0] = v[0];
  d[1] = v[1];
  d[2] = v[2];		// use v[2] to have jumping affect shot velocity
  o[0] = shotPath.getPosition()[0];
  o[1] = shotPath.getPosition()[1];
  o[2] = shotPath.getPosition()[2];

  segments.clear();
  ShotPathSegment::Reason reason = ShotPathSegment::Initial;
  int i;
  const int maxSegment = 100;
  float worldSize = BZDBCache::worldSize / 2.0f - 0.01f;
  for (i = 0; (i < maxSegment) && (timeLeft > Epsilon); i++) {
    // construct ray and find the first building, teleporter, or outer wall
    float o2[3];
    o2[0] = o[0] - minTime * d[0];
    o2[1] = o[1] - minTime * d[1];
    o2[2] = o[2] - minTime * d[2];

    // Sometime shot start outside world
    if (o2[0] <= -worldSize)
      o2[0] = -worldSize;
    if (o2[0] >= worldSize)
      o2[0] = worldSize;
    if (o2[1] <= -worldSize)
      o2[1] = -worldSize;
    if (o2[1] >= worldSize)
      o2[1] = worldSize;

    Ray r(o2, d);
    Ray rs(o, d);
    float t = timeLeft + minTime;
    int face;
    bool hitGround = getGround(r, Epsilon, t);
    Obstacle* building = (Obstacle*)((e != Through) ? getFirstBuilding(r, Epsilon, t) : NULL);
    const Teleporter* teleporter = getFirstTeleporter(r, Epsilon, t, face);
    t -= minTime;
    minTime = 0.0f;
    bool ignoreHit = false;

    // if hit outer wall with ricochet and hit is above top of wall
    // then ignore hit.
    if (!teleporter && building && (e == Reflect) &&
	(building->getType() == WallObstacle::getClassName()) &&
	((o[2] + t * d[2]) > building->getHeight())) {
      ignoreHit = true;
    }

    // construct next shot segment and add it to list
    TimeKeeper endTime(startTime);
    if (t < 0.0f) {
      endTime += Epsilon;
    } else {
      endTime += t;
    }
    ShotPathSegment segm(startTime, endTime, rs, reason);
    segments.push_back(segm);
    startTime = endTime;

    // used up this much time in segment
    if (t < 0.0f) {
      timeLeft -= Epsilon;
    } else {
      timeLeft -= t;
    }

    // check in reverse order to see what we hit first
    reason = ShotPathSegment::Through;
    if (ignoreHit) {
      // uh...ignore this.  usually used if you shoot over the boundary wall.
      // just move the point of origin and build the next segment
      o[0] += t * d[0];
      o[1] += t * d[1];
      o[2] += t * d[2];
      reason = ShotPathSegment::Boundary;
    } else if (teleporter) {
      // entered teleporter -- teleport it
      unsigned int seed = shotPath.getShotId() + i;
      int source = world->getTeleporter(teleporter, face);
      int target = world->getTeleportTarget(source, seed);

      int outFace;
      const Teleporter* outTeleporter = world->getTeleporter(target, outFace);
      o[0] += t * d[0];
      o[1] += t * d[1];
      o[2] += t * d[2];
      teleporter->getPointWRT(*outTeleporter, face, outFace,
					    o, d, 0.0f, o, d, NULL);
      reason = ShotPathSegment::Teleport;
    }
    else if (building) {
      // hit building -- can bounce off or stop, buildings ignored for Through
      switch (e) {
	case Stop:
	  timeLeft = 0.0f;
	break;

      case Reflect: {
	// move origin to point of reflection
	o[0] += t * d[0];
	o[1] += t * d[1];
	o[2] += t * d[2];

	// reflect direction about normal to building
	float normal[3];
	building->get3DNormal(o, normal);
	reflect(d, normal);
	reason = ShotPathSegment::Ricochet;
	}
	break;

      case Through:
	assert(0);
      }
    }
    else if (hitGround)	// we hit the ground
    {
      switch (e) {
	case Stop:
	case Through:
	  timeLeft = 0.0f;
	  break;

	case Reflect: {
	  // move origin to point of reflection
	  o[0] += t * d[0];
	  o[1] += t * d[1];
	  o[2] += t * d[2];

	  // reflect direction about normal to ground
	  float normal[3];
	  normal[0] = 0.0f;
	  normal[1] = 0.0f;
	  normal[2] = 1.0f;
	  reflect(d, normal);
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
    const ShotPathSegment& firstSeg = segments[0];
    bbox[0][0] = firstSeg.bbox[0][0];
    bbox[0][1] = firstSeg.bbox[0][1];
    bbox[0][2] = firstSeg.bbox[0][2];
    bbox[1][0] = firstSeg.bbox[1][0];
    bbox[1][1] = firstSeg.bbox[1][1];
    bbox[1][2] = firstSeg.bbox[1][2];
    for (size_t j = 1; j < numSegments; ++j) {
      const ShotPathSegment& segm = segments[j];
      if (bbox[0][0] > segm.bbox[0][0]) bbox[0][0] = segm.bbox[0][0];
      if (bbox[1][0] < segm.bbox[1][0]) bbox[1][0] = segm.bbox[1][0];
      if (bbox[0][1] > segm.bbox[0][1]) bbox[0][1] = segm.bbox[0][1];
      if (bbox[1][1] < segm.bbox[1][1]) bbox[1][1] = segm.bbox[1][1];
      if (bbox[0][2] > segm.bbox[0][2]) bbox[0][2] = segm.bbox[0][2];
      if (bbox[1][2] < segm.bbox[1][2]) bbox[1][2] = segm.bbox[1][2];
    }
  } else {
    bbox[0][0] = bbox[1][0] = 0.0f;
    bbox[0][1] = bbox[1][1] = 0.0f;
    bbox[0][2] = bbox[1][2] = 0.0f;
  }
}

const std::vector<ShotPathSegment>&	SegmentedShotStrategy::getSegments() const
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
  f.shot.vel[0] *= fireAdVel;
  f.shot.vel[1] *= fireAdVel;
  f.shot.vel[2] *= fireAdVel;
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
  f.shot.vel[0] *= thiefAdVel;
  f.shot.vel[1] *= thiefAdVel;
  f.shot.vel[2] *= thiefAdVel;
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
    const float* rawdir = ray.getDirection();
    float dir[3];
    dir[0] = t * rawdir[0];
    dir[1] = t * rawdir[1];
    dir[2] = t * rawdir[2];
    thiefNodes[i] = new LaserSceneNode(ray.getOrigin(), dir);
    if (texture >= 0)
      thiefNodes[i]->setTexture(texture);

	if (i == 0)
		thiefNodes[i]->setFirst();

	thiefNodes[i]->setColor(0,1,1);
	thiefNodes[i]->setCenterColor(0,0,0);
  }
  setCurrentSegment(numSegments - 1);
}

ThiefStrategy::~ThiefStrategy()
{
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++)
    delete thiefNodes[i];
  delete[] thiefNodes;
}

void			ThiefStrategy::update(float dt)
{
  cumTime += dt;
  if (cumTime >= endTime) setExpired();
}

void			ThiefStrategy::addShot(SceneDatabase* scene, bool)
{
  // thief is so fast we always show every segment
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++)
    scene->addDynamicNode(thiefNodes[i]);
}

void			ThiefStrategy::radarRender() const
{
  // draw all segments
  const std::vector<ShotPathSegment>& segmts = getSegments();
  const size_t numSegments = segmts.size();
  glBegin(GL_LINES);
    for (size_t i = 0; i < numSegments; i++) {
      const ShotPathSegment& segm = segmts[i];
      const float* origin = segm.ray.getOrigin();
      const float* direction = segm.ray.getDirection();
      const float dt = float(segm.end - segm.start);
      glVertex2fv(origin);
      glVertex2f(origin[0] + dt * direction[0], origin[1] + dt * direction[1]);
    }
  glEnd();
}

bool			ThiefStrategy::isStoppedByHit() const
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
  f.shot.vel[0] *= mgunAdVel;
  f.shot.vel[1] *= mgunAdVel;
  f.shot.vel[2] *= mgunAdVel;
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
  f.shot.vel[0] *= laserAdVel;
  f.shot.vel[1] *= laserAdVel;
  f.shot.vel[2] *= laserAdVel;
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
  int texture = tm.getTextureID(imageName.c_str());

  for (int i = 0; i < numSegments; i++) {
    const ShotPathSegment& segm = getSegments()[i];
    const float t = float(segm.end - segm.start);
    const Ray& ray = segm.ray;
    const float* rawdir = ray.getDirection();
    float dir[3];
    dir[0] = t * rawdir[0];
    dir[1] = t * rawdir[1];
    dir[2] = t * rawdir[2];
    laserNodes[i] = new LaserSceneNode(ray.getOrigin(), dir);
    if (texture >= 0)
      laserNodes[i]->setTexture(texture);

	const float *color = Team::getRadarColor(tmpTeam);
	laserNodes[i]->setColor(color[0],color[1],color[2]);

	if (i == 0)
		laserNodes[i]->setFirst();
  }
  setCurrentSegment(numSegments - 1);
}

LaserStrategy::~LaserStrategy()
{
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++)
    delete laserNodes[i];
  delete[] laserNodes;
}

void			LaserStrategy::update(float dt)
{
  cumTime += dt;
  if (cumTime >= endTime) setExpired();
}

void			LaserStrategy::addShot(SceneDatabase* scene, bool)
{
  // laser is so fast we always show every segment
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++)
    scene->addDynamicNode(laserNodes[i]);
}

void			LaserStrategy::radarRender() const
{
  // draw all segments
  const std::vector<ShotPathSegment>& segmts = getSegments();
  const size_t numSegments = segmts.size();
  glBegin(GL_LINES);
    for (size_t i = 0; i < numSegments; i++) {
      const ShotPathSegment& segm = segmts[i];
      const float* origin = segm.ray.getOrigin();
      const float* direction = segm.ray.getDirection();
      const float dt = float(segm.end - segm.start);
      glVertex2fv(origin);
      glVertex2f(origin[0] + dt * direction[0], origin[1] + dt * direction[1]);
    }
  glEnd();
}

bool			LaserStrategy::isStoppedByHit() const
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
