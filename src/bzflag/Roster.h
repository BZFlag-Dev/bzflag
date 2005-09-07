/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __ROSTER_H__
#define __ROSTER_H__

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "global.h"
#include "Team.h"

/* local interface headers */
#include "RobotPlayer.h"
#include "BaseLocalPlayer.h"
#include "RemotePlayer.h"


//
// misc utility routines
//

typedef std::vector<std::string> NameList;

extern int curMaxPlayers;
extern RemotePlayer** player;
extern RobotPlayer* robots[MAX_ROBOTS];
extern int numRobots;

extern NameList silencePlayers;


Player*			lookupPlayer(PlayerId id);
int			lookupPlayerIndex(PlayerId id);
Player*			getPlayerByIndex(int index);
Player*			getPlayerByName(const char* name);
BaseLocalPlayer*	getLocalPlayer(PlayerId id);
TeamColor		PlayerIdToTeam(PlayerId id);
PlayerId		TeamToPlayerId(TeamColor team);


#endif /* __ROSTER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
