/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_PLAYER_H
#define	BZF_PLAYER_H

#include "common.h"
#include "global.h"
#include "TimeKeeper.h"
#include "Address.h"
#include "AList.h"
#include "ShotPath.h"
#include "OpenGLTexture.h"

class ShotPath;
class SceneDatabase;
class TankSceneNode;
class TankIDLSceneNode;
class SphereSceneNode;

const int		PlayerUpdatePLen = PlayerIdPLen + 34;

class Player {
  public:
    enum PStatus {				// bit masks
			DeadStatus =	0x0000,	// not alive, not paused, etc.
			Alive =		0x0001,	// player is alive
			Paused = 	0x0002,	// player is paused
			Exploding =	0x0004,	// currently blowing up
			Teleporting =	0x0008,	// teleported recently
			FlagActive =	0x0010,	// flag special powers active
			CrossingWall =	0x0020,	// tank crossing building wall
			Falling =	0x0040	// tank accel'd by gravity
    };

			Player(const PlayerId&, TeamColor,
				const char* callsign, const char* emailAddress);
    virtual		~Player();

    const PlayerId&	getId() const;
    TeamColor		getTeam() const;
    const char*		getCallSign() const;
    const char*		getEmailAddress() const;
    FlagId		getFlag() const;
    short		getStatus() const;
    const float*	getPosition() const;
    float		getAngle() const;
    const float*	getForward() const;
    const float*	getVelocity() const;
    float		getAngularVelocity() const;
    float		getRadius() const;
    void		getMuzzle(float*) const;
    short		getWins() const;
    short		getLosses() const;
    short		getScore() const;
    short		getLocalWins() const;
    short		getLocalLosses() const;
    const TimeKeeper&	getExplodeTime() const;
    const TimeKeeper&	getTeleportTime() const;
    short		getFromTeleporter() const;
    short		getToTeleporter() const;
    float		getTeleporterProximity() const;
    virtual ShotPath*	getShot(int index) const = 0;

    void		addPlayer(SceneDatabase*, boolean colorblind,
							boolean showIDL);
    void		addShots(SceneDatabase*, boolean colorblind) const;
    void		setHidden(boolean hidden = True);
    void		setInvisible(boolean invisible = True);

    static void		setTexture(const OpenGLTexture&);

    boolean		isAlive() const;
    boolean		isPaused() const;
    boolean		isFlagActive() const;
    boolean		isTeleporting() const;
    boolean		isExploding() const;
    boolean		isCrossingWall() const;
    boolean		isNotResponding() const;
    void		resetNotResponding();

    // returns true iff dead reckoning is too different from the
    // current tank state.
    boolean		isDeadReckoningWrong() const;

    // update state based on dead reckoning
    void		doDeadReckoning();

    // called to update state according to incoming packets
    void		move(const float* pos, float azimuth);
    void		setVelocity(const float* velocity);
    void		setAngularVelocity(float);
    void		changeTeam(TeamColor);
    virtual void	setFlag(FlagId);
    virtual void	changeScore(short deltaWins, short deltaLosses);
    void		changeLocalScore(short deltaWins, short deltaLosses);
    void		setStatus(short);
    void		setExplode(const TimeKeeper&);
    void		setTeleport(const TimeKeeper&, short from, short to);
    void		updateSparks(float dt);
    void		endShot(int index, boolean isHit = False,
				boolean showExplosion = False);

    void*		pack(void*) const;
    void*		unpack(void*);

    void		setDeadReckoning();

  private:
    // return true if the shot had to be terminated or false if it
    // was already terminated.  position must be set to the shot's
    // position if you return true (it's okay to return false if
    // there's no meaningful shot position).
    virtual boolean	doEndShot(int index, boolean isHit, float* position) = 0;
    boolean		getDeadReckoning(float* predictedPos,
				float* predictedAzimuth,
				float* predictedVel) const;

  private:
    // data not communicated with other players
    TankSceneNode*	tankNode;
    TankIDLSceneNode*	tankIDLNode;
    SphereSceneNode*	pausedSphere;
    GLfloat		color[4];
    boolean		notResponding;
    static OpenGLTexture tankTexture;

    // permanent data
    PlayerId		id;			// my credentials
    TeamColor		team;			// my team
    char		callSign[CallSignLen];	// my pseudonym
    char		email[EmailLen];	// my email address

    // relatively stable data
    FlagId		flag;			// flag I'm holding
    TimeKeeper		explodeTime;		// time I started exploding
    TimeKeeper		teleportTime;		// time I started teleporting
    short		fromTeleporter;		// teleporter I entered
    short		toTeleporter;		// teleporter I exited
    float		teleporterProximity;	// how close to a teleporter
    short		wins;			// number of kills
    short		losses;			// number of deaths

    // score of local player against this player
    short		localWins;		// local player won this many
    short		localLosses;		// local player lost this many

    // highly dynamic data
    short		status;			// see PStatus enum
    float		pos[3];			// position of tank
    float		azimuth;		// orientation of tank
    float		velocity[3];		// velocity of tank
    float		angVel;			// angular velocity of tank

    // computable highly dynamic data
    float		forward[3];		// forward unit vector

    // dead reckoning stuff
    TimeKeeper		inputTime;		// time of input
    TimeKeeper		inputPrevTime;		// time of last dead reckoning
    int			inputStatus;		// tank status
    float		inputPos[3];		// tank position
    float		inputSpeed;		// tank horizontal speed
    float		inputZSpeed;		// tank vertical speed
    float		inputAzimuth;		// direction tank is pointing
    float		inputSpeedAzimuth;	// direction of speed
    float		inputAngVel;		// tank turn rate
};

// shot data goes in LocalPlayer or RemotePlayer so shot type isn't lost.

//
// Player
//

inline const PlayerId&	Player::getId() const
{
  return id;
}

inline TeamColor	Player::getTeam() const
{
  return team;
}

inline const char*	Player::getCallSign() const
{
  return callSign;
}

inline const char*	Player::getEmailAddress() const
{
  return email;
}

inline FlagId		Player::getFlag() const
{
  return flag;
}

inline short		Player::getStatus() const
{
  return status;
}

inline const float*	Player::getPosition() const
{
  return pos;
}

inline float		Player::getAngle() const
{
  return azimuth;
}

inline const float*	Player::getForward() const
{
  return forward;
}

inline const float*	Player::getVelocity() const
{
  return velocity;
}

inline float		Player::getAngularVelocity() const
{
  return angVel;
}

inline short		Player::getWins() const
{
  return wins;
}

inline short		Player::getLosses() const
{
  return losses;
}

inline short		Player::getLocalWins() const
{
  return localWins;
}

inline short		Player::getLocalLosses() const
{
  return localLosses;
}

inline short		Player::getScore() const
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

inline short		Player::getFromTeleporter() const
{
  return fromTeleporter;
}

inline short		Player::getToTeleporter() const
{
  return toTeleporter;
}

inline float		Player::getTeleporterProximity() const
{
  return teleporterProximity;
}

inline boolean		Player::isAlive() const
{
  return (status & short(Alive)) != 0;
}

inline boolean		Player::isPaused() const
{
  return (status & short(Paused)) != 0;
}

inline boolean		Player::isFlagActive() const
{
  return (status & short(FlagActive)) != 0;
}

inline boolean		Player::isTeleporting() const
{
  return (status & short(Teleporting)) != 0;
}

inline boolean		Player::isExploding() const
{
  return (status & short(Exploding)) != 0;
}

inline boolean		Player::isCrossingWall() const
{
  return (status & short(CrossingWall)) != 0;
}

inline boolean		Player::isNotResponding() const
{
  return notResponding;
}

inline void		Player::resetNotResponding()
{
  notResponding = False;
}

#endif // BZF_PLAYER_H
