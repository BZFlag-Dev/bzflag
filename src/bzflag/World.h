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

class Ray;
class Player;
class DeadPlayer;
class RemotePlayer;
class SceneDatabase;
class FlagSceneNode;
class FlagWarpSceneNode;

class World {
  friend class WorldBuilder;
  public:
			World();
			~World();

    boolean		allowTeamFlags() const;
    boolean		allowSuperFlags() const;
    boolean		allowRogues() const;
    boolean		allowJumping() const;
    boolean		allowInertia() const;
    boolean		allShotsRicochet() const;
    boolean		allowAntidote() const;
    boolean		allowShakeTimeout() const;
    boolean		allowShakeWins() const;
    boolean		allowTimeOfDayAdjust() const;
    float		getLinearAcceleration() const;
    float		getAngularAcceleration() const;
    float		getFlagShakeTimeout() const;
    int			getFlagShakeWins() const;
    int			getMaxPlayers() const;
    int			getMaxDeadPlayers() const;
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
    DeadPlayer**	getDeadPlayers() const;
    Flag&		getFlag(int index) const;
    const float*	getBase(int) const;
    const WallObstacles	&getWalls() const;
    const BoxBuildings&	getBoxes() const;
    const PyramidBuildings& getPyramids() const;
    const BaseBuildings &getBases() const;
    const Teleporters&	getTeleporters() const;
    const Teleporter*	getTeleporter(int source, int& face) const;
    int			getTeleporter(const Teleporter*, int face) const;
    int			getTeleportTarget(int source) const;
    EighthDimSceneNode*	getInsideSceneNode(const Obstacle*) const;

    TeamColor		whoseBase(const float* pos) const;
    const Obstacle*	inBuilding(const float* pos, float radius) const;
    const Obstacle*	hitBuilding(const float* pos, float angle,
					float tankWidth,
					float tankBreadth) const;
    boolean		crossingTeleporter(const float* oldPos, float angle,
					float tankWidth, float tankBreadth,
					float* plane) const;
    const Teleporter*	crossesTeleporter(const float* oldPos,
					const float* newPos, int& face) const;
    const Teleporter*	crossesTeleporter(const Ray& r, int& face) const;
    float		getProximity(const float* pos, float radius) const;

    void		initFlag(int index);
    void		updateFlag(int index, float dt);
    void		addFlags(SceneDatabase*);

    void		reviveDeadPlayer(Player* revivedPlayer);
    void		addDeadPlayer(Player* dyingPlayer);

    static World*	getWorld();
    static void		setWorld(World*);

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
    int			maxShots;
    int			maxFlags;
    float		shakeTimeout;
    int			shakeWins;
    uint32_t		epochOffset;
    float		bases[NumTeams][9];
    BoxBuildings	boxes;
    PyramidBuildings	pyramids;
    BaseBuildings	basesR;
    Teleporters		teleporters;
    WallObstacles	walls;
    int*		teleportTargets;
    Team		team[NumTeams];
    RemotePlayer**	players;
    DeadPlayer**	deadPlayers;
    Flag*		flags;
    FlagSceneNode**	flagNodes;
    FlagWarpSceneNode**	flagWarpNodes;
    EighthDimSceneNode** boxInsideNodes;
    EighthDimSceneNode** pyramidInsideNodes;
    EighthDimSceneNode** baseInsideNodes;
    static World*	playingField;
    static const int	maxDeadPlayers;
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
    void		growTargetList(int newMinSize);
    void		preGetWorld();

  private:
    boolean		owned;
    World*		world;
    int			targetArraySize;
    int*		teleportTargets;
};

//
// World
//

inline boolean		World::allowTeamFlags() const
{
  return (gameStyle & short(TeamFlagGameStyle)) != 0;
}

inline boolean		World::allowSuperFlags() const
{
  return (gameStyle & short(SuperFlagGameStyle)) != 0;
}

inline boolean		World::allowRogues() const
{
  return (gameStyle & short(RoguesGameStyle)) != 0;
}

inline boolean		World::allowJumping() const
{
  return (gameStyle & short(JumpingGameStyle)) != 0;
}

inline boolean		World::allowInertia() const
{
  return (gameStyle & short(InertiaGameStyle)) != 0;
}

inline boolean		World::allShotsRicochet() const
{
  return (gameStyle & short(RicochetGameStyle)) != 0;
}

inline boolean		World::allowAntidote() const
{
  return (gameStyle & short(AntidoteGameStyle)) != 0;
}

inline boolean		World::allowShakeTimeout() const
{
  return (gameStyle & short(ShakableGameStyle)) != 0 && shakeTimeout != 0.0f;
}

inline boolean		World::allowShakeWins() const
{
  return (gameStyle & short(ShakableGameStyle)) != 0 && shakeWins != 0;
}

inline boolean		World::allowTimeOfDayAdjust() const
{
  return (gameStyle & short(TimeSyncGameStyle)) == 0;
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

inline int		World::getMaxDeadPlayers() const
{
  return maxDeadPlayers;
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

inline DeadPlayer**	World::getDeadPlayers() const
{
  return deadPlayers;
}

inline Flag&		World::getFlag(int index) const
{
  return flags[index];
}

inline const float*	World::getBase(int team) const
{
  return bases[team];
}

inline const WallObstacles&	World::getWalls() const
{
  return walls;
}

inline const BaseBuildings&	World::getBases() const
{
  return basesR;
}

inline const BoxBuildings&	World::getBoxes() const
{
  return boxes;
}

inline const PyramidBuildings&	World::getPyramids() const
{
  return pyramids;
}

inline const Teleporters&	World::getTeleporters() const
{
  return teleporters;
}

inline World*		World::getWorld()
{
  return playingField;
}

#endif // BZF_WORLD_H
// ex: shiftwidth=2 tabstop=8
