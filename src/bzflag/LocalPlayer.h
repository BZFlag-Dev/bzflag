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

#ifndef __LOCALPLAYER_H__
#define __LOCALPLAYER_H__

/* interface header */
#include "BaseLocalPlayer.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "Obstacle.h"
#include "TimeKeeper.h"
#include "FlagSceneNode.h"
#include "BzfEvent.h"

/* local interface headers */
#include "Player.h"
#include "ServerLink.h"


class LocalPlayer : public BaseLocalPlayer {
public:
  enum FiringStatus {
    Deceased,		// can't shoot cos I'm dead
    Ready,		// ready to shoot
    Loading,		// reloading
    Sealed,		// I'm inside a building
    Zoned		// I'm zoned
  };
  enum Location {
    Dead,		// dead, explosion over
    Exploding,		// dead and exploding
    OnGround,		// playing on ground
    InBuilding,		// playing in building
    OnBuilding,		// playing on building
    InAir		// playing in air
  };
  enum InputMethod {	// what device am I using to move around
    Keyboard = 0,
    Mouse,
    Joystick
  };

  LocalPlayer(const PlayerId&,
	      const char* name, const char* email);
  ~LocalPlayer();

  Location	getLocation() const;
  FiringStatus	getFiringStatus() const;
  float		getReloadTime() const;
  float		getFlagShakingTime() const;
  int		getFlagShakingWins() const;
  const float*	getAntidoteLocation() const;
  ShotPath*	getShot(int index) const;
  const Player*	getTarget() const;
  const Obstacle*	getContainingBuilding() const;

  void		setTeam(TeamColor);
  void		setDesiredSpeed(float fracOfMaxSpeed);
  void		setDesiredAngVel(float fracOfMaxAngVel);
  void		setPause(bool = true);
  bool		fireShot();
  void		forceReload(float time = 0.0f);
  void		explodeTank();
  void		jump();
  void		setTarget(const Player*);

  void		setNemesis(const Player*);
  const Player*	getNemesis() const;

  void		setRecipient(const Player*);
  const Player*	getRecipient() const;

  void		restart(const float* pos, float azimuth);
  bool		checkHit(const Player* source, const ShotPath*& hit,
			 float& minTime) const;
  void		setFlag(FlagType*);
  void		changeScore(short deltaWins, short deltaLosses, short deltaTeamKills);
  float		updateHandicap();

  void		addAntidote(SceneDatabase*);

  InputMethod	getInputMethod() const;
  void		setInputMethod(InputMethod newInput);
  void		setInputMethod(std::string newInput);
  static std::string	getInputMethodName(InputMethod whatInput);
  bool		queryInputChange();
  void		setKey(int button, bool pressed);
  int           getRotation();
  int           getSpeed();
  bool		isSpawning();
  void		setSpawning( bool spawn );

  static LocalPlayer*	getMyTank();
  static void		setMyTank(LocalPlayer*);

  const Obstacle*	getHitBuilding(const float* pos, float angle,
				       bool phased, bool& expel) const;
  const Obstacle*	getHitBuilding(const float* oldPos, float oldAngle,
				       const float* pos, float angle,
				       bool phased, bool& expel) const;
  bool		getHitNormal(const Obstacle* o,
			     const float* pos1, float azimuth1,
			     const float* pos2, float azimuth2,
			     float* normal) const;


protected:
  bool		doEndShot(int index, bool isHit, float* pos);
  void		doUpdate(float dt);
  void		doUpdateMotion(float dt);
  void		doMomentum(float dt, float& speed, float& angVel);
  void		doFriction(float dt, const float *oldVelocity, float *newVelocity);
  void		doForces(float dt, float* velocity, float& angVel);
  LocalShotPath**	shots;
  bool          gettingSound;
  ServerLink*	server;

private:
  Location	location;
  FiringStatus	firingStatus;
  TimeKeeper	jamTime;
  TimeKeeper    bounceTime;
  TimeKeeper	agilityTime;
  float		flagShakingTime;
  int		flagShakingWins;
  float		flagAntidotePos[3];
  FlagSceneNode*	antidoteFlag;
  float		desiredSpeed;
  float		desiredAngVel;
  float		lastSpeed;
  const Obstacle*	insideBuilding;
  float		crossingPlane[4];
  bool		anyShotActive;
  const Player*	target;
  const Player*	nemesis;
  const Player*	recipient;
  static LocalPlayer*	mainPlayer;
  InputMethod	inputMethod;
  bool		inputChanged;
  int           stuckingFrameCount;
  bool		spawning;
  int		wingsFlapCount;
  float		handicap;
  int           rotation;
  int           speed;
};


inline LocalPlayer::Location LocalPlayer::getLocation() const
{
  return location;
}

inline LocalPlayer::FiringStatus LocalPlayer::getFiringStatus() const
{
  return firingStatus;
}

inline const Player* LocalPlayer::getTarget() const
{
  return target;
}

inline const Player* LocalPlayer::getNemesis() const
{
  return nemesis;
}

inline const Player* LocalPlayer::getRecipient() const
{
  return recipient;
}

inline const Obstacle* LocalPlayer::getContainingBuilding() const
{
  return insideBuilding;
}

inline LocalPlayer::InputMethod LocalPlayer::getInputMethod() const
{
  return inputMethod;
}

inline void LocalPlayer::setInputMethod(InputMethod newInput)
{
  inputMethod = newInput;
  inputChanged = true;
}

inline void LocalPlayer::setInputMethod(std::string newInput)
{
  // FIXME - using hardcoded upper bound is ugly
  for (int i = 0; i < 3; i++) {
    if (newInput == getInputMethodName((InputMethod)i))
      setInputMethod((InputMethod)i);
  }
}

inline bool LocalPlayer::queryInputChange()
{
  const bool returnVal = inputChanged;
  inputChanged = false;
  return returnVal;
}

inline bool LocalPlayer::isSpawning()
{
  return spawning;
}

inline void LocalPlayer::setSpawning( bool spawn )
{
  spawning = spawn;
}

inline int LocalPlayer::getRotation() {
  return rotation;
}

inline int LocalPlayer::getSpeed() {
  return speed;
}

#endif /* __LOCALPLAYER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
