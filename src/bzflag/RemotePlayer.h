/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_REMOTE_PLAYER_H
#define	BZF_REMOTE_PLAYER_H

#include "common.h"
#include "Player.h"
#include "ShotPath.h"

class RemotePlayer : public Player {
  public:
			RemotePlayer(const PlayerId&, TeamColor team,
					const char* name, const char* email);
			~RemotePlayer();

    void		addShot(const FiringInfo&);
    ShotPath*		getShot(int index) const;
    void		updateShots(float dt);

  private:
    bool		doEndShot(int index, bool isHit, float* pos);

  private:
    int			numShots;
    RemoteShotPath**	shots;
};

#endif // BZF_REMOTE_PLAYER_H
// ex: shiftwidth=2 tabstop=8
