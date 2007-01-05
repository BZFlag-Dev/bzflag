/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_WORLD_PLAYER_H
#define	BZF_WORLD_PLAYER_H

#include "common.h"
#include "Player.h"
#include "ShotPath.h"
#include <vector>

class WorldPlayer : public Player {
  public:
			WorldPlayer();
			~WorldPlayer();

    void		addShot(const FiringInfo&);
    void		updateShots(float dt);
    void		addShots(SceneDatabase* scene, bool colorblind) const;

  private:
    bool		doEndShot(int index, bool isHit, float* pos);
};

#endif // BZF_WORLD_PLAYER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

