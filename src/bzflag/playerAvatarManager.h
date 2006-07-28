/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef	__PLAYER_AVATAR_MANAGER_H__
#define	__PLAYER_AVATAR_MANAGER_H__

#include "common.h"
#include "TankSceneNode.h"

TankSceneNode* getTankSceneNode ( int playerID, const float pos[3], const float forward[3] );
TankIDLSceneNode* getTankIDLSceneNode ( int playerID, TankSceneNode *tankNode );


#endif /* __PLAYER_AVATAR_MANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
