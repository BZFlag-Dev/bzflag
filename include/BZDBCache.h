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

#ifndef	BZDBCACHE_H
#define	BZDBCACHE_H

#include <string>

#include "StateDatabase.h"

class BZDBCache
{
public:
	static void init();

	static bool  displayMainFlags;
	static bool  blend;
	static bool  texture;
	static bool  shadows;
	static bool  zbuffer;
	static bool  tesselation;
	static bool  lighting;
	static bool  smooth;
	static bool  colorful;
	static int   flagChunks;
	static bool  animatedTreads;
	static int   radarStyle;
	static bool  leadingShotLine;
	static int   linedRadarShots;
	static int   sizedRadarShots;
	static float pulseRate;
	static float pulseDepth;
	static bool  showCollisionGrid;
	static bool  showCullingGrid;

	static float maxLOD;
	static float worldSize;
	static float radarLimit;
	static float gravity;
	static float tankWidth;
	static float tankLength;
	static float tankHeight;
	static float tankSpeed;
	static float tankRadius;
	static float flagRadius;
	static float flagPoleSize;
	static float flagPoleWidth;

public:
  /** public method to update cached variable
      has to be called at best opportunity
      (e.g. at beginnig of main loop)
   */
  static void update();

private:
	static void clientCallback(const std::string &name, void *);
	static void serverCallback(const std::string &name, void *);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
