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

#ifndef	__STD_TANK_AVATAR_H__
#define	__STD_TANK_AVATAR_H__

#include "PlayerAvatarManager.h"
#include "SphereSceneNode.h"
#include "TankSceneNode.h"
#include "vectors.h"

class StandardTankAvatar : public PlayerAvatar
{
public:
  StandardTankAvatar(int playerID, const fvec3& pos, const fvec3& forward);
  virtual ~StandardTankAvatar();

  virtual void move(const fvec3& pos, const fvec3& forward);
  virtual void moveIDL(const fvec4& plane);
  virtual void movePause(const fvec3& pos, float rad);
  virtual void setTurnOffsets(const float left, const float right);
  virtual void setScale(const fvec3& scale);
  virtual void setScale(teAvatarScaleModes mode);
  virtual void explode();
  virtual void setVisualTeam(TeamColor visualTeam, const fvec4& color);
  virtual void setVisualMode(bool inCockpit, bool showTreads);
  virtual void setAnimationValues(float explodeParam, float jumpParam);
  virtual void setClippingPlane( const fvec4& plane);
  virtual void setColor(const fvec4& color);

  virtual void renderRadar();

  virtual std::vector<SceneNode*> getSceneNodes();
  virtual std::vector<SceneNode*> getIDLSceneNodes();
  virtual std::vector<SceneNode*> getPauseSceneNodes();

protected:
  TankSceneNode		*tankNode;
  TankIDLSceneNode	*IDLNode;
  SphereSceneNode	*pausedSphere;

  TeamColor		lastVisualTeam;
  int			tankTexture;
};


#endif /* __STD_TANK_AVATAR_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
