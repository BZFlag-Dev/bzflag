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

#ifndef __WORLDWEAPON_H__
#define __WORLDWEAPON_H__

/* common header */
#include "common.h"

/* system headers */
#include <vector>

/* common interface headers */
#include "game/Flag.h"
#include "common/BzTime.h"
#include "vectors.h"

#include "bzfs/WorldEventManager.h"

#define _MAX_WORLD_SHOTS 30


/** WorldWeapons is a container class that holds weapons
 */
class WorldWeapons {
  public:
    WorldWeapons();
    ~WorldWeapons();
    void fire();
    void add(const FlagType* type, const fvec3& origin,
             float direction, float tilt, TeamColor teamColor,
             float initdelay, const std::vector<float> &delay,
             BzTime& sync, bool fromMesh = false);
    float nextTime();
    void clear();
    unsigned int count(); // returns the number of world weapons
    int packSize() const;
    void* pack(void* buf) const;

    int getNewWorldShotID(void);

  public:
    struct Weapon {
      const FlagType* type;
      TeamColor   teamColor;
      fvec3   origin;
      float   direction;
      float   tilt;
      float   initDelay;
      std::vector<float>  delay;
      BzTime    nextTime;
      int     nextDelay;
      bool    fromMesh;
    };
    const std::vector<Weapon*>& getWeapons() const { return weapons; }

  private:

    std::vector<Weapon*> weapons;
    int worldShotId;

    WorldWeapons(const WorldWeapons& w);
    WorldWeapons& operator=(const WorldWeapons& w) const;
};

class WorldWeaponGlobalEventHandler : public bz_EventHandler {
  public:
    WorldWeaponGlobalEventHandler(FlagType* type, const fvec3& origin,
                                  float direction, float tilt,
                                  TeamColor teamColor);
    virtual ~WorldWeaponGlobalEventHandler();

    virtual void process(bz_EventData* eventData);

  protected:
    FlagType* type;
    fvec3   origin;
    float   direction;
    float   tilt;
    bz_eTeamType  team;
};

int fireWorldWep(FlagType* type, float lifetime, PlayerId player,
                 const fvec3& pos, float tilt, float direction,
                 int shotID, float dt);

int fireWorldGM(FlagType* type, PlayerId targetPlayerID, float lifetime,
                PlayerId player, const fvec3& pos, float tilt, float direction,
                int shotID, float dt);

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
