/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
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

#include "playerAvatarManager.h"
#include "SphereSceneNode.h"
#include "TankSceneNode.h"

class StandardTankAvatar : public PlayerAvatar
{
public:
  StandardTankAvatar ( int playerID, const float pos[3], const float forward[3] );
  virtual ~StandardTankAvatar ( void );

  virtual void move ( const float pos[3], const float forward[3] );
  virtual void moveIDL ( const float plane[4] );
  virtual void movePause ( const float pos[3], float rad );
  virtual void setTurnOffsets ( const float left, const float right );
  virtual void setScale ( const float scale[3]);
  virtual void setScale ( teAvatarScaleModes mode );
  virtual void explode ( void );
  virtual void setVisualTeam (TeamColor visualTeam, const float color[4] );
  virtual void setVisualMode ( bool inCockpit, bool showTreads );
  virtual void setAnimationValues ( float explodeParam, float jumpParam );
  virtual void setClipingPlane (  const float plane[4] );
  virtual void setColor ( const float color[4] );

  virtual void renderRadar ( void );

  virtual std::vector<SceneNode*> getSceneNodes ( void );
  virtual std::vector<SceneNode*> getIDLSceneNodes ( void );
  virtual std::vector<SceneNode*> getPauseSceneNodes ( void );

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
