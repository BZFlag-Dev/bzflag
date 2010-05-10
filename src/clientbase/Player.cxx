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

// interface header
#include "Player.h"

// common interface headers
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "CollisionManager.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "ObstacleList.h"
#include "WallObstacle.h"
#include "MeshFace.h"
#include "MeshObstacle.h"
#include "ClientIntangibilityManager.h"
#include "MotionUtils.h"
#include "EventHandler.h"

// local implementation headers
#include "World.h"
#include "TrackMarks.h"
#include "sound.h"
#include "EffectsRenderer.h"
#include "Roaming.h"
#include "GameTime.h"

#include "playing.h"
// FIXME: Shouldn't need to depend on GUI elements
#include "guiplaying.h"

// for dead reckoning
static const float MaxUpdateTime = 1.0f; // seconds


//============================================================================//
//============================================================================//
//
//  Dead-reckoning gets its own debugging routine, woohoo
//


static BZDB_bool bzdbDebugDR("debugDR");


static inline void debugDR(const char* fmt, ...)
{
  if (!bzdbDebugDR || BZDBCache::forbidDebug) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  logDebugMessageArgs(0, fmt, ap);
  va_end(ap);
}


//============================================================================//
//============================================================================//
//
// Player
//

int Player::tankTexture = -1;


Player::Player(const PlayerId& _id, TeamColor _team,
	       const char* name, const PlayerType _type)
: lastObstacle(NULL)
, handicap(0.0f)
, notResponding(false)
, hunted(false)
, autoHuntLevel(0)
, id(_id)
, admin(false)
, registered(false)
, verified(false)
, playerList(false)
, gfxBlock(GfxBlock::Tank, id, true)
, radarGfxBlock(GfxBlock::TankRadar, id, true)
, lastVisualTeam(NoTeam)
, team(_team)
, type(_type)
, flagID(-1)
, flagType(Flags::Null)
, teleLinkSrcID(0)
, teleLinkDstID(0)
, teleporterProximity(0.0f)
, rank(0.42f)			// rank is received from the server on join
, wins(0)
, losses(0)
, tks(0)
, allow(0)
, localWins(0)
, localLosses(0)
, localTks(0)
, paused(false)
, autoPilot(false)
, deltaTime(0.0)
, offset(0.0)
, deadReckoningState(0)
, oldStatus(0)
, oldZSpeed(0.0f)
, lag(0.0f)
, jitter(0.0f)
, packetLoss(0.0f)
{
  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  move(zero, 0.0f);
  setVelocity(zero);
  setAngularVelocity(0.0f);
  setPhysicsDriver(-1);
  setDeadReckoning();
  setRelativeMotion();
  setUserSpeed(0.0f);
  setUserAngVel(0.0f);

  lastLanding = BzTime::getCurrent();

  // set call sign
  ::strncpy(callSign, name, CallSignLen);
  callSign[CallSignLen - 1] = '\0';

  // only get an avtar if it can be drawn
  if (id != ServerPlayer && !headless) {
    avatar = getPlayerAvatar(id,state.pos,forward);
  } else {
    avatar = NULL;
  }

  // setup the dimension properties
  dimensions.x = 0.5f * BZDBCache::tankLength;
  dimensions.y = 0.5f * BZDBCache::tankWidth;
  dimensions.z = BZDBCache::tankHeight;

  for (int i = 0; i < 3; i++) {
    dimensionsRate[i] = 0.0f;
    dimensionsScale[i] = 1.0f;
    dimensionsTarget[i] = 1.0f;
  }
  useDimensions = false;

  // setup alpha properties
  alpha = 1.0f;
  alphaRate = 0.0f;
  alphaTarget = 1.0f;
  teleAlpha = 1.0f;

  haveIpAddr = false; // no IP address yet
  lastTrackDraw = BzTime::getCurrent();

  spawnTime = BzTime::getCurrent();

  shotType = StandardShot;
}


Player::~Player()
{
  // free shots
  const int numShots = getMaxShots();
  for (int i = 0; i < numShots; i++) {
    if (shots[i]) {
      deleteShot(i);
    }
  }
  freePlayerAvatar (avatar);
}


float Player::getTKRatio() const
{
  if (wins == 0) {
    return (float)tks / 1.0f;    // well, we have to return *something* if they have no wins
  } else {
    return (float)tks / (float)wins;
  }
}


// returns a value between 1.0 and -1.0
float Player::getNormalizedScore() const
{
  return ((float)wins - losses) / ((wins+losses>20) ? wins+losses : 20);
}


void Player::forceReload(float time)
{
  jamTime = BzTime::getCurrent();
  jamTime+= time;
}


float Player::getReloadTime() const
{
  World *world = World::getWorld();
  if (!world) {
    return 0.0f;
  }

  const int numShots = world->getMaxShots();
  if (numShots <= 0) {
    return 0.0f;
  }

  float time = float(jamTime - BzTime::getCurrent());
  if (time > 0.0f) {
    return time;
  }

  // look for an empty slot
  int i;
  for (i = 0; i < numShots; i++) {
    if (!shots[i]) {
      return 0.0f;
    }
  }

  // look for the shot fired least recently
  float minTime = float(shots[0]->getReloadTime() - (shots[0]->getCurrentTime() - shots[0]->getStartTime()));
  for (i = 1; i < numShots; i++) {
    const float t = float(shots[i]->getReloadTime() - (shots[i]->getCurrentTime() - shots[i]->getStartTime()));
    if (minTime > t) {
      minTime = t;
    }
  }

  if (minTime < 0.0f) {
    minTime = 0.0f;
  }

  return minTime;
}


float Player::getLocalNormalizedScore() const
{
  return ((float)localWins - localLosses) /
         ((localWins+localLosses > 5) ? localWins+localLosses : 5);
}


float Player::getRabbitScore() const
{
  return rank;
}


float Player::getRadius() const
{
  // NOTE: this encompasses everything but Narrow
  //       the Obese, Tiny, and Thief flags adjust
  //       the radius, but Narrow does not.
  return (dimensionsScale.x * BZDBCache::tankRadius);
}


float Player::getMaxSpeed ( void ) const
{
  // BURROW and AGILITY will not be taken into account
  const FlagType* flag = getFlagType();
  float maxSpeed = BZDBCache::tankSpeed;
  if (flag == Flags::Velocity) {
    maxSpeed *= BZDB.eval(BZDBNAMES.VELOCITYAD);
  } else if (flag == Flags::Thief) {
    maxSpeed *= BZDB.eval(BZDBNAMES.THIEFVELAD);
  }
  return maxSpeed;
}


void Player::getMuzzle(fvec3& m) const
{
  // NOTE: like getRadius(), we only use dimensionsScale.x.
  //       as well, we do not use BZDB_MUZZLEFRONT, but the
  //       0.1f value listed in global.cxx is added on to the
  //       scaled version of tankRadius.

  float front = dimensions.x;
  static BZDB_fvec3 shotProxy(BZDBNAMES.TANKSHOTPROXIMITY);
  const fvec3& sp = shotProxy;
  if (!isnan(sp.x)) {
    front += sp.x;
  }

  if (dimensionsRate.x > 0.0f) {
    front += (dimensionsRate.x * 0.1f);
  }

  front += 0.1f;

  m = state.pos;
  m.xy() += front * forward.xy();
  m.z    += (BZDBCache::muzzleHeight * dimensionsScale.z);
}


float Player::getMuzzleHeight() const
{
  return (dimensionsScale.z * BZDBCache::muzzleHeight);
}


void Player::move(const fvec3& _pos, float _azimuth)
{
  // update the speed of the state
  BzTime currentTime = BzTime::getCurrent();
  const double timeDiff = (currentTime - state.lastUpdateTime);
  if (timeDiff > 0.0f) {
    state.apparentVelocity = (_pos - state.pos) / timeDiff;
  }

  state.lastUpdateTime = currentTime;

  // assumes _forward is normalized
  state.pos = _pos;
  state.azimuth = _azimuth;

  // limit angle
  if (state.azimuth < 0.0f) {
    state.azimuth = (float)((2.0 * M_PI) - fmodf(-state.azimuth, (float)(2.0 * M_PI)));
  } else if (state.azimuth >= (2.0f * M_PI)) {
    state.azimuth = fmodf(state.azimuth, (float)(2.0 * M_PI));
  }

  // update forward vector (always in horizontal plane)
  forward.x = cosf(state.azimuth);
  forward.y = sinf(state.azimuth);
  forward.z = 0.0f;

  // compute teleporter proximity
  if (World::getWorld()) {
    teleporterProximity =
      World::getWorld()->getProximity(state.pos, BZDBCache::tankRadius);
  }
}


void Player::setVelocity(const fvec3& _velocity)
{
  state.velocity = _velocity;
}


void Player::setAngularVelocity(float _angVel)
{
  state.angVel = _angVel;
}


void Player::setPhysicsDriver(int driver)
{
  state.phydrv = driver;

  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
  if (phydrv != NULL) {
    state.status |= PlayerState::OnDriver;
  } else {
    state.status &= ~PlayerState::OnDriver;
  }
}


void Player::setUserSpeed(float speed)
{
  state.userSpeed = speed;
}


void Player::setUserAngVel(float angVel)
{
  state.userAngVel = angVel;
}


void Player::changeTeam(TeamColor _team)
{
  const TeamColor oldTeam = team;

  // set team
  team = _team;

  if (team == ObserverTeam) {
    state.status = PlayerState::DeadStatus;
  }

  // set the scene node
  if (!headless) {
    setVisualTeam(team);
  }

  if (team != oldTeam) {
    eventHandler.PlayerTeamChange(*this, (int)oldTeam);
  }
}


void Player::setStatus(short _status)
{
  state.status = _status;
}


void Player::setExplode(const BzTime& t)
{
  if (!isAlive()) {
    return;
  }

  explodeTime = t;

  const short setBits = short(PlayerState::Exploding)
                      | short(PlayerState::Falling);
  const short clearBits = short(PlayerState::Alive);
  setStatus((getStatus() | setBits) & ~clearBits);

  paused = false;

  if (avatar) {
    avatar->explode();
    updateFlagEffect(Flags::Null);  // setup the flag effect to revert to normal
  }
}


void Player::setTeleport(const BzTime& t, short srcID, short dstID)
{
  if (!isAlive()) {
    return;
  }
  teleportTime = t;
  teleLinkSrcID = srcID;
  teleLinkDstID = dstID;
  setStatus(getStatus() | short(PlayerState::Teleporting));
}


void Player::updateTank(float dt, bool local)
{
  updateDimensions(dt, local);
  updateTranslucency(dt);
  updateTreads(dt);
  updateJumpJets(dt);
  updateTrackMarks();
}


void Player::updateJumpJets(float dt)
{
  float jumpVel = computeJumpVelocity(getFlagType());
  const float jetTime = 0.5f * (jumpVel / -BZDBCache::gravity);
  state.jumpJetsScale -= (dt / jetTime);
  if (state.jumpJetsScale < 0.0f) {
    state.jumpJetsScale = 0.0f;
    state.status &= ~PlayerState::JumpJets;
  }
}


void Player::updateTrackMarks()
{
  const float minSpeed = 0.1f; // relative speed slop

  if (isAlive() && !isFalling() && !isPhantomZoned()) {
    const float lifeTime = float(BzTime::getCurrent() - lastTrackDraw);
    if (lifeTime > TrackMarks::updateTime) {
      bool drawMark = true;
      fvec3 markPos;
      markPos.z = state.pos.z;
      // FIXME - again, this should be pulled for TankGeometryMgr
      const float fullLength = 6.0f;
      const float treadHeight = 1.2f;
      const float dist =
	dimensions.x * ((fullLength - treadHeight) / fullLength);

      if (relativeSpeed > +minSpeed) {
	// draw the mark at the back of the treads
	markPos.x = state.pos.x - (forward.x * dist);
	markPos.y = state.pos.y - (forward.y * dist);
      } else if (relativeSpeed < -minSpeed) {
	// draw the mark at the front of the treads
	markPos.x = state.pos.x + (forward.x * dist);
	markPos.y = state.pos.y + (forward.y * dist);
      } else {
	drawMark = false;
      }

      if (drawMark) {
	TrackMarks::addMark(markPos, dimensionsScale.y,
			    state.azimuth, state.phydrv);
	lastTrackDraw = BzTime::getCurrent();
      }
    }
  }
}


void Player::updateDimensions(float dt, bool local)
{
  // copy the current information
  const fvec3 oldRates      = dimensionsRate;
  const fvec3 oldScales     = dimensionsScale;
  const fvec3 oldDimensions = dimensions;

  // update the dimensions
  bool resizing = false;
  for (int i = 0; i < 3; i++) {
    if (dimensionsRate[i] != 0.0f) {
      resizing = true;
      dimensionsScale[i] += (dt * dimensionsRate[i]);
      if (dimensionsRate[i] < 0.0f) {
	if (dimensionsScale[i] < dimensionsTarget[i]) {
	  dimensionsScale[i] = dimensionsTarget[i];
	  dimensionsRate[i] = 0.0f;
	}
      } else {
	if (dimensionsScale[i] > dimensionsTarget[i]) {
	  dimensionsScale[i] = dimensionsTarget[i];
	  dimensionsRate[i] = 0.0f;
	}
      }
    } else {
      // safety play, should not be required
      dimensionsScale[i] = dimensionsTarget[i];
    }
  }

  // set the actual dimensions based on the scale
  dimensions.x = dimensionsScale.x * (0.5f * BZDBCache::tankLength);
  dimensions.y = dimensionsScale.y * (0.5f * BZDBCache::tankWidth);
  dimensions.z = dimensionsScale.z * BZDBCache::tankHeight;

  // do not resize if it will cause a collision
  // only checked for the local player, remote is computationally expensive
  if (local) {
    // also do not bother with collision checking if we are not resizing
    if (resizing && hitObstacleResizing()) {
      // copy the old information
      dimensions      = oldDimensions;
      dimensionsScale = oldScales;
      dimensionsRate  = oldRates;
    }
  }

  // check if the dimensions are at a steady state
  if (dimensionsScale == dimensionsTarget) {
    useDimensions = false;
  } else {
    useDimensions = true;
  }
}


bool Player::hitObstacleResizing()
{
  const fvec3& dims = dimensions;

  // check walls
  const World* world = World::getWorld();
  if (!world) {
    return false;
  }

  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (unsigned int i = 0; i < walls.size(); i++) {
    const WallObstacle* wall = (const WallObstacle*) walls[i];
    if (wall->inBox(getPosition(), getAngle(), dims.x, dims.y, dims.z)) {
      return true;
    }
  }

  // check everything else
  const ObsList* olist =
    COLLISIONMGR.boxTest(getPosition(), getAngle(), dims.x, dims.y, dims.z);
  if (!olist) {
    return false;
  }

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const bool onTop = obs->isFlatTop() &&
      ((obs->getPosition().z + obs->getHeight()) <= getPosition().z);
    if (ClientIntangibilityManager::instance().getWorldObjectTangibility(obs) == 0 && !onTop &&
	obs->inBox(getPosition(), getAngle(), dims.x, dims.y, dims.z)) {
      return true;
    }
  }

  return false;
}


const Obstacle* Player::getHitBuilding(const fvec3& oldP, float oldA,
                                       const fvec3& p, float a,
                                       bool phased, bool& expel)
{
  const bool hasOOflag = (getFlagType() == Flags::OscillationOverthruster);
  const fvec3& dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(oldP, oldA, p, a,
                                                dims.x, dims.y, dims.z,
                                                !hasOOflag);

  expel = (obstacle != NULL);
  if (expel && phased) {
    const ObstacleType obsType = obstacle->getTypeID();
    expel = ((obsType == wallType) ||
             ((obsType == faceType) &&
              ((const MeshFace*)obstacle)->isLinkFace()));
  }

  return obstacle;
}


bool Player::getHitNormal(const Obstacle* o,
                          const fvec3& pos1, float azimuth1,
                          const fvec3& pos2, float azimuth2,
                          fvec3& normal) const
{
  const fvec3& dims = getDimensions();
  return o->getHitNormal(pos1, azimuth1, pos2, azimuth2,
			 dims.x, dims.y, dims.z, normal);
}


void Player::updateTranslucency(float dt)
{
  // update the alpha value
  if (alphaRate != 0.0f) {
    alpha += dt * alphaRate;
    if (alphaRate < 0.0f) {
      if (alpha < alphaTarget) {
	alpha = alphaTarget;
	alphaRate = 0.0f;
      }
    } else {
      if (alpha > alphaTarget) {
	alpha = alphaTarget;
	alphaRate = 0.0f;
      }
    }
  }

  // set the tankNode color
  if (isPhantomZoned()) {
    teleAlpha = 1.0f;
    color.a = 0.25f; // barely visible, regardless of teleporter proximity
  }
  else {
    World *world = World::getWorld();
    if (!world) {
      return;
    }
    teleporterProximity = world->getProximity(state.pos, BZDBCache::tankRadius);
    teleAlpha = (1.0f - (0.75f * teleporterProximity));

    if (alpha == 0.0f) {
      color.a = 0.0f; // not trusting FP accuracy
    } else {
      color.a = teleAlpha * alpha;
    }
  }
}


void Player::updateTreads(float dt)
{
  float speedFactor;
  float angularFactor;

  if ((state.status & PlayerState::UserInputs) != 0) {
    speedFactor   = state.userSpeed;
    angularFactor = state.userAngVel;
  } else {
    speedFactor   = relativeSpeed;
    angularFactor = relativeAngVel;
  }

  // setup the linear component
  if (dimensionsScale.x > 1.0e-6f) {
    speedFactor = speedFactor / dimensionsScale.x;
  } else {
    speedFactor = speedFactor * 1.0e6f;
  }

  // setup the angular component
  const float angularScale = 4.0f; // spin factor (at 1.0, the edges line up)
  angularFactor *= angularScale;
  const float halfWidth = 0.5f * BZDBCache::tankWidth;
  // not using dimensions.y, because it may be set to 0.001 by a Narrow flag
  angularFactor *= dimensionsScale.x * halfWidth;

  const float leftOff  = dt * (speedFactor - angularFactor);
  const float rightOff = dt * (speedFactor + angularFactor);

  if (avatar) {
    avatar->setTurnOffsets(leftOff, rightOff);
  }
}


void Player::changeScore(float newRank,
			 short newWins, short newLosses, short newTeamKills)
{
  rank   = newRank;
  wins   = newWins;
  losses = newLosses;
  tks    = newTeamKills;

  eventHandler.PlayerScoreChange(*this);
}


void Player::changeLocalScore(short dWins, short dLosses, short dTeamKills)
{
  localWins += dWins;
  localLosses += dLosses;
  localTks += dTeamKills;

//FIXME - changeLocalScore() needs eventHandler.PlayerScoreChange(*this); ?
}


void Player::setFlagID(int _flagID)
{
  const World* world = World::getWorld();

  if ((_flagID < 0) || !world || (_flagID >= world->getMaxFlags())) {
    flagID   = -1;
    flagType = Flags::Null;
  }
  else {
    flagID   = _flagID;
    flagType = world->getFlag(flagID).type;
  }
  updateFlagEffect(flagType);
}


void Player::updateFlagEffect(FlagType* effectFlag)
{
  float FlagEffectTime = BZDB.eval(BZDBNAMES.FLAGEFFECTTIME);
  if (FlagEffectTime <= 0.0f) {
    FlagEffectTime = 0.001f; // safety
  }

  // set the dimension targets
  dimensionsTarget = fvec3(1.0f, 1.0f, 1.0f);
  if (effectFlag == Flags::Obesity) {
    const float factor = BZDB.eval(BZDBNAMES.OBESEFACTOR);
    dimensionsTarget.x = factor;
    dimensionsTarget.y = factor;
  }
  else if (effectFlag == Flags::Tiny) {
    const float factor = BZDB.eval(BZDBNAMES.TINYFACTOR);
    dimensionsTarget.x = factor;
    dimensionsTarget.y = factor;
  }
  else if (effectFlag == Flags::Thief) {
    const float factor = BZDB.eval(BZDBNAMES.THIEFTINYFACTOR);
    dimensionsTarget.x = factor;
    dimensionsTarget.y = factor;
  }
  else if (effectFlag == Flags::Narrow) {
    dimensionsTarget.y = 0.001f;
  }

  // set the dimension rates
  for (int i = 0; i < 3; i++) {
    if (dimensionsTarget[i] != dimensionsScale[i]) {
      dimensionsRate[i] = dimensionsTarget[i] - dimensionsScale[i];
      dimensionsRate[i] = dimensionsRate[i] / FlagEffectTime;
    }
  }

  // set the alpha target
  if (effectFlag == Flags::Cloaking) {
    alphaTarget = 0.0f;
  } else {
    alphaTarget = 1.0f;
  }

  // set the alpha rate
  if (alphaTarget != alpha) {
    alphaRate = (alphaTarget - alpha) / FlagEffectTime;
  }
}


void Player::endShot(int index, bool isHit, bool showExplosion)
{
  fvec3 pos;
  if (doEndShot(index, isHit, pos) && showExplosion) {
    addShotExplosion(pos);
  }
}


void Player::setVisualTeam (TeamColor visualTeam)
{
  color.rgb() = Team::getTankColor(visualTeam).rgb();

  if (avatar) {
    avatar->setVisualTeam(visualTeam, color);
  }
}


void Player::fireJumpJets()
{
  state.jumpJetsScale = 1.0f;
  state.status |= PlayerState::JumpJets;
}


const std::string & Player::getCustomField ( const std::string & key )const
{
  static std::string emptyField;

  std::map<std::string,std::string>::const_iterator itr = customData.find( key);
  if (itr != customData.end()) {
    return itr->second;
  }

  return emptyField;
}


void Player::clearRemoteSounds()
{
  state.sounds = PlayerState::NoSounds;
  state.status &= ~PlayerState::PlaySound;
  return;
}


void Player::addRemoteSound(int sound)
{
  state.sounds |= sound;
  if (state.sounds != PlayerState::NoSounds) {
    state.status |= PlayerState::PlaySound;
  }
}


void Player::addToScene(SceneDatabase* scene, TeamColor effectiveTeam,
			bool inCockpit, bool seerView,
			bool showTreads, bool showIDL, bool thirdPerson)
{
  const fvec4 groundPlane(0.0f, 0.0f, 1.0f, 0.0f);

  if (gfxBlock.blocked()) {
    return; // don't draw anything
  }

  if (!isAlive() && !isExploding()) {
    return; // don't draw anything
  }

  World *world = World::getWorld();
  if (!world) {
    return; // no world, shouldn't add to scene
  }

  if (!avatar) {
    return;
  }

  // place the tank
  avatar->move(state.pos, forward);

  // only use dimensions if we aren't at steady state.
  // this is done because it's more expensive to use
  // GL_NORMALIZE then to use precalculated normals.
  if (useDimensions)
    avatar->setScale(dimensionsScale);
  else {
    teAvatarScaleModes mode = eNormal;	// TODO, have the server set a player scale for these things, so it's not up to the client

    if (flagType == Flags::Obesity)
      mode = eFat;
    else if (flagType == Flags::Tiny)
      mode = eTiny;
    else if (flagType == Flags::Narrow)
      mode = eThin;
    else if (flagType == Flags::Thief)
      mode = eThief;
    avatar->setScale(mode);
  }

  // is this tank fully cloaked?
  bool cloaked = (flagType == Flags::Cloaking) && (color.a == 0.0f);

  // in third person we draw like the radar does. cloak is visible, and ST is not
  if (thirdPerson)
    cloaked = (flagType == Flags::Stealth);

  if (cloaked && !seerView)
    return; // don't draw anything

  avatar->setVisualMode(inCockpit, showTreads);

  // adjust alpha for seerView
  if (seerView) {
    if (isPhantomZoned()) {
      color.a = 0.25f;
    } else {
      color.a = teleAlpha;
    }
  }

  setVisualTeam(effectiveTeam);

  if (isAlive()) {
    avatar->setAnimationValues(0,state.jumpJetsScale);

    std::vector<SceneNode*> nodeList = avatar->getSceneNodes();
    for ( int i = 0; i < (int)nodeList.size(); i++ ) {
      if (nodeList[i]) {
	scene->addDynamicNode(nodeList[i]);
      }
    }

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      fvec4 plane;
      const float a = atan2f(forward.y, forward.x);
      const fvec3& d = dimensions;

      const Obstacle* obs = world->hitBuilding(state.pos, a, d.x, d.y, d.z);
      if (obs && !obs->isCrossing(state.pos, a, d.x, d.y, d.z, &plane)) {
        obs = NULL;
      }

      if (!obs) {
        const MeshFace* face =
          world->crossingTeleporter(state.pos, a, d.x, d.y, d.z);
        if (face) {
          plane = face->getPlane();
          obs = face;
        }
      }

      if (obs && (obs->getTypeID() == faceType) &&
          (((const MeshFace*)obs)->linkSrcNoEffect())) {
        obs = NULL;
      }

      if (obs) {
	// stick in interdimensional lights node
	if (showIDL && GfxBlockMgr::halos.notBlocked()) {
	  avatar->moveIDL(plane);
	  nodeList = avatar->getIDLSceneNodes();
	  for ( int i = 0; i < (int)nodeList.size(); i++ ) {
	    if (nodeList[i])
	      scene->addDynamicNode(nodeList[i]);
	  }
	}

	// add clipping plane to tank node
        avatar->setClippingPlane(&plane);
      }
    }
    else if (getPosition().z < 0.0f) {
      // this should only happen with Burrow flags
      avatar->setClippingPlane(&groundPlane);
    } // isCrossingWall()
  }
  else if (isExploding() && (state.pos.z > ZERO_TOLERANCE)) {
    // isAlive()
    float t = float((BzTime::getTick() - explodeTime) /
		    BZDB.eval(BZDBNAMES.EXPLODETIME));
    if (t > 1.0f) {
      // FIXME - setStatus(DeadStatus);
      t = 1.0f;
    } else if (t < 0.0f) {
      // shouldn't happen but why take chances
      t = 0.0f;
    }
    // fade at the end of the explosion
    const float fadeRatio = 0.8f;
    if (t > fadeRatio) {
      fvec4 newColor = color;
      const float fadeFactor = (1.0f - t) / (1.0f - fadeRatio);
      newColor.a = color.a * fadeFactor;
      avatar->setColor(newColor);
    }
    avatar->setAnimationValues(t,0);
    avatar->setClippingPlane(&groundPlane); // shadows are not clipped

    std::vector<SceneNode*> nodeList = avatar->getSceneNodes();
    for ( int i = 0; i < (int)nodeList.size(); i++ ) {
      if (nodeList[i])
	scene->addDynamicNode(nodeList[i]);
    }
  }

  if (isAlive() && (isPaused() || isNotResponding())) {
    avatar->movePause(state.pos, 1.5f * BZDBCache::tankRadius * dimensionsScale.x);

    std::vector<SceneNode*> nodeList = avatar->getPauseSceneNodes();
    for (int i = 0; i < (int)nodeList.size(); i++) {
      if (nodeList[i]) {
	scene->addDynamicNode(nodeList[i]);
      }
    }
  }
}


void Player::setLandingSpeed(float velocity)
{
  static BZDB_float squishiness(BZDBNAMES.SQUISHFACTOR);
  if (squishiness < 0.001f) {
    return;
  }
  static BZDB_float squishTime(BZDBNAMES.SQUISHTIME);
  if (squishTime < 0.001f) {
    return;
  }
  const float gravity = BZDBCache::gravity;
  if (velocity > 0.0f) {
    velocity = 0.0f;
  }

  // Setup so that a drop height of BZDB_GRAVITY squishes
  // by a factor of 1/11, when BZDB_SQUISHFACTOR is set to 1
  //
  // G = gravity;  V = velocity;  D = fall distance; K = factor
  //
  // V = sqrt (2 * D * G)
  // V = sqrt(2) * G  { @ D = G)
  // scale = 1 / (1 + (K * V^2))
  // scale = 1 / (1 + (K * 2 * G^2))
  // set: (K * 2 * G^2) = 0.1
  // K = 0.1 / (2 * G^2)
  //
  float k = 0.1f / (2.0f * gravity * gravity);
  k = k * squishiness;
  if (flagType == Flags::Bouncy) {
    k = k * 4.0f;
  }
  const float newZscale = 1.0f / (1.0f + (k * (velocity * velocity)));

  // don't update if the tank is still recovering
  // from a spawn effect or a larger fall.
  if (newZscale < dimensionsScale.z) {
    dimensionsScale.z = newZscale;
    // use a fixed decompression rate
    dimensionsRate.z = 1.0f / squishTime;
  }

  eventHandler.PlayerLanded(*this, velocity);

  return;
}


void Player::spawnEffect()
{
  const float squishiness = BZDB.eval(BZDBNAMES.SQUISHFACTOR);
  if (squishiness >= 0.001f) {
    const float effectTime = BZDB.eval(BZDBNAMES.FLAGEFFECTTIME);
    const float factor = (1.0f / effectTime);
    dimensionsRate = fvec3(factor, factor, factor);
    dimensionsScale = fvec3(0.01f, 0.01f, 0.01f);
  }
  spawnTime = BzTime::getCurrent();
  return;
}


void Player::addShots(SceneDatabase* scene, bool colorblind ) const
{
  const int count = getMaxShots();
  for (int i = 0; i < count; i++) {
    ShotPath* shot = getShot(i);
    if (shot && !shot->isExpiring() && !shot->isExpired()) {
      if (shot->getGfxBlock().notBlocked()) {
        shot->addShot(scene, colorblind);
      }
    }
  }
}



bool Player::validTeamTarget(const Player *possibleTarget) const
{
  TeamColor myTeam = getTeam();
  TeamColor targetTeam = possibleTarget->getTeam();
  if (myTeam != targetTeam) {
    return true;
  }

  if (myTeam != RogueTeam) {
    return false;
  }

  World *world = World::getWorld();
  return (world && !world->allowRabbit());
}


void Player::renderRadar() const
{
  if (!isAlive()) {
    return; // don't draw anything
  }
  if (avatar) {
    avatar->renderRadar();
  }
}


bool Player::getIpAddress(Address& addr)
{
  if (!haveIpAddr) {
    return false;
  }
  addr = ipAddr;
  return true;
}


void Player::setIpAddress(const Address& addr)
{
  ipAddr = addr;
  haveIpAddr = true;
}


ShotPath* Player::getShot(int index) const
{
  index &= 0x00FF;
  if (index >= (int)shots.size()) {
    return NULL;
  }
  return shots[index];
}


void Player::prepareShotInfo(FiringInfo &firingInfo, bool local)
{
  firingInfo.timeSent = GameTime::getStepTime();
  firingInfo.shot.dt = 0.0f;
  firingInfo.lifetime = BZDB.eval(BZDBNAMES.RELOADTIME);

  firingInfo.flagType = getFlagType();
  // wee bit o hack -- if phantom flag but not phantomized
  // the shot flag is normal -- otherwise FiringInfo will have
  // to be changed to add a real bitwise status variable
  if (getFlagType() == Flags::PhantomZone && !isFlagActive()) {
    firingInfo.shotType = StandardShot;
  }

  firingInfo.shot.team = getTeam();

  if (local) {
    if (firingInfo.shotType == ShockWaveShot) {
      firingInfo.shot.pos = getPosition();
    }
    else {
      getMuzzle(firingInfo.shot.pos);

      const fvec3& tankDir     = getForward();
      const fvec3& tankVel = getVelocity();
      float shotSpeed      = BZDB.eval(BZDBNAMES.SHOTSPEED);

      if (handicap > 0.0f) {
	// apply any handicap advantage to shot speed
	const float speedAd = 1.0f + (handicap * (BZDB.eval(BZDBNAMES.HANDICAPSHOTAD) - 1.0f));
	shotSpeed *= speedAd;
      }

      firingInfo.shot.vel = tankVel + (shotSpeed * tankDir);

      // Set _shotsKeepVerticalVelocity on the server if you want shots
      // to have the same vertical velocity as the tank when fired.
      // keeping shots moving horizontally makes the game more playable.
      if (!BZDB.isTrue(BZDBNAMES.SHOTSKEEPVERTICALV)) {
	firingInfo.shot.vel.z = 0.0f;
      }
    }
  }
}

ShotPath *Player::addShot(ShotPath *shot, const FiringInfo &info)
{
  int shotNum = int(shot->getShotId() & 255);

  if (shotNum >= (int)shots.size()) {
    shots.resize(shotNum+1);
  } else if (shots[shotNum] != NULL) {
    deleteShot(shotNum);
  }

  shots[shotNum] = shot;
  shotStatistics.recordFire(info.flagType, getForward(), shot->getVelocity());

  return shot;
}

void Player::deleteShot(int index)
{
  if (shots[index] != NULL) {
    delete shots[index];
    shots[index] = NULL;
  }
}

void Player::updateShot(FiringInfo &info, int shotID)
{
  // kill the old shot
  if ((shotID < 0) || (shotID >= (int)shots.size())) {
    return;
  }

  if (shots[shotID] != NULL) {
    deleteShot(shotID);
  }

  // build a new shot with the new info
  prepareShotInfo(info);
  shots[shotID] = new LocalShotPath(info);
}


void Player::setHandicap(float _handicap)
{
  handicap = _handicap;
}


//============================================================================//
//============================================================================//
//
//  Dead-reckoning related routine
//

void* Player::pack(void* buf, uint16_t& code)
{
  setDeadReckoning();
  return state.pack(buf, code);
}


void* Player::unpack(void* buf, uint16_t code)
{
  double timestamp;
  PlayerId ident;

  buf = nboUnpackUInt8(buf, ident);
  buf = nboUnpackDouble(buf, timestamp);

  debugDR("unpack: playerID = %i, timestamp = %g\n", getId(), timestamp);

  buf = state.unpack(buf, code);

  setDeadReckoning(timestamp);
//FIXME  setRelativeMotion();

  return buf;
}


void Player::setDeadReckoning()
{
  debugDR("setDeadReckoning: local\n");
  setDeadReckoning(GameTime::getStepTime());
}


void Player::setDeadReckoning(double timestamp)
{
  // set the current state
  debugDR("setDeadReckoning: playerID = %i, timestamp = %.14g\n",
          getId(), timestamp);

  // copy stuff for dead reckoning
  inputTime      = BzTime::getTick();
  inputTimestamp = timestamp;
  inputStatus    = state.status;
  inputPos       = state.pos;
  inputVel       = state.velocity;
  inputAzimuth   = state.azimuth;
  inputAngVel    = state.angVel;
  inputPhyDrv    = state.phydrv;

  // the relative motion information (with respect to the physics drivers)
  calcRelativeMotion(inputRelVel, inputRelSpeed, inputRelAngVel);

  // setup the turning parameters
  inputTurning = false;
  if (fabsf(inputRelAngVel) > 0.001f) {
    inputTurning = true;
    const float radius = (inputRelSpeed / inputRelAngVel);
    inputTurnVector.x = +sinf(inputAzimuth) * radius;
    inputTurnVector.y = -cosf(inputAzimuth) * radius;
    inputTurnCenter.x = inputPos.x - inputTurnVector.x;
    inputTurnCenter.y = inputPos.y - inputTurnVector.y;
  }

  setRelativeMotion();
}


void Player::doDeadReckoning()
{
  if (!isAlive() && !isExploding()) {
    return;
  }

  // get predicted state
  fvec3 predictedPos;
  fvec3 predictedVel;
  float predictedAzimuth;

  const double dt = BzTime::getTick() - inputTime;
  getDeadReckoning(predictedPos, predictedAzimuth, predictedVel, (float)dt);

  // setup notResponding
  if (!isAlive()) {
    notResponding = false;
  } else {
    notResponding = (dt > BZDB.eval(BZDBNAMES.NOTRESPONDINGTIME));
  }

  bool hitWorld = false;
  bool ZHit = false;

  // if the tanks hits something in Z then update input state (we don't want to fall anymore)
  float zLimit = 0.0f;
  if (getFlagType() == Flags::Burrow) {
    zLimit = BZDB.eval(BZDBNAMES.BURROWDEPTH);
  }

#ifdef DR_USES_HIT_CORRECTION // FIXME - this should probably be moved
  // check for collisions and correct accordingly
  World *world = World::getWorld();
  if (world) {
    bool expel;
    const Obstacle* obstacle = getHitBuilding(inputPos, inputAzimuth,
		  predictedPos, predictedAzimuth, !isSolid(), expel);

    // did they hit something?
    if (obstacle && expel) {
      fvec3 hitPos;
      fvec3 hitVelocity;
      fvec3 hitNormal;
      float hitAzimuth;

      // get the normal for the collision
      if (!getHitNormal(obstacle, inputPos, inputAzimuth,
		  predictedPos, predictedAzimuth, hitNormal)) {
        obstacle->getNormal(inputPos, hitNormal);
      }

      // get the corrected position
      // FIXME: Azimuth isn't corrected properly
      hitWorld = getHitCorrection(inputPos, inputAzimuth, predictedPos,
		  predictedAzimuth, inputVel, dt, zLimit, hitVelocity, hitPos,
		  &hitAzimuth);

      if (hitWorld) {
        // there was a collision, copy corrected position and velocity
        predictedPos = hitPos;
        predictedVel = hitVelocity;
        //predictedAzimuth = hitAzimuth;

        // check if the collision was in the Z direction
        if (hitNormal.z > 0.001f) {
          zLimit = hitPos.z;
          ZHit = true;
        }
      }
    }
  }
#endif // DR_USES_HIT_CORRECTION

  // the velocity check is for when a Burrow flag is dropped
  if (ZHit || ((predictedPos.z <= zLimit) &&
	       (predictedVel.z <= 0.0f))) {
    predictedPos.z = zLimit;
    predictedVel.z = 0.0f;
    inputStatus &= ~PlayerState::Falling;
    inputVel.z = 0.0f;
    hitWorld = true;
  }

  // setup remote players' landing sounds and graphics, and jumping sounds
  if (isAlive() && !headless) {
    // the importance level of the remote sounds
    const bool soundImportance = false;
    const bool localSound = (ROAM.isRoaming() && (ROAM.getMode() == Roaming::roamViewFP) && (ROAM.getTargetTank() == this));

    static BZDB_bool remoteSounds("remoteSounds");

    // check for a landing
    if (((oldStatus   & PlayerState::Falling) != 0) &&
        ((inputStatus & PlayerState::Falling) == 0)) {
      // setup the squish effect
      setLandingSpeed(oldZSpeed);

      // make it "land"
      EFFECTS.addLandEffect(getColor(), predictedPos, state.azimuth);

      // setup the sound
      if (remoteSounds) {
        if ((getFlagType() != Flags::Burrow) || (predictedPos.z > 0.0f)) {
          SOUNDSYSTEM.play(SFX_LAND, state.pos, soundImportance, localSound);
        } else {
          // probably never gets played
          SOUNDSYSTEM.play(SFX_BURROW, state.pos, soundImportance, localSound);
        }
      }
    }

    // play jumping type sounds, and then clear them
    if (state.sounds != PlayerState::NoSounds) {
      if (remoteSounds) {
        if ((state.sounds & PlayerState::JumpSound) != 0) {
          SOUNDSYSTEM.play(SFX_JUMP, state.pos, soundImportance, localSound);
        }
        if ((state.sounds & PlayerState::WingsSound) != 0) {
          SOUNDSYSTEM.play(SFX_FLAP, state.pos, soundImportance, localSound);
        }
        if ((state.sounds & PlayerState::BounceSound) != 0) {
          SOUNDSYSTEM.play(SFX_BOUNCE, state.pos, soundImportance, localSound);
        }
      }
      state.sounds = PlayerState::NoSounds;
    }
  }

  // copy some old state
  oldZSpeed = state.velocity.z;
  oldStatus = inputStatus;

  move(predictedPos, predictedAzimuth);
  setVelocity(predictedVel);
  setRelativeMotion();

#ifdef DR_USES_HIT_CORRECTION
  // if we hit something, then we want to DR from
  // here now, instead of from the starting point
  if (hitWorld) {
    debugDR("setDeadReckoning: hitWorld\n");
    BzTime newTimestamp = inputTimestamp;
    newTimestamp += (BzTime::getTick() - inputTime);
    setDeadReckoning(newTimestamp);
  }
#endif // DR_USES_HIT_CORRECTION
}


void Player::getDeadReckoning(fvec3& predictedPos, float& predictedAzimuth,
                              fvec3& predictedVel, float dt) const
{
  predictedAzimuth = inputAzimuth;

  if (paused) {
    // don't move when paused
    predictedPos = inputPos;
    predictedVel = fvec3(0.0f, 0.0f, 0.0f);
  }
  else if (inputStatus & PlayerState::Falling) {
    // no control when falling
    predictedVel.x = inputVel.x;
    predictedVel.y = inputVel.y;
    predictedPos.x = inputPos.x + (dt * inputVel.x);
    predictedPos.y = inputPos.y + (dt * inputVel.y);
    // only turn if alive
    if (inputStatus & PlayerState::Alive) {
      predictedAzimuth += (dt * inputAngVel);
    }
    // following the parabola
    predictedVel.z = inputVel.z + (BZDBCache::gravity * dt);
    predictedPos.z = inputPos.z + (inputVel.z * dt) +
      (0.5f * BZDBCache::gravity * dt * dt);
  }
  else {
    // velocity.z is zero when not falling, except for Burrow flag
    predictedVel.z = inputVel.z;
    predictedPos.z = inputPos.z + (inputVel.z * dt);

    // different algorithms for tanks moving in
    // a straight line vs. turning in a circle
    if (!inputTurning) {
      // move straight
      predictedVel.x = inputRelVel.x;
      predictedVel.y = inputRelVel.y;
      predictedPos.x = inputPos.x + (dt * inputRelVel.x);
      predictedPos.y = inputPos.y + (dt * inputRelVel.y);
    }
    else {
      // make a sweeping arc
      const float angle = (dt * inputRelAngVel);
      predictedAzimuth += angle;
      const float cos_val = cosf(angle);
      const float sin_val = sinf(angle);
      const fvec2& tc = inputTurnCenter;
      const fvec2& tv = inputTurnVector;
      predictedPos.x = tc.x + ((tv.x * cos_val) - (tv.y * sin_val));
      predictedPos.y = tc.y + ((tv.y * cos_val) + (tv.x * sin_val));
      const fvec2& rv = inputRelVel;
      predictedVel.x = (rv.x * cos_val) - (rv.y * sin_val);
      predictedVel.y = (rv.y * cos_val) + (rv.x * sin_val);
    }

    // make the physics driver adjustments
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(inputPhyDrv);
    if (phydrv != NULL) {
      if (phydrv->getIsSlide()) {
	predictedVel.x = inputRelVel.x;
	predictedVel.y = inputRelVel.y;
	predictedPos.x = inputPos.x + (dt * inputRelVel.x);
	predictedPos.y = inputPos.y + (dt * inputRelVel.y);
      }
      else {
	// radial velocity adjustment
	const float pdRadVel = phydrv->getRadialVel();
	if (pdRadVel != 0.0f) {
	  const fvec2& pdRadPos = phydrv->getRadialPos();
	  const fvec2 dir = (predictedPos.xy() - pdRadPos).normalize();
	  predictedPos.xy() += dt * pdRadVel * dir;
	  predictedVel.xy() += pdRadVel * dir;
	}
	// angular velocity adjustment
	const float pdAngVel = phydrv->getAngularVel();
	if (pdAngVel != 0.0f) {
	  const float angle = (dt * pdAngVel);
	  predictedAzimuth += angle;
	  const fvec2& pdAngPos = phydrv->getAngularPos();
	  const float dx = predictedPos.x - pdAngPos.x;
	  const float dy = predictedPos.y - pdAngPos.y;
	  const float cos_val = cosf(angle);
	  const float sin_val = sinf(angle);
	  predictedPos.x = pdAngPos.x + ((dx * cos_val) - (dy * sin_val));
	  predictedPos.y = pdAngPos.y + ((dy * cos_val) + (dx * sin_val));
	  predictedVel.x += (-dy * pdAngVel);
	  predictedVel.y += (+dx * pdAngVel);
	}
	// linear velocity adjustment
	const fvec3& pdVel = phydrv->getLinearVel();
	predictedPos.xy() += dt * pdVel.xy();
	predictedVel.xy() += pdVel.xy();
      }
    }
  }
}


bool Player::isDeadReckoningWrong() const
{
  // always send a new packet when some kinds of status change
  const uint16_t checkStates = (PlayerState::Alive | PlayerState::Falling);
  if ((state.status & checkStates) != (inputStatus & checkStates)) {
    debugDR("isDRWrong() status mismatch\n");
    return true;
  }

  // never send a packet when dead
  if ((state.status & PlayerState::Alive) == 0) {
    return false;
  }

  //  send a packet if we've made some noise
  if (state.sounds != PlayerState::NoSounds) {
    debugDR("isDRWrong() sounds\n");
    return true;
  }

  //  send a packet if we've crossed a physics driver boundary
  if (state.phydrv != inputPhyDrv) {
    debugDR("isDRWrong() phydrv\n");
    return true;
  }

  // time since setDeadReckoning
  const double dt = BzTime::getTick() - inputTime;

  // otherwise always send at least one packet per second
  if (dt >= MaxUpdateTime) {
    debugDR("isDRWrong() MaxUpdateTime %.3g vs %.3g\n",
                    dt, MaxUpdateTime);
    return true;
  }

  // get predicted state
  fvec3 predictedPos;
  fvec3 predictedVel;
  float predictedAzimuth;
  getDeadReckoning(predictedPos, predictedAzimuth, predictedVel, (float)dt);

  // always send a new packet on reckoned touchdown
  float groundLimit = 0.0f;
  if (getFlagType() == Flags::Burrow) {
    groundLimit = BZDB.eval(BZDBNAMES.BURROWDEPTH);
  }
  if (predictedPos.z < groundLimit) {
    debugDR("isDRWrong() predictPos.z < groundLimit\n");
    return true;
  }

  // client side throttling
  static BZDB_int throttleRate(BZDBNAMES.UPDATETHROTTLERATE);
  const float minUpdateTime = (throttleRate > 0) ? (1.0f / throttleRate) : 0.0f;
  if (dt < minUpdateTime) {
    return false;
  }

  // see if position and azimuth are close enough
  static BZDB_float positionTolerance(BZDBNAMES.POSITIONTOLERANCE);
  if ((fabsf(state.pos.x - predictedPos.x) > positionTolerance) ||
      (fabsf(state.pos.y - predictedPos.y) > positionTolerance) ||
      (fabsf(state.pos.z - predictedPos.z) > positionTolerance)) {
    if (bzdbDebugDR && !BZDBCache::forbidDebug) {
      if (fabsf(state.pos.x - predictedPos.x) > positionTolerance) {
	debugDR("isDRWrong() diff(pos.x) > posTolerance: %.3g vs %.3g\n",
		fabsf(state.pos.x - predictedPos.x), (float)positionTolerance);
      }
      if (fabsf(state.pos.y - predictedPos.y) > positionTolerance) {
	debugDR("isDRWrong() diff(pos.y) > posTolerance: %.3g vs %.3g\n",
		fabsf(state.pos.y - predictedPos.y), (float)positionTolerance);
      }
      if (fabsf(state.pos.z - predictedPos.z) > positionTolerance) {
	debugDR("isDRWrong() diff(pos.z) > posTolerance: %.3g vs %.3g\n",
		fabsf(state.pos.z - predictedPos.z), (float)positionTolerance);
      }
    }
    return true;
  }

  static BZDB_float angleTolerance(BZDBNAMES.ANGLETOLERANCE);
  if (fabsf(state.azimuth - predictedAzimuth) > angleTolerance) {
    debugDR("isDRWrong() diff(azimuth) > angleTolerance: %.3g vs %.3g\n",
            fabsf(state.azimuth - predictedAzimuth), (float)angleTolerance);
    return true;
  }

  // prediction is good enough
  return false;
}


void Player::setRelativeMotion()
{
  bool falling = (state.status & short(PlayerState::Falling)) != 0;
  if (falling && (getFlagType() != Flags::Wings)) {
    return;    // no adjustments while falling
  }

  // set 'relativeSpeed' and 'relativeAngVel'
  fvec2 relativeVel;
  Player::calcRelativeMotion(relativeVel, relativeSpeed, relativeAngVel);
}


void Player::calcRelativeMotion(fvec2& vel, float& speed, float& angVel)
{
  vel.x = state.velocity.x;
  vel.y = state.velocity.y;

  angVel = state.angVel;

  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(state.phydrv);
  if (phydrv != NULL) {
    // adjust for driver velocity
    const fvec3& v = phydrv->getLinearVel();
    vel.x -= v.x;
    vel.y -= v.y;

    // adjust for driver angular velocity
    const float av = phydrv->getAngularVel();
    if (av != 0.0f) {
      const fvec2& ap = phydrv->getAngularPos();
      const float dx = state.pos.x - ap.x;
      const float dy = state.pos.y - ap.y;
      vel.x += av * dy;
      vel.y -= av * dx;
      angVel = state.angVel - av;
    }

    // adjust for driver radial velocity
    const float rv = phydrv->getRadialVel();
    if (rv != 0.0f) {
      const fvec2& rp = phydrv->getRadialPos();
      vel += rv * (state.pos.xy() - rp).normalize();
    }
  }

  // speed relative to the tank's direction
  // (could use forward[] instead of re-doing the trig, but this is
  //  used in the setDeadReckoning(), when forward[] is not yet set)
  speed = (vel.x * cosf(state.azimuth)) + (vel.y * sinf(state.azimuth));
}


bool Player::getHitCorrection(const fvec3& startPos, const float startAzimuth,
                              const fvec3& /*endPos*/, const float /*endAzimuth*/,
                              const fvec3& startVelocity, double dt,
                              float groundLimit, fvec3& velocity, fvec3& position,
                              float* azimuth)
{
  // constants
  static const float MinSearchStep = 0.0001f;
  static const int MaxSearchSteps = 7;
  static const int MaxSteps = 4;
  static const float TinyDistance = 0.0001f;

  fvec3 newPos;
  fvec3 newVelocity;
  float newAzimuth;
  float newAngVel;
  bool hitWall;
  const Obstacle* obstacle;
  bool expel;
  float timeStep = (float)dt;

  newPos = startPos;
  newVelocity = startVelocity;
  newAngVel = state.angVel;
  newAzimuth = startAzimuth;
  hitWall = false;

  bool phased = !isSolid();

  for (int numSteps = 0; numSteps < MaxSteps; numSteps++) {
    // record position at beginning of time step
    fvec3 tmpPos;
    float tmpAzimuth;
    tmpAzimuth = newAzimuth;
    tmpPos = newPos;

    // get position at end of time step
    newAzimuth = tmpAzimuth + timeStep * newAngVel;
    newPos = tmpPos + (timeStep * newVelocity);
    if ((newPos.z < groundLimit) && (newVelocity.z < 0)) {
      // Hit lower limit, stop falling
      newPos.z = groundLimit;
	  if ((state.status & PlayerState::Exploding) != 0) {
	    // tank pieces reach the ground, friction
	    // stop them, & mainly player view
	    newPos.x = tmpPos.x;
	    newPos.y = tmpPos.y;
      }
    }

    // see if we hit anything.  if not then we're done.
    obstacle = getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (!obstacle || !expel)
      break;

    float obstacleTop = obstacle->getPosition().z + obstacle->getHeight();

    bool hasFlatTop = obstacle->isFlatTop();
    if (obstacle->getTypeID() == faceType) {
      MeshFace* topFace = ((MeshFace*)obstacle)->getTopNeighbor();
      if (topFace != NULL) {
        hasFlatTop = true;
      }
    }

    if (((inputStatus & PlayerState::Falling) == 0) && hasFlatTop &&
	(obstacleTop != tmpPos.z) &&
	(obstacleTop < (tmpPos.z + BZDB.eval(BZDBNAMES.MAXBUMPHEIGHT)))) {
      newPos.x = startPos.x;
      newPos.y = startPos.y;
      newPos.z = obstacleTop;

      // drive over bumps
      const Obstacle* bumpObstacle = getHitBuilding(newPos, tmpAzimuth,
						    newPos, newAzimuth,
						    phased, expel);
      if (!bumpObstacle) {
        move(newPos, getAngle());
        const float speedFactor = BZDB.eval("_bumpSpeedFactor");
        newPos.x += newVelocity.x * ((float)dt * speedFactor);
        newPos.y += newVelocity.y * ((float)dt * speedFactor);
        break;
      }
    }

    hitWall = true;

    // record position when hitting
    fvec3 hitPos;
    float hitAzimuth;
    hitAzimuth = newAzimuth;
    hitPos = newPos;

    // find the latest time before the collision
    float searchTime = 0.0f;
    float searchStep = 0.5f * timeStep;
    for (int i = 0; searchStep > MinSearchStep && i < MaxSearchSteps;
		searchStep *= 0.5f, i++) {
      // get intermediate position
      const float t = searchTime + searchStep;
      newAzimuth = tmpAzimuth + (t * newAngVel);
      newPos = tmpPos + (t * newVelocity);
      if ((newPos.z < groundLimit) && (newVelocity.z < 0)) {
        newPos.z = groundLimit;
      }

      // see if we hit anything
      bool searchExpel;
      const Obstacle* searchObstacle =
        getHitBuilding(tmpPos, tmpAzimuth,
                       newPos, newAzimuth, phased, searchExpel);

      if (!searchObstacle || !searchExpel) {
        // if no hit then search latter half of time step
        searchTime = t;
      }
      else if (searchObstacle) {
        // if we hit a building then record which one and where
        obstacle = searchObstacle;

        expel = searchExpel;
        hitAzimuth = newAzimuth;
        hitPos = newPos;
      }
    }

    // get position just before impact
    newAzimuth = tmpAzimuth + (searchTime * newAngVel);
    newPos = tmpPos + (searchTime * newVelocity);
    if (startPos.z < groundLimit) {
      newVelocity.z = std::max(newVelocity.z, -startPos.z / 2.0f + 0.5f);
    }

    // record how much time is left in time step
    timeStep -= searchTime;

    // get normal at intersection.  sometimes fancy test says there's
    // no intersection but we're expecting one so, in that case, fall
    // back to simple normal calculation.
    fvec3 normal;
    if (!getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal)) {
      obstacle->getNormal(newPos, normal);
    }

    // check for being on a building
    if ((newPos.z > 0.0f) && (normal.z > 0.001f)) {
      if (((state.status & PlayerState::DeadStatus) == 0) &&
          ((state.status & PlayerState::Exploding) == 0) && expel) {
        lastObstacle = obstacle;
      }
      newVelocity.z = 0.0f;
    }
    else {
      // get component of velocity in normal direction (in horizontal plane)
      float mag = (normal.x * newVelocity.x) +
		  (normal.y * newVelocity.y);

      // handle upward normal component to prevent an upward force
      if (!NEAR_ZERO(normal.z, ZERO_TOLERANCE)) {
        // if going down then stop falling
        if (newVelocity.z < 0.0f && newVelocity.z -
            (mag + normal.z * newVelocity.z) * normal.z > 0.0f) {
          newVelocity.z = 0.0f;
        }

        // normalize force magnitude in horizontal plane
        float horNormal = (normal.x * normal.x) + (normal.y * normal.y);
        if (!NEAR_ZERO(horNormal, ZERO_TOLERANCE)) {
          mag /= horNormal;
        }
      }

      // cancel out component in normal direction (if velocity and
      // normal point in opposite directions).  also back off a tiny
      // amount to prevent a spurious collision against the same
      // obstacle.
      if (mag < 0.0f) {
        newVelocity.x -= mag * normal.x;
        newVelocity.y -= mag * normal.y;
        if (!(getStatus() & PlayerState::BackedOff)) {
          newPos.x -= TinyDistance * mag * normal.x;
          newPos.y -= TinyDistance * mag * normal.y;
          setStatus(getStatus() | PlayerState::BackedOff);
        }
      }
      if (mag > -0.01f) {
        // assume we're not allowed to turn anymore if there's no
        // significant velocity component to cancel out.
        newAngVel = 0.0f;
      }
    }
  }

  velocity = newVelocity;
  position = newPos;
  *azimuth = newAzimuth;

  return hitWall;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
