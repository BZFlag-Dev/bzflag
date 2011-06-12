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

#ifndef __PLAYER_AVATAR_MANAGER_H__
#define __PLAYER_AVATAR_MANAGER_H__

#include "common.h"
#include "game/global.h"
#include "vectors.h"

typedef enum {
  eNormal,
  eFat,
  eTiny,
  eThin,
  eThief
} teAvatarScaleModes;

class SceneNode;

class PlayerAvatar {
  public:
    virtual ~PlayerAvatar(void) {}

    virtual void move(const fvec3& pos, const fvec3& forward) = 0;
    virtual void moveIDL(const fvec4& plane) = 0;
    virtual void movePause(const fvec3& pos, float rad) = 0;

    virtual void setTurnOffsets(const float left, const float right) = 0;

    virtual void setScale(const fvec3& scale) = 0;
    virtual void setScale(teAvatarScaleModes mode) = 0;

    virtual void explode(void) = 0;
    virtual void setVisualTeam(TeamColor visualTeam, const fvec4& color) = 0;
    virtual void setColor(const fvec4& color) = 0;

    virtual void setVisualMode(bool inCockpit, bool showTreads) = 0;
    virtual void setAnimationValues(float explodeParam, float jumpParam) = 0;

    virtual void setClippingPlane(const fvec4* plane) = 0;

    virtual void renderRadar(void) = 0;

    virtual std::vector<SceneNode*> getSceneNodes(void) = 0;
    virtual std::vector<SceneNode*> getIDLSceneNodes(void) = 0;
    virtual std::vector<SceneNode*> getPauseSceneNodes(void) = 0;
};


PlayerAvatar* getPlayerAvatar(int playerID, const fvec3& pos,
                              const fvec3& forward);

void freePlayerAvatar(PlayerAvatar* avatar);


#endif /* __PLAYER_AVATAR_MANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
