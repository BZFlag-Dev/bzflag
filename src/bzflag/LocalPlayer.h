/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_LOCAL_PLAYER_H
#define	BZF_LOCAL_PLAYER_H

#include "common.h"
#include "Player.h"
#include "ShotPath.h"
#include "FlagSceneNode.h"
#include "Ray.h"
#include "BzfEvent.h"

class Obstacle;

// FIXME -- clean this up (needed for the robot tanks)
class BaseLocalPlayer : public Player {
  public:
			BaseLocalPlayer(const PlayerId&,
					const char* name, const char* email);
			~BaseLocalPlayer();

    void		update();
    Ray			getLastMotion() const;
#ifdef __MWERKS__
    const float	(*getLastMotionBBox() )[3] const;
#else
    const float		(*getLastMotionBBox() const)[3];
#endif

    virtual void	explodeTank() = 0;
    virtual bool	checkHit(const Player* source,
				const ShotPath*& hit, float& minTime) const = 0;

  protected:
    int			getSalt();
    virtual void	doUpdate(float dt) = 0;
    virtual void	doUpdateMotion(float dt) = 0;

  protected:
    TimeKeeper		lastTime;
    float		lastPosition[3];
    float		bbox[2][3];		// bbox of last motion

  private:
    int			salt;
};

class LocalPlayer : public BaseLocalPlayer {
  public:
    enum FiringStatus {
			Deceased,		// can't shoot cos I'm dead
			Ready,			// ready to shoot
			Loading,		// reloading
			Sealed,			// I'm inside a building
			Zoned			// I'm zoned
    };
    enum Location {
			Dead,			// dead, explosion over
			Exploding,		// dead and exploding
			OnGround,		// playing on ground
			InBuilding,		// playing in building
			OnBuilding,		// playing on building
			InAir			// playing in air
    };

			LocalPlayer(const PlayerId&,
					const char* name, const char* email);
			~LocalPlayer();

    Location		getLocation() const;
    FiringStatus	getFiringStatus() const;
    float		getReloadTime() const;
    float		getFlagShakingTime() const;
    int			getFlagShakingWins() const;
    const GLfloat*	getAntidoteLocation() const;
    ShotPath*		getShot(int index) const;
    int			getMagnify() const;
    const Player*	getTarget() const;
    const Obstacle*	getContainingBuilding() const;

    void		setTeam(TeamColor);
    void		setDesiredSpeed(float fracOfMaxSpeed);
    void		setDesiredAngVel(float fracOfMaxAngVel);
    void		setPause(bool = true);
    bool		fireShot();
    void		explodeTank();
    void		jump();
    void		setMagnify(int zoom);
    void		setTarget(const Player*);

    void		setNemesis(const Player*);
    const Player*	getNemesis() const;

    void		setRecipient(const Player*);
    const Player*	getRecipient() const;

    void		restart(const float* pos, float azimuth);
    bool		checkHit(const Player* source, const ShotPath*& hit,
							float& minTime) const;

    void		setFlag(FlagDesc*);
    void		changeScore(short deltaWins, short deltaLosses, short deltaTeamKills);

    void		addAntidote(SceneDatabase*);

    bool		isKeyboardMoving() const;
    void		setKeyboardMoving(bool status);
    void		setKeyboardSpeed(float speed);
    void		setKeyboardAngVel(float angVel);
    float		getKeyboardSpeed() const;
    float		getKeyboardAngVel() const;
    void		setKey(int button, bool pressed);
    bool		getKeyPressed() const;
    int			getKeyButton() const;
    void		resetKey();
    void		setSlowKeyboard(bool slow);
    bool		hasSlowKeyboard() const;

    static LocalPlayer*	getMyTank();
    static void		setMyTank(LocalPlayer*);

  protected:
    bool		doEndShot(int index, bool isHit, float* pos);
    void		doUpdate(float dt);
    void		doUpdateMotion(float dt);
    void		doMomentum(float dt, float& speed, float& angVel);
    void		doForces(float dt, float* velocity, float& angVel);
    const Obstacle*	getHitBuilding(const float* pos, float angle,
				bool phased, bool& expel) const;
    bool		getHitNormal(const Obstacle* o,
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float* normal) const;

  private:
    Location		location;
    FiringStatus	firingStatus;
    float		flagShakingTime;
    int			flagShakingWins;
    float		flagAntidotePos[3];
    FlagSceneNode*	antidoteFlag;
    float		desiredSpeed;
    float		desiredAngVel;
    float		lastSpeed;
    const Obstacle*	insideBuilding;
    GLfloat		crossingPlane[4];
    LocalShotPath**	shots;
    bool		anyShotActive;
    int			magnify;
    const Player*	target;
    const Player*	nemesis;
    const Player*	recipient;
    static LocalPlayer*	mainPlayer;
    bool		keyboardMoving;
    float		keyboardSpeed;
    float		keyboardAngVel;
    int			keyButton;
    bool		keyPressed;
    bool		slowKeyboard;
};

//
// LocalPlayer
//

inline LocalPlayer::Location		LocalPlayer::getLocation() const
{
  return location;
}

inline LocalPlayer::FiringStatus	LocalPlayer::getFiringStatus() const
{
  return firingStatus;
}

inline int		LocalPlayer::getMagnify() const
{
  return magnify;
}

inline const Player*	LocalPlayer::getTarget() const
{
  return target;
}

inline const Player*	LocalPlayer::getNemesis() const
{
  return nemesis;
}

inline const Player*	LocalPlayer::getRecipient() const
{
  return recipient;
}

inline const Obstacle*	LocalPlayer::getContainingBuilding() const
{
  return insideBuilding;
}

inline bool LocalPlayer::isKeyboardMoving() const
{
  return keyboardMoving;
}

inline void LocalPlayer::setKeyboardMoving(bool status)
{
  keyboardMoving = status;
}

inline void LocalPlayer::setKeyboardSpeed(float speed)
{
  keyboardSpeed = speed;
}

inline void LocalPlayer::setKeyboardAngVel(float angVel)
{
  keyboardAngVel = angVel;
}

inline float LocalPlayer::getKeyboardSpeed() const
{
  return keyboardSpeed;
}

inline float LocalPlayer::getKeyboardAngVel() const
{
  return keyboardAngVel;
}

inline void LocalPlayer::setKey(int button, bool pressed)
{
  keyButton = button;
  keyPressed = pressed;
}

inline void LocalPlayer::resetKey()
{
  keyButton = BzfKeyEvent::NoButton;
  keyPressed = false;
}

inline bool LocalPlayer::getKeyPressed() const
{
  return keyPressed;
}

inline int LocalPlayer::getKeyButton() const
{
  return keyButton;
}

inline void LocalPlayer::setSlowKeyboard(bool slow)
{
  slowKeyboard = slow;
}

inline bool LocalPlayer::hasSlowKeyboard() const
{
  return slowKeyboard;
}

#endif // BZF_LOCAL_PLAYER_H
// ex: shiftwidth=2 tabstop=8
