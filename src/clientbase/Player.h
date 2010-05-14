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

#ifndef	__PLAYER_H__
#define	__PLAYER_H__

#include "common.h"

// system headers
#include <string>

// common headers
#include "global.h"
#include "BzTime.h"
#include "Address.h"
#include "Flag.h"
#include "PlayerState.h"
#include "ShotStatistics.h"
#include "PlayerAvatarManager.h"
#include "GfxBlock.h"
#include "vectors.h"

// local headers
class ShotPath;
class SceneDatabase;
class Obstacle;
struct FiringInfo;

class Player {
public:
  Player(const PlayerId&, TeamColor,
	 const char* callsign,
	 const PlayerType);
  virtual	~Player();

  PlayerId	getId() const;
  TeamColor	getTeam() const;
  void		setTeam(TeamColor);
  void		updateTank(float dt, bool local);
  const char*	getCallSign() const;
  PlayerType	getPlayerType() const;
  int		getFlagID() const;
  FlagType*	getFlagType() const;
  long		getOrder() const;
  short		getStatus() const;
  const fvec3&	getPosition() const;
  float		getAngle() const;
  const fvec3&	getForward() const;
  const fvec3&	getVelocity() const;
  float         getUserSpeed() const;
  float		getAngularVelocity() const;
  float         getUserAngVel() const;
  int		getPhysicsDriver() const;
  int		getDeathPhysicsDriver() const;
  float		getRadius() const;
  fvec3		getMuzzleShotPos() const;
  float		getMuzzleHeight() const;
  short		getWins() const;
  short		getLosses() const;
  short		getTeamKills() const;
  float		getTKRatio() const;
  float		getNormalizedScore() const;
  float		getLocalNormalizedScore() const;
  short		getScore() const;
  const fvec3&	getDimensions() const;
  const fvec3&	getDimensionsScale() const;
  float		getAlpha() const { return alpha; }
  float		getReloadTime() const;

  inline const PlayerState& getState() const { return state; }

  const fvec3&	getApparentVelocity() const;
  BzTime	getLastUpdateTime() const;

  inline float	getLag() const { return lag; }
  inline void	setLag(float lt) { lag = lt; }
  inline float	getJitter() const { return jitter; }
  inline void	setJitter(float jt) { jitter = jt; }
  inline float	getPacketLoss() const { return packetLoss; }
  inline void	setPacketLoss(float pl) { packetLoss = pl; }

  inline bool hasWings() const { return getFlagType() == Flags::Wings; }

  inline const fvec4& getColor() const {
    return color;
  }

  float	getRabbitScore() const;
  short	getLocalWins() const;
  short	getLocalLosses() const;
  short	getLocalTeamKills() const;
  short	getLinkSrcID() const;
  short	getLinkDstID() const;
  float	getTeleporterProximity() const;
  const BzTime&	getExplodeTime() const;
  const BzTime&	getTeleportTime() const;

  // shots
  int		getMaxShots() const;
  ShotPath*	getShot(int index) const;
  ShotType	getShotType() const { return shotType; }
  void		setShotType(ShotType _shotType) { shotType = _shotType; }

  const ShotStatistics*	getShotStatistics() const;

  void		addToScene(SceneDatabase*, TeamColor effectiveTeam,
			   bool inCockpit, bool seerView,
			   bool showTreads, bool showIDL, bool thirdPerson = false);

  bool		getIpAddress(Address&);
  void		setIpAddress(const Address& addr);

  virtual void	addShots(SceneDatabase*, bool colorblind ) const;
  void		setLandingSpeed(float velocity);
  void		spawnEffect();
  void		fireJumpJets();

  GfxBlock&       getGfxBlock()       { return gfxBlock; }
  const GfxBlock& getGfxBlock() const { return gfxBlock; }
  GfxBlock&       getRadarGfxBlock()       { return radarGfxBlock; }
  const GfxBlock& getRadarGfxBlock() const { return radarGfxBlock; }

  void		forceReload(float time = 0.0f);

  bool		isAlive() const;
  bool		isPaused() const;
  bool		isFalling() const;
  bool		isFlagActive() const;
  bool		isTeleporting() const;
  bool		isExploding() const;
  bool		isPhantomZoned() const;
  bool		isCrossingWall() const;
  bool		isObserver() const;
  bool		canMove() const;
  bool		canJump() const;
  bool		canTurnLeft() const;
  bool		canTurnRight() const;
  bool		canMoveForward() const;
  bool		canMoveBackward() const;
  bool		canShoot() const;
  bool		isNotResponding() const;
  void		resetNotResponding();
  bool		isHunted() const;
  void		setHunted(bool _hunted);
  int		getAutoHuntLevel() const;
  void		setAutoHuntLevel(int level);
  bool		isAutoPilot() const;
  void		setAutoPilot(bool = true);
  bool		isAdmin() const;
  void		setAdmin(bool = true);
  bool		isRegistered() const;
  void		setRegistered(bool = true);
  bool		isVerified() const;
  void		setVerified(bool = true);
  bool		hasPlayerList() const;
  void		setPlayerList(bool = true);

  virtual void	setPause(bool value) { paused = value; }

  bool		validTeamTarget(const Player *possibleTarget) const;

  // returns true iff dead reckoning is too different from the
  // current tank state.
  bool		isDeadReckoningWrong() const;

  // update state based on dead reckoning
  void		doDeadReckoning();

  // called to update state according to incoming packets
  void		move(const fvec3& pos, float azimuth);
  void		setVelocity(const fvec3& velocity);
  void		setAngularVelocity(float);
  void		setPhysicsDriver(int);
  void		setRelativeMotion();
  void		setUserSpeed(float speed);
  void		setUserAngVel(float angvel);
  virtual void	setFlagID(int id);
  virtual void	changeTeam(TeamColor);
  virtual void	changeScore(float newRank, short newWins, short newLosses, short newTeamKills);
  void		changeLocalScore(short deltaWins, short deltaLosses, short deltaTeamKills);
  void		setHandicap(float handicap);
  void		setStatus(short);
  void		setExplode(const BzTime&);
  void		setAllow(unsigned char _allow);
  unsigned char	getAllow();
  void		setTeleport(const BzTime&, short src, short dst);
  void		endShot(int index, bool isHit = false,
			bool showExplosion = false);

  void*		pack(void*, uint16_t& code);
  void*		unpack(void*, uint16_t code);

  void		setDeadReckoning(); // local version
  void		setDeadReckoning(double timestamp);

  void		setUserTexture(const char *tex) { if (tex) { userTexture = tex; } }

  void		renderRadar() const;

  void		setZpos(float z);
  float		getMaxSpeed() const;

  void		updateShot(FiringInfo &info, int shotID);

  void		land();

  std::map<std::string,std::string> customData;

  bool hasCustomField(const std::string & key ) const {
    return customData.find(key)!= customData.end();
  }
  const std::string& getCustomField(const std::string& key) const;

  const Obstacle*	getHitBuilding(const fvec3& oldPos, float oldAngle,
				       const fvec3& pos, float angle,
				       bool phased, bool& expel);
  bool		getHitNormal(const Obstacle* o,
			     const fvec3& pos1, float azimuth1,
			     const fvec3& pos2, float azimuth2,
			     fvec3& normal) const;

protected:
  void	  clearRemoteSounds();
  void	  addRemoteSound(int sound);
  void    prepareShotInfo(FiringInfo &info, bool local = false);

  ShotPath *addShot(ShotPath *shot, const FiringInfo &info);
  void deleteShot(int index);

protected:
  // shot statistics
  ShotStatistics	  shotStatistics;
  const Obstacle*	  lastObstacle; // last obstacle touched

  std::vector<ShotPath*>  shots;
  float			  handicap;
  BzTime		  jamTime;
  BzTime		  lastLanding;

private:
  // return true if the shot had to be terminated or false if it
  // was already terminated.  position must be set to the shot's
  // position if you return true (it's okay to return false if
  // there's no meaningful shot position).
  virtual bool	doEndShot(int index, bool isHit, fvec3& position) = 0;

  void getDeadReckoning(fvec3& predictedPos, float& predictedAzimuth,
			fvec3& predictedVel, float time) const;
  void calcRelativeMotion(fvec2& vel, float& speed, float& angvel);

  void setVisualTeam (TeamColor team );
  void updateFlagEffect(FlagType* flag);
  void updateTranslucency(float dt);
  void updateDimensions(float dt, bool local);
  void updateTreads(float dt);
  void updateJumpJets(float dt);
  void updateTrackMarks();

  bool hitObstacleResizing();

  // used by doDeadReckoning()
  bool getHitCorrection(const fvec3& startPos, const float startAzimuth,
                        const fvec3& endPos,   const float endAzimuth,
                        const fvec3& startVelocity, double timeStep,
                        float groundLimit, fvec3& velocity, fvec3& position,
                        float* azimuth);

private:
  // data not communicated with other players
  bool			notResponding;
  bool			hunted;
  int			autoHuntLevel;

  // credentials
  PlayerId		id;
  bool			admin;
  bool			registered;
  bool			verified;
  bool			playerList;
  Address		ipAddr;
  bool			haveIpAddr;

  // data use for drawing
  GfxBlock		gfxBlock;
  GfxBlock		radarGfxBlock;
  PlayerAvatar		*avatar;

  fvec4			color;
  float			teleAlpha;

  std::string		userTexture;
  static int		tankTexture;
  static int		tankOverideTexture;
  TeamColor		lastVisualTeam;
  BzTime		lastTrackDraw;

  // permanent data
  TeamColor		team;			// my team

  char			callSign[CallSignLen];	// my pseudonym
  PlayerType		type;			// Human/Computer

  // relatively stable data
  int			flagID;			// flag ID I'm holding (or -1)
  FlagType*		flagType;		// flag type I'm holding
  ShotType		shotType;		// the shots I fire
  fvec3			dimensions;		// current tank dimensions
  fvec3			dimensionsScale;	// use to scale the dimensions
  fvec3			dimensionsRate;		// relative to scale
  fvec3			dimensionsTarget;	// relative to scale
  bool			useDimensions;		// use the varying dimensions for gfx
  float			alpha;			// current tank translucency
  float			alphaRate;		// current tank translucency
  float			alphaTarget;		// current tank translucency
  BzTime		spawnTime;		// time I started spawning
  BzTime		explodeTime;		// time I started exploding
  BzTime		teleportTime;		// time I started teleporting
  short			teleLinkSrcID;		// teleporter I entered
  short			teleLinkDstID;		// teleporter I exited
  float			teleporterProximity;	// how close to a teleporter
  float			rank;			// server ranking
  short			wins;			// number of kills
  short			losses;			// number of deaths
  short			tks;			// number of teamkills
  unsigned char			allow;		// tank allowed actions

  // score of local player against this player
  short			localWins;		// local player won this many
  short			localLosses;		// local player lost this many
  short			localTks;		// local player team killed this many

  // highly dynamic data
  PlayerState		state;

  // additional state
  bool			paused;
  bool			autoPilot;

  // computable highly dynamic data
  fvec3			forward;		// forward unit vector

  // relative motion information
  float			relativeSpeed;		// relative speed
  float			relativeAngVel;		// relative angular velocity

  // dead reckoning stuff
  double inputTimestamp;	// input timestamp of the sender
  BzTime inputTime;		// time of input
  int    inputStatus;		// tank status
  fvec3  inputPos;		// tank position
  fvec3  inputVel;		// tank velocity
  float  inputAzimuth;		// direction tank is pointing
  float  inputAngVel;		// tank turn rate
  bool   inputTurning;		// tank is turning
  fvec2  inputRelVel;		// relative velocity
  float  inputRelSpeed;		// relative speed
  float  inputRelAngVel;	// relative angular velocity
  fvec2  inputTurnCenter;	// tank turn center
  fvec2  inputTurnVector;	// tank turn vector
  int    inputPhyDrv;		// physics driver

  // average difference between time source and time destination
  float			deltaTime;

  // time offset on last measurement
  float			offset;

  // 0 -> not received any sample
  // 1 -> 1 sample rx
  // 2 -> 2 or more sample rx
  int			deadReckoningState;

  int			oldStatus;		// old tank status bits
  float			oldZSpeed;		// old tank vertical speed

  float lag, jitter, packetLoss;

  bool isSolid();
};


// shot data goes in LocalPlayer or RemotePlayer so shot type isn't lost.


//
// Player
//

inline bool Player::isSolid()
{
  return getFlagType() != Flags::OscillationOverthruster;
}

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

inline PlayerType	Player::getPlayerType() const
{
  return type;
}

inline int		Player::getFlagID() const
{
  return flagID;
}

inline FlagType*	Player::getFlagType() const
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

inline const fvec3&	Player::getPosition() const
{
  return state.pos;
}

inline float		Player::getAngle() const
{
  return state.azimuth;
}

inline const fvec3&	Player::getDimensions() const
{
  return dimensions;
}

inline const fvec3&	Player::getDimensionsScale() const
{
  return dimensionsScale;
}

inline const fvec3&	Player::getForward() const
{
  return forward;
}

inline const fvec3&	Player::getVelocity() const
{
  return state.velocity;
}
inline float         Player::getUserSpeed() const
{
  return state.userSpeed;
}

inline float		Player::getAngularVelocity() const
{
  return state.angVel;
}

inline float		Player::getUserAngVel() const
{
  return state.userAngVel;
}

inline BzTime	Player::getLastUpdateTime() const
{
  return state.lastUpdateTime;
}

inline const fvec3&	Player::getApparentVelocity() const
{
  return state.apparentVelocity;
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

inline const BzTime	&Player::getExplodeTime() const
{
  return explodeTime;
}

inline const BzTime	&Player::getTeleportTime() const
{
  return teleportTime;
}

inline short		Player::getLinkSrcID() const
{
  return teleLinkSrcID;
}

inline short		Player::getLinkDstID() const
{
  return teleLinkDstID;
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
  return paused;
}

inline bool		Player::isAutoPilot() const
{
  return autoPilot;
}

inline void		Player::setAutoPilot(bool autopilot)
{
  autoPilot = autopilot;
}

inline bool		Player::isFalling() const
{
  return (state.status & short(PlayerState::Falling)) != 0;
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

inline bool		Player::isPhantomZoned() const
{
  return (isFlagActive() && (getFlagType() == Flags::PhantomZone));
}

inline bool		Player::isCrossingWall() const
{
  return (state.status & short(PlayerState::CrossingWall)) != 0;
}

inline bool		Player::isObserver() const
{
  return (team == ObserverTeam);
}

inline bool		Player::canMove() const
{
  //return (state.status & short(PlayerState::AllowMovement)) != 0;
  return (canTurnLeft() && canTurnRight() && canMoveForward() && canMoveBackward());
}

inline bool		Player::canJump() const
{
  const float reJumpTime = BZDB.eval(BZDBNAMES.REJUMPTIME);
  if ((BzTime::getCurrent() - lastLanding) < reJumpTime) {
    return false;
  }
  return (allow & AllowJump) != 0;
}

inline void		Player::land()
{
  lastLanding = BzTime::getCurrent();
}

inline bool		Player::canTurnLeft() const
{
  return (allow & AllowTurnLeft) != 0;
}

inline bool		Player::canTurnRight() const
{
  return (allow & AllowTurnRight) != 0;
}

inline bool		Player::canMoveForward() const
{
  return (allow & AllowMoveForward) != 0;
}

inline bool		Player::canMoveBackward() const
{
  return (allow & AllowMoveBackward) != 0;
}

inline bool		Player::canShoot() const
{
  //return (state.status & short(PlayerState::AllowShooting)) != 0;
  return (allow & AllowShoot) && getShotType() != NoShot;
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

inline int		Player::getAutoHuntLevel() const
{
  return autoHuntLevel;
}

inline void		Player::setAutoHuntLevel(int level)
{
  autoHuntLevel = level;
}

inline bool		Player::isAdmin() const
{
  return admin;
}

inline void		Player::setAdmin(bool _admin)
{
  admin = _admin;
}

inline bool		Player::isRegistered() const
{
  return registered;
}

inline void		Player::setRegistered(bool _registered)
{
  registered = _registered;
}

inline bool		Player::isVerified() const
{
  return verified;
}

inline void		Player::setVerified(bool _verified)
{
  verified = _verified;
}

inline bool		Player::hasPlayerList() const
{
  return playerList;
}

inline void		Player::setPlayerList(bool _playerList)
{
  playerList = _playerList;
}

inline void		Player::setAllow(unsigned char _allow)
{
  allow = _allow;
}

inline unsigned char	Player::getAllow()
{
  return allow;
}

inline void Player::setZpos (float z)
{
  state.pos[2] = z;
}

inline int Player::getMaxShots() const
{
  return (int)shots.size();
}

#endif // __PLAYER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
