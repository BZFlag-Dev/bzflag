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

/*
 * World:
 *	Game database -- buildings, teleporters, game style
 */

#ifndef	BZF_WORLD_H
#define	BZF_WORLD_H

#include "common.h"
#include "global.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "WallObstacle.h"
#include "Flag.h"
#include "Team.h"
#include "EighthDimSceneNode.h"
#include "BundleMgr.h"


class Ray;
class Player;
class RemotePlayer;
class WorldPlayer;
class SceneDatabase;
class FlagSceneNode;
class FlagWarpSceneNode;

class World {
  friend class WorldBuilder;
  public:
			World();
			~World();

    bool		allowTeamFlags() const;
    bool		allowSuperFlags() const;
    bool		allowRogues() const;
    bool		allowJumping() const;
    bool		allowInertia() const;
    bool		allShotsRicochet() const;
    bool		allowAntidote() const;
    bool		allowShakeTimeout() const;
    bool		allowShakeWins() const;
    bool		allowTimeOfDayAdjust() const;
    bool		allowRabbit() const;
    float		getLinearAcceleration() const;
    float		getAngularAcceleration() const;
    float		getFlagShakeTimeout() const;
    int			getFlagShakeWins() const;
    int			getMaxPlayers() const;
    int			getCurMaxPlayers() const;
    void		setCurMaxPlayers(int curMaxPlayers);
    int			getMaxShots() const;
    int			getMaxFlags() const;
    float		getShakeTimeout() const;
    int			getShakeWins() const;
    uint32_t		getEpochOffset() const;
    const Team*		getTeams() const;
    const Team&		getTeam(int index) const;
    Team*		getTeams();
    Team&		getTeam(int index);
    RemotePlayer**	getPlayers() const;
    RemotePlayer*&	getPlayer(int index) const;
    WorldPlayer*	getWorldWeapons() const;
    Flag&		getFlag(int index) const;
    const float*	getBase(int) const;
    const std::vector<WallObstacle>	&getWalls() const;
    const std::vector<BoxBuilding>&	getBoxes() const;
    const std::vector<PyramidBuilding>& getPyramids() const;
    const std::vector<BaseBuilding> &getBases() const;
    const std::vector<Teleporter>&	getTeleporters() const;
    const Teleporter*	getTeleporter(int source, int& face) const;
    int			getTeleporter(const Teleporter*, int face) const;
    int			getTeleportTarget(int source) const;
    EighthDimSceneNode*	getInsideSceneNode(const Obstacle*) const;

    TeamColor		whoseBase(const float* pos) const;
    const Obstacle*	inBuilding(const float* pos, float radius) const;
    const Obstacle*	hitBuilding(const float* pos, float angle,
					float tankWidth,
					float tankBreadth) const;
    bool		crossingTeleporter(const float* oldPos, float angle,
					float tankWidth, float tankBreadth,
					float* plane) const;
    const Teleporter*	crossesTeleporter(const float* oldPos,
					const float* newPos, int& face) const;
    const Teleporter*	crossesTeleporter(const Ray& r, int& face) const;
    float		getProximity(const float* pos, float radius) const;

    void		initFlag(int index);
    void		updateFlag(int index, float dt);
    void		addFlags(SceneDatabase*);

    static World*	getWorld();
    static void		setWorld(World*);

    static BundleMgr*	getBundleMgr();
    static void		setBundleMgr(BundleMgr *bundleMgr);

    static std::string	getLocale();
    static void		setLocale(const std::string &locale);

    static void		init();
    static void		done();
    static void		setFlagTexture(FlagSceneNode*);

  private:
    // disallow copy and assignment
			World(const World&);
    World&		operator=(const World&);

    void		freeFlags();
    void		freeInsideNodes();

  private:
    short		gameStyle;
    float		linearAcceleration;
    float		angularAcceleration;
    int			maxPlayers;
    int			curMaxPlayers;
    int			maxShots;
    int			maxFlags;
    float		shakeTimeout;
    int			shakeWins;
    uint32_t		epochOffset;
    float		bases[NumTeams][9];
    std::vector<BoxBuilding>		boxes;
    std::vector<PyramidBuilding>	pyramids;
    std::vector<BaseBuilding>		basesR;
    std::vector<Teleporter>		teleporters;
    std::vector<WallObstacle>		walls;
    std::vector<int>			teleportTargets;
    Team		team[NumTeams];
    RemotePlayer**	players;
    WorldPlayer*	worldWeapons;
    Flag*		flags;
    FlagSceneNode**	flagNodes;
    FlagWarpSceneNode**	flagWarpNodes;
    EighthDimSceneNode** boxInsideNodes;
    EighthDimSceneNode** pyramidInsideNodes;
    EighthDimSceneNode** baseInsideNodes;
    static World*	playingField;
    static BundleMgr	*bundleMgr;
    static std::string	locale;
};

class WorldBuilder {
  public:
			WorldBuilder();
			~WorldBuilder();

    void*		unpack(void*);

    World*		getWorld();
    World*		peekWorld();	// doesn't give up ownership

    void		setGameStyle(short gameStyle);
    void		setInertia(float linearAccel, float angularAccel);
    void		setMaxPlayers(int maxPlayers);
    void		setMaxShots(int maxSimultaneousShots);
    void		setMaxFlags(int maxFlags);
    void		setShakeTimeout(float timeout) const;
    void		setShakeWins(int wins) const;
    void		setEpochOffset(uint32_t seconds) const;
    void		append(const WallObstacle&);
    void		append(const BoxBuilding&);
    void		append(const PyramidBuilding&);
    void		append(const BaseBuilding&);
    void		append(const Teleporter&);
    void		setTeleporterTarget(int source, int target);
    void		setBase(TeamColor team,
				const float* pos, float rotation,
				float w, float b, const float* safety);

  private:
    void		preGetWorld();

  private:
    bool		owned;
    World*		world;
    std::vector<int>	teleportTargets;
};

//
// World
//

inline bool		World::allowTeamFlags() const
{
  return (gameStyle & short(TeamFlagGameStyle)) != 0;
}

inline bool		World::allowSuperFlags() const
{
  return (gameStyle & short(SuperFlagGameStyle)) != 0;
}

inline bool		World::allowRogues() const
{
  return (gameStyle & short(RoguesGameStyle)) != 0;
}

inline bool		World::allowJumping() const
{
  return (gameStyle & short(JumpingGameStyle)) != 0;
}

inline bool		World::allowInertia() const
{
  return (gameStyle & short(InertiaGameStyle)) != 0;
}

inline bool		World::allShotsRicochet() const
{
  return (gameStyle & short(RicochetGameStyle)) != 0;
}

inline bool		World::allowAntidote() const
{
  return (gameStyle & short(AntidoteGameStyle)) != 0;
}

inline bool		World::allowShakeTimeout() const
{
  return (gameStyle & short(ShakableGameStyle)) != 0 && shakeTimeout != 0.0f;
}

inline bool		World::allowShakeWins() const
{
  return (gameStyle & short(ShakableGameStyle)) != 0 && shakeWins != 0;
}

inline bool		World::allowTimeOfDayAdjust() const
{
  return (gameStyle & short(TimeSyncGameStyle)) == 0;
}

inline bool		World::allowRabbit() const
{
  return (gameStyle & short(RabbitChaseGameStyle)) != 0;
}

inline float		World::getLinearAcceleration() const
{
  return linearAcceleration;
}

inline float		World::getAngularAcceleration() const
{
  return angularAcceleration;
}

inline float		World::getFlagShakeTimeout() const
{
  return shakeTimeout;
}

inline int		World::getFlagShakeWins() const
{
  return shakeWins;
}

inline int		World::getMaxPlayers() const
{
  return maxPlayers;
}

inline int		World::getCurMaxPlayers() const
{
  return curMaxPlayers;
}

inline void		World::setCurMaxPlayers(int curMaxPlayers)
{
  this->curMaxPlayers = curMaxPlayers;
}

inline int		World::getMaxShots() const
{
  return maxShots;
}

inline int		World::getMaxFlags() const
{
  return maxFlags;
}

inline float		World::getShakeTimeout() const
{
  return shakeTimeout;
}

inline int		World::getShakeWins() const
{
  return shakeWins;
}

inline uint32_t		World::getEpochOffset() const
{
  return epochOffset;
}

inline const Team*	World::getTeams() const
{
  return team;
}

inline Team*		World::getTeams()
{
  return team;
}

inline const Team&	World::getTeam(int index) const
{
  return team[index];
}

inline Team&		World::getTeam(int index)
{
  return team[index];
}

inline RemotePlayer**	World::getPlayers() const
{
  return players;
}

inline RemotePlayer*&	World::getPlayer(int index) const
{
  return players[index];
}

inline WorldPlayer*	World::getWorldWeapons() const
{
  return worldWeapons;
}

inline Flag&		World::getFlag(int index) const
{
  return flags[index];
}

inline const float*	World::getBase(int team) const
{
  return bases[team];
}

inline const std::vector<WallObstacle>&	World::getWalls() const
{
  return walls;
}

inline const std::vector<BaseBuilding>&	World::getBases() const
{
  return basesR;
}

inline const std::vector<BoxBuilding>&	World::getBoxes() const
{
  return boxes;
}

inline const std::vector<PyramidBuilding>&	World::getPyramids() const
{
  return pyramids;
}

inline const std::vector<Teleporter>&	World::getTeleporters() const
{
  return teleporters;
}

inline World*		World::getWorld()
{
  return playingField;
}

inline BundleMgr* World::getBundleMgr()
{
	return World::bundleMgr;
}

inline void World::setBundleMgr(BundleMgr *bundleMgr)
{
	World::bundleMgr = bundleMgr;
}

inline std::string World::getLocale()
{
	return locale;
}

inline void World::setLocale(const std::string& locale)
{
	World::locale = locale;
}

#endif // BZF_WORLD_H
// ex: shiftwidth=2 tabstop=8
