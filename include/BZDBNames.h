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

#ifndef BZF_BZDB_NAMES_H
#define BZF_BZDB_NAMES_H

// common header first
#include "common.h"

// system headers
#include <string>

// common headers
#include "Singleton.h"


#define BZDBNAMES (BZDBNames::instance())


class BZDBNames : public Singleton<BZDBNames> {
  public:
    BZDBNames();

  public:
    const std::string AGILITYADVEL;
    const std::string AGILITYTIMEWINDOW;
    const std::string AGILITYVELDELTA;
    const std::string AMBIENTLIGHT;
    const std::string ANGLETOLERANCE;
    const std::string ANGULARAD;
    const std::string AUTOALLOWTIME;
    const std::string AVENUESIZE;
    const std::string BASESIZE;
    const std::string BOXBASE;
    const std::string BOXHEIGHT;
    const std::string BURROWANGULARAD;
    const std::string BURROWDEPTH;
    const std::string BURROWSPEEDAD;
    const std::string COLDETDEPTH;
    const std::string COLDETELEMENTS;
    const std::string COUNTDOWNRESTIME;
    const std::string CULLDEPTH;
    const std::string CULLELEMENTS;
    const std::string CULLOCCLUDERS;
    const std::string DISABLEBOTS;
    const std::string DRAWCELESTIAL;
    const std::string DRAWCLOUDS;
    const std::string DRAWGROUND;
    const std::string DRAWGROUNDLIGHTS;
    const std::string DRAWMOUNTAINS;
    const std::string DRAWSKY;
    const std::string ENDSHOTDETECTION;
    const std::string EXPLODETIME;
    const std::string FLAGALTITUDE;
    const std::string FLAGEFFECTTIME;
    const std::string FLAGHEIGHT;
    const std::string FLAGPOLESIZE;
    const std::string FLAGPOLEWIDTH;
    const std::string FLAGRADIUS;
    const std::string FOGCOLOR;
    const std::string FOGDENSITY;
    const std::string FOGEND;
    const std::string FOGMODE;
    const std::string FOGSTART;
    const std::string FORBIDDEBUG;
    const std::string FRICTION;
    const std::string GMACTIVATIONTIME;
    const std::string GMADLIFE;
    const std::string GMADSPEED;
    const std::string GMTURNANGLE;
    const std::string GRABOWNFLAG;
    const std::string GRAVITY;
    const std::string HANDICAPANGAD;
    const std::string HANDICAPSCOREDIFF;
    const std::string HANDICAPSHOTAD;
    const std::string HANDICAPVELAD;
    const std::string HEIGHTCHECKTOL;
    const std::string IDENTIFYRANGE;
    const std::string INERTIAANGULAR;
    const std::string INERTIALINEAR;
    const std::string JUMPVELOCITY;
    const std::string LASERADLIFE;
    const std::string LASERADRATE;
    const std::string LASERADVEL;
    const std::string LATITUDE;
    const std::string LGGRAVITY;
    const std::string LOCKONANGLE;
    const std::string LONGITUDE;
    const std::string LRADRATE;
    const std::string MAXBUMPHEIGHT;
    const std::string MAXFLAGGRABS;
    const std::string MAXLOD;
    const std::string MAXSELFDESTRUCTVEL;
    const std::string MGUNADLIFE;
    const std::string MGUNADRATE;
    const std::string MGUNADVEL;
    const std::string MIRROR;
    const std::string MOMENTUMANGACC;
    const std::string MOMENTUMFRICTION;
    const std::string MOMENTUMLINACC;
    const std::string MUZZLEFRONT;
    const std::string MUZZLEHEIGHT;
    const std::string NOCLIMB;
    const std::string NOSHADOWS;
    const std::string NOSMALLPACKETS;
    const std::string NOTRESPONDINGTIME;
    const std::string OBESEFACTOR;
    const std::string PAUSEDROPTIME;
    const std::string POSITIONTOLERANCE;
    const std::string PYRBASE;
    const std::string PYRHEIGHT;
    const std::string RADARLIMIT;
    const std::string REJOINTIME;
    const std::string REJUMPTIME;
    const std::string RELOADTIME;
    const std::string RFIREADLIFE;
    const std::string RFIREADRATE;
    const std::string RFIREADVEL;
    const std::string SCOREBOARDCUSTOMFIELD;
    const std::string SCOREBOARDCUSTOMROWLEN;
    const std::string SCOREBOARDCUSTOMROWNAME;
    const std::string SHIELDFLIGHT;
    const std::string SHOCKADLIFE;
    const std::string SHOCKINRADIUS;
    const std::string SHOCKOUTRADIUS;
    const std::string SHOTRADIUS;
    const std::string SHOTRANGE;
    const std::string SHOTSKEEPVERTICALV;
    const std::string SHOTSPEED;
    const std::string SHOTTAILLENGTH;
    const std::string SPEEDCHECKSLOGONLY;
    const std::string SQUISHFACTOR;
    const std::string SQUISHTIME;
    const std::string SRRADIUSMULT;
    const std::string STARTINGRANK;
    const std::string SYNCLOCATION;
    const std::string SYNCTIME;
    const std::string TANKANGVEL;
    const std::string TANKEXPLOSIONSIZE;
    const std::string TANKHEIGHT;
    const std::string TANKLENGTH;
    const std::string TANKRADIUS;
    const std::string TANKSHOTPROXIMITY;
    const std::string TANKSPEED;
    const std::string TANKWIDTH;
    const std::string TARGETINGANGLE;
    const std::string TCPTIMEOUT;
    const std::string TELEBREADTH;
    const std::string TELEHEIGHT;
    const std::string TELEPORTTIME;
    const std::string TELEWIDTH;
    const std::string THIEFADLIFE;
    const std::string THIEFADRATE;
    const std::string THIEFADSHOTVEL;
    const std::string THIEFDROPTIME;
    const std::string THIEFTINYFACTOR;
    const std::string THIEFVELAD;
    const std::string TINYFACTOR;
    const std::string TRACKFADE;
    const std::string UPDATETHROTTLERATE;
    const std::string VELOCITYAD;
    const std::string WALLHEIGHT;
    const std::string WEAPONS;
    const std::string WIDEANGLEANG;
    const std::string WINGSGRAVITY;
    const std::string WINGSJUMPCOUNT;
    const std::string WINGSJUMPVELOCITY;
    const std::string WINGSSLIDETIME;
    const std::string WORLDSIZE;
};


#endif // BZF_BZDB_NAMES_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
