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

#ifndef	__PLAYER_H__
#define	__PLAYER_H__

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "global.h"
#include "bzfgl.h"
#include "TimeKeeper.h"
#include "Address.h"
#include "Flag.h"
#include "PlayerState.h"
#include "ShotStatistics.h"

/* local interface headers */
class ShotPath;
class SceneDatabase;
class TankSceneNode;
class TankIDLSceneNode;
class SphereSceneNode;
class Obstacle;

const int PlayerUpdatePLen = PlayerIdPLen + 42;
const int PlayerUpdateSmallPLen = PlayerUpdatePLen - 16;


class Player {
public:
  Player(const PlayerId&, TeamColor,
	 const char* callsign, const char* emailAddress,
	 const PlayerType);
  virtual	~Player();

  PlayerId	getId() const;
  TeamColor	getTeam() const;
  void		setTeam(TeamColor);
  void		updateTank(float dt, bool local);
  const char*	getCallSign() const;
  const char*	getEmailAddress() const;
  PlayerType	getPlayerType() const;
  FlagType*	getFlag() const;
  long		getOrder() const;
  short		getStatus() const;
  const float*	getPosition() const;
  float		getAngle() const;
  const float*	getForward() const;
  const float*	getVelocity() const;
  float		getAngularVelocity() const;
  int		getPhysicsDriver() const;
  float		getRadius() const;
  void		getMuzzle(float*) const;
  float		getMuzzleHeight() const;
  short		getWins() const;
  short		getLosses() const;
  short		getTeamKills() const;
  short		getScore() const;
  const float*	getDimensions() const;
  short		getRabbitScore() const;
  short		getLocalWins() const;
  short		getLocalLosses() const;
  short		getLocalTeamKills() const;
  const TimeKeeper&	getExplodeTime() const;
  const TimeKeeper&	getTeleportTime() const;
  short		getFromTeleporter() const;
  short		getToTeleporter() const;
  float		getTeleporterProximity() const;
  virtual int		getMaxShots() const;
  virtual ShotPath*	getShot(int index) const = 0;

  const ShotStatistics*	getShotStatistics() const;

  void		addToScene(SceneDatabase*, TeamColor effectiveTeam,
			   bool inCockpit, bool showIDL);
  virtual void	addShots(SceneDatabase*, bool colorblind) const;
  void		setLandingSpeed(float velocity);
  void		spawnEffect();
  bool		needsToBeRendered(bool cloaked, bool showTreads);

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
  bool		isAutoPilot() const;
  void		setAutoPilot(bool = true);

  bool		validTeamTarget(const Player *possibleTarget) const;

  // returns true iff dead reckoning is too different from the
  // current tank state.
  bool		isDeadReckoningWrong() const;

  // update state based on dead reckoning
  void		doDeadReckoning();

  // called to update state according to incoming packets
  void		move(const float* pos, float azimuth);
  void		setVelocity(const float* velocity);
  void		setAngularVelocity(float);
  void		setPhysicsDriver(int);
  void		changeTeam(TeamColor);
  virtual void	setFlag(FlagType*);
  virtual void	changeScore(short deltaWins, short deltaLosses, short deltaTeamKills);
  void		changeLocalScore(short deltaWins, short deltaLosses, short deltaTeamKills);
  void		setStatus(short);
  void		setExplode(const TimeKeeper&);
  void		setTeleport(const TimeKeeper&, short from, short to);
  void		endShot(int index, bool isHit = false,
			bool showExplosion = false);

  void*		pack(void*, uint16_t& code);
  void*		unpack(void*, uint16_t code);

  void		setDeadReckoning();
  void		setDeadReckoning(float timestamp);

  void		setUserTexture ( const char *tex ) { if(tex) userTexture = tex;}

  void setZpos (float z);

protected:
  // shot statistics
  ShotStatistics	shotStatistics;
  const Obstacle* lastObstacle; // last obstacle touched

private:
  // return true if the shot had to be terminated or false if it
  // was already terminated.  position must be set to the shot's
  // position if you return true (it's okay to return false if
  // there's no meaningful shot position).
  virtual bool	doEndShot(int index, bool isHit, float* position) = 0;
  bool		getDeadReckoning(float* predictedPos,
				 float* predictedAzimuth,
				 float* predictedVel) const;
  void setVisualTeam (TeamColor team );
  void updateFlagEffect(FlagType* flag);
  void updateDimensions(float dt, bool local);
  void updateTreads(float dt);
  void updateTranslucency(float dt);
  bool hitObstacleResizing();
private:
  // data not communicated with other players
  bool			notResponding;
  bool			autoPilot;
  bool			hunted;
  PlayerId		id;			// my credentials

  // data use for drawing
  TankSceneNode*	tankNode;
  TankIDLSceneNode*	tankIDLNode;
  SphereSceneNode*	pausedSphere;
  GLfloat		color[4];
  std::string		userTexture;
  static int	    tankTexture;
  static int	    tankOverideTexture;
  TeamColor		lastVisualTeam;
  TimeKeeper	lastTrackDraw;

  // permanent data
  TeamColor		team;			// my team

  char			callSign[CallSignLen];	// my pseudonym
  char			email[EmailLen];	// my email address
  PlayerType		type;		   // Human/Computer

  // relatively stable data
  FlagType*		flagType;		// flag type I'm holding
  float			dimensions[3];		// current tank dimensions
  float			dimensionsScale[3];	// use to scale the dimensions
  float			dimensionsRate[3];	 // relative to scale
  float			dimensionsTarget[3]; // relative to scale
  bool			useDimensions;		// use the varying dimensions for gfx
  float			alpha;			// current tank translucency
  float			alphaRate;		// current tank translucency
  float			alphaTarget;	// current tank translucency
  TimeKeeper		explodeTime;		// time I started exploding
  TimeKeeper		teleportTime;		// time I started teleporting
  short			fromTeleporter;		// teleporter I entered
  short			toTeleporter;		// teleporter I exited
  float			teleporterProximity;	// how close to a teleporter
  short			wins;			// number of kills
  short			losses;			// number of deaths
  short			tks;			// number of teamkills

  // score of local player against this player
  short			localWins;		// local player won this many
  short			localLosses;		// local player lost this many
  short			localTks;		// local player team killed this many

  // highly dynamic data
  PlayerState		state;

  // computable highly dynamic data
  float			forward[3];		// forward unit vector

  // dead reckoning stuff
  TimeKeeper		inputTime;		// time of input
  mutable TimeKeeper	inputPrevTime;		// time of last dead reckoning
  int			inputStatus;		// tank status
  mutable float		inputPos[3];		// tank position
  float			inputSpeed;		// tank horizontal speed
  mutable float	inputZSpeed;			// tank vertical speed
  float			inputAzimuth;		// direction tank is pointing
  float			inputSpeedAzimuth;	// direction of speed
  float			inputAngVel;		// tank turn rate
  float		 deltaTime;	      // average difference between
						// time source and
						// time destination
  float		 offset;		 // time offset on last
						// measurement
  int		   deadReckoningState;     // 0 -> not received any sample
						// 1 -> 1 sample rx
						// 2 -> 2 or more sample rx
  float			oldZSpeed;		// old tank vertical speed
  int			oldStatus;		// old tank status bits
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

inline void		Player::setTeam(TeamColor _team)
{
  team = _team;
}

inline const char*	Player::getCallSign() const
{
  return callSign;
}

inline const char*	Player::getEmailAddress() const
{
  return email;
}

inline PlayerType	Player::getPlayerType() const
{
  return type;
}

inline FlagType*	Player::getFlag() const
{
  return flagType;
}

inline long		Player::getOrder() const
{
  return state.order;
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

inline const float*	Player::getDimensions() const
{
  return dimensions;
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

inline int		Player::getPhysicsDriver() const
{
  return state.phydrv;
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

inline const ShotStatistics*  Player::getShotStatistics() const
{
  return &shotStatistics;
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

inline bool		Player::isHunted() const
{
  return hunted;
}

inline void		Player::setHunted(bool _hunted)
{
  hunted = _hunted;
}

inline bool		Player::isAutoPilot() const
{
  return autoPilot;
}

inline void		Player::setAutoPilot(bool _autoPilot)
{
  autoPilot = _autoPilot;
}

inline void*		Player::pack(void* buf, uint16_t& code)
{
  setDeadReckoning();
  return state.pack(buf, code);
}

inline void Player::setZpos (float z)
{
  state.pos[2] = z;
}

#endif /* __PLAYER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
