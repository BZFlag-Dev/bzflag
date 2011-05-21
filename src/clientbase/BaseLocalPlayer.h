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

#ifndef __BASELOCALPLAYER_H__
#define __BASELOCALPLAYER_H__

// common - 1st one
#include "common.h"

/* common interface headers */
#include "Ray.h"
#include "Extents.h"
#include "BzTime.h"

/* local interface headers */
#include "Player.h"
#include "ShotPath.h"


class BaseLocalPlayer : public Player {
  public:
    BaseLocalPlayer(const PlayerId&, const char* name,
                    const PlayerType type);
    ~BaseLocalPlayer();

    virtual void update(float inputDT = -1.0f);
    Ray getLastMotion() const;
    const Extents& getLastMotionBBox() const;

    virtual void changeTeam(TeamColor newTeam) { Player::changeTeam(newTeam); }
    virtual void explodeTank() = 0;
    virtual bool checkHit(const Player* source,
                          const ShotPath*& hit, float& minTime) const = 0;
  protected:
    int getSalt();
    virtual void doUpdate(float dt) = 0;
    virtual void doUpdateMotion(float dt) = 0;

  protected:
    BzTime lastTime;
    fvec3 lastPosition;
    // bbox of last motion
    Extents bbox;

  private:
    int salt;
};


#endif /* __BASELOCALPLAYER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
