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

#ifndef	BZF_PLAYER_H
#define	BZF_PLAYER_H

#include "common.h"
#include "global.h"
#include "TimeKeeper.h"
#include "Address.h"
#include "ShotPath.h"
#include "OpenGLTexture.h"
#include "PlayerState.h"
#include "Flag.h"

class ShotPath;
class SceneDatabase;
class TankSceneNode;
class TankIDLSceneNode;
class SphereSceneNode;

const int		PlayerUpdatePLen = PlayerIdPLen + 34;

class Player {
  public:
			Player(const PlayerId&, TeamColor,
				const char* callsign, const char* emailAddress);
    virtual		~Player();

    PlayerId		getId() const;
    TeamColor		getTeam() const;
    const char*		getCallSign() const;
    const char*		getEmailAddress() const;
    FlagDesc*		getFlag() const;
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
    short		getTeamKills() const;
    short		getScore() const;
    short		getLocalWins() const;
    short		getLocalLosses() const;
    short		getLocalTeamKills() const;
    const TimeKeeper&	getExplodeTime() const;
    const TimeKeeper&	getTeleportTime() const;
    short		getFromTeleporter() const;
    short		getToTeleporter() const;
    float		getTeleporterProximity() const;
    virtual ShotPath*	getShot(int index) const = 0;

    void		setId(PlayerId&);

    void		addPlayer(SceneDatabase*, const float* colorOverride,
							bool showIDL);
    void		addShots(SceneDatabase*, bool colorblind) const;
    void		setHidden(bool hidden = true);
    void		setInvisible(bool invisible = true);

    static void		setTexture(const OpenGLTexture&);

    bool		isAlive() const;
    bool		isPaused() const;
    bool		isFlagActive() const;
    bool		isTeleporting() const;
    bool		isExploding() const;
    bool		isCrossingWall() const;
    bool		isNotResponding() const;
    void		resetNotResponding();
    bool		isHunted() const;
    void		setHunted(bool _hunted);

    // returns true iff dead reckoning is too different from the
    // current tank state.
    bool		isDeadReckoningWrong() const;

    // update state based on dead reckoning
    void		doDeadReckoning();

    // called to update state according to incoming packets
    void		move(const float* pos, float azimuth);
    void		setVelocity(const float* velocity);
    void		setAngularVelocity(float);
    void		changeTeam(TeamColor);
    virtual void	setFlag(FlagDesc*);
    virtual void	changeScore(short deltaWins, short deltaLosses, short deltaTeamKills);
    void		changeLocalScore(short deltaWins, short deltaLosses, short deltaTeamKills);
    void		setStatus(short);
    void		setExplode(const TimeKeeper&);
    void		setTeleport(const TimeKeeper&, short from, short to);
    void		updateSparks(float dt);
    void		endShot(int index, bool isHit = false,
				bool showExplosion = false);

    void*		pack(void*) const;
    void*		unpack(void*);

    void		setDeadReckoning();

  private:
    // return true if the shot had to be terminated or false if it
    // was already terminated.  position must be set to the shot's
    // position if you return true (it's okay to return false if
    // there's no meaningful shot position).
    virtual bool	doEndShot(int index, bool isHit, float* position) = 0;
    bool		getDeadReckoning(float* predictedPos,
				float* predictedAzimuth,
				float* predictedVel) const;

  private:
    // data not communicated with other players
    TankSceneNode*	tankNode;
    TankIDLSceneNode*	tankIDLNode;
    SphereSceneNode*	pausedSphere;
    GLfloat		color[4];
    bool		notResponding;
    static OpenGLTexture* tankTexture;
    static int		totalCount;
    bool		hunted;
    PlayerId		id;			// my credentials

    // permanent data
    TeamColor		team;			// my team
    char		callSign[CallSignLen];	// my pseudonym
    char		email[EmailLen];	// my email address

    // relatively stable data
    FlagDesc*		flag;			// flag I'm holding
    TimeKeeper		explodeTime;		// time I started exploding
    TimeKeeper		teleportTime;		// time I started teleporting
    short		fromTeleporter;		// teleporter I entered
    short		toTeleporter;		// teleporter I exited
    float		teleporterProximity;	// how close to a teleporter
    short		wins;			// number of kills
    short		losses;			// number of deaths
    short		tks;			// number of teamkills

    // score of local player against this player
    short		localWins;		// local player won this many
    short		localLosses;		// local player lost this many
    short		localTks;		// local player team killed this many

    // highly dynamic data
    PlayerState		state;

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

inline PlayerId		Player::getId() const
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

inline FlagDesc*	Player::getFlag() const
{
  return flag;
}

inline short		Player::getStatus() const
{
  return state.status;
}

inline const float*	Player::getPosition() const
{
  return state.pos;
}

inline float		Player::getAngle() const
{
  return state.azimuth;
}

inline const float*	Player::getForward() const
{
  return forward;
}

inline const float*	Player::getVelocity() const
{
  return state.velocity;
}

inline float		Player::getAngularVelocity() const
{
  return state.angVel;
}

inline short		Player::getWins() const
{
  return wins;
}

inline short		Player::getLosses() const
{
  return losses;
}

inline short		Player::getTeamKills() const
{
  return tks;
}

inline short		Player::getLocalWins() const
{
  return localWins;
}

inline short		Player::getLocalLosses() const
{
  return localLosses;
}

inline short		Player::getLocalTeamKills() const
{
  return localTks;
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

inline bool		Player::isAlive() const
{
  return (state.status & short(PlayerState::Alive)) != 0;
}

inline bool		Player::isPaused() const
{
  return (state.status & short(PlayerState::Paused)) != 0;
}

inline bool		Player::isFlagActive() const
{
  return (state.status & short(PlayerState::FlagActive)) != 0;
}

inline bool		Player::isTeleporting() const
{
  return (state.status & short(PlayerState::Teleporting)) != 0;
}

inline bool		Player::isExploding() const
{
  return (state.status & short(PlayerState::Exploding)) != 0;
}

inline bool		Player::isCrossingWall() const
{
  return (state.status & short(PlayerState::CrossingWall)) != 0;
}

inline bool		Player::isNotResponding() const
{
  return notResponding;
}

inline void		Player::resetNotResponding()
{
  notResponding = false;
}

inline bool  Player::isHunted() const
{
  return hunted;
}

inline void  Player::setHunted(bool _hunted)
{
  hunted = _hunted;
}

#endif // BZF_PLAYER_H
// ex: shiftwidth=2 tabstop=8
