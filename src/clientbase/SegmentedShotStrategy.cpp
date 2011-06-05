/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "BZDBCache.h"
#include "TextUtils.h"
#include "TextureManager.h"
#include "Intersect.h"
#include "LinkManager.h"
#include "EventHandler.h"
#include "MeshFace.h"
#include "Roster.h"
#include "WallObstacle.h"
#include "Protocol.h"

/* local implementation headers */
#include "sound.h"
#include "LocalPlayer.h"
#include "World.h"
#include "EffectsRenderer.h"
#include "playing.h"
// FIXME: Shouldn't need to depend on GUI elements
#include "guiplaying.h"

static BZDB_int debugShotSegments("debugShotSegments");


SegmentedShotStrategy::SegmentedShotStrategy(ShotPath* _path,
                                             bool useSuperTexture, bool faint)
  : PointShotStrategy(_path)
  , endObstacle(NULL) {
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
  }
  else {
    Player* p = lookupPlayer(_path->getPlayer());
    team = p ? p->getTeam() : RogueTeam;
  }

  // initialize scene nodes
  if (!headless) {
    boltSceneNode = new BoltSceneNode(_path->getPosition(),
                                      _path->getVelocity());

    static BZDB_float shotVisualScale("shotVisualScale");
    boltSceneNode->setSize(shotVisualScale * BZDBCache::shotRadius);

    const fvec4& color = Team::getRadarColor(team);
    if (faint) {
      boltSceneNode->setColor(color.r, color.g, color.b, 0.2f);
      boltSceneNode->setTextureColor(1.0f, 1.0f, 1.0f, 0.3f);
    }
    else {
      boltSceneNode->setColor(color.r, color.g, color.b, 1.0f);
    }
    if (_path->getShotType() == CloakedShot) {
      const Player* myTank = LocalPlayer::getMyTank();
      if ((myTank == NULL) ||
          ((myTank->getId() != _path->getPlayer()) &&
           (myTank->getFlagType() != Flags::Seer))) {
        boltSceneNode->setInvisible(true);
      }
    }

    TextureManager& tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(team);
    if (useSuperTexture) {
      imageName += BZDB.get("superPrefix");
    }
    imageName += BZDB.get("boltTexture");

    boltSceneNode->phasingShot = useSuperTexture;

    int texture = tm.getTextureID(imageName);
    if (texture >= 0) {
      boltSceneNode->setTexture(texture);
    }
  }
}


SegmentedShotStrategy::~SegmentedShotStrategy() {
  // free scene nodes
  if (!headless) {
    delete boltSceneNode;
  }
}


static bool wantShotInfo(char type) {
  static BZDB_string sendShotInfo("_sendShotInfo");
  const std::string& si = sendShotInfo;
  if (si == "1") {
    return true;
  }
  return (si.find(type) != std::string::npos);
}


void SegmentedShotStrategy::update(float dt) {
  prevTime = currentTime;
  currentTime += dt;

  const PlayerId myTankId = LocalPlayer::getMyTank()->getId();

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
            const bool important = (getPath().getPlayer() == myTankId);
            const fvec3& pos = segm.ray.getOrigin();
            SOUNDSYSTEM.play(SFX_RICOCHET, pos, important, false);

            // this is fugly but it's what we do
            const fvec3& newDir = segm.ray.getDirection();
            const fvec3& oldDir = segments[segment - 1].ray.getDirection();
            const fvec3 normal = (newDir - oldDir).normalize();
            eventHandler.ShotRicochet(getPath(), pos, normal);

            const MeshFace* srcFace = linkManager.getLinkSrcFace(segm.linkSrcID);
            if ((srcFace == NULL) || !srcFace->linkSrcNoEffect()) {
              EFFECTS.addRicoEffect(pos, normal);
            }
            if (wantShotInfo(ShotInfoRicochet) &&
                (getPath().getPlayer() == myTankId)) {
              const uint32_t guid =
                segm.hitObstacle ? segm.hitObstacle->getGUID() : (uint32_t) - 1;
              serverLink->sendShotInfo(getPath(), ShotInfoRicochet, pos, guid);
            }
            break;
          }
          case ShotPathSegment::Teleport: {
            const MeshFace* srcFace = linkManager.getLinkSrcFace(segm.linkSrcID);
            if ((srcFace == NULL) || !srcFace->linkSrcNoEffect()) {
              const MeshFace* dstFace = linkManager.getLinkDstFace(segm.linkDstID);
              const fvec4* clipPlane = &dstFace->getPlane();
              EFFECTS.addShotTeleportEffect(segm.ray.getOrigin(),
                                            segm.ray.getDirection(),
                                            clipPlane);
              eventHandler.ShotTeleported(getPath(), segm.linkSrcID,
                                          segm.linkDstID);
            }
            if (wantShotInfo(ShotInfoTeleport) &&
                (getPath().getPlayer() == myTankId)) {
              const ShotPathSegment& prevSeg = segments[segment - 1];
              const float timeDiff = (float)(prevSeg.end - prevSeg.start);
              const fvec3 prevPos = prevSeg.ray.getPoint(timeDiff);
              serverLink->sendShotInfo(getPath(), ShotInfoTeleport, prevPos,
                                       (uint32_t) - 1,
                                       segm.linkSrcID, segm.linkDstID);
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
      const fvec3 pos = segm.ray.getPoint(float(segm.end - segm.start));
      addShotExplosion(pos);
      if (getPath().getPlayer() == myTankId) {
        if (endObstacle == NULL) {
          if (wantShotInfo(ShotInfoExpired)) {
            serverLink->sendShotInfo(getPath(), ShotInfoExpired, pos);
          }
        }
        else {
          if (wantShotInfo(ShotInfoStopped)) {
            serverLink->sendShotInfo(getPath(), ShotInfoStopped, pos,
                                     endObstacle->getGUID());
          }
        }
      }
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


bool SegmentedShotStrategy::predictPosition(float dt, fvec3& p) const {
  BzTime ctime = currentTime;
  ctime += dt;
  int cur = 0;
  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  while (cur < numSegments && segments[cur].end < ctime) { cur++; }
  if (cur >= numSegments) { return false; }

  segments[segment].ray.getPoint(float(ctime - segments[segment].start), p);

  return true;
}


bool SegmentedShotStrategy::predictVelocity(float dt, fvec3& p) const {
  BzTime ctime = currentTime;
  ctime += dt;
  int cur = 0;
  // see if we've moved to another segment
  const int numSegments = (const int)segments.size();
  while (cur < numSegments && segments[cur].end < ctime) { cur++; }
  if (cur >= numSegments) { return false; }

  p = segments[segment].ray.getDirection();

  return true;
}


void SegmentedShotStrategy::setCurrentTime(const BzTime& _currentTime) {
  currentTime = _currentTime;
}


const BzTime& SegmentedShotStrategy::getLastTime() const {
  return lastTime;
}


void SegmentedShotStrategy::setCurrentSegment(int _segment) {
  segment = _segment;
}


float SegmentedShotStrategy::checkHit(const ShotCollider& tank,
                                      fvec3& position) const {
  return checkShotHit(tank, position, BZDBCache::shotRadius);
}


void SegmentedShotStrategy::addShot(SceneDatabase* scene, bool colorblind) {
  const ShotPath& shotPath = getPath();
  boltSceneNode->move(shotPath.getPosition(), shotPath.getVelocity());
  if (boltSceneNode->getColorblind() != colorblind) {
    boltSceneNode->setColorblind(colorblind);
    TeamColor currentTeam = colorblind ? RogueTeam : team;

    const fvec4& color = Team::getRadarColor(currentTeam);
    boltSceneNode->setColor(color.r, color.g, color.b);

    TextureManager& tm = TextureManager::instance();
    std::string imageName = Team::getImagePrefix(currentTeam);
    imageName += BZDB.get("boltTexture");
    int texture = tm.getTextureID(imageName);
    if (texture >= 0) {
      boltSceneNode->setTexture(texture);
    }
  }
  scene->addDynamicNode(boltSceneNode);
}


void SegmentedShotStrategy::radarRender() const {
  const fvec3& orig = getPath().getPosition();
  const int length  = BZDBCache::linedRadarShots;
  const int size    = BZDBCache::sizedRadarShots;

  float shotTailLength = BZDB.eval(BZDBNAMES.SHOTTAILLENGTH);

  // Display leading lines
  if (length > 0) {
    const fvec3& vel = getPath().getVelocity();
    const fvec3  dir = vel.normalize() * shotTailLength * (float)length;
    glBegin(GL_LINES);
    glVertex2fv(orig);
    if (BZDBCache::leadingShotLine) {
      glVertex2fv(orig.xy() + dir.xy());
    }
    else {
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
  }
  else {
    if (size > 0) {
      // draw a sized bullet
      glPointSize((float)size);
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
      glPointSize(1.0f);

    }
    else {
      // draw the tiny little bullet
      glBegin(GL_POINTS);
      glVertex2fv(orig);
      glEnd();
    }
  }

}


void SegmentedShotStrategy::makeSegments(ObstacleEffect e) {
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
  BzTime startTime = shotPath.getStartTime();
  float  timeLeft  = shotPath.getLifetime();

  // minTime is used to move back to the tank's origin during the first for loop
  float minTime = 0.0f;
  const float speed = vel.length();
  if (speed > 0.0f) {
    minTime = BZDB.eval(BZDBNAMES.MUZZLEFRONT) / speed;
  }

  // if all shots ricochet and obstacle effect is stop, then make it ricochet
  if ((e == Stop) && world->allShotsRicochet()) {
    e = Reflect;
  }

  // prepare first segment
  fvec3 orig = shotPath.getPosition();

  ShotPathSegment::Reason reason = ShotPathSegment::Initial;
  const Obstacle* hitObstacle = NULL;
  int linkSrcID = -1;
  int linkDstID = -1;

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
    BzTime endTime = startTime;
    endTime += double((t < 0.0f) ? Epsilon : t);
    const Ray startRay(orig, vel);
    ShotPathSegment segm(startTime, endTime, startRay, reason);
    segm.hitObstacle = hitObstacle;
    if (reason == ShotPathSegment::Teleport) {
      segm.linkSrcID = linkSrcID;
      segm.linkDstID = linkDstID;
    }
    segments.push_back(segm);
    startTime = endTime;
    hitObstacle = NULL;
    linkSrcID = -1;
    linkDstID = -1;

    const fvec3 nextOrig = (orig + (t * vel));

    // if hit outer wall with ricochet and the hit is
    // above the top of the wall then ignore the hit.
    const bool ignoreHit = (building != NULL) && (e == Reflect) &&
                           (building->getTypeID() == wallType) &&
                           (nextOrig.z > building->getHeight());

    // check for teleportation
    const MeshFace* linkSrc = MeshFace::getShotLinkSrc(building);
    const MeshFace* linkDst = NULL;
    const LinkPhysics* physics = NULL;
    if (linkSrc != NULL) {
      const ShotPath& myPath = getPath();
      const FlagType* flagType = myPath.getFlagType();
      const TeamColor teamNum = myPath.getTeam();
      const unsigned int seed = shotPath.getShotId() + i;
      linkDst = linkManager.getShotLinkDst(linkSrc, seed,
                                           linkSrcID, linkDstID, physics,
                                           nextOrig, vel, teamNum, flagType);
      if (linkDst == NULL) {
        if (!teleFail) {
          if ((startTime - shotPath.getStartTime() + (double)t) < 1.0) {
            const MeshFace::SpecialData* sd = linkSrc->getSpecialData();
            const std::string& failMsg = sd->linkSrcShotFailText;
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
      endObstacle = building;
    }
    else if (linkSrc) {
      // move origin to point of teleport
      orig = nextOrig;
      // entered teleporter -- teleport it
      linkSrc->teleportShot(*linkDst, *physics, orig, orig, vel, vel);
      if (!physics->shotPassText.empty()) {
        addMessage(NULL, TextUtils::unescape_colors(physics->shotPassText));
      }
      reason = ShotPathSegment::Teleport;
    }
    else if (building) {
      // hit building -- can bounce off or stop, buildings ignored for Through

      hitObstacle = building;

      switch (e) {
        case Stop: {
          if (!building->canRicochet()) {
            timeLeft = 0.0f;
            endObstacle = building;
            break;
          }
          else {
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
    else if (hitGround) { // we hit the ground
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
  }
  else {
    bbox.reset();
  }

  if ((debugShotSegments >= 1) && !BZDBCache::forbidDebug) {
    debugf(0, "\n");
    debugf(0, "SegShotStrategy %i\n", (int)getPath().getFiringInfo().shot.id);
    for (size_t s = 0; s < segments.size(); s++) {
      const ShotPathSegment& sps = segments[s];
      const fvec3 endPos = sps.ray.getPoint((float)(sps.end - sps.start));
      const std::string reasonStr = ShotPathSegment::getReasonString(sps.reason).c_str();
      debugf(0, "  segment   %i\n", (int)s);
      debugf(0, "    start   %f\n", sps.start.getSeconds());
      debugf(0, "    end     %f\n", sps.end.getSeconds());
      debugf(0, "    orig    %s\n", sps.ray.getOrigin().tostring().c_str());
      debugf(0, "    endPos  %s\n", endPos.tostring().c_str());
      debugf(0, "    dir     %s\n", sps.ray.getDirection().tostring().c_str());
      debugf(0, "    reason  %s\n", reasonStr.c_str());
      debugf(0, "    hitObs  %p\n", sps.hitObstacle);
      debugf(0, "    mins    %s\n", sps.bbox.mins.tostring().c_str());
      debugf(0, "    maxs    %s\n", sps.bbox.maxs.tostring().c_str());
    }
    debugf(0, "  path mins:  %s\n", bbox.mins.tostring().c_str());
    debugf(0, "  path maxs:  %s\n", bbox.maxs.tostring().c_str());
  }
}


const std::vector<ShotPathSegment>& SegmentedShotStrategy::getSegments() const {
  return segments;
}


//
// NormalShotStrategy
//

NormalShotStrategy::NormalShotStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false) {
  // make segments
  makeSegments(Stop);
}


NormalShotStrategy::~NormalShotStrategy() {
  // do nothing
}


//
// RapidFireStrategy
//

RapidFireStrategy::RapidFireStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false) {
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(BZDBNAMES.RFIREADLIFE);
  float fireAdVel = BZDB.eval(BZDBNAMES.RFIREADVEL);
  f.shot.vel *= fireAdVel;
  setReloadTime(_path->getReloadTime()
                / BZDB.eval(BZDBNAMES.RFIREADRATE));

  // make segments
  makeSegments(Stop);
}


RapidFireStrategy::~RapidFireStrategy() {
  // do nothing
}


//
// ThiefStrategy
//

ThiefStrategy::ThiefStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false)
  , cummTime(0.0f) {
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(BZDBNAMES.THIEFADLIFE);
  float thiefAdVel = BZDB.eval(BZDBNAMES.THIEFADSHOTVEL);
  f.shot.vel *= thiefAdVel;
  setReloadTime(_path->getReloadTime()
                / BZDB.eval(BZDBNAMES.THIEFADRATE));

  // make segments
  makeSegments(Stop);
  setCurrentTime(getLastTime());
  endTime = f.lifetime;

  // make thief scene nodes
  const int numSegments = (const int)(getSegments().size());
  thiefNodes = new LaserSceneNode*[numSegments];

  TextureManager& tm = TextureManager::instance();
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

    thiefNodes[i]->setColor(0, 1, 1);
    thiefNodes[i]->setCenterColor(0, 0, 0);
  }
  setCurrentSegment(numSegments - 1);
}


ThiefStrategy::~ThiefStrategy() {
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++) {
    delete thiefNodes[i];
  }
  delete[] thiefNodes;
}


void ThiefStrategy::update(float dt) {
  cummTime += dt;
  if (cummTime >= endTime) {
    setExpired();
  }
}


void ThiefStrategy::addShot(SceneDatabase* scene, bool) {
  // thief is so fast we always show every segment
  const size_t numSegments = (getSegments().size());
  for (size_t i = 0; i < numSegments; i++) {
    scene->addDynamicNode(thiefNodes[i]);
  }
}


void ThiefStrategy::radarRender() const {
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


bool ThiefStrategy::isStoppedByHit() const {
  return false;
}


//
// MachineGunStrategy
//

MachineGunStrategy::MachineGunStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false) {
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(BZDBNAMES.MGUNADLIFE);
  float mgunAdVel = BZDB.eval(BZDBNAMES.MGUNADVEL);
  f.shot.vel *= mgunAdVel;
  setReloadTime(_path->getReloadTime()
                / BZDB.eval(BZDBNAMES.MGUNADRATE));

  // make segments
  makeSegments(Stop);
}


MachineGunStrategy::~MachineGunStrategy() {
  // do nothing
}


//
// RicochetStrategy
//

RicochetStrategy::RicochetStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false) {
  // make segments that bounce
  makeSegments(Reflect);
}


RicochetStrategy::~RicochetStrategy() {
  // do nothing
}


//
// SuperBulletStrategy
//

SuperBulletStrategy::SuperBulletStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, true) {
  // make segments that go through buildings
  makeSegments(Through);
}

SuperBulletStrategy::~SuperBulletStrategy() {
  // do nothing
}


//
// PhantomBulletStrategy
//

PhantomBulletStrategy::PhantomBulletStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false, true) {
  // make segments that go through buildings
  makeSegments(Through);
}

PhantomBulletStrategy::~PhantomBulletStrategy() {
  // do nothing
}


//
// LaserStrategy
//

LaserStrategy::LaserStrategy(ShotPath* _path)
  : SegmentedShotStrategy(_path, false)
  , cummTime(0.0f) {
  // speed up shell and decrease lifetime
  FiringInfo& f = getFiringInfo(_path);
  f.lifetime *= BZDB.eval(BZDBNAMES.LASERADLIFE);
  float laserAdVel = BZDB.eval(BZDBNAMES.LASERADVEL);
  f.shot.vel *= laserAdVel;
  setReloadTime(_path->getReloadTime()
                / BZDB.eval(BZDBNAMES.LASERADRATE));

  // make segments
  makeSegments(Stop);
  setCurrentTime(getLastTime());
  endTime = f.lifetime;

  // make laser scene nodes
  const int numSegments = (const int)(getSegments().size());
  laserNodes = new LaserSceneNode*[numSegments];
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  TeamColor tmpTeam =
    (myTank->getFlagType() == Flags::Colorblindness) ? RogueTeam : team;

  TextureManager& tm = TextureManager::instance();
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
    if (texture >= 0) {
      laserNodes[i]->setTexture(texture);
    }

    const fvec4& color = Team::getRadarColor(tmpTeam);
    laserNodes[i]->setColor(color.r, color.g, color.b);

    if (i == 0) {
      laserNodes[i]->setFirst();
    }
  }
  setCurrentSegment(numSegments - 1);
}


LaserStrategy::~LaserStrategy() {
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++) {
    delete laserNodes[i];
  }
  delete[] laserNodes;
}


void LaserStrategy::update(float dt) {
  cummTime += dt;
  if (cummTime >= endTime) {
    setExpired();
  }
}


void LaserStrategy::addShot(SceneDatabase* scene, bool) {
  // laser is so fast we always show every segment
  const size_t numSegments = getSegments().size();
  for (size_t i = 0; i < numSegments; i++) {
    scene->addDynamicNode(laserNodes[i]);
  }
}


void LaserStrategy::radarRender() const {
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


bool LaserStrategy::isStoppedByHit() const {
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
