/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZDBCACHE_H
#define	BZDBCACHE_H

// implementation headers
#include "StateDatabase.h"

class BZDBCache
{
public:
	static void init();

	static bool  displayMainFlags;
	static bool  blend;
	static bool  texture;
	static bool  shadows;
	static bool  stencilShadows;
	static bool  zbuffer;
	static bool  tesselation;
	static bool  lighting;
	static bool  smooth;
	static bool  colorful;
	static int   flagChunks;
	static bool  animatedTreads;
	static int   radarStyle;
	static float radarTankPixels;
	static float shadowAlpha;
	static bool  leadingShotLine;
	static bool  showShotGuide;
	static int   linedRadarShots;
	static int   sizedRadarShots;
	static float shotLength;
	static float pulseRate;
	static float pulseDepth;
	static bool  showCollisionGrid;
	static bool  showCullingGrid;
	static int   maxFlagLOD;
	static int   vsync;

	static bool drawCelestial;
	static bool drawClouds;
	static bool drawGround;
	static bool drawGroundLights;
	static bool drawMountains;
	static bool drawSky;

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

	static float freezeTagRadius;
	static float collisionLimit;
	static float dmzWidth;

	static float hudGUIBorderOpacityFactor;

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


//
// these classes should be used as static variables,
// and only when the bzdb variable is not being used
// in several different files.
//

#define BZDB_VALUE_CLASS(className, type, evalFunc)                  \
  class className {                                                  \
    public:                                                          \
      className(const std::string& _name) : name(_name) {            \
        update();                                                    \
        BZDB.addCallback(name, callback, this);                      \
      }                                                              \
      ~className() {                                                 \
        BZDB.removeCallback(name, callback, this);                   \
      }                                                              \
      operator       const type&() const { return data; }            \
      const type&        getData() const { return data; }            \
      const std::string& getName() const { return name; }            \
    private: /* no copying */                                        \
      className(const className&);                                   \
      className& operator=(const className&);                        \
    private:                                                         \
      void update() {                                                \
        data = BZDB.evalFunc(name);                                  \
      }                                                              \
      static void callback(const std::string& /*name*/, void* ptr) { \
        ((className*)ptr)->update();                                 \
      }                                                              \
    private:                                                         \
      std::string name;                                              \
      type data;                                                     \
  };

BZDB_VALUE_CLASS(BZDB_int,    int,         evalInt)
BZDB_VALUE_CLASS(BZDB_bool,   bool,        isTrue)
BZDB_VALUE_CLASS(BZDB_float,  float,       eval)
BZDB_VALUE_CLASS(BZDB_string, std::string, get)


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
