/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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

#include <string>


namespace BZDB_Eval {
  template <typename T> inline T eval(const std::string&);
  template <> inline int         eval(const std::string& n) { return BZDB.evalInt(n);   }
  template <> inline bool        eval(const std::string& n) { return BZDB.isTrue(n);    }
  template <> inline float       eval(const std::string& n) { return BZDB.eval(n);      }
  template <> inline fvec2       eval(const std::string& n) { return BZDB.evalFVec2(n); }
  template <> inline fvec3       eval(const std::string& n) { return BZDB.evalFVec3(n); }
  template <> inline fvec4       eval(const std::string& n) { return BZDB.evalFVec4(n); }
  template <> inline std::string eval(const std::string& n) { return BZDB.get(n);       }
}


class BZDBCache
{
  public:
    static void init();

    /** public method to update cached variable
        has to be called at best opportunity
        (e.g. at beginnig of main loop)
     */
    static void update();

  public:
    // prohibit external write access
    template <class T>
    class ReadOnly {
      friend class BZDBCache;
      public:
        inline operator const T&() const { return data; }
      private:
        ReadOnly() {}
        ReadOnly& operator=(const T& value) { data = value; return *this; }
      private:
        ReadOnly(const ReadOnly&);
        ReadOnly& operator=(const ReadOnly&);
      private:
        T data;
    };

    // the basics types
    typedef ReadOnly<int>         Int;
    typedef ReadOnly<bool>        Bool;
    typedef ReadOnly<float>       Float;
    typedef ReadOnly<std::string> String;

  public:
    // client-side
    static Bool  displayMainFlags;
    static Bool  blend;
    static Bool  texture;
    static Bool  zbuffer;
    static Bool  tesselation;
    static Bool  lighting;
    static Bool  smooth;
    static Bool  colorful;
    static Int   flagChunks;
    static Bool  animatedTreads;
    static Int   radarStyle;
    static Float radarTankPixels;
    static Int   shadowMode;
    static Float shadowAlpha;
    static Bool  leadingShotLine;
    static Bool  showShotGuide;
    static Int   linedRadarShots;
    static Int   sizedRadarShots;
    static Float shotLength;
    static Float pulseRate;
    static Float pulseDepth;
    static Bool  showCollisionGrid;
    static Bool  showCullingGrid;
    static Int   maxFlagLOD;
    static Int   vsync;
    static Float hudGUIBorderOpacityFactor;

    // server-side
    static Bool  drawCelestial;
    static Bool  drawClouds;
    static Bool  drawGround;
    static Bool  drawGroundLights;
    static Bool  drawMountains;
    static Bool  drawSky;

    static Bool  forbidDebug;

    static Float flagPoleSize;
    static Float flagPoleWidth;
    static Float flagRadius;
    static Float gravity;
    static Float maxLOD;
    static Float muzzleHeight;
    static Float radarLimit;
    static Float tankHeight;
    static Float tankLength;
    static Float tankRadius;
    static Float tankSpeed;
    static Float tankAngVel;
    static Float tankWidth;
    static Float worldSize;
    static Float minGameFrameTime;
    static Float maxGameFrameTime;

  private:
    static void clientCallback(const std::string &name, void *);
    static void serverCallback(const std::string &name, void *);

  public:
    template <typename T>
    class static_hook {
      public:
        static_hook(const std::string& _name) : name(_name) {
          update();
          BZDB.addCallback(name, callback, this);
        }

        ~static_hook() {
          BZDB.removeCallback(name, callback, this);
        }

        inline operator const T&() const { return data; }
        inline const T& getData()  const { return data; }

        inline const std::string& getName() const { return name; }

      private: /* no copying */
        static_hook(const static_hook&);
        static_hook& operator=(const static_hook&);

      private:
        void update() {
          data = BZDB_Eval::eval<T>(name);
        }
        static void callback(const std::string& /*name*/, void* ptr) {
          ((static_hook*)ptr)->update();
        }

      private:
        const std::string name;
        T data;
    };
};


//
// these classes should be used as static variables,
// and only when the bzdb variable is not being used
// in several different files.
//
typedef BZDBCache::static_hook<int>         BZDB_int;
typedef BZDBCache::static_hook<bool>        BZDB_bool;
typedef BZDBCache::static_hook<float>       BZDB_float;
typedef BZDBCache::static_hook<fvec2>       BZDB_fvec2;
typedef BZDBCache::static_hook<fvec3>       BZDB_fvec3;
typedef BZDBCache::static_hook<fvec4>       BZDB_fvec4;
typedef BZDBCache::static_hook<std::string> BZDB_string;



#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
