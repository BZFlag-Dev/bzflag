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

#ifndef	__REMOTEPLAYER_H__
#define	__REMOTEPLAYER_H__

#include "common.h"

/* interface header */
#include "Player.h"

/* common interface headers */
#include "global.h"
#include "ShotUpdate.h"

/* local interface headers */
#include "ShotPath.h"


class RemotePlayer : public Player {
public:
  RemotePlayer(const PlayerId&, TeamColor team,
	       const char* name, const char* email,
	       const PlayerType);
  ~RemotePlayer();

  void addShot(FiringInfo&);
  void updateShots(float dt);

private:
  bool doEndShot(int index, bool isHit, float* pos);

private:
  int numShots;
};


#endif /* __REMOTE_PLAYER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
