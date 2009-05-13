/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__WORLD_H__
#define	__WORLD_H__

#include "common.h"

// system headers
#include <vector>
#include <string>

// common headers
#include "Team.h"
#include "FlagWarpSceneNode.h"
#include "BundleMgr.h"
#include "MapInfo.h"
#include "global.h"
#include "vectors.h"

// local headers
#include "Weapon.h"
#include "EntryZone.h"


class WorldPlayer;
class RemotePlayer;
class FlagSceneNode;
class Obstacle;
class MeshFace;
class MeshDrawInfo;
class BzMaterial;
class SceneDatabase;


/**
 * World:
 *	Game database -- buildings, teleporters, game style
 */
class World {

  friend class WorldBuilder;

  public:
    World();
    ~World();

    GameType		getGameType() const;
    bool		allowTeamFlags() const;
    bool		allowTeamKills() const;
    bool		allowSuperFlags() const;
    bool		allowJumping() const;
    bool		allShotsRicochet() const;
    bool		allowAntidote() const;
    bool		allowShakeTimeout() const;
    bool		allowShakeWins() const;
    bool		allowRabbit() const;
    bool		allowHandicap() const;
    bool		allowTeams() const;
    float		getWaterLevel() const;
    const BzMaterial*	getWaterMaterial() const;
    float		getFlagShakeTimeout() const;
    int			getFlagShakeWins() const;
    int			getMaxPlayers() const;
    int			getCurMaxPlayers() const;
    void		setCurMaxPlayers(int curMaxPlayers);
    int			getMaxShots() const;
    int			getMaxFlags() const;
    float		getShakeTimeout() const;
    int			getShakeWins() const;
    const Team*		getTeams() const;
    const Team&		getTeam(int index) const;
    Team*		getTeams();
    Team&		getTeam(int index);
    RemotePlayer**	getPlayers() const;
    RemotePlayer*&	getPlayer(int index) const;
    void		setPlayersSize(int _playersSize);
    int		 getPlayersSize();
    RemotePlayer*	getCurrentRabbit() const;
    WorldPlayer*	getWorldWeapons() const;
    Flag&		getFlag(int index) const;
    const Obstacle*	getBase(int team, int base = 0) const;

    TeamColor		whoseBase(const fvec3& pos) const;
    const Obstacle*	inBuilding(const fvec3& pos, float radius,
				   float tankHeight) const;
    const Obstacle*	inBuilding(const fvec3& pos, float angle,
				   float tankWidth, float tankBreadth,
				   float tankHeight) const;
    const Obstacle*	hitBuilding(const fvec3& pos, float angle,
				    float tankWidth, float tankBreadth,
				    float tankHeight) const;
    const Obstacle*	hitBuilding(const fvec3& oldPos, float oldAngle,
				    const fvec3& pos, float angle,
				    float tankWidth, float tankBreadth,
				    float tankHeight, bool directional) const;

    const MeshFace*	crossingTeleporter(const fvec3& oldPos, float angle,
                                           float tankWidth, float tankBreadth,
                                           float tankHeight) const;
    const MeshFace*	crossesTeleporter(const fvec3& oldPos,
                                          const fvec3& newPos) const;
    float		getProximity(const fvec3& pos, float radius) const;

    void		initFlag(int index);
    void		updateFlag(int index, float dt);
    void		updateAnimations(float dt);
    void		addFlags(SceneDatabase*, bool seerView);
    void		updateWind(float dt);
    void		getWind(fvec3& wind, const fvec3& pos) const;

    void		makeMeshDrawMgrs();

    static World*	getWorld();
    static void		setWorld(World*);

    static BundleMgr*	getBundleMgr();
    static void		setBundleMgr(BundleMgr *bundleMgr);

    static std::string	getLocale();
    static void		setLocale(const std::string &locale);

    static void		init();
    static void		done();
    static void		setFlagTexture(FlagSceneNode*);

    void		loadCollisionManager();
    void		checkCollisionManager();

    bool		writeWorld(const std::string& filename,
				   std::string& fullname);

    void		drawCollisionGrid() const;

    void		freeInsideNodes() const;

    const MapInfo&	getMapInfo() const { return mapInfo; }
    const std::string&	getMapHash() const { return mapHash; }
    void		setMapHash(const std::string& hash) { mapHash = hash; }

  private:
    // disallow copy and assignment
			World(const World&);
    World&		operator=(const World&);

    void		freeFlags();
    void		freeMeshDrawMgrs();

  private:
    GameType		gameType;
    short		gameOptions;
    int			maxPlayers;
    int			curMaxPlayers;
    int			maxShots;
    int			maxFlags;
    float		shakeTimeout;
    int			shakeWins;
    float		waterLevel;
    const BzMaterial*	waterMaterial;

    typedef std::vector<const Obstacle*> TeamBases;
    TeamBases		bases[NumTeams];
    Team		team[NumTeams];

    std::vector<Weapon>	weapons;
    std::vector<EntryZone> entryZones;

    RemotePlayer**	players;
    int			playersSize;
    WorldPlayer*	worldWeapons;
    Flag*		flags;
    FlagSceneNode**	flagNodes;
    FlagWarpSceneNode**	flagWarpNodes;

    int			drawInfoCount;
    MeshDrawInfo**	drawInfoArray;

    fvec3		wind;

    // required graphics settings
    int			oldFogEffect;
    bool		oldUseDrawInfo;

    MapInfo		mapInfo;
    std::string		mapHash;

    static World*	playingField;
    static BundleMgr	*bundleMgr;
    static std::string	locale;
    static int flagTexture;
};


//
// World
//

inline bool	World::allowTeams() const
{
  return gameType != OpenFFA;
}


inline bool		World::allowTeamFlags() const
{
  return gameType == ClassicCTF;
}

inline bool		World::allowTeamKills() const
{
  return (gameOptions & short(NoTeamKills)) == 0;
}

inline bool		World::allowSuperFlags() const
{
  return (gameOptions & short(SuperFlagGameStyle)) != 0;
}

inline bool		World::allowJumping() const
{
  return (gameOptions & short(JumpingGameStyle)) != 0;
}

inline bool		World::allShotsRicochet() const
{
  return (gameOptions & short(RicochetGameStyle)) != 0;
}

inline bool		World::allowAntidote() const
{
  return (gameOptions & short(AntidoteGameStyle)) != 0;
}

inline bool		World::allowShakeTimeout() const
{
  return (gameOptions & short(ShakableGameStyle)) != 0 && shakeTimeout != 0.0f;
}

inline bool		World::allowShakeWins() const
{
  return (gameOptions & short(ShakableGameStyle)) != 0 && shakeWins != 0;
}

inline bool		World::allowRabbit() const
{
  return gameType == RabbitChase;
}

inline bool		World::allowHandicap() const
{
  return (gameOptions & short(HandicapGameStyle)) != 0;
}

inline float		World::getWaterLevel() const
{
  return waterLevel;
}

inline const BzMaterial*	World::getWaterMaterial() const
{
  return waterMaterial;
}

inline GameType		World::getGameType() const
{
  return gameType;
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

inline void		World::setCurMaxPlayers(int _curMaxPlayers)
{
  curMaxPlayers = _curMaxPlayers;
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

inline int	      World::getPlayersSize()
{
  return playersSize;
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

inline const Obstacle* World::getBase(int _team, int base) const
{
  const TeamBases &b = bases[_team];
  if ((base < 0) || (base >= (int)b.size())) {
    return NULL;
  }
  return b[base];
}

inline World*		World::getWorld()
{
  return playingField;
}

inline BundleMgr*	World::getBundleMgr()
{
  return World::bundleMgr;
}

inline void		World::setBundleMgr(BundleMgr *_bundleMgr)
{
  bundleMgr = _bundleMgr;
}

inline std::string	World::getLocale()
{
  return locale;
}

inline void		World::setLocale(const std::string& _locale)
{
  locale = _locale;
}

inline void		World::getWind(fvec3& w, const fvec3&) const
{
  // homogeneous, for now
  w[0] = wind[0];
  w[1] = wind[1];
  w[2] = wind[2];
  return;
}

#endif /* __WORLD_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
