/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __WORLDWEAPON_H__
#define __WORLDWEAPON_H__

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

/* common header */
#include "common.h"

/* system headers */
#include <vector>

/* common interface headers */
#include "Flag.h"
#include "TimeKeeper.h"

#include "WorldEventManager.h"

#define _MAX_WORLD_SHOTS 30

/** WorldWeapons is a container class that holds weapons
 */
class WorldWeapons
{
public:
  WorldWeapons();
  ~WorldWeapons();
  void fire();
  void add(const FlagType *type, const float *origin,
	   float direction, float tilt, TeamColor teamColor,
	   float initdelay, const std::vector<float> &delay,
	   TimeKeeper &sync);
  float nextTime();
  void clear();
  unsigned int count(); // returns the number of world weapons
  int packSize() const;
  void *pack(void *buf) const;

  int getNewWorldShotID ( void );

private:
  struct Weapon
  {
    const FlagType	*type;
    TeamColor		teamColor;
    float		origin[3];
    float		direction;
    float		tilt;
    float	initDelay;
    std::vector<float>  delay;
    TimeKeeper		nextTime;
    int			nextDelay;
  };

  std::vector<Weapon*> weapons;
  int worldShotId;

  WorldWeapons( const WorldWeapons &w);
  WorldWeapons& operator=(const WorldWeapons &w) const;
};

class WorldWeaponGlobalEventHandler : public bz_EventHandler
{
public:
	WorldWeaponGlobalEventHandler(FlagType *type, const float *origin,float direction, float tilt,TeamColor teamColor );
	virtual ~WorldWeaponGlobalEventHandler();

	virtual void process ( bz_EventData *eventData );

protected:
	FlagType	*type;
	float		origin[3];
	float		direction;
	float		tilt;
	bz_eTeamType	team;
};

int fireWorldWep ( FlagType* type, float lifetime, PlayerId player, float *pos, float tilt, float direction, int shotID, float dt, TeamColor shotTeam = RogueTeam );

int fireWorldGM(FlagType* type, PlayerId targetPlayerID, float lifetime, PlayerId player, float *pos, float tilt, float direction, int shotID, float dt, TeamColor shotTeam = RogueTeam);
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
