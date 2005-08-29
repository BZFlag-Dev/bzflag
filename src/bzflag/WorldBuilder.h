/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__WORLDBUILDER_H__
#define	__WORLDBUILDER_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "WallObstacle.h"
#include "MeshObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "TetraBuilding.h"

/* local interface headers */
#include "World.h"


/** builds a bzlfag world
 */
class WorldBuilder {
  public:
			WorldBuilder();
			~WorldBuilder();

    void*		unpack(void*);
    void	       *unpackGameSettings(void*);

    World*		getWorld();
    World*		peekWorld();	// doesn't give up ownership

    void		setGameStyle(short gameStyle);
    void		setMaxPlayers(int maxPlayers);
    void		setMaxShots(int maxSimultaneousShots);
    void		setMaxFlags(int maxFlags);
    void		setShakeTimeout(float timeout) const;
    void		setShakeWins(int wins) const;
    void		setBase(TeamColor team,
				const float* pos, float rotation,
				float w, float b, float h);

  private:
    void		preGetWorld();

  private:
    bool		owned;
    World*		world;
};


#endif /* __WORLDBUILDER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
