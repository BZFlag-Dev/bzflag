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

#ifndef	__WORLD_H__
#define	__WORLD_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "Team.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "TetraBuilding.h"
#include "Teleporter.h"
#include "EighthDimSceneNode.h"
#include "FlagWarpSceneNode.h"
#include "Obstacle.h"
#include "BundleMgr.h"
#include "FlagSceneNode.h"

/* local interface headers */
#include "RemotePlayer.h"
#include "WorldPlayer.h"
#include "Weapon.h"
#include "EntryZone.h"
#include "CollisionManager.h"


/**
 * World:
 *	Game database -- buildings, teleporters, game style
 */
class World {
  friend class WorldBuilder;
  public:
			World();
			~World();

    bool		allowTeamFlags() const;
    bool		allowSuperFlags() const;
    bool		allowJumping() const;
    bool		allowInertia() const;
    bool		allShotsRicochet() const;
    bool		allowAntidote() const;
    bool		allowShakeTimeout() const;
    bool		allowShakeWins() const;
    bool		allowRabbit() const;
    bool		allowHandicap() const;
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
    const float*	getBase(int, int=0) const;
    const std::vector<WallObstacle>	&getWalls() const;
    const std::vector<BoxBuilding>&	getBoxes() const;
    const std::vector<PyramidBuilding>& getPyramids() const;
    const std::vector<BaseBuilding> &getBases() const;
    const std::vector<TetraBuilding> &getTetras() const;
    const std::vector<Teleporter>&	getTeleporters() const;
    const Teleporter*	getTeleporter(int source, int& face) const;
    int			getTeleporter(const Teleporter*, int face) const;
    int			getTeleportTarget(int source) const;
    EighthDimSceneNode*	getInsideSceneNode(const Obstacle*) const;

    TeamColor		whoseBase(const float* pos) const;
    const Obstacle*	inBuilding(const float* pos, float radius,
                                   float tankHeight) const;
    const Obstacle*	hitBuilding(const float* pos, float angle,
                                    float tankWidth, float tankBreadth,
                                    float tankHeight) const;
    const Obstacle*	hitBuilding(const float* oldPos, float oldAngle,
				    const float* pos, float angle,
				    float tankWidth, float tankBreadth,
				    float tankHeight) const;
    bool		crossingTeleporter(const float* oldPos, float angle,
					float tankWidth, float tankBreadth,
					float tankHeight, float* plane) const;
    const Teleporter*	crossesTeleporter(const float* oldPos,
					  const float* newPos, int& face) const;
    const Teleporter*	crossesTeleporter(const Ray& r, int& face) const;
    float		getProximity(const float* pos, float radius) const;

    void		initFlag(int index);
    void		updateFlag(int index, float dt);
    void		addFlags(SceneDatabase*);

    static World*	getWorld();
    static void		setWorld(World*);

    static const CollisionManager* getCollisionManager();

    static BundleMgr*	getBundleMgr();
    static void		setBundleMgr(BundleMgr *bundleMgr);

    static std::string	getLocale();
    static void		setLocale(const std::string &locale);

    static void		init();
    static void		done();
    static void		setFlagTexture(FlagSceneNode*);

    void                loadCollisionManager();
    void                checkCollisionManager();

    bool		writeWorld(std::string filename);

    void		drawCollisionGrid();


  private:
    // disallow copy and assignment
			World(const World&);
    World&		operator=(const World&);

    void		freeFlags();
    void		freeInsideNodes();

  private:
    typedef struct { float p[7]; } BaseParms;
    typedef std::vector<BaseParms> TeamBases;
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
    TeamBases		bases[NumTeams];
    std::vector<BoxBuilding>		boxes;
    std::vector<PyramidBuilding>	pyramids;
    std::vector<BaseBuilding>		basesR;
    std::vector<TetraBuilding>		tetras;
    std::vector<Teleporter>		teleporters;
    std::vector<WallObstacle>		walls;
    std::vector<Weapon>		        weapons;
    std::vector<EntryZone>		entryZones;
    std::vector<int>			teleportTargets;
    CollisionManager                    collisionManager;
    Team		team[NumTeams];
    RemotePlayer**	players;
    WorldPlayer*	worldWeapons;
    Flag*		flags;
    FlagSceneNode**	flagNodes;
    FlagWarpSceneNode**	flagWarpNodes;
    EighthDimSceneNode** boxInsideNodes;
    EighthDimSceneNode** tetraInsideNodes;
    EighthDimSceneNode** pyramidInsideNodes;
    EighthDimSceneNode** baseInsideNodes;
    static World*	playingField;
    static BundleMgr	*bundleMgr;
    static std::string	locale;
    static int flagTexture;
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

inline bool		World::allowRabbit() const
{
  return (gameStyle & short(RabbitChaseGameStyle)) != 0;
}

inline bool		World::allowHandicap() const
{
  return (gameStyle & short(HandicapGameStyle)) != 0;
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

inline const float*	World::getBase(int team, int base) const
{
  const TeamBases &b = bases[team];
  if ((base < 0) || (base >= (int)b.size()))
    return NULL;

  return b[base].p;
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

inline const std::vector<PyramidBuilding>& World::getPyramids() const
{
  return pyramids;
}

inline const std::vector<TetraBuilding>& World::getTetras() const
{
  return tetras;
}

inline const std::vector<Teleporter>&	World::getTeleporters() const
{
  return teleporters;
}

inline World*		World::getWorld()
{
  return playingField;
}

inline const CollisionManager* World::getCollisionManager()
{
  if (playingField != NULL) {
    return &playingField->collisionManager;
  }
  else {
    return NULL;
  }
}

inline BundleMgr*	World::getBundleMgr()
{
  return World::bundleMgr;
}

inline void		World::setBundleMgr(BundleMgr *bundleMgr)
{
  World::bundleMgr = bundleMgr;
}

inline std::string	World::getLocale()
{
  return locale;
}

inline void		World::setLocale(const std::string& locale)
{
  World::locale = locale;
}


#endif /* __WORLD_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
