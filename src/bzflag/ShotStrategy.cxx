/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

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
#include "texture.h"
#include "playing.h"
#include "PlayerLink.h"
#include "Team.h"

static OpenGLTexture*	boltTexture[NumTeams];
static OpenGLTexture*	laserTexture;
static OpenGLTexture*	gmTexture;

//
// ShotPathStrategy
//

ShotStrategy::ShotStrategy(ShotPath* _path) :
				path(_path)
{
  // do nothing
}

ShotStrategy::~ShotStrategy()
{
  // do nothing
}

void			ShotStrategy::init()
{
  for (int i = 0; i < (int)(sizeof(boltTexture) / sizeof(boltTexture[0])); i++)
    boltTexture[i] = new OpenGLTexture;
  laserTexture = new OpenGLTexture;
  gmTexture = new OpenGLTexture;

  *boltTexture[RogueTeam] = getTexture("ybolt", OpenGLTexture::Linear);
  *boltTexture[RedTeam] = getTexture("rbolt", OpenGLTexture::Linear);
  *boltTexture[GreenTeam] = getTexture("gbolt", OpenGLTexture::Linear);
  *boltTexture[BlueTeam] = getTexture("bbolt", OpenGLTexture::Linear);
  *boltTexture[PurpleTeam] = getTexture("pbolt", OpenGLTexture::Linear);
  *laserTexture = getTexture("laser", OpenGLTexture::Max);
  *gmTexture = getTexture("missile", OpenGLTexture::Max);
}

void			ShotStrategy::done()
{
  delete gmTexture;
  delete laserTexture;
  gmTexture = NULL;
  laserTexture = NULL;
  for (int i = 0; i < (int)(sizeof(boltTexture) / sizeof(boltTexture[0])); i++) {
    delete boltTexture[i];
    boltTexture[i] = NULL;
  }
}

boolean			ShotStrategy::isStoppedByHit() const
{
  return True;
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
						float min, float& t) const
{
  const Obstacle* closestObstacle = NULL;

  // check walls
  WallObstaclesCIteratorPtr wallScan(World::getWorld()->
					getWalls().newCIterator());
  while (!wallScan->isDone()) {
    const WallObstacle& wall = wallScan->getItem();
    const float wallt = wall.intersect(ray);
    if (wallt > min && wallt < t) {
      t = wallt;
      closestObstacle = &wall;
    }
    wallScan->next();
  }

  // check teleporter borders
  TeleportersCIteratorPtr teleporterScan(World::getWorld()->
					getTeleporters().newCIterator());
  while (!teleporterScan->isDone()) {
    const Teleporter& teleporter = teleporterScan->getItem();
    const float telet = teleporter.intersect(ray);
    int face;
    if (telet > min && telet < t &&
			teleporter.isTeleported(ray, face) < 0.0f) {
      t = telet;
      closestObstacle = &teleporter;
    }
    teleporterScan->next();
  }

  // check boxes
  BoxBuildingsCIteratorPtr boxScan(World::getWorld()->
						getBoxes().newCIterator());
  while (!boxScan->isDone()) {
    const BoxBuilding& box = boxScan->getItem();
    const float boxt = box.intersect(ray);
    if (boxt > min && boxt < t) {
      t = boxt;
      closestObstacle = &box;
    }
    boxScan->next();
  }

  // check pyramids
  PyramidBuildingsCIteratorPtr pyramidScan(World::getWorld()->
						getPyramids().newCIterator());
  while (!pyramidScan->isDone()) {
    const PyramidBuilding& pyramid = pyramidScan->getItem();
    const float pyrt = pyramid.intersect(ray);
    if (pyrt > min && pyrt < t) {
      t = pyrt;
      closestObstacle = &pyramid;
    }
    pyramidScan->next();
  }

  return closestObstacle;
}

const Teleporter*	ShotStrategy::getFirstTeleporter(const Ray& ray,
					float min, float& t, int& f) const
{
  const Teleporter* closestTeleporter = NULL;
  int face;

  TeleportersCIteratorPtr teleporterScan(World::getWorld()->
					getTeleporters().newCIterator());
  while (!teleporterScan->isDone()) {
    const Teleporter& teleporter = teleporterScan->getItem();
    const float telet = teleporter.isTeleported(ray, face);
    if (telet > min && telet < t) {
      t = telet;
      f = face;
      closestTeleporter = &teleporter;
    }
    teleporterScan->next();
  }

  return closestTeleporter;
}

//
// ShotPathSegment
//

ShotPathSegment::ShotPathSegment()
{
  // do nothing
}

ShotPathSegment::ShotPathSegment(const TimeKeeper& _start,
				const TimeKeeper& _end, const Ray& _ray,
				Reason _reason) :
				start(_start),
				end(_end),
				ray(_ray),
				reason(_reason)
{
  // compute bounding box
  ray.getPoint(0.0f, bbox[0]);
  ray.getPoint(end - start, bbox[1]);
  if (bbox[0][0] > bbox[1][0]) {
    const float tmp = bbox[0][0];
    bbox[0][0] = bbox[1][0];
    bbox[1][0] = tmp;
  }
  if (bbox[0][1] > bbox[1][1]) {
    const float tmp = bbox[0][1];
    bbox[0][1] = bbox[1][1];
    bbox[1][1] = tmp;
  }
  if (bbox[0][2] > bbox[1][2]) {
    const float tmp = bbox[0][2];
    bbox[0][2] = bbox[1][2];
    bbox[1][2] = tmp;
  }
}

ShotPathSegment::ShotPathSegment(const ShotPathSegment& segment) :
				start(segment.start),
				end(segment.end),
				ray(segment.ray),
				reason(segment.reason)
{
  // copy bounding box
  bbox[0][0] = segment.bbox[0][0];
  bbox[0][1] = segment.bbox[0][1];
  bbox[0][2] = segment.bbox[0][2];
  bbox[1][0] = segment.bbox[1][0];
  bbox[1][1] = segment.bbox[1][1];
  bbox[1][2] = segment.bbox[1][2];
}

ShotPathSegment::~ShotPathSegment()
{
  // do nothing
}

ShotPathSegment&	ShotPathSegment::operator=(const
					ShotPathSegment& segment)
{
  if (this != &segment) {
    start = segment.start;
    end = segment.end;
    ray = segment.ray;
    reason = segment.reason;
    bbox[0][0] = segment.bbox[0][0];
    bbox[0][1] = segment.bbox[0][1];
    bbox[0][2] = segment.bbox[0][2];
    bbox[1][0] = segment.bbox[1][0];
    bbox[1][1] = segment.bbox[1][1];
    bbox[1][2] = segment.bbox[1][2];
  }
  return *this;
}

//
// SegmentedShotStrategy
//

SegmentedShotStrategy::SegmentedShotStrategy(ShotPath* _path) :
				ShotStrategy(_path)
{
  // initialize times
  prevTime = getPath().getStartTime();
  lastTime = currentTime = prevTime;

  // start at first segment
  lastSegment = segment = 0;

  // get team
  Player* p = lookupPlayer(_path->getPlayer());
  team = p ? p->getTeam() : RogueTeam;

  // initialize scene nodes
  boltSceneNode = new BoltSceneNode(_path->getPosition());
  const float* c = Team::getRadarColor(team);
  boltSceneNode->setColor(c[0], c[1], c[2]);
  if (boltTexture[team] && boltTexture[team]->isValid())
    boltSceneNode->setTexture(*boltTexture[team]);
}

SegmentedShotStrategy::~SegmentedShotStrategy()
{
  // free scene nodes
  delete boltSceneNode;
}

void			SegmentedShotStrategy::update(float dt)
{
  prevTime = currentTime;
  currentTime += dt;

  // see if we've moved to another segment
  const int numSegments = segments.getLength();
  if (segment < numSegments && segments[segment].end <= currentTime) {
    lastSegment = segment;
    while (segment < numSegments && segments[segment].end <= currentTime) {
      if (++segment < numSegments) {
	switch (segments[segment].reason) {
	  case ShotPathSegment::Ricochet: {
	    // play ricochet sound.  ricochet of local player's shots
	    // are important, others are not.
	    const float* pos = segments[segment].ray.getOrigin();
	    playWorldSound(SFX_RICOCHET, pos[0], pos[1], pos[2],
		getPath().getPlayer() == LocalPlayer::getMyTank()->getId());
	    break;
	  }
	  default:
	    // currently no sounds for anything else
	    break;
	}
      }
    }
  }

  // if ran out of segments then expire shot on next update
  if (segment == numSegments) {
    setExpiring();

    ShotPathSegment& segment = segments[numSegments - 1];
    const float* dir = segment.ray.getDirection();
    const float speed = hypotf(dir[0], hypotf(dir[1], dir[2]));
    float pos[3];
    segment.ray.getPoint(segment.end - segment.start - 1.0f / speed, pos);
    /* NOTE -- comment out to not explode when shot expires */
    addShotExplosion(pos);
  }

  // otherwise update position and velocity
  else {
    float p[3];
    segments[segment].ray.getPoint(currentTime - segments[segment].start, p);
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

boolean			SegmentedShotStrategy::isOverlapping(
				const float (*bbox1)[3],
				const float (*bbox2)[3]) const
{
  if (bbox1[1][0] < bbox2[0][0]) return False;
  if (bbox1[0][0] > bbox2[1][0]) return False;
  if (bbox1[1][1] < bbox2[0][1]) return False;
  if (bbox1[0][1] > bbox2[1][1]) return False;
  if (bbox1[1][2] < bbox2[0][2]) return False;
  if (bbox1[0][2] > bbox2[1][2]) return False;
  return True;
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
  float radius = TankRadius;
  if (tank->getFlag() == ObesityFlag) radius *= ObeseFactor;
  else if (tank->getFlag() == TinyFlag) radius *= TinyFactor;
  const float radius2 = radius * radius;

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  Ray tankLastMotionRaw = tank->getLastMotion();
  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tankLastMotionRaw.getOrigin()[0];
  lastTankPositionRaw[1] = tankLastMotionRaw.getOrigin()[1];
  lastTankPositionRaw[2] = tankLastMotionRaw.getOrigin()[2] + 0.5f * TankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tankLastMotionRaw.getDirection());

  // if bounding box of tank and entire shot doesn't overlap then no hit
  const float (*tankBBox)[3] = tank->getLastMotionBBox();
  if (!isOverlapping(bbox, tankBBox)) return minTime;

  // check each segment in interval (prevTime,currentTime]
  const float dt = currentTime - prevTime;
  const int numSegments = segments.getLength();
  for (int i = lastSegment; i <= segment && i < numSegments; i++) {
    // can never hit your own first laser segment
    if (i == 0 && getPath().getFlag() == LaserFlag &&
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
    Ray relativeRay(rayMinusRay(s.ray, prevTime - s.start,
						tankLastMotion, 0.0f));

    // get hit time
    float t;
    if (tank->getFlag() == NarrowFlag) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float origin[3] = { 0.0f, 0.0f, 0.0f };
      t = timeRayHitsBlock(relativeRay, origin, tank->getAngle(),
			0.5f * TankLength, ShotRadius, TankHeight);
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
    }
  }
  return minTime;
}

void			SegmentedShotStrategy::addShot(
				SceneDatabase* scene, boolean colorblind)
{
  const ShotPath& path = getPath();
  boltSceneNode->move(path.getPosition(), path.getVelocity());
  if (boltSceneNode->getColorblind() != colorblind) {
    boltSceneNode->setColorblind(colorblind);
    TeamColor tmpTeam = colorblind ? RogueTeam : team;
    const float* c = Team::getRadarColor(tmpTeam);
    boltSceneNode->setColor(c[0], c[1], c[2]);
    if (boltTexture[tmpTeam] && boltTexture[tmpTeam]->isValid())
      boltSceneNode->setTexture(*boltTexture[tmpTeam]);
  }
  scene->addDynamicNode(boltSceneNode);
}

void			SegmentedShotStrategy::radarRender() const
{
  glBegin(GL_POINTS);
    glVertex2fv(getPath().getPosition());
  glEnd();
}

void			SegmentedShotStrategy::makeSegments(ObstacleEffect e)
{
  // compute segments of shot until total length of segments exceeds the
  // lifetime of the shot.
  const ShotPath& path = getPath();
  const float* v = path.getVelocity();
  TimeKeeper startTime = path.getStartTime();
  float timeLeft = path.getLifetime();
  float minTime = MuzzleFront / hypotf(v[0], hypotf(v[1], v[2]));

  // if all shots ricochet and obstacle effect is stop, then make it ricochet
  if (e == Stop && World::getWorld()->allShotsRicochet())
    e = Reflect;

  // prepare first segment
  float o[3], d[3];
  d[0] = v[0];
  d[1] = v[1];
  d[2] = 0.0f;		// use v[2] to have jumping affect shot velocity
  o[0] = path.getPosition()[0];
  o[1] = path.getPosition()[1];
  o[2] = path.getPosition()[2];

  segments.removeAll();
  ShotPathSegment::Reason reason = ShotPathSegment::Initial;
  while (timeLeft > Epsilon) {
    // construct ray and find the first building, teleporter, or outer wall
    float o2[3];
    o2[0] = o[0] - minTime * d[0];
    o2[1] = o[1] - minTime * d[1];
    o2[2] = o[2] - minTime * d[2];
    Ray r(o2, d);
    Ray rs(o, d);
    float t = timeLeft + minTime;
    int face;
    const Obstacle* building = (const Obstacle*)((e != Through) ?
				getFirstBuilding(r, Epsilon, t) : NULL);
    const Teleporter* teleporter = getFirstTeleporter(r, Epsilon, t, face);
    t -= minTime;
    minTime = 0.0f;

    // if hit outer wall with ricochet and hit is above top of wall
    // then ignore hit.
    if (!teleporter && building && e == Reflect &&
	building->getType() == WallObstacle::getClassName() &&
	o[2] + t * d[2] > building->getHeight())
      t = timeLeft;

    // construct next shot segment and add it to list
    TimeKeeper endTime(startTime);
    if (t < 0.0f) endTime += Epsilon;
    else endTime += t;
    ShotPathSegment segment(startTime, endTime, rs, reason);
    segments.append(segment);
    startTime = endTime;

    // used up this much time in segment
    if (t < 0.0f) timeLeft -= Epsilon;
    else timeLeft -= t;

    // check in reverse order to see what we hit first
    reason = ShotPathSegment::Through;
    if (teleporter) {
      // entered teleporter -- teleport it
      int source = World::getWorld()->getTeleporter(teleporter, face);
      int target = World::getWorld()->getTeleportTarget(source);
      int outFace;
      const Teleporter* outTeleporter =
			World::getWorld()->getTeleporter(target, outFace);
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
	  building->getNormal(o, normal);
	  reflect(d, normal);
	  reason = ShotPathSegment::Ricochet;
	  break;

        case Through:
	  assert(0);
	}
      }
    }
  }
  lastTime = startTime;

  // make bounding box for entire path
  const int numSegments = segments.getLength();
  if (numSegments > 0) {
    const ShotPathSegment& firstSegment = segments[0];
    bbox[0][0] = firstSegment.bbox[0][0];
    bbox[0][1] = firstSegment.bbox[0][1];
    bbox[0][2] = firstSegment.bbox[0][2];
    bbox[1][0] = firstSegment.bbox[1][0];
    bbox[1][1] = firstSegment.bbox[1][1];
    bbox[1][2] = firstSegment.bbox[1][2];
    for (int i = 1; i < numSegments; i++) {
      const ShotPathSegment& segment = segments[i];
      if (bbox[0][0] > segment.bbox[0][0]) bbox[0][0] = segment.bbox[0][0];
      if (bbox[1][0] < segment.bbox[1][0]) bbox[1][0] = segment.bbox[1][0];
      if (bbox[0][1] > segment.bbox[0][1]) bbox[0][1] = segment.bbox[0][1];
      if (bbox[1][1] < segment.bbox[1][1]) bbox[1][1] = segment.bbox[1][1];
      if (bbox[0][2] > segment.bbox[0][2]) bbox[0][2] = segment.bbox[0][2];
      if (bbox[1][2] < segment.bbox[1][2]) bbox[1][2] = segment.bbox[1][2];
    }
  }
  else {
    bbox[0][0] = bbox[1][0] = 0.0f;
    bbox[0][1] = bbox[1][1] = 0.0f;
    bbox[0][2] = bbox[1][2] = 0.0f;
  }
}

const ShotPathSegments&	SegmentedShotStrategy::getSegments() const
{
  return segments;
}

void			SegmentedShotStrategy::reflect(float* v,
							const float* n) // const
{
  // normal is assumed to be normalized, v needn't be
  const float d = -2.0f * (n[0] * v[0] + n[1] * v[1] + n[2] * v[2]);
  v[0] += d * n[0];
  v[1] += d * n[1];
  v[2] += d * n[2];
}

//
// NormalShotStrategy
//

NormalShotStrategy::NormalShotStrategy(ShotPath* path) :
				SegmentedShotStrategy(path)
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

RapidFireStrategy::RapidFireStrategy(ShotPath* path) :
				SegmentedShotStrategy(path)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(path);
  f.lifetime *= RFireAdLife;
  f.shot.vel[0] *= RFireAdVel;
  f.shot.vel[1] *= RFireAdVel;
  f.shot.vel[2] *= RFireAdVel;
  setReloadTime(path->getReloadTime() / RFireAdRate);

  // make segments
  makeSegments(Stop);
}

RapidFireStrategy::~RapidFireStrategy()
{
  // do nothing
}

//
// MachineGunStrategy
//

MachineGunStrategy::MachineGunStrategy(ShotPath* path) :
				SegmentedShotStrategy(path)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(path);
  f.lifetime *= MGunAdLife;
  f.shot.vel[0] *= MGunAdVel;
  f.shot.vel[1] *= MGunAdVel;
  f.shot.vel[2] *= MGunAdVel;
  setReloadTime(path->getReloadTime() / MGunAdRate);

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

RicochetStrategy::RicochetStrategy(ShotPath* path) :
				SegmentedShotStrategy(path)
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

SuperBulletStrategy::SuperBulletStrategy(ShotPath* path) :
				SegmentedShotStrategy(path)
{
  // make segments that go through buildings
  makeSegments(Through);
}

SuperBulletStrategy::~SuperBulletStrategy()
{
  // do nothing
}

//
// LaserStrategy
//

LaserStrategy::LaserStrategy(ShotPath* path) :
				SegmentedShotStrategy(path),
				cumTime(0.0f)
{
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(path);
  f.lifetime *= LaserAdLife;
  f.shot.vel[0] *= LaserAdVel;
  f.shot.vel[1] *= LaserAdVel;
  f.shot.vel[2] *= LaserAdVel;
  setReloadTime(path->getReloadTime() / LaserAdRate);

  // make segments
  makeSegments(Stop);
  setCurrentTime(getLastTime());
  endTime = f.lifetime;

  // make laser scene nodes
  const int numSegments = getSegments().getLength();
  laserNodes = new LaserSceneNode*[numSegments];
  for (int i = 0; i < numSegments; i++) {
    const ShotPathSegment& segment = getSegments()[i];
    const float t = segment.end - segment.start;
    const Ray& ray = segment.ray;
    const float* rawdir = ray.getDirection();
    float dir[3];
    dir[0] = t * rawdir[0];
    dir[1] = t * rawdir[1];
    dir[2] = t * rawdir[2];
    laserNodes[i] = new LaserSceneNode(ray.getOrigin(), dir);
    if (laserTexture && laserTexture->isValid())
      laserNodes[i]->setTexture(*laserTexture);
  }
  setCurrentSegment(numSegments - 1);
}

LaserStrategy::~LaserStrategy()
{
  const int numSegments = getSegments().getLength();
  for (int i = 0; i < numSegments; i++)
    delete laserNodes[i];
  delete[] laserNodes;
}

void			LaserStrategy::update(float dt)
{
  cumTime += dt;
  if (cumTime >= endTime) setExpired();
}

void			LaserStrategy::addShot(SceneDatabase* scene, boolean)
{
  // laser is so fast we always show every segment
  const int numSegments = getSegments().getLength();
  for (int i = 0; i < numSegments; i++)
    scene->addDynamicNode(laserNodes[i]);
}

void			LaserStrategy::radarRender() const
{
  // draw all segments
  const ShotPathSegments& segments = getSegments();
  const int numSegments = segments.getLength();
  glBegin(GL_LINES);
    for (int i = 0; i < numSegments; i++) {
      const ShotPathSegment& segment = segments[i];
      const float* origin = segment.ray.getOrigin();
      const float* direction = segment.ray.getDirection();
      const float dt = segment.end - segment.start;
      glVertex2fv(origin);
      glVertex2f(origin[0] + dt * direction[0], origin[1] + dt * direction[1]);
    }
  glEnd();
}

boolean			LaserStrategy::isStoppedByHit() const
{
  return False;
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
				needUpdate(True)
{
  ptSceneNode = new BoltSceneNode(_path->getPosition());
  if (gmTexture && gmTexture->isValid()) {
    ptSceneNode->setTexture(*gmTexture);
    ptSceneNode->setTextureAnimation(4, 4);
    ptSceneNode->setColor(1.0f, 0.2f, 0.0f);
    ptSceneNode->setFlares(True);
  }

  // get initial shot info
  FiringInfo& f = getFiringInfo(_path);
  const float* vel = getPath().getVelocity();
  const float d = 1.0f / hypotf(vel[0], hypotf(vel[1], vel[2]));
  float dir[3];
  dir[0] = vel[0] * d;
  dir[1] = vel[1] * d;
  dir[2] = vel[2] * d;
  azimuth = limitAngle(atan2f(dir[1], dir[0]));
  elevation = limitAngle(atan2f(dir[2], hypotf(dir[1], dir[0])));

  // mark early segments for special treatment
  earlySegment = 2;

  // initialize segments
  currentTime = getPath().getStartTime();
  Ray ray = Ray(f.shot.pos, dir);
  ShotPathSegment segment(currentTime, currentTime, ray);
  segments.append(segment);
  segments.append(segment);
  segments.append(segment);
  segments.append(segment);

  // setup shot
  f.shot.vel[0] = ShotSpeed * dir[0];
  f.shot.vel[1] = ShotSpeed * dir[1];
  f.shot.vel[2] = ShotSpeed * dir[2];

  // set next position to starting position
  nextPos[0] = f.shot.pos[0];
  nextPos[1] = f.shot.pos[1];
  nextPos[2] = f.shot.pos[2];

  // check that first segment doesn't start inside a building
  float startPos[3];
  startPos[0] = f.shot.pos[0] - MuzzleFront * dir[0];
  startPos[1] = f.shot.pos[1] - MuzzleFront * dir[1];
  startPos[2] = f.shot.pos[2] - MuzzleFront * dir[2];
  Ray firstRay = Ray(startPos, dir);
  prevTime = currentTime;
  prevTime += -MuzzleFront / ShotSpeed;
  checkBuildings(firstRay);
  prevTime = currentTime;

  // no last target
  lastTarget.serverHost = Address();
  lastTarget.port = 0;
  lastTarget.number = -1;
}

GuidedMissileStrategy::~GuidedMissileStrategy()
{
  delete ptSceneNode;
}

// NOTE -- ray is base of shot segment and normalized direction of flight.
//	distance traveled is ShotSpeed * dt.

void			GuidedMissileStrategy::update(float dt)
{
  const boolean isRemote = (getPath().getPlayer() !=
				LocalPlayer::getMyTank()->getId());

  // ignore packets that arrive out of order
  if (isRemote && dt < 0.0f) return;

  // update time
  prevTime = currentTime;
  currentTime += dt;

  // countdown early segments
  if (earlySegment) earlySegment--;

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
    if (lastTarget.number != -1)
      target = lookupPlayer(lastTarget);
  }
  else {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (myTank) target = myTank->getTarget();

    // see if the target changed
    if (target) {
      if (lastTarget != target->getId()) {
	needUpdate = True;
	lastTarget = target->getId();
      }
    }
    else {
      if (lastTarget.number != -1) {
	needUpdate = True;
	lastTarget.number = -1;
      }
    }
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
    desiredDir[2] += MuzzleHeight;	// aim for turret

    // compute desired angles
    float newAzimuth = atan2f(desiredDir[1], desiredDir[0]);
    float newElevation = atan2f(desiredDir[2],
				hypotf(desiredDir[1], desiredDir[0]));

    // compute new azimuth
    float deltaAzimuth = limitAngle(newAzimuth - azimuth);
    if (fabsf(deltaAzimuth) <= dt * GMissileAng)
      azimuth = limitAngle(newAzimuth);
    else if (deltaAzimuth > 0.0f)
      azimuth = limitAngle(azimuth + dt * GMissileAng);
    else
      azimuth = limitAngle(azimuth - dt * GMissileAng);

    // compute new elevation
    float deltaElevation = limitAngle(newElevation - elevation);
    if (fabsf(deltaElevation) <= dt * GMissileAng)
      elevation = limitAngle(newElevation);
    else if (deltaElevation > 0.0f)
      elevation = limitAngle(elevation + dt * GMissileAng);
    else
      elevation = limitAngle(elevation - dt * GMissileAng);
  }
  float newDirection[3];
  newDirection[0] = cosf(azimuth) * cosf(elevation);
  newDirection[1] = sinf(azimuth) * cosf(elevation);
  newDirection[2] = sinf(elevation);
  Ray ray = Ray(nextPos, newDirection);

  // get next position
  ray.getPoint(dt * ShotSpeed, nextPos);

  // see if we hit something
  TimeKeeper segmentEndTime(currentTime);
  /* if (!isRemote) */ {
    if (nextPos[2] <= 0.0f) {
      // hit ground -- expire it and shorten life of segment to time of impact
      setExpiring();
      float t = ray.getOrigin()[2] / (ray.getOrigin()[2] - nextPos[2]);
      segmentEndTime = prevTime;
      segmentEndTime += t * (currentTime - prevTime);
      ray.getPoint(t / ShotSpeed, nextPos);
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
  segments.rotate(3, 0);
  segments[3] = nextSegment;

  // update shot
  newDirection[0] *= ShotSpeed;
  newDirection[1] *= ShotSpeed;
  newDirection[2] *= ShotSpeed;
  setPosition(nextPos);
  setVelocity(newDirection);
}

float			GuidedMissileStrategy::checkBuildings(const Ray& ray)
{
  float t = (currentTime - prevTime) * ShotSpeed;
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
    return t / ShotSpeed;
  }

  else if (building) {
    // expire on next update
    setExpiring();
    float pos[3];
    ray.getPoint(t / ShotSpeed, pos);
    addShotExplosion(pos);
    return t / ShotSpeed;
  }
  return -1.0f;
}

float			GuidedMissileStrategy::checkHit(const BaseLocalPlayer* tank,
							float position[3]) const
{
  float minTime = Infinity;
  if (getPath().isExpired()) return minTime;

  // can't shoot myself for first few segments (kludge!)
  if (earlySegment && tank->getId() == getPath().getPlayer())
    return minTime;

  // get tank radius
  float radius = TankRadius;
  if (tank->getFlag() == ObesityFlag) radius *= ObeseFactor;
  else if (tank->getFlag() == TinyFlag) radius *= TinyFactor;
  const float radius2 = radius * radius;

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  Ray tankLastMotionRaw = tank->getLastMotion();
  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tankLastMotionRaw.getOrigin()[0];
  lastTankPositionRaw[1] = tankLastMotionRaw.getOrigin()[1];
  lastTankPositionRaw[2] = tankLastMotionRaw.getOrigin()[2] + 0.5f * TankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tankLastMotionRaw.getDirection());

  // check each segment
  const int numSegments = segments.getLength();
  int i = 0;
  // only test most recent segment if shot is from my tank
  if (numSegments > 1 && tank->getId() == getPath().getPlayer())
    i = numSegments - 1;
  for (; i < numSegments; i++) {
    const Ray& ray = segments[i].ray;

    // construct ray with correct velocity
    float speed[3];
    const float* dir = ray.getDirection();
    speed[0] = ShotSpeed * dir[0];
    speed[1] = ShotSpeed * dir[1];
    speed[2] = ShotSpeed * dir[2];
    Ray speedRay(ray.getOrigin(), speed);

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(rayMinusRay(speedRay, 0.0, tankLastMotion, 0.0));

    // get closest approach time
    float t;
    if (tank->getFlag() == NarrowFlag) {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float origin[3] = { 0.0f, 0.0f, 0.0f };
      t = timeRayHitsBlock(relativeRay, origin, tank->getAngle(),
		      0.5f * TankLength, ShotRadius, TankHeight);
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
  ((GuidedMissileStrategy*)this)->needUpdate = False;

  // construct and send packet
  char packet[ShotUpdatePLen + PlayerIdPLen];
  void *buf = (void*)packet;
  buf = firingInfo.shot.pack(buf);
  buf = lastTarget.pack(buf);
  PlayerLink::getMulticast()->send(MsgGMUpdate, sizeof(packet), packet);
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
  lastTarget.unpack(msg);

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
				SceneDatabase* scene, boolean)
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
  glBegin(GL_POINTS);
    glVertex2fv(getPath().getPosition());
  glEnd();
}

//
// ShockWaveStrategy
//

ShockWaveStrategy::ShockWaveStrategy(ShotPath* path) :
				ShotStrategy(path),
				radius(ShockInRadius),
				radius2(ShockInRadius * ShockInRadius)
{
  // setup shot
  FiringInfo& f = getFiringInfo(path);
  f.lifetime *= ShockAdLife;

  // make scene node
  shockNode = new SphereSceneNode(path->getPosition(), ShockInRadius);
  shockNode->setColor(1.0f, 1.0f, 1.0f, 0.75f);
}

ShockWaveStrategy::~ShockWaveStrategy()
{
  delete shockNode;
}

void			ShockWaveStrategy::update(float dt)
{
  radius += dt * (ShockOutRadius - ShockInRadius) / getPath().getLifetime();
  radius2 = radius * radius;

  // update shock wave scene node
  const GLfloat frac = (radius - ShockInRadius) /
			(ShockOutRadius - ShockInRadius);
  shockNode->move(getPath().getPosition(), radius);
  shockNode->setColor(1.0f, 1.0f, 1.0f, 0.75f - 0.5f * frac);

  // expire when full size
  if (radius >= ShockOutRadius) setExpired();
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

boolean			ShockWaveStrategy::isStoppedByHit() const
{
  return False;
}

void			ShockWaveStrategy::addShot(
				SceneDatabase* scene, boolean)
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
