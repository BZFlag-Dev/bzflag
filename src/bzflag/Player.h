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

#ifndef BZF_PLAYER_H
#define BZF_PLAYER_H

#include "common.h"
#include "global.h"
#include "TimeKeeper.h"
#include "Address.h"

class ShotPath;
class SceneNode;
class SceneNodeGroup;
class SceneNodeTransform;
class SceneNodeGState;

const int				PlayerUpdatePLen = PlayerIdPLen + 34;

class Player {
public:
	enum PStatus {						// bit masks
		DeadStatus =		0x0000,		// not alive, not paused, etc.
		Alive =				0x0001,		// player is alive
		Paused =	 		0x0002,		// player is paused
		Exploding =			0x0004,		// currently blowing up
		Teleporting =		0x0008,		// teleported recently
		FlagActive =		0x0010,		// flag special powers active
		CrossingWall =		0x0020,		// tank crossing building wall
		Falling =			0x0040		// tank accel'd by gravity
	};

	Player(PlayerId, TeamColor,
				const char* callsign, const char* emailAddress);
	virtual ~Player();

	PlayerId				getId() const;
	TeamColor				getTeam() const;
	const char*				getCallSign() const;
	const char*				getEmailAddress() const;
	FlagId					getFlag() const;
	short					getStatus() const;
	const float*			getPosition() const;
	float					getAngle() const;
	const float*			getForward() const;
	const float*			getVelocity() const;
	float					getAngularVelocity() const;
	float					getRadius() const;
	void					getMuzzle(float*) const;
	short					getWins() const;
	short					getLosses() const;
	short					getScore() const;
	short					getLocalWins() const;
	short					getLocalLosses() const;
	const TimeKeeper&		getExplodeTime() const;
	const TimeKeeper&		getTeleportTime() const;
	short					getFromTeleporter() const;
	short					getToTeleporter() const;
	float					getTeleporterProximity() const;
	virtual ShotPath*		getShot(int index) const = 0;

	void					setId(PlayerId);

	void					addPlayerSceneNode(SceneNodeGroup*,
								bool viewerIsColorblind,
								bool masqueraded, TeamColor masquerade);
	void					addShotsSceneNodes(SceneNodeGroup*,
								bool viewerIsColorblind);

	bool					isAlive() const;
	bool					isPaused() const;
	bool					isFlagActive() const;
	bool					isTeleporting() const;
	bool					isExploding() const;
	bool					isCrossingWall() const;
	bool					isNotResponding() const;
	void					resetNotResponding();
	bool					isAutoPilot() const;
	void					setAutoPilot(bool = true);

	// returns true iff dead reckoning is too different from the
	// current tank state.
	bool					isDeadReckoningWrong() const;

	// update state based on dead reckoning
	void					doDeadReckoning();

	// called to update state according to incoming packets
	void					move(const float* pos, float azimuth);
	void					setVelocity(const float* velocity);
	void					setAngularVelocity(float);
	void					changeTeam(TeamColor);
	virtual void			setFlag(FlagId);
	virtual void			changeScore(short deltaWins, short deltaLosses);
	void					changeLocalScore(short deltaWins, short deltaLosses);
	void					setStatus(short);
	void					setExplode(const TimeKeeper&);
	void					setTeleport(const TimeKeeper&, short from, short to);
	void					updateSparks(float dt);
	void					endShot(int index, bool isHit = false,
								bool showExplosion = false);

	void*					pack(void*) const;
	void*					unpack(void*);

	void					setDeadReckoning();

private:
	// return true if the shot had to be terminated or false if it
	// was already terminated.  position must be set to the shot's
	// position if you return true (it's okay to return false if
	// there's no meaningful shot position).
	virtual bool			doEndShot(int index, bool isHit, float* position) = 0;
	bool					getDeadReckoning(float* predictedPos,
								float* predictedAzimuth,
								float* predictedVel) const;

	void					unrefNodes();

private:
	// scene nodes
	SceneNode*				teamPlayerSceneNode;
	SceneNode*				roguePlayerSceneNode;
	SceneNode*				blackhatSceneNode;
	SceneNodeTransform*		transformSceneNode;
	SceneNodeGState*		gStateSceneNode;
	// need a black sphere for paused or not responding
	// need a clip plane if isCrossingWall().

	// data not communicated with other players
	bool					notResponding;
	bool					autoPilot;
	PlayerId				id;						// my credentials

	// permanent data
	TeamColor				team;					// my team
	char					callSign[CallSignLen];	// my pseudonym
	char					email[EmailLen];		// my email address

	// relatively stable data
	FlagId					flag;					// flag I'm holding
	TimeKeeper				explodeTime;			// time I started exploding
	TimeKeeper				teleportTime;			// time I started teleporting
	short					fromTeleporter;			// teleporter I entered
	short					toTeleporter;			// teleporter I exited
	float					teleporterProximity;	// how close to a teleporter
	short					wins;					// number of kills
	short					losses;					// number of deaths

	// score of local player against this player
	short					localWins;				// local player won this many
	short					localLosses;			// local player lost this many

	// highly dynamic data
	float					pos[3];					// position of tank
	float					azimuth;				// orientation of tank
	short					status;					// see PStatus enum
	float					velocity[3];			// velocity of tank
	float					angVel;					// angular velocity of tank

	// computable highly dynamic data
	float					forward[3];				// forward unit vector

	// dead reckoning stuff
	TimeKeeper				inputTime;				// time of input
	TimeKeeper				inputPrevTime;			// time of last dead reckoning
	int						inputStatus;			// tank status
	float					inputPos[3];			// tank position
	float					inputSpeed;				// tank horizontal speed
	float					inputZSpeed;			// tank vertical speed
	float					inputAzimuth;			// direction tank is pointing
	float					inputSpeedAzimuth;		// direction of speed
	float					inputAngVel;			// tank turn rate
};

// shot data goes in LocalPlayer or RemotePlayer so shot type isn't lost.

//
// Player
//

inline PlayerId	Player::getId() const
{
	return id;
}

inline TeamColor		Player::getTeam() const
{
	return team;
}

inline const char*		Player::getCallSign() const
{
	return callSign;
}

inline const char*		Player::getEmailAddress() const
{
	return email;
}

inline FlagId			Player::getFlag() const
{
	return flag;
}

inline short			Player::getStatus() const
{
	return status;
}

inline const float*		Player::getPosition() const
{
	return pos;
}

inline float			Player::getAngle() const
{
	return azimuth;
}

inline const float*		Player::getForward() const
{
	return forward;
}

inline const float*		Player::getVelocity() const
{
	return velocity;
}

inline float			Player::getAngularVelocity() const
{
	return angVel;
}

inline short			Player::getWins() const
{
	return wins;
}

inline short			Player::getLosses() const
{
	return losses;
}

inline short			Player::getLocalWins() const
{
	return localWins;
}

inline short			Player::getLocalLosses() const
{
	return localLosses;
}

inline short			Player::getScore() const
{
	return wins - losses;
}

inline const TimeKeeper	&Player::getExplodeTime() const
{
	return explodeTime;
}

inline const TimeKeeper	&Player::getTeleportTime() const
{
	return teleportTime;
}

inline short			Player::getFromTeleporter() const
{
	return fromTeleporter;
}

inline short			Player::getToTeleporter() const
{
	return toTeleporter;
}

inline float			Player::getTeleporterProximity() const
{
	return teleporterProximity;
}

inline bool				Player::isAlive() const
{
	return (status & short(Alive)) != 0;
}

inline bool				Player::isPaused() const
{
	return (status & short(Paused)) != 0;
}

inline bool				Player::isFlagActive() const
{
	return (status & short(FlagActive)) != 0;
}

inline bool				Player::isTeleporting() const
{
	return (status & short(Teleporting)) != 0;
}

inline bool				Player::isExploding() const
{
	return (status & short(Exploding)) != 0;
}

inline bool				Player::isCrossingWall() const
{
	return (status & short(CrossingWall)) != 0;
}

inline bool				Player::isNotResponding() const
{
	return notResponding;
}

inline void				Player::resetNotResponding()
{
	notResponding = false;
}

inline bool				Player::isAutoPilot() const
{
	return (autoPilot);
}

inline void				Player::setAutoPilot(bool _autoPilot)
{
		autoPilot = _autoPilot;
}


#endif // BZF_PLAYER_H
// ex: shiftwidth=4 tabstop=4
