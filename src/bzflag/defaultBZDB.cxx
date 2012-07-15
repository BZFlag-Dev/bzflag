/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// BZFlag common header
#include "common.h"

#include <assert.h>

#include "defaultBZDB.h"
#include "Protocol.h"
#include "callbacks.h"

DefaultDBItem	defaultDBItems[] = {
  { "fpsLimit",			"30",			true,	StateDatabase::ReadWrite,	NULL },
  { "saveEnergy",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "saveSettings",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "udpnet",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "timedate",			"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "motto",			"",			true,	StateDatabase::ReadWrite,	NULL },
  { "team",			"Rogue",		true,	StateDatabase::ReadWrite,	NULL },
  { "list",			DefaultListServerURL,	true,	StateDatabase::ReadWrite,	NULL },
  { "motdServer",		DefaultMOTDServer,	true,	StateDatabase::ReadWrite,	NULL },
  { "volume",			"10",			true,	StateDatabase::ReadWrite,	NULL },
  { "latitude",			"37.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "longitude",		"122",			true,	StateDatabase::ReadWrite,	NULL },
  { "radarStyle",		"3",			true,	StateDatabase::ReadWrite,	NULL },
  { "radarTankPixels",		"2.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "coloredradarshots",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "linedradarshots",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "sizedradarshots",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "panelopacity",		"0.3",			true,	StateDatabase::ReadWrite,	NULL },
  { "radaropacity",             "0.3",                  true,   StateDatabase::ReadWrite,       NULL },
  { "radarsize",		"4",			true,	StateDatabase::ReadWrite,	NULL },
  { "mouseboxsize",		"5",			true,	StateDatabase::ReadWrite,	NULL },
  { "mouseClamp",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "cpanelfontsize",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "scorefontsize",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "colorful",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "tkwarnratio",		"0.0",			true,   StateDatabase::ReadWrite,	NULL },
  { "showtabs",			"2",			true,   StateDatabase::ReadWrite,	NULL },
  { "underlineColor",		"cyan",			true,	StateDatabase::ReadWrite,	NULL },
  { "useMeshForRadar",			"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "zbuffer",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "highlightPattern",		"",			true,	StateDatabase::ReadWrite,	NULL },
  { "killerhighlight",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "serverCacheAge",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "slowKeyboard",		"0",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayRadarFlags",	"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayMainFlags",		"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayScore",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "displayFlagHelp",		"1",			true,	StateDatabase::ReadWrite,	setFlagHelp },
  { "displayConsole",		"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayReloadTimer",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "displayRadar",		"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayRadarRange",	"0.5",			false,	StateDatabase::ReadWrite,	NULL },
  { "radarPosition",		"0",			true, StateDatabase::ReadWrite, NULL },
  { "defaultFOV",		"60.0",			false,  StateDatabase::ReadWrite,	NULL },
  { "displayFOV",		"60.0",			false,  StateDatabase::ReadWrite,	NULL },
  { "roamZoomMax",		"120",			false,	StateDatabase::ReadWrite,	NULL },
  { "roamZoomMin",		"15",			false,	StateDatabase::ReadWrite,	NULL },
  { "maxQuality",		"3",			false,	StateDatabase::ReadWrite,	NULL },
  { "groundTexRepeat",		"0.1",			true,	StateDatabase::ReadWrite,	NULL },
  { "groundHighResTexRepeat",	"0.05",			true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallTexRepeat",		"1.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallHighResTexRepeat",	"5.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallTexRepeat",		"3.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallHighResTexRepeat",	"8.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "allowInputChange",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "pulseDepth",		"0.4",			true,	StateDatabase::ReadWrite,	NULL },
  { "pulseRate",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "userRainScale",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "userMirror",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "showTreads",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "animatedTreads",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "shotLength",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "treadStyle",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "userTrackFade",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "trackMarkCulling",		"3",			true,	StateDatabase::ReadWrite,	NULL },
  { "scrollPages",		"20",			true,	StateDatabase::ReadWrite,	NULL },
  { "remoteSounds",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "leadingShotLine",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "saveIdentity",		"2",			true,	StateDatabase::ReadWrite,	NULL },
  { "showCollisionGrid",	"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "showCullingGrid",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "showCoordinates",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "showVelocities",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "jumpTyping",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "maxCacheMB",		"32",			true,	StateDatabase::ReadWrite,	NULL },
  { "doDownloads",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "updateDownloads",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "roamSmoothTime",		"0.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "roamView",			"fps",			true,	StateDatabase::ReadWrite,	NULL },
  { "listIcons",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "maxTextureSize",		"512",			true,	StateDatabase::ReadWrite,	NULL },
  { "lodScale",			"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "radarLodScale",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "stencilShadows",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "shadowAlpha",		"0.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "aniso",			"1",			true,	StateDatabase::ReadWrite,	NULL },

  // roam smooth follow settings
  { "followDist",		"32.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "followHeight",		"8.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "followOffsetZ",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "followSpeedX",		"2.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "followSpeedY",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "followSpeedZ",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },

  // hidden graphics rendering params
  { "useDrawInfo",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "f2bsort",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "meshLists",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "flagLists",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "lightLists",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "noMeshClusters",		"0",			true,	StateDatabase::ReadWrite,	NULL },

  // default texture names
  { "stdGroundTexture",		"std_ground",		true,	StateDatabase::ReadWrite,	NULL },
  { "zoneGroundTexture",	"zone_ground",		true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallTexture",		"boxwall",		true,	StateDatabase::ReadWrite,	NULL },
  { "boxTopTexture",		"roof",			true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallTexture",		"pyrwall",		true,	StateDatabase::ReadWrite,	NULL },
  { "cautionTexture",		"caution",		true,	StateDatabase::ReadWrite,	NULL },
  { "waterTexture",		"water",		true,	StateDatabase::ReadWrite,	NULL },

  // default fonts
  { "consoleFont",		"DejaVuSansMonoBold",	true,	StateDatabase::ReadWrite,	NULL },
  { "sansSerifFont",		"DejaVuSansCondensedBold",	true,	StateDatabase::ReadWrite,	NULL },
  { "serifFont",		"DejaVuSerifBold",	true,	StateDatabase::ReadWrite,	NULL },

  // team based object sufixes
  { "tankTexture",		"tank",			true,	StateDatabase::ReadWrite,	NULL },
  { "boltTexture",		"bolt",			true,	StateDatabase::ReadWrite,	NULL },
  { "laserTexture",		"laser",		true,	StateDatabase::ReadWrite,	NULL },
  { "baseTopTexture",		"basetop",		true,	StateDatabase::ReadWrite,	NULL },
  { "baseWallTexture",		"basewall",		true,	StateDatabase::ReadWrite,	NULL },

  // team prefixes
  { "redTeamPrefix",		"red_",			true,	StateDatabase::ReadWrite,	NULL },
  { "blueTeamPrefix",		"blue_",		true,	StateDatabase::ReadWrite,	NULL },
  { "greenTeamPrefix",		"green_",		true,	StateDatabase::ReadWrite,	NULL },
  { "purpleTeamPrefix",		"purple_",		true,	StateDatabase::ReadWrite,	NULL },
  { "rabbitTeamPrefix",		"rabbit_",		true,	StateDatabase::ReadWrite,	NULL },
  { "hunterTeamPrefix",		"hunter_",		true,	StateDatabase::ReadWrite,	NULL },
  { "rogueTeamPrefix",		"rogue_",		true,	StateDatabase::ReadWrite,	NULL },
  { "observerTeamPrefix",	"observer_",		true,	StateDatabase::ReadWrite,	NULL },

  // type prefixes
  { "superPrefix",		"super_",		true,	StateDatabase::ReadWrite,	NULL },

  // effects options
  { "useFancyEffects",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "spawnEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "enableLocalSpawnEffect",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "shotEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "enableLocalShotEffect",	"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "deathEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "useVelOnShotEffects",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "landEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "gmPuffEffect",		"3",			true,	StateDatabase::ReadWrite,	NULL },
  { "gmPuffTime",		"1/8",			true,	StateDatabase::ReadWrite,	NULL },
  { "ricoEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "tpEffect",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "fogEffect",		"1",			true,	StateDatabase::ReadWrite,	NULL },

  // URL timeouts
  { "httpTimeout",		"15",			true,	StateDatabase::ReadWrite,	NULL },

  // hud drawing
  { "hudGUIBorderOpacityFactor",	"0.75",		true,  StateDatabase::ReadWrite, NULL },
  { "hudWayPMarkerSize",		"15",		true,  StateDatabase::ReadWrite, NULL },


  // hud drawing
  { "hudGUIBorderOpacityFactor","0.75",			true,	StateDatabase::ReadWrite,	NULL },
  { "hideMottos",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "mottoDispLen",		"32",			true,	StateDatabase::ReadWrite,	NULL },
  { "scoreboardSort",	"0",			true,	StateDatabase::ReadWrite,	NULL },

  // listFilters
  { "listFilter",  "",					true, StateDatabase::ReadWrite, NULL },
  { "listFilter0", "",					true, StateDatabase::ReadWrite, NULL },
  { "listFilter1", "/p>1, # busy",			    true, StateDatabase::ReadWrite, NULL },
  { "listFilter2", "/p>1,s>1,s<4, # busy - 2 or 3 shots",     true, StateDatabase::ReadWrite, NULL },
  { "listFilter3", "/p>1,+rabbit, # busy rabbit",	     true, StateDatabase::ReadWrite, NULL },
  { "listFilter4", "/-j,+r,-f,s=2,+t,vt=2, # ducati",	 true, StateDatabase::ReadWrite, NULL },
  { "listFilter5", "/vt=3,+t,Rm<1, # three-way",	      true, StateDatabase::ReadWrite, NULL },
  { "listFilter6", "/d)replay, # replay servers",	     true, StateDatabase::ReadWrite, NULL },
  { "listFilter7", "/a]\\.bz(:[0-9]+)?$, # .bz servers",      true, StateDatabase::ReadWrite, NULL },
  { "listFilter8", "/d)louman/d)ahs3/d)spazzy, # fancy maps", true, StateDatabase::ReadWrite, NULL },
  { "listFilter9", "/d]louman|ahs3|spazzy, # fancy maps2",    true, StateDatabase::ReadWrite, NULL },

  // We don't want to keep the geometry settings
  { "geometry",		"",			false,	StateDatabase::ReadWrite, NULL }
};


void loadBZDBDefaults ( void )
{
  for (int i = 0; i < (int)countof(defaultDBItems); ++i) {
    assert(defaultDBItems[i].name != NULL);

    if (defaultDBItems[i].value != NULL) {
      BZDB.set(defaultDBItems[i].name, defaultDBItems[i].value);
      BZDB.setDefault(defaultDBItems[i].name, defaultDBItems[i].value);
    }

    BZDB.setPersistent(defaultDBItems[i].name, defaultDBItems[i].persistent);
    BZDB.setPermission(defaultDBItems[i].name, defaultDBItems[i].permission);
    BZDB.addCallback(defaultDBItems[i].name, defaultDBItems[i].callback, NULL);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

