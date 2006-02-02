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

// bzflag global header
#include "global.h"

#include "bzfsMessages.h"
#include "GameKeeper.h"
#include "bzfs.h"

void sendRemovePlayerMessage ( int playerID )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerID);
	broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
		if ((p == NULL) || !p->playerHandler || playerID == p->getIndex())
			continue;
		p->playerHandler->playerRemoved(playerID);
	}
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
