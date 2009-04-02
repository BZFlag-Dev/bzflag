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


// BZFlag common header
#include "common.h"

#include <assert.h>

#include "defaultBZDB.h"
#include "Protocol.h"
#include "callbacks.h"


// default client permission level
const StateDatabase::Permission DefPerm = StateDatabase::ReadWrite;


DefaultDBItem	defaultDBItems[] = {
//  name,                       value,              persist, permission, callback
  { "animatedTreads",		"1",			true,  DefPerm, NULL },
  { "aniso",			"1",			true,  DefPerm, NULL },
  { "autoFlagDisplay",		"0",			true,  DefPerm, NULL },
  { "autohunt",			"",			true,  DefPerm, NULL },
  { "autoJoin",			"1",			true,  DefPerm, NULL },
  { "boxWallHighResTexRepeat",	"5.0",			true,  DefPerm, NULL },
  { "boxWallTexRepeat",		"1.5",			true,  DefPerm, NULL },
  { "coloredradarshots",	"1",			true,  DefPerm, NULL },
  { "colorful",			"1",			true,  DefPerm, NULL },
  { "debugLevel",		"0",			false, DefPerm, Callbacks::setDebugLevel },
  { "defaultFOV",		"60.0",			false, DefPerm, NULL },
  { "displayConsole",		"1",			false, DefPerm, NULL },
  { "displayFlagHelp",		"1",			true,  DefPerm, Callbacks::setFlagHelp },
  { "displayFOV",		"60.0",			false, DefPerm, NULL },
  { "displayMainFlags",		"1",			false, DefPerm, NULL },
  { "displayRadar",		"1",			false, DefPerm, NULL },
  { "displayRadarFlags",	"1",			false, DefPerm, NULL },
  { "displayRadarRange",	"0.5",			false, DefPerm, NULL },
  { "displayReloadTimer",	"1",			true,  DefPerm, NULL },
  { "displayScore",		"1",			true,  DefPerm, NULL },
  { "doDownloads",		"1",			true,  DefPerm, NULL },
  { "fpsLimit",			"50",			true,  DefPerm, NULL },
  { "groundHighResTexRepeat",	"0.05",			true,  DefPerm, NULL },
  { "groundTexRepeat",		"0.1",			true,  DefPerm, NULL },
  { "highlightPattern",		"",			true,  DefPerm, NULL },
  { "killerhighlight",		"1",			true,  DefPerm, NULL },
  { "latitude",			"37.5",			true,  DefPerm, NULL },
  { "leadingShotLine",		"0",			true,  DefPerm, NULL },
  { "linedradarshots",		"0",			true,  DefPerm, NULL },
  { "list",			DefaultListServerURL,	true,  DefPerm, NULL },
  { "listIcons",		"1",			true,  DefPerm, NULL },
  { "lodScale",			"1.0",			true,  DefPerm, NULL },
  { "longitude",		"122",			true,  DefPerm, NULL },
  { "luaBzOrg",			"1",			true,  DefPerm, NULL },
  { "luaUser",			"1",			true,  DefPerm, NULL },
  { "luaWorld",			"1",			true,  DefPerm, NULL },
  { "maxCacheMB",		"96",			true,  DefPerm, NULL },
  { "maxFlagLOD",		"8",			true,  DefPerm, NULL },
  { "maxFontSize",		"128",			true,  DefPerm, NULL },
  { "maxQuality",		"3",			false, DefPerm, NULL },
  { "motdServer",		DefaultMOTDServer,	true,  DefPerm, NULL },
  { "mouseboxsize",		"5",			true,  DefPerm, NULL },
  { "panelopacity",		"0.3",			true,  DefPerm, NULL },
  { "pauseOnMinimize",		"1",			true,  DefPerm, NULL },
  { "processorAffinity",	"0",			true,  DefPerm, Callbacks::setProcessorAffinity },
  { "pulseDepth",		"0.4",			true,  DefPerm, NULL },
  { "pulseRate",		"1.0",			true,  DefPerm, NULL },
  { "pyrWallHighResTexRepeat",	"8.0",			true,  DefPerm, NULL },
  { "pyrWallTexRepeat",		"3.0",			true,  DefPerm, NULL },
  { "radarLodScale",		"1.0",			true,  DefPerm, NULL },
  { "radarsize",		"4",			true,  DefPerm, NULL },
  { "radarStyle",		"3",			true,  DefPerm, NULL },
  { "radarTankPixels",		"2.0",			true,  DefPerm, NULL },
  { "remoteSounds",		"1",			true,  DefPerm, NULL },
  { "roamSmoothTime",		"0.5",			true,  DefPerm, NULL },
  { "roamZoomMax",		"120",			false, DefPerm, NULL },
  { "roamZoomMin",		"15",			false, DefPerm, NULL },
  { "saveEnergy",		"0",			true,  DefPerm, NULL },
  { "saveIdentity",		"1",			true,  DefPerm, NULL },
  { "saveSettings",		"1",			true,  DefPerm, NULL },
  { "scrollPages",		"20",			true,  DefPerm, NULL },
  { "serverCacheAge",		"0",			true,  DefPerm, NULL },
  { "shadowAlpha",		"0.5",			true,  DefPerm, NULL },
  { "showCollisionGrid",	"0",			true,  DefPerm, NULL },
  { "showCoordinates",		"0",			true,  DefPerm, NULL },
  { "showCullingGrid",		"0",			true,  DefPerm, NULL },
  { "showShotGuide",		"0",			true,  DefPerm, NULL },
  { "showtabs",			"2",			true,  DefPerm, NULL },
  { "showTreads",		"0",			true,  DefPerm, NULL },
  { "showVelocities",		"0",			true,  DefPerm, NULL },
  { "sizedradarshots",		"0",			true,  DefPerm, NULL },
  { "stencilShadows",		"1",			true,  DefPerm, NULL },
  { "team",			"Rogue",		true,  DefPerm, NULL },
  { "timedate",			"0",			true,  DefPerm, NULL },
  { "tkwarnratio",		"0.0",			true,  DefPerm, NULL },
  { "trackMarkCulling",		"3",			true,  DefPerm, NULL },
  { "treadStyle",		"0",			true,  DefPerm, NULL },
  { "udpnet",			"1",			true,  DefPerm, NULL },
  { "underlineColor",		"Cyan",			true,  DefPerm, NULL },
  { "updateDownloads",		"0",			true,  DefPerm, NULL },
  { "userMirror",		"1",			true,  DefPerm, NULL },
  { "userRainScale",		"1.0",			true,  DefPerm, NULL },
  { "userTrackFade",		"1.0",			true,  DefPerm, NULL },
  { "volume",			"10",			true,  DefPerm, NULL },
  { "vsync",			"-1",			true,  DefPerm, NULL },
  { "zbuffer",			"1",			true,  DefPerm, NULL },

  //ping
  { "pingLow",		        "100",			true,  DefPerm, NULL },
  { "pingMed",		        "200",			true,  DefPerm, NULL },
  { "pingHigh",                 "400",			true,  DefPerm, NULL },

  // input
  { "allowInputChange",		"1",			true,  DefPerm, NULL },
  { "jumpTyping",		"1",			true,  DefPerm, NULL },
  { "parabolicSlope",		"1",			true,  DefPerm, NULL },
  { "slowBinoculars",		"1",			true,  DefPerm, NULL },
  { "slowMotion",		"0",			false, DefPerm, NULL },

  // hidden graphics rendering params
  { "f2bsort",			"1",			true,  DefPerm, NULL },
  { "flagLists",		"0",			true,  DefPerm, NULL },
  { "lightLists",		"0",			true,  DefPerm, NULL },
  { "meshLists",		"1",			true,  DefPerm, NULL },
  { "multisamples",		"0",			true,  DefPerm, NULL },
  { "noMeshClusters",		"0",			true,  DefPerm, NULL },
  { "useDrawInfo",		"1",			true,  DefPerm, NULL },

  // default texture names
  { "stdGroundTexture",		"std_ground",		true,  DefPerm, NULL },
  { "zoneGroundTexture",	"zone_ground",		true,  DefPerm, NULL },
  { "boxWallTexture",		"boxwall",		true,  DefPerm, NULL },
  { "boxTopTexture",		"roof",			true,  DefPerm, NULL },
  { "pyrWallTexture",		"pyrwall",		true,  DefPerm, NULL },
  { "cautionTexture",		"caution",		true,  DefPerm, NULL },
  { "waterTexture",		"water",		true,  DefPerm, NULL },

  // default fonts
  { "consoleFont",		"VeraMonoBold",		true,  DefPerm, NULL },
  { "sansSerifFont",		"TogaSansBold",		true,  DefPerm, NULL },
  { "serifFont",		"TogaSerifBold",	true,  DefPerm, NULL },

  // default font sizes for fixed-size items
  { "tinyFontSize",		"8",			true,  DefPerm, NULL },
  { "smallFontSize",		"16",			true,  DefPerm, NULL },
  { "mediumFontSize",		"24",			true,  DefPerm, NULL },
  { "largeFontSize",		"32",			true,  DefPerm, NULL },
  { "hugeFontSize",		"40",			true,  DefPerm, NULL },

  // default font sizes for specific sections (relative 0 to 1 percentage of smallest aspect)
  { "titleFontSize",		"1/8",			true,  DefPerm, NULL },
  { "headerFontSize",		"1/24",			true,  DefPerm, NULL },
  { "menuFontSize",		"1/40",			true,  DefPerm, NULL },
  { "hudFontSize",		"1/64",			true,  DefPerm, NULL },
  { "alertFontSize",		"1/64",			true,  DefPerm, NULL },
  { "infoFontSize",		"1/96",			true,  DefPerm, NULL },
  { "consoleFontSize",		"1/128",		true,  DefPerm, NULL },
  { "scoreFontSize",		"1/128",		true,  DefPerm, NULL },

  // team based object sufixes
  { "tankTexture",		"tank",			true,  DefPerm, NULL },
  { "boltTexture",		"bolt",			true,  DefPerm, NULL },
  { "laserTexture",		"laser",		true,  DefPerm, NULL },
  { "baseTopTexture",		"basetop",		true,  DefPerm, NULL },
  { "baseWallTexture",		"basewall",		true,  DefPerm, NULL },

  // team prefixes
  { "redTeamPrefix",		"skins/red/",		true,  DefPerm, NULL },
  { "blueTeamPrefix",		"skins/blue/",		true,  DefPerm, NULL },
  { "greenTeamPrefix",		"skins/green/",		true,  DefPerm, NULL },
  { "purpleTeamPrefix",		"skins/purple/",	true,  DefPerm, NULL },
  { "rabbitTeamPrefix",		"skins/rabbit/",	true,  DefPerm, NULL },
  { "hunterTeamPrefix",		"skins/hunter/",	true,  DefPerm, NULL },
  { "rogueTeamPrefix",		"skins/rogue/",		true,  DefPerm, NULL },
  { "observerTeamPrefix",	"skins/observer/",	true,  DefPerm, NULL },

  // type prefixes
  { "superPrefix",		"super_",		true,  DefPerm, NULL },

  // effects options
  { "deathEffect",		"1",			true,  DefPerm, NULL },
  { "enableLocalShotEffect",	"0",			true,  DefPerm, NULL },
  { "enableLocalSpawnEffect",	"1",			true,  DefPerm, NULL },
  { "fogEffect",		"1",			true,  DefPerm, NULL },
  { "gmPuffEffect",		"2",			true,  DefPerm, NULL },
  { "gmPuffTime",		"1/8",			true,  DefPerm, NULL },
  { "landEffect",		"1",			true,  DefPerm, NULL },
  { "ricoEffect",		"1",			true,  DefPerm, NULL },
  { "shotEffect",		"1",			true,  DefPerm, NULL },
  { "shotLength",		"0",			true,  DefPerm, NULL },
  { "spawnEffect",		"1",			true,  DefPerm, NULL },
  { "tpEffect",			"1",			true,  DefPerm, NULL },
  { "useFancyEffects",		"1",			true,  DefPerm, NULL },
  { "useVelOnShotEffects",	"1",			true,  DefPerm, NULL },

  // URL timeouts
  { "httpTimeout",		"15",			true,  DefPerm, NULL },

  // hud drawing
  { "hudGUIBorderOpacityFactor","0.75",			true,  DefPerm, NULL },
  { "hudWayPMarkerSize",	"15",			true,  DefPerm, NULL },

  // 3rdPerson Camera
  { "3rdPersonCam",			"0",		true,  DefPerm, NULL },
  { "3rdPersonCamXYOffset",		"10.0",		true,  DefPerm, NULL },
  { "3rdPersonCamZOffset",		"2.5",		true,  DefPerm, NULL },
  { "3rdPersonCamTargetMult",		"50.0",		true,  DefPerm, NULL },
  { "3rdPersonNearTargetSize",		"1.0",		true,  DefPerm, NULL },
  { "3rdPersonNearTargetDistance",	"40.0",		true,  DefPerm, NULL },
  { "3rdPersonFarTargetSize",		"1.5",		true,  DefPerm, NULL },
  { "3rdPersonFarTargetDistance",	"180.0",	true,  DefPerm, NULL },

  // models
  { "playerModel",			"tank",		true,  DefPerm, NULL },


#ifdef USE_XFIRE
  /* Xfire support
   * 0: Disable: prevent Xfire from detecting that BZFlag is running
   * 1: Private: do NOT send player-specific information (callsign, team, score...)
   * 2: Full: send all available information (to friends only)
   */
  { "xfireCommunicationLevel",		"2",		true,  DefPerm, NULL },
#endif
  // seconds between pulses to textOutput (0 is disabled)
  { "statsOutputFrequency",		"0",		true,  DefPerm, NULL },
  { "statsOutputFilename",		"",		true,  DefPerm, NULL },

  // debugging (none should be persistent)
  { "debugNewAngVel",			"0",		false, DefPerm, NULL }

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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
