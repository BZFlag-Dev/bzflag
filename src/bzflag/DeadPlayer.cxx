/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "DeadPlayer.h"

DeadPlayer::DeadPlayer(const Player& livePlayer) :
								Player(livePlayer.getId(),
										livePlayer.getTeam(),
										livePlayer.getCallSign(),
										livePlayer.getEmailAddress())
{
	// get wins and losses
	changeScore(livePlayer.getWins(), livePlayer.getLosses());
	changeLocalScore(livePlayer.getLocalWins(), livePlayer.getLocalLosses());
}

DeadPlayer::~DeadPlayer()
{
	// do nothing
}
