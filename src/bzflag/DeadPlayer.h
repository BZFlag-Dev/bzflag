/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * DeadPlayer is just a way to hold on to player info if a player
 * quits and rejoins.  it's used to keep around personal kill
 * ratio info for players so you don't lose it when they quit.
 *
 * it holds far more info than we really need but so what.
 */

#ifndef	BZF_DEADPLAYER_H
#define	BZF_DEADPLAYER_H

#include "Player.h"

class DeadPlayer : public Player {
  public:
			DeadPlayer(const Player&);
			~DeadPlayer();

    ShotPath*		getShot(int) const { return NULL; }

  private:
    bool		doEndShot(int, bool, float*) { return false; }
};

#endif // BZF_DEADPLAYER_H
// ex: shiftwidth=2 tabstop=8
