/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_LOCAL_PLAYER_H
#define BZF_LOCAL_PLAYER_H

#include <string>
#include "Player.h"
#include "ShotPath.h"
#include "math3D.h"
#include "BzfEvent.h"
#include "RemotePlayer.h"
#include "World.h"

class Obstacle;
class SceneNodeGroup;
class SceneNodeTransform;

// FIXME -- clean this up (was needed for old robot tanks)
class BaseLocalPlayer : public Player {
public:
	BaseLocalPlayer(PlayerId,
						const char* name, const char* email);
	~BaseLocalPlayer();

	void					update();
	Ray						getLastMotion() const;
#ifdef __MWERKS__
    const float				(*getLastMotionBBox() )[3] const;
#else
    const float				(*getLastMotionBBox() const)[3];
#endif

    virtual void			explodeTank() = 0;
    virtual bool			checkHit(const Player* source,
							const ShotPath*& hit, float& minTime) const = 0;

protected:
	int						getSalt();
	virtual void			doUpdate(float dt) = 0;
	virtual void			doUpdateMotion(float dt) = 0;

protected:
	TimeKeeper				lastTime;
	float					lastPosition[3];
	float					bbox[2][3];				// bbox of last motion

private:
	int						salt;
};

class LocalPlayer : public BaseLocalPlayer {
public:
	enum FiringStatus {
		Deceased,				// can't shoot cos I'm dead
		Ready,					// ready to shoot
		Loading,				// reloading
		Sealed,					// I'm inside a building
		Zoned					// I'm zoned
	};
	enum Location {
		Dead,					// dead, explosion over
		Exploding,				// dead and exploding
		OnGround,				// playing on ground
		InBuilding,				// playing in building
		OnBuilding,				// playing on building
		InAir					// playing in air
	};
	enum _RoamView {
		RoamViewTrack = 0,			// tracking another player
		RoamViewFollow,				// following closely behind another player
		RoamViewFP,				// first-person 'driving with' view
		RoamViewFlag,				// tracking a specific flag
		RoamViewFree				// free roaming
	} RoamView;
	static const int		roamViewCount;

	LocalPlayer(PlayerId, const char* name, const char* email);
	~LocalPlayer();

	Location				getLocation() const;
	FiringStatus			getFiringStatus() const;
	float					getReloadTime() const;
	float					getFlagShakingTime() const;
	int						getFlagShakingWins() const;
	const float*			getAntidoteLocation() const;
	ShotPath*				getShot(int index) const;
	const Player*			getTarget() const;
	const Obstacle*			getContainingBuilding() const;

	void					setTeam(TeamColor);
	void					setDesiredSpeed(float fracOfMaxSpeed);
	void					setDesiredAngVel(float fracOfMaxAngVel);
	void					setPause(bool = true);
	bool					fireShot();
	void					explodeTank();
	void					jump();
	void					setTarget(const Player*);

	void					setNemesis(const Player*);
	const Player*			getNemesis() const;

	void					restart(const float* pos, float azimuth);
	bool					checkHit(const Player* source, const ShotPath*& hit,
								float& minTime) const;

	void					setFlag(FlagId);
	void					changeScore(short deltaWins, short deltaLosses);

	void					addAntidoteSceneNode(SceneNodeGroup*);

	bool					isKeyboardMoving() const;
	void					setKeyboardMoving(bool status);
	void 					setKeyboardSpeed(float speed);
	void 					setKeyboardAngVel(float angVel);
	float 					getKeyboardSpeed() const;
	float 					getKeyboardAngVel() const;
	void 					setKey(int button, bool pressed);
	bool					getKeyPressed() const;
	int 					getKeyButton() const;
	void					resetKey();
	void					setSlowKeyboard(bool slow);
	bool					hasSlowKeyboard() const;

	static LocalPlayer*		getMyTank();
	static void				setMyTank(LocalPlayer*);

	std::string				getRoamingStatus(RemotePlayer** players, World* world);

	// observer roaming
	int						roamTrackTank, roamTrackFlag;
	Vec3					roamPos, roamDPos;
	float					roamTheta, roamDTheta;
	float					roamPhi, roamDPhi;
	float					roamZoom, roamDZoom;

protected:
	bool					doEndShot(int index, bool isHit, float* pos);
	void					doUpdate(float dt);
	void					doUpdateMotion(float dt);
	void					doMomentum(float dt, float& speed, float& angVel);
	void					doForces(float dt, float* velocity, float& angVel);
	const Obstacle*			getHitBuilding(const float* pos, float angle,
							bool phased, bool& expel) const;
	bool					getHitNormal(const Obstacle* o,
								const float* pos1, float azimuth1,
								const float* pos2, float azimuth2,
								float* normal) const;

private:
	Location				location;
	FiringStatus			firingStatus;
	float					flagShakingTime;
	int						flagShakingWins;
	float					flagAntidotePos[3];
	SceneNodeTransform*		antidoteFlag;
	SceneNode*				rainSceneNode;
	float					desiredSpeed;
	float					desiredAngVel;
	float					lastSpeed;
	const Obstacle*			insideBuilding;
	float					crossingPlane[4];
	LocalShotPath**			shots;
	bool					anyShotActive;
	const Player*			target;
	const Player*			nemesis;
	static LocalPlayer*		mainPlayer;
	bool					keyboardMoving;
	float					keyboardSpeed;
	float					keyboardAngVel;
	int 					keyButton;
	bool            		keyPressed;
	bool            		slowKeyboard;
};

//
// LocalPlayer
//

inline LocalPlayer::Location	LocalPlayer::getLocation() const
{
	return location;
}

inline LocalPlayer::FiringStatus	LocalPlayer::getFiringStatus() const
{
	return firingStatus;
}

inline const Player*	LocalPlayer::getTarget() const
{
	return target;
}

inline const Player*	LocalPlayer::getNemesis() const
{
	return nemesis;
}

inline const Obstacle*	LocalPlayer::getContainingBuilding() const
{
	return insideBuilding;
}

inline bool				LocalPlayer::isKeyboardMoving() const
{
	return keyboardMoving;
}

inline void				LocalPlayer::setKeyboardMoving(bool status)
{
	keyboardMoving = status;
}

inline void				LocalPlayer::setKeyboardSpeed(float speed)
{
	keyboardSpeed = speed;
}

inline void				LocalPlayer::setKeyboardAngVel(float angVel)
{
	keyboardAngVel = angVel;
}

inline float			LocalPlayer::getKeyboardSpeed() const
{
	return keyboardSpeed;
}

inline float			LocalPlayer::getKeyboardAngVel() const
{
	return keyboardAngVel;
}

inline void				LocalPlayer::setKey(int button, bool pressed)
{
	keyButton = button;
	keyPressed = pressed;
}

inline void				LocalPlayer::resetKey()
{
	keyButton = BzfKeyEvent::NoButton;
	keyPressed = false;
}

inline bool				LocalPlayer::getKeyPressed() const
{
	return keyPressed;
}

inline int				LocalPlayer::getKeyButton() const
{
	return keyButton;
}

inline void				LocalPlayer::setSlowKeyboard(bool slow)
{
	slowKeyboard = slow;
}

inline bool				LocalPlayer::hasSlowKeyboard() const
{
	return slowKeyboard;
}

#endif // BZF_LOCAL_PLAYER_H
// ex: shiftwidth=4 tabstop=4
