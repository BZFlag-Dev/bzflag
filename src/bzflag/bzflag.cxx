/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

#elif !defined(macintosh)
#include <pwd.h>
#include <dirent.h>
#endif /* defined(_WIN32) */
#include <stdarg.h>
#include "bzfSDL.h"
#include "bzfio.h"
#include <fstream>
#include "bzfgl.h"
#include "ErrorHandler.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "Address.h"
#include "Protocol.h"
#include "playing.h"
#include "TimeBomb.h"
#include "Team.h"
#include "sound.h"
#include "ConfigFileManager.h"
#include "DirectoryNames.h"
#include "CommandsStandard.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfMedia.h"
#include "PlatformFactory.h"
#include "BundleMgr.h"
#include "World.h"
#include "StateDatabase.h"
#include "FileManager.h"
#include "CommandManager.h"
#include "KeyManager.h"
#include "callbacks.h"
#include "ServerListCache.h"
#include "BZDBCache.h"
#include "WordFilter.h"
#include "TextUtils.h"
#include "ActionBinding.h"
#include "ServerStartMenu.h"
#include "FontManager.h"
#include "OSFile.h"
#include "TextureManager.h"
// invoke incessant rebuilding for build versioning
#include "version.h"

int beginendCount = 0;

extern std::vector<std::string>& getSilenceList();
const char*		argv0;
static bool		anonymous = false;
static std::string	anonymousName("anonymous");
static std::string	alternateConfig;
static bool		noAudio = false;
struct tm		userTime;
static StartupInfo	startupInfo;
bool			echoToConsole = false;
bool			echoAnsi = false;
int			debugLevel = 0;

static BzfDisplay*	display = NULL;

// default database entries
struct DefaultDBItem {
  const char*			name;
  const char*			value;
  bool				persistent;
  StateDatabase::Permission	permission;
  StateDatabase::Callback	callback;
};
static DefaultDBItem	defaultDBItems[] = {
  { "udpnet",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "email",			"default",		true,	StateDatabase::ReadWrite,	NULL },
  { "team",			"Rogue",		true,	StateDatabase::ReadWrite,	NULL },
  { "list",			DefaultListServerURL,	true,	StateDatabase::ReadWrite,	NULL },
  { "volume",			"10",			true,	StateDatabase::ReadWrite,	NULL },
  { "latitude",			"37.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "longitude",		"122",			true,	StateDatabase::ReadWrite,	NULL },
  { "enhancedradar",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "coloredradarshots",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "linedradarshots",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "sizedradarshots",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "panelopacity",		"0.3",			true,	StateDatabase::ReadWrite,	NULL },
  { "radarsize",		"4",			true,	StateDatabase::ReadWrite,	NULL },
  { "mouseboxsize",		"5",			true,	StateDatabase::ReadWrite,	NULL },
  { "cpanelfontsize",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "scorefontsize",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "colorful",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "showtabs",			"2",			true,   StateDatabase::ReadWrite,	NULL },
  { "underlineColor",		"cyan",			true,	StateDatabase::ReadWrite,	NULL },
  { "zbuffer",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "killerhighlight",		"2",			true,	StateDatabase::ReadWrite,	NULL },
  { "serverCacheAge",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "slowKeyboard",		"0",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayRadarFlags",	"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayMainFlags",		"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayScore",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "displayZoom",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "displayFlagHelp",		"1",			true,	StateDatabase::ReadWrite,	setFlagHelp },
  { "displayConsoleAndRadar",	"1",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayRadarRange",	"0.5",			false,	StateDatabase::ReadWrite,	NULL },
  { "displayFOV",	        "60.0",			false,  StateDatabase::ReadWrite,	NULL },
  { "roamZoomMax",		"120",			false,	StateDatabase::ReadWrite,	NULL },
  { "roamZoomMin",		"15",			false,	StateDatabase::ReadWrite,	NULL },
  { "maxQuality",		"3",			false,	StateDatabase::ReadWrite,	NULL },
  { "altImageDir",		"alternate",		true,	StateDatabase::ReadWrite,	NULL },
  { "groundTexRepeat",		"0.1",                  true,	StateDatabase::ReadWrite,	NULL },
  { "groundHighResTexRepeat",	"0.05",                 true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallTexRepeat",		"1.5",                  true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallHighResTexRepeat",	"5.0",                  true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallTexRepeat",		"3.0",                  true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallHighResTexRepeat",	"8.0",                  true,	StateDatabase::ReadWrite,	NULL },
  { "tetraWallTexRepeat",	"3.0",                  true,	StateDatabase::ReadWrite,	NULL },
  { "tetraWallHighResTexRepeat","8.0",                  true,	StateDatabase::ReadWrite,	NULL },
  { "allowInputChange",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "pulseDepth",		"0.4",			true,	StateDatabase::ReadWrite,	NULL },
  { "pulseRate",		"1.0",			true,	StateDatabase::ReadWrite,	NULL },
  { "showTreads",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "scrollPages",		"20",			true,	StateDatabase::ReadWrite,	NULL },

  // default texture names
  { "stdGroundTexture",		"std_ground",		true,	StateDatabase::ReadWrite,	NULL },
  { "zoneGroundTexture",	"zone_ground",          true,	StateDatabase::ReadWrite,	NULL },
  { "boxWallTexture",	        "boxwall",              true,	StateDatabase::ReadWrite,	NULL },
  { "boxTopTexture",	        "roof",                 true,	StateDatabase::ReadWrite,	NULL },
  { "pyrWallTexture",	        "pyrwall",              true,	StateDatabase::ReadWrite,	NULL },
  { "tetraWallTexture",	        "tetrawall",            true,	StateDatabase::ReadWrite,	NULL },
  { "cautionTexture",	        "caution",              true,	StateDatabase::ReadWrite,	NULL },
  { "waterTexture",	        "water",                true,	StateDatabase::ReadWrite,	NULL },

  // default fonts
  { "consoleFont",		"VeraMonoBold",		true,	StateDatabase::ReadWrite,	NULL },
  { "sansSerifFont",		"TogaSansBold",		true,	StateDatabase::ReadWrite,	NULL },
  { "serifFont",		"TogaSerifBold",	true,	StateDatabase::ReadWrite,	NULL },

  // team based object sufixes
  { "tankTexture",	        "tank",                 true,	StateDatabase::ReadWrite,	NULL },
  { "boltTexture",	        "bolt",                 true,	StateDatabase::ReadWrite,	NULL },
  { "laserTexture",	        "laser",                true,	StateDatabase::ReadWrite,	NULL },
  { "baseTopTexture",	        "basetop",              true,	StateDatabase::ReadWrite,	NULL },
  { "baseWallTexture",	        "basewall",             true,	StateDatabase::ReadWrite,	NULL },

  // team prefixes
  { "redTeamPrefix",	        "red_",                 true,	StateDatabase::ReadWrite,	NULL },
  { "blueTeamPrefix",	        "blue_",                true,	StateDatabase::ReadWrite,	NULL },
  { "greenTeamPrefix",	        "green_",               true,	StateDatabase::ReadWrite,	NULL },
  { "purpleTeamPrefix",	        "purple_",              true,	StateDatabase::ReadWrite,	NULL },
  { "rabbitTeamPrefix",	        "rabbit_",              true,	StateDatabase::ReadWrite,	NULL },
  { "hunterTeamPrefix",	        "hunter_",              true,	StateDatabase::ReadWrite,	NULL },
  { "rogueTeamPrefix",	        "rogue_",               true,	StateDatabase::ReadWrite,	NULL },

  // type prefixes
  { "superPrefix",	        "super_",               true,	StateDatabase::ReadWrite,	NULL }

};

#ifdef ROBOT
// ROBOT -- tidy up
int numRobotTanks = 0;
#endif


//
// application initialization
//

static const char*	configQualityValues[] = {
				"low",
				"medium",
				"high",
				"experimental"
			};
static const char*	configViewValues[] = {
				"normal",
				"stereo",
				"stacked",
				"three",
				"anaglyph"
			};

static std::string	getOldConfigFileName()
{
#if !defined(_WIN32) & !defined(macintosh)

  std::string name = getConfigDirName();
  name += "config";

  // add in hostname on UNIX
  if (getenv("HOST")) {
    name += ".";
    name += getenv("HOST");
  }

  return name;

#elif defined(_WIN32) /* !defined(_WIN32) */

  // get location of personal files from system.  this appears to be
  // the closest thing to a home directory on windows.  use root of
  // C drive as a default in case we can't get the path or it doesn't
  // exist.
  std::string oldConfigName = "bzflag19.bzc";
  std::string name = getConfigDirName();
  name += oldConfigName;
  return name;

#elif defined(macintosh)

  return oldConfigName; // FIXME - use getConfigDirName() ?

#endif /* !defined(_WIN32) & !defined(macintosh) */
}

#if !defined(_WIN32) & !defined(macintosh)		// who uses this sucker any more?
static std::string	getReallyOldConfigFileName()
{
  std::string name = getConfigDirName();
  name += "config";
  return name;
}
#endif

std::string getCurrentConfigFileName(void)
{
  std::string configFile = BZ_CONFIG_FILE_NAME;

  std::string name = getConfigDirName(BZ_CONFIG_DIR_VERSION);
  name += configFile;

#if !defined(_WIN32) & !defined(macintosh)
  // add in hostname on UNIX
  if (getenv("HOST")) {
    name += ".";
    name += getenv("HOST");
  }
#endif
  return name;
}

// this function will look for the config, if it's not there,
// it will TRY and find an old one and copy it
// so that the update function can upgrade it to the current version
// the assumption is that there is a unique config per version
void findConfigFile(void)
{
  // look for the current file
  std::string configName = getCurrentConfigFileName();
  FILE *fp = fopen(configName.c_str(), "rb");
  if (fp) {
    // we found the current file, nothing to do, just return
    fclose(fp);
    return;
  }

  // try and find the old file
  std::string oldConfigName = getOldConfigFileName();
  fp = fopen(oldConfigName.c_str(), "rb");
  if (fp) {
    // there is an old config so lets copy it to the new dir and let the update take care of it.
#if defined(_WIN32)
    fclose(fp);
    // make the dir if we need to
    mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str());
    // copy the old config to the new dir location with the new name
    CopyFile(oldConfigName.c_str(), configName.c_str(),true);

//#elif defined(macintosh) // for the 'tosh
#else	// the other OSs should do what they need to do
    mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str(), 0755);

    FILE *newFile = fopen(configName.c_str(),"wb");
    if (newFile) {
      fseek(fp, 0, SEEK_END);
      int len = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      unsigned char* temp = (unsigned char*) malloc(len);

      fread(temp, len, 1, fp);
      fwrite(temp, len, 1, newFile);

      free(temp);

      fclose(newFile);
      fclose(fp);
    }
#endif
  }

  // try and find the REALLY old file
  // who uses this sucker any more?
#if !defined(_WIN32) & !defined(macintosh)
  std::string realyOldConfigName = getReallyOldConfigFileName();
  fp = fopen(realyOldConfigName.c_str(), "rb");
  if (fp) {
    // there is an old config so lets copy it to the new dir and let the update take care of it.
    // apparently only linux needs this so do the magic
    mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str(), 0755);

    FILE *newFile = fopen(configName.c_str(),"wb");
    if (newFile) {
      fseek(fp, 0, SEEK_END);
      int len = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      unsigned char* temp = (unsigned char*) malloc(len);

      fread(temp, len, 1, fp);
      fwrite(temp, len, 1, newFile);

      free(temp);
      fclose(newFile);
      fclose(fp);
    }
  }
#endif
}

void updateConfigFile(void)
{
  int		configVersion = 0;
  if (BZDB.isSet("config_version"))
    configVersion = (int)BZDB.eval("config_version");

  switch (configVersion) {
  case 0: // 1.10-1.12
    // update from old unversioned config
    // roaming fixes - remove keys bound to "roam translate *" and "roam rotate *"
    KEYMGR.unbindCommand("roam translate left");
    KEYMGR.unbindCommand("roam translate right");
    KEYMGR.unbindCommand("roam translate up");
    KEYMGR.unbindCommand("roam translate down");
    KEYMGR.unbindCommand("roam translate forward");
    KEYMGR.unbindCommand("roam translate backward");
    KEYMGR.unbindCommand("roam rotate left");
    KEYMGR.unbindCommand("roam rotate right");
    KEYMGR.unbindCommand("roam rotate up");
    KEYMGR.unbindCommand("roam rotate down");

    // add new default keybindings if there's no conflict
    // iconify
    BzfKeyEvent key;
    if (KEYMGR.stringToKeyEvent("F4", key)
        && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "iconify");
    // toggle console & radar
    if (KEYMGR.stringToKeyEvent("F3", key)
        && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "toggleConsoleAndRadar");
    // controlpanel tabs - all or nothing
    if (KEYMGR.stringToKeyEvent("Shift+F1", key)
        && (KEYMGR.get(key, true) == "")
	&& KEYMGR.stringToKeyEvent("Shift+F2", key)
        && (KEYMGR.get(key, true) == "")
	&& KEYMGR.stringToKeyEvent("Shift+F3", key)
        && (KEYMGR.get(key, true) == "")
	&& KEYMGR.stringToKeyEvent("Shift+F4", key)
        && (KEYMGR.get(key, true) == "")) {
      KEYMGR.stringToKeyEvent("Shift+F1", key);
      KEYMGR.bind(key, true, "messagepanel all");
      KEYMGR.stringToKeyEvent("Shift+F2", key);
      KEYMGR.bind(key, true, "messagepanel chat");
      KEYMGR.stringToKeyEvent("Shift+F3", key);
      KEYMGR.bind(key, true, "messagepanel server");
      KEYMGR.stringToKeyEvent("Shift+F4", key);
      KEYMGR.bind(key, true, "messagepanel misc");
    }

    // TODO - any other breaking changes from 1.10 to 1.12

  case 1: // 1.12
    break; // no action, current version

  default: // hm, we don't know about this one...
    printError(TextUtils::format("Config file is tagged version \"%d\", "
                                   "which was not expected (too new perhaps). "
				   "Trying to load anyhow.", configVersion));
    break;
  }

  // set us as the updated version
  configVersion = BZ_CONFIG_FILE_VERSION;
  BZDB.setInt("config_version", configVersion);
}

static void		setTeamColor(TeamColor team, const std::string& value)
{
  float color[3];
  if (sscanf(value.c_str(), "%f %f %f", color+0, color+1, color+2) != 3)
    return;
  if (color[0] < 0.0f) color[0] = 0.0f;
  else if (color[0] > 1.0f) color[0] = 1.0f;
  if (color[1] < 0.0f) color[1] = 0.0f;
  else if (color[1] > 1.0f) color[1] = 1.0f;
  if (color[2] < 0.0f) color[2] = 0.0f;
  else if (color[2] > 1.0f) color[2] = 1.0f;
  Team::setColors(team, color, Team::getRadarColor(team));
}

static void		setRadarColor(TeamColor team, const std::string& value)
{
  float color[3];
  if (sscanf(value.c_str(), "%f %f %f", color+0, color+1, color+2) != 3)
    return;
  if (color[0] < 0.0f) color[0] = 0.0f;
  else if (color[0] > 1.0f) color[0] = 1.0f;
  if (color[1] < 0.0f) color[1] = 0.0f;
  else if (color[1] > 1.0f) color[1] = 1.0f;
  if (color[2] < 0.0f) color[2] = 0.0f;
  else if (color[2] > 1.0f) color[2] = 1.0f;
  Team::setColors(team, Team::getTankColor(team), color);
}

static void		setVisual(BzfVisual* visual)
{
  // sine qua non
  visual->setLevel(0);
  visual->setDoubleBuffer(true);
  visual->setRGBA(1, 1, 1, 0);

  // ask for a zbuffer if not disabled.  we might choose not to use it
  // if we do ask for it.
  if (!BZDB.isSet("zbuffer") || BZDB.get("zbuffer") != "disable")
    visual->setDepth(16);

  // optional
#if defined(DEBUG_RENDERING)
  visual->setStencil(4);
#endif
  if (BZDB.isTrue("multisample"))
    visual->setMultisample(4);
#ifdef USE_GL_STEREO
  if (BZDB.isSet("view") && BZDB.get("view") == configViewValues[1])
    visual->setStereo(true);
#endif
}

static void		usage()
{
  printFatalError("usage: %s"
	" [-3dfx]"
	" [-no3dfx]"
	" [-anonymous]"
	" [-badwords <filterfile>]"
	" [-config <configfile>]"
	" [-directory <data-directory>]"
	" [-echo]"
	" [-echoAnsi]"
	" [-geometry <geometry-spec>]"
	" [-latitude <latitude>] [-longitude <longitude>]"
	" [-list <server-list-url>] [-nolist]"
	" [-locale <locale>]"
	" [-multisample]"
	" [-mute]"
#ifdef ROBOT
	" [-solo <num-robots>]"
#endif
	" [-team {red|green|blue|purple|rogue|observer}]"
	" [-time hh:mm:ss] [-notime]"
	" [-version]"
	" [-view {normal|stereo|stacked|three|anaglyph}]"
	" [-window]"
	" [-zoom <zoom-factor>]"
	" [callsign@]server[:port]\n\nExiting.", argv0);
  if (display != NULL) {
    delete display;
    display=NULL;
  }
  exit(1);
}

static void checkArgc(int& i, int argc, const char* option, const char *type = "Missing")
{
  if ((i+1) == argc) {
    printFatalError("%s argument for %s\n", type, option);
    usage();
  }
  i++; // just skip the option argument string
}

static void		parse(int argc, char** argv)
{
// = 9;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0 ||
		strcmp(argv[i], "-anonymous") == 0) {
      anonymous = true;
    } else if (strcmp(argv[i], "-config") == 0) {
      checkArgc(i, argc, argv[i]);
      // the setting has already been done in parseConfigName()
    } else if (strcmp(argv[i], "-debug") == 0) {
      debugLevel++;
    } else if (strcmp(argv[i], "-d") == 0 ||
		strcmp(argv[i], "-directory") == 0) {
      checkArgc(i, argc, argv[i]);
      if (strlen(argv[i]) == 0)
	BZDB.unset("directory");
	  else
		BZDB.set("directory", argv[i]);
    } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "-echo") == 0) {
      echoToConsole = true;
    } else if (strcmp(argv[i], "-ea") == 0 || strcmp(argv[i], "-echoAnsi") == 0) {
      echoToConsole = true;
      echoAnsi = true;
    } else if (strcmp(argv[i], "-h") == 0 ||
	     strcmp(argv[i], "-help") == 0 ||
	     strcmp(argv[i], "--help") == 0) {
      usage();
    } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "-geometry") == 0) {
      checkArgc(i, argc, argv[i]);
      int w, h, x, y, count;
      char xs = '+', ys = '+';
      if (strcmp(argv[i], "default") != 0 &&
	  (((count = sscanf(argv[i], "%dx%d%c%d%c%d",
		&w, &h, &xs, &x, &ys, &y)) != 6 && count != 2) ||
	  (xs != '-' && xs != '+') || (ys != '-' && ys != '+'))) {
	printFatalError("Invalid argument for %s.\n"
			"Correct format is <width>x<height>[+|-]<x>[+|-]<y>.",
			argv[i-1]);
	usage();
      }
      BZDB.set("geometry", argv[i]);
    } else if (strcmp(argv[i], "-latitude") == 0) {
      checkArgc(i, argc, argv[i]);
      double latitude = atof(argv[i]);
      if (latitude < -90.0 || latitude > 90.0) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      BZDB.set("latitude", argv[i]);
    } else if (strcmp(argv[i], "-longitude") == 0) {
      checkArgc(i, argc, argv[i]);
      double longitude = atof(argv[i]);
      if (longitude < -180.0 || longitude > 180.0) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      BZDB.set("longitude", argv[i]);
    } else if (strcmp(argv[i], "-list") == 0) {
      checkArgc(i, argc, argv[i]);
      if (strcmp(argv[i], "default") == 0) {
	BZDB.unset("list");
      } else {
	startupInfo.listServerURL = argv[i];
	BZDB.set("list", argv[i]);
      }
    } else if (strcmp(argv[i], "-locale") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("locale", argv[i]);
    } else if (strcmp(argv[i], "-motd") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("motd", argv[i]);
    } else if (strcmp(argv[i], "-nolist") == 0) {
      startupInfo.listServerURL = "";
      BZDB.set("list", "");
    } else if (strcmp(argv[i], "-m") == 0 ||
		strcmp(argv[i], "-mute") == 0) {
      noAudio = true;
    } else if (strcmp(argv[i], "-multisample") == 0) {
      BZDB.set("_multisample", "1");
#ifdef ROBOT
    } else if (strcmp(argv[i], "-solo") == 0) {
      checkArgc(i, argc, argv[i]);
      numRobotTanks = atoi(argv[i]);
      if (numRobotTanks < 1 || numRobotTanks > MAX_ROBOTS) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
#endif
    } else if (strcmp(argv[i], "-team") == 0) {
      checkArgc(i, argc, argv[i]);
      if ((strcmp(argv[i], "a") == 0) ||
	  (strcmp(argv[i], "auto") == 0) ||
	  (strcmp(argv[i], "automatic") == 0)) {
	startupInfo.team = AutomaticTeam;
      }	else if (strcmp(argv[i], "r") == 0 || strcmp(argv[i], "red") == 0) {
	startupInfo.team = RedTeam;
      } else if (strcmp(argv[i], "g") == 0 || strcmp(argv[i], "green") == 0) {
	startupInfo.team = GreenTeam;
      } else if (strcmp(argv[i], "b") == 0 || strcmp(argv[i], "blue") == 0) {
	startupInfo.team = BlueTeam;
      } else if (strcmp(argv[i], "p") == 0 || strcmp(argv[i], "purple") == 0) {
	startupInfo.team = PurpleTeam;
      } else if (strcmp(argv[i], "z") == 0 || strcmp(argv[i], "rogue") == 0) {
	startupInfo.team = RogueTeam;
      } else if (strcmp(argv[i], "o") == 0 || strcmp(argv[i], "observer") == 0) {
	startupInfo.team = ObserverTeam;
      } else {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    } else if (strcmp(argv[i], "-v") == 0 ||
	     strcmp(argv[i], "-version") == 0 ||
	     strcmp(argv[i], "--version") == 0) {
      printFatalError("BZFlag client, version %s\n"
		"  protocol %s",
		getAppVersion(),
		getProtocolVersion());
      exit(0);
    } else if (strcmp(argv[i], "-window") == 0) {
      BZDB.set("_window", "1");
    } else if (strcmp(argv[i], "-3dfx") == 0 || strcmp(argv[i], "-3Dfx") == 0) {
#if !defined(__linux__)
      putenv("MESA_GLX_FX=fullscreen");
#else
      setenv("MESA_GLX_FX", "fullscreen", 1);
#endif
    } else if (strcmp(argv[i], "-no3dfx") == 0 || strcmp(argv[i], "-no3Dfx") == 0) {
#if !defined(__linux__)
      putenv("MESA_GLX_FX=");
#else
      unsetenv("MESA_GLX_FX");
#endif
#ifdef DEBUG
    } else if (strcmp(argv[i], "-date") == 0) {
      checkArgc(i, argc, argv[i]);
      int month, day, year;
      if (sscanf(argv[i], "%d/%d/%d", &month, &day, &year) != 3 ||
		day < 1 || day > 31 ||		// FIXME -- upper limit loose
		month < 1 || month > 12 ||
		(year < 0 || (year > 100 && (year < 1970 || year > 2100)))) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      if (year > 100) year = year - 1900;
      else if (year < 70) year += 100;
      userTime.tm_mday = day;
      userTime.tm_mon = month - 1;
      userTime.tm_year = year;
#endif
    } else if (strcmp(argv[i], "-time") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("fixedTime", argv[i]);
    } else if (strcmp(argv[i], "-notime") == 0) {
      BZDB.unset("fixedTime");
    } else if (strcmp(argv[i], "-view") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("view", argv[i]);
    } else if (strcmp(argv[i], "-zoom") == 0) {
      checkArgc(i, argc, argv[i]);
      const int zoom = atoi(argv[i]);
      if (zoom < 1 || zoom > 8) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      BZDB.set("displayZoom", argv[i]);
    } else if (strcmp(argv[i], "-zbuffer") == 0) {
      checkArgc(i, argc, argv[i]);
      if (strcmp(argv[i], "on") == 0) {
	BZDB.set("zbuffer", "1");
      } else if (strcmp(argv[i], "off") == 0) {
	BZDB.set("zbuffer", "disable");
      } else {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    } else if (strcmp(argv[i], "-eyesep") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("eyesep", argv[i]);
    } else if (strcmp(argv[i], "-focal") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("focal", argv[i]);
    } else if (strncmp(argv[i], "-psn", 4) == 0) {
      std::vector<std::string> args;
      args.push_back(argv[i]);
      printError("Ignoring Finder argument \"{1}\"", &args);
      // ignore process serial number argument (-psn_x_xxxx for MacOS X
    } else if (strcmp(argv[i], "-badwords") == 0) {
      checkArgc(i, argc, argv[i], "Missing bad word filter file");
      BZDB.set("filterFilename", argv[i], StateDatabase::ReadOnly);
    } else if (argv[i][0] != '-') {
      if (i == argc-1) {

	// find the beginning of the server name, parse the callsign
	char* serverName;
	if ((serverName = strchr(argv[i], '@')) != NULL) {
	  *serverName = '\0';
	  if (strlen(argv[i]) >= sizeof(startupInfo.callsign))
	    printFatalError("Callsign truncated.");
	  strncpy(startupInfo.callsign, argv[i],
		  sizeof(startupInfo.callsign) - 1);
	  startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
	  ++serverName;
	} else {
	  serverName = argv[i];
	}

	// find the beginning of the port number, parse it
	char* portNumber;
	if ((portNumber = strchr(serverName, ':')) != NULL) {
	  *portNumber = '\0';
	  ++portNumber;
	  startupInfo.serverPort = atoi(portNumber);
	  if (startupInfo.serverPort < 1 || startupInfo.serverPort > 65535) {
	    startupInfo.serverPort = ServerPort;
	    printFatalError("Bad port, using default %d.",
			    startupInfo.serverPort);
	  }
	}
	if (strlen(serverName) >= sizeof(startupInfo.serverName)) {
	  printFatalError("Server name too long.  Ignoring.");
	} else {
	  strcpy(startupInfo.serverName, serverName);
	  startupInfo.autoConnect = true;
	}
      } else {
	printFatalError("Unexpected: %s. Server must go after all options.", argv[i]);
      }
    } else {
      printFatalError("Unknown option %s.", argv[i]);
      usage();
    }
  }
}

static void		parseConfigName(int argc, char** argv)
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-config") == 0) {
      checkArgc(i, argc, argv[i]);
      alternateConfig = getConfigDirName();
      alternateConfig += argv[i];
    }
  }
}

//
// resource database dumping.  used during initial startup to save
// preferences in case anything catastrophic goes wrong afterwards
// (so user won't have to wait through performance testing again).
//

void			dumpResources(BzfDisplay* display,
				SceneRenderer& renderer)
{
  // collect new configuration
  BZDB.set("callsign", startupInfo.callsign);
  BZDB.set("team", Team::getName(startupInfo.team));
  BZDB.set("server", startupInfo.serverName);
  if (startupInfo.serverPort != ServerPort) {
    BZDB.set("port", TextUtils::format("%d", startupInfo.serverPort));
  } else {
    BZDB.unset("port");
  }
  BZDB.set("list", startupInfo.listServerURL);
  if (isSoundOpen()) {
    BZDB.set("volume", TextUtils::format("%d", getSoundVolume()));
  }
  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0) {
    BZDB.set("zbuffer", "0");
  }

  if (renderer.getWindow().getWindow()->hasGammaControl()) {
    BZDB.set("gamma", TextUtils::format("%f", renderer.getWindow().getWindow()->getGamma()));
  }

  BZDB.set("quality", configQualityValues[renderer.useQuality()]);
  if (!BZDB.isSet("_window") && display->getResolution() != -1 &&
      display->getResolution(display->getResolution())) {
    BZDB.set("resolution", display->getResolution(display->getResolution())->name);
  }
  BZDB.set("startcode", ServerStartMenu::getSettings());

  BZDB.set("panelopacity", TextUtils::format("%f", renderer.getPanelOpacity()));

  BZDB.set("radarsize", TextUtils::format("%d", renderer.getRadarSize()));

  BZDB.set("mouseboxsize", TextUtils::format("%d", renderer.getMaxMotionFactor()));

  // don't save these configurations
  BZDB.setPersistent("_window", false);
  BZDB.setPersistent("_multisample", false);

  const std::vector<std::string> list = getSilenceList();

  // add entries silencedPerson0 silencedPerson1 etc..
  // to the database. Stores silenceList
  // By only allowing up to a certain # of people can prevent
  // the vague chance of buffer overrun.
  const int bufferLength = 30;
  int maxListSize = 1000000; //do even that many play bzflag?
  char buffer [bufferLength];

  if ((int)list.size() < maxListSize) maxListSize = list.size();
  for (int i = 0; i < maxListSize; i++) {
    sprintf(buffer,"silencedPerson%d",i);
    BZDB.set(TextUtils::format("silencedPerson%d", i), list[i]);
  }

  BZDB.set("email",startupInfo.email); // note email of zero length does not stick

  BZDB.set("serverCacheAge", TextUtils::format("%1d", (long)(ServerListCache::get())->getMaxCacheAge()));

  (ServerListCache::get())->saveCache();
}

static bool		needsFullscreen()
{
  // fullscreen if not in a window
  if (!BZDB.isSet("_window")) return true;

  // not fullscreen if view is default (normal)
  if (!BZDB.isSet("view")) return false;

  // fullscreen if view is not default
  std::string value = BZDB.get("view");
  for (int i = 1; i < (int)(sizeof(configViewValues) /
			sizeof(configViewValues[0])); i++)
    if (value == configViewValues[i])
      return true;

  // bogus view, default to normal so no fullscreen
  return false;
}

static void createCacheSignature ()
{
  // This file is to be used by archiving and mirroring tools avoid
  // this directory (and any of its sub-directories). Please see:
  //         < http://www.brynosaurus.com/cachedir/ >

  const char cacheSignature[] = "Signature: 8a477f597d28d172789f06886806bc55\n";
  const char cacheComment[] =
    "# This file is a cache directory tag created by BZFlag.\n"
    "# For information about cache directory tags, see:\n"
    "#     http://www.brynosaurus.com/cachedir/\n";
  std::string cacheTagName =  getCacheDirName();
  cacheTagName += "CACHEDIR.TAG";
  std::ostream* cacheTag = FILEMGR.createDataOutStream(cacheTagName, true, true);
  if (cacheTag != NULL) {
    cacheTag->write(cacheSignature, strlen(cacheSignature));
    cacheTag->write(cacheComment, strlen(cacheComment));
  }
  delete cacheTag;

  return;
}

//
// main()
//	initialize application and enter event loop
//

#if defined(_WIN32) && !defined(HAVE_SDL)
int			myMain(int argc, char** argv)
#else /* defined(_WIN32) */
int			main(int argc, char** argv)
#endif /* defined(_WIN32) */
{
#ifdef _WIN32
  // startup winsock
  static const int major = 2, minor = 2;
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
    printFatalError("Failed to initialize winsock.  Terminating.\n");
    return 1;
  }
  if (LOBYTE(wsaData.wVersion) != major ||
      HIBYTE(wsaData.wVersion) != minor) {
    printFatalError("Version mismatch in winsock;"
		    "  got %d.%d, expected %d.%d.  Terminating.\n",
		    (int)LOBYTE(wsaData.wVersion),
		    (int)HIBYTE(wsaData.wVersion),
		    major, minor);
    WSACleanup();
    return 1;
  }
#endif

  WordFilter *filter = (WordFilter *)NULL;

  argv0 = argv[0];

  // init libs

  //init_packetcompression();

  // check time bomb
  if (timeBombBoom()) {
    printFatalError("This release expired on %s. \n"
		"Please upgrade to the latest release. \n"
		"Exiting.", timeBombString());
    exit(0);
  }

  createCacheSignature();

  // initialize global objects and classes
  bzfsrand(time(0));

  // set default MOTD URL
  BZDB.set("motd", "http://bzflag.org/motd.php");

    // set default DB entries
  for (unsigned int gi = 0; gi < numGlobalDBItems; ++gi) {
    assert(globalDBItems[gi].name != NULL);
    if (globalDBItems[gi].value != NULL) {
      BZDB.set(globalDBItems[gi].name, globalDBItems[gi].value);
      BZDB.setDefault(globalDBItems[gi].name, globalDBItems[gi].value);
    }
    BZDB.setPersistent(globalDBItems[gi].name, globalDBItems[gi].persistent);
    BZDB.setPermission(globalDBItems[gi].name, globalDBItems[gi].permission);
  }

  BZDBCache::init();
  Flags::init();

  if (getenv("BZFLAGID")) {
    BZDB.set("callsign", getenv("BZFLAGID"));
    strncpy(startupInfo.callsign, getenv("BZFLAGID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  } else if (getenv("BZID")) {
    BZDB.set("callsign", getenv("BZID"));
    strncpy(startupInfo.callsign, getenv("BZID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  }
  time_t timeNow;
  time(&timeNow);
  userTime = *localtime(&timeNow);

  CommandsStandard::add();
  unsigned int i;

  // prepare DB entries
  for (i = 0; i < countof(defaultDBItems); ++i) {
    assert(defaultDBItems[i].name != NULL);
    if (defaultDBItems[i].value != NULL) {
      BZDB.set(defaultDBItems[i].name, defaultDBItems[i].value);
      BZDB.setDefault(defaultDBItems[i].name, defaultDBItems[i].value);
    }
    BZDB.setPersistent(defaultDBItems[i].name, defaultDBItems[i].persistent);
    BZDB.setPermission(defaultDBItems[i].name, defaultDBItems[i].permission);
    BZDB.addCallback(defaultDBItems[i].name, defaultDBItems[i].callback, NULL);
  }

  // parse for the config filename
  // the rest of the options are parsed after the config file
  // has been loaded to allow for command line overrides
  parseConfigName(argc, argv);

  // read resources
  if (alternateConfig != "") {
    if (CFGMGR.read(alternateConfig)) {
      startupInfo.hasConfiguration = true;
    }
  }
  if (!startupInfo.hasConfiguration) {
    findConfigFile();
    if (CFGMGR.read(getCurrentConfigFileName())) {
      startupInfo.hasConfiguration = true;
      updateConfigFile();
    }
  }

  if (startupInfo.hasConfiguration)
    ActionBinding::instance().getFromBindings();
  else
    // bind default keys
    ActionBinding::instance().resetBindings();

  ServerListCache::get()->loadCache();

  // restore some configuration (command line overrides these)
  if (startupInfo.hasConfiguration) {
    if (BZDB.isSet("callsign")) {
      strncpy(startupInfo.callsign, BZDB.get("callsign").c_str(),
					sizeof(startupInfo.callsign) - 1);
      startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
    }
    if (BZDB.isSet("team")) {
      std::string value = BZDB.get("team");
      for (int i = 0; i < NumTeams; i++) {
	if (value == Team::getName((TeamColor)i)) {
	  startupInfo.team = (TeamColor)i;
	  break;
	}
      }
      if (value == Team::getName(AutomaticTeam)) {
	startupInfo.team = AutomaticTeam;
      }
    }
    if (BZDB.isSet("server")) {
      strncpy(startupInfo.serverName, BZDB.get("server").c_str(),
					sizeof(startupInfo.serverName) - 1);
      startupInfo.serverName[sizeof(startupInfo.serverName) - 1] = '\0';
    }
    if (BZDB.isSet("port")) {
      startupInfo.serverPort = atoi(BZDB.get("port").c_str());
    }

    // check for reassigned team colors
    if (BZDB.isSet("roguecolor"))
      setTeamColor(RogueTeam, BZDB.get("roguecolor"));
    if (BZDB.isSet("redcolor"))
      setTeamColor(RedTeam, BZDB.get("redcolor"));
    if (BZDB.isSet("greencolor"))
      setTeamColor(GreenTeam, BZDB.get("greencolor"));
    if (BZDB.isSet("bluecolor"))
      setTeamColor(BlueTeam, BZDB.get("bluecolor"));
    if (BZDB.isSet("purplecolor"))
      setTeamColor(PurpleTeam, BZDB.get("purplecolor"));

    // check for reassigned radar colors
    if (BZDB.isSet("rogueradar"))
      setRadarColor(RogueTeam, BZDB.get("rogueradar"));
    if (BZDB.isSet("redradar"))
      setRadarColor(RedTeam, BZDB.get("redradar"));
    if (BZDB.isSet("greenradar"))
      setRadarColor(GreenTeam, BZDB.get("greenradar"));
    if (BZDB.isSet("blueradar"))
      setRadarColor(BlueTeam, BZDB.get("blueradar"));
    if (BZDB.isSet("purpleradar"))
      setRadarColor(PurpleTeam, BZDB.get("purpleradar"));


    // ignore window name in config file (it's used internally)
    BZDB.unset("_window");
    BZDB.unset("_multisample");
  }

  // use UDP? yes
  startupInfo.useUDPconnection=true;

  // parse arguments
  parse(argc, argv);

  if (BZDB.isSet("directory")) {
    //Convert to unix paths so that escaping isn't an issue
    std::string directory = BZDB.get("directory");
    OSFileOSToStdDir((char *)directory.c_str()); //ok this is quasi-cheating
    BZDB.set("directory", directory);
  }

  if (debugLevel >= 4)
    BZDB.setDebug(true);

  // set time from BZDB
  if (BZDB.isSet("fixedTime")) {
    int hours, minutes, seconds;
    char dbTime[256];
    strncpy(dbTime, BZDB.get("fixedTime").c_str(), sizeof(dbTime) - 1);
    if (sscanf(dbTime, "%d:%d:%d", &hours, &minutes, &seconds) != 3 ||
	hours < 0 || hours > 23 ||
	minutes < 0 || minutes > 59 ||
	seconds < 0 || seconds > 59) {
      printFatalError("Invalid argument for fixedTime = %s", dbTime);
    }
    userTime.tm_sec = seconds;
    userTime.tm_min = minutes;
    userTime.tm_hour = hours;
  }

  // see if there is a _default_ badwords file
  if (!BZDB.isSet("filterFilename")) {
    std::string name;
#if !defined(_WIN32) & !defined(macintosh)
    name = getConfigDirName();
    name += "badwords.txt";
#elif defined(_WIN32) /* !defined(_WIN32) */
    name = getConfigDirName();
    name += "badwords.txt";
#else
    name = "badwords.txt"; // FIXME - use getConfigDirName() ?
#endif /* !defined(_WIN32) & !defined(macintosh) */

    // get a handle on a filter object to attempt a load
    if (BZDB.isSet("filter")) {
      filter = (WordFilter *)BZDB.getPointer("filter");
      if (filter == NULL) {
	filter = new WordFilter();
      }
    } else {
      // filter is not set
      filter = new WordFilter();
    }
    // XXX should stat the file first and load with interactive feedback
    unsigned int count = filter->loadFromFile(name, false);
    if (count > 0) {
      std::cout << "Loaded " << count << " words from \"" << name << "\"" << std::endl;
    }
  }

  // load the bad word filter, regardless of a default, if it was set
  if (BZDB.isSet("filterFilename")) {
    std::string filterFilename = BZDB.get("filterFilename");
    std::cout << "Filter file name specified is \"" << filterFilename << "\"" << std::endl;
    if (filterFilename.length() != 0) {
      if (filter == NULL) {
	filter = new WordFilter();
      }
      std::cout << "Loading " << filterFilename << std::endl;
      unsigned int count = filter->loadFromFile(filterFilename, true);
      std::cout << "Loaded " << count << " words" << std::endl;

      // stash the filter into the database for retrieval later
      BZDB.setPointer("filter", (void *)filter, StateDatabase::ReadOnly );
      BZDB.setPersistent("filter", false);
    } else {
      std::cerr << "WARNING: A proper file name was not given for the -badwords argument" << std::endl;
    }
  }

  // get email address if not anonymous
  std::string email = "default";
  if (!anonymous) {
    if (BZDB.isSet("email")) {
      email = BZDB.get("email");
    }

    if (email == "default") {
      email = anonymousName;
      std::string hostname = Address::getHostName();
#if defined(_WIN32)
      char username[256];
      DWORD usernameLen = sizeof(username);
      GetUserName(username, &usernameLen);
#elif defined(macintosh)
      const char *username = "mac_user";
#else
      struct passwd* pwent = getpwuid(getuid());
      const char* username = pwent ? pwent->pw_name : NULL;
#endif
      if (hostname == "") {
	hostname = "unknown";
      }
      if (username) {
	email = username;
	email += "@";
	email += hostname;
      }
    }
  }
  email = email.substr(0, sizeof(startupInfo.email) - 1);
  strcpy(startupInfo.email, email.c_str());

  // make platform factory
  PlatformFactory* platformFactory = PlatformFactory::getInstance();

  // open display
  display = platformFactory->createDisplay(NULL, NULL);
  if (!display) {
    printFatalError("Can't open display.  Exiting.");
    return 1;
  }

  // choose visual
  BzfVisual* visual = platformFactory->createVisual(display);
  setVisual(visual);

  // make the window
  BzfWindow* window = platformFactory->createWindow(display, visual);
  if (!window->isValid()) {
    printFatalError("Can't create window.  Exiting.");
    return 1;
  }
  window->setTitle("bzflag");

  // create & initialize the joystick
  BzfJoystick* joystick = platformFactory->createJoystick();
  joystick->initJoystick(BZDB.get("joystickname").c_str());

  // Change audio driver if requested
  if (BZDB.isSet("audioDriver"))
    PlatformFactory::getMedia()->setDriver(BZDB.get("audioDriver"));
  // Change audio device if requested
  if (BZDB.isSet("audioDevice"))
    PlatformFactory::getMedia()->setDevice(BZDB.get("audioDevice"));

  // set data directory if user specified
  if (BZDB.isSet("directory")) {
    PlatformFactory::getMedia()->setMediaDirectory(BZDB.get("directory"));
  } else {

    // !!! fix this stupid check.. GetMacOSXDataPath() is NULL without app bundle.

#if defined(__APPLE__)
    extern char *GetMacOSXDataPath(void);
    PlatformFactory::getMedia()->setMediaDirectory(GetMacOSXDataPath());
    BZDB.set("directory", GetMacOSXDataPath());
    BZDB.setPersistent("directory", false);
#elif (defined(_WIN32) || defined(WIN32))
    char exePath[MAX_PATH];
    GetModuleFileName(NULL,exePath,MAX_PATH);
    char* last = strrchr(exePath,'\\');
    if (last)
      *last = '\0';
    strcat(exePath,"\\data");
    PlatformFactory::getMedia()->setMediaDirectory(exePath);
#else
    // It's only checking existence of l10n directory
    DIR *localedir = opendir("data/l10n/");
    if (localedir == NULL) {
      PlatformFactory::getMedia()->setMediaDirectory(INSTALL_DATA_DIR);
    } else {
      closedir(localedir);
    }
#endif
  }

  // initialize font system
  FontManager &fm = FontManager::instance();
  // load fonts from data directory
  fm.loadAll(PlatformFactory::getMedia()->getMediaDirectory() + "/fonts");
  // try to get a font - only returns -1 if there are no fonts at all
  if (fm.getFaceID(BZDB.get("consoleFont")) < 0) {
    printFatalError("No fonts found.  Exiting");
    return 1;
  }

  // initialize locale system

  BundleMgr *bm = new BundleMgr(PlatformFactory::getMedia()->getMediaDirectory(), "bzflag");
  World::setBundleMgr(bm);

  std::string locale = BZDB.isSet("locale") ? BZDB.get("locale") : "default";
  World::setLocale(locale);
  bm->getBundle(World::getLocale());

  bool setPosition = false, setSize = false;
  int x = 0, y = 0, w = 0, h = 0;

  // set window size (we do it here because the OpenGL context isn't yet bound)

  if (BZDB.isSet("geometry")) {
    char xs, ys;
    const std::string geometry = BZDB.get("geometry");
    const int count = sscanf(geometry.c_str(), "%dx%d%c%d%c%d", &w, &h, &xs, &x, &ys, &y);

    if (geometry == "default" || (count != 6 && count != 2) || w < 0 || h < 0) {
      BZDB.unset("geometry");
    } else if (count == 6 && ((xs != '-' && xs != '+') || (ys != '-' && ys != '+'))) {
      BZDB.unset("geometry");
    } else {
      setSize = true;
      if (w < 256)
	w = 256;
      if (h < 192)
	h = 192;
      if (count == 6) {
	if (xs == '-')
	  x = display->getWidth() - x - w;
	if (ys == '-')
	  y = display->getHeight() - y - h;
	setPosition = true;
      }
      // must call this before setFullscreen() is called
      display->setPassthroughSize(w, h);
    }
  }

  // set window size (we do it here because the OpenGL context isn't yet
  // bound and 3Dfx passthrough cards use the window size to determine
  // the resolution to use)
  const bool useFullscreen = needsFullscreen();
  if (useFullscreen) {
    // tell window to be fullscreen
    window->setFullscreen();

    // set the size if one was requested.  this overrides the default
    // size (which is the display or passthrough size).
    if (setSize)
      window->setSize(w, h);
  } else if (setSize) {
    window->setSize(w, h);
  } else {
    window->setSize(640, 480);
  }
  if (setPosition)
    window->setPosition(x, y);

  // now make the main window wrapper.  this'll cause the OpenGL context
  // to be bound for the first time.
  MainWindow* pmainWindow = new MainWindow(window, joystick);
  MainWindow& mainWindow = *pmainWindow;
  std::string videoFormat;
  int format = -1;
  if (BZDB.isSet("resolution")) {
    videoFormat = BZDB.get("resolution");
    if (videoFormat.length() != 0) {
      format = display->findResolution(videoFormat.c_str());
      if (format >= 0) {
	display->setFullScreenFormat(format);
      }
    }
  };
  // set fullscreen again so MainWindow object knows it's full screen
  if (useFullscreen)
    // this will also call window create
    mainWindow.setFullscreen();
  else
    window->create();

  // get sound files.  must do this after creating the window because
  // DirectSound is a bonehead API.
  if (!noAudio) {
    openSound("bzflag");
    if (startupInfo.hasConfiguration && BZDB.isSet("volume"))
      setSoundVolume(static_cast<int>(BZDB.eval("volume")));
  }

  // set main window's minimum size (arbitrary but should be big enough
  // to see stuff in control panel)
  mainWindow.setMinSize(256, 192);

  // initialize graphics state
  mainWindow.getWindow()->makeCurrent();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0);
  glClearStencil(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_SCISSOR_TEST);
//  glEnable(GL_CULL_FACE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  OpenGLGState::init();

  // add the zbuffer callback here, after the OpenGL context is initialized
  BZDB.addCallback("zbuffer", setDepthBuffer, NULL);

  // if we're running on 3Dfx fullscreen add a fake cursor.
  // let the defaults file override this, though.
  if (!BZDB.isSet("fakecursor")) {
    // check that the glrenderer is Mesa Glide
    const char* glrenderer = (const char*)glGetString(GL_RENDERER);
    if ((glrenderer != NULL) && (strncmp(glrenderer, "Mesa Glide", 10) == 0 ||
	strncmp(glrenderer, "3Dfx", 4) == 0))
      BZDB.set("fakecursor", "1");
  }

  // set gamma if set in resources and we have gamma control
  if (BZDB.isSet("gamma")) {
    if (mainWindow.getWindow()->hasGammaControl())
      mainWindow.getWindow()->setGamma((float)atof(BZDB.get("gamma").c_str()));
  }

  // set the scene renderer's window
  RENDERER.setWindow(pmainWindow);

  // restore rendering configuration
  if (startupInfo.hasConfiguration) {
    if (BZDB.isSet("zbuffersplit")) {
      RENDERER.setZBufferSplit(BZDB.isTrue("zbuffersplit"));
    }
    if (BZDBCache::texture) {
      TextureManager::instance().setMaxFilter(BZDB.get("texture"));
    }
    if (BZDB.isSet("quality")) {
      std::string value = BZDB.get("quality");
      for (int i = 0; i < (int)(sizeof(configQualityValues) /
				sizeof(configQualityValues[0])); i++)
	if (value == configQualityValues[i]) {
	  RENDERER.setQuality(i);
	  break;
	}
    }
    BZDB.set("_texturereplace", (!BZDB.isTrue("lighting") &&
	      RENDERER.useQuality() < 2) ? "1" : "0");
    BZDB.setPersistent("_texturereplace", false);
    if (BZDB.isSet("view")) {
      RENDERER.setViewType(SceneRenderer::Normal);
      std::string value = BZDB.get("view");
      for (int i = 0; i < (int)(sizeof(configViewValues) /
				sizeof(configViewValues[0])); i++)
	if (value == configViewValues[i]) {
	  RENDERER.setViewType((SceneRenderer::ViewType)i);
	  break;
	}
    }

    if (BZDB.isSet("startcode"))
      ServerStartMenu::setSettings(BZDB.get("startcode").c_str());

    if (BZDB.isSet("panelopacity"))
      RENDERER.setPanelOpacity(BZDB.eval("panelopacity"));
    if (BZDB.isSet("radarsize"))
      RENDERER.setRadarSize(atoi(BZDB.get("radarsize").c_str()));
    if (BZDB.isSet("mouseboxsize"))
      RENDERER.setMaxMotionFactor(atoi(BZDB.get("mouseboxsize").c_str()));
  }

  // grab the mouse only if allowed
  if (BZDB.isSet("mousegrab") && !BZDB.isTrue("mousegrab")) {
    mainWindow.setNoMouseGrab();
    mainWindow.enableGrabMouse(false);
  } else {
    mainWindow.enableGrabMouse(true);
  }

  // set window quadrant
  if (RENDERER.getViewType() == SceneRenderer::ThreeChannel)
    mainWindow.setQuadrant(MainWindow::UpperRight);
  else if (RENDERER.getViewType() == SceneRenderer::Stacked)
    mainWindow.setQuadrant(MainWindow::LowerHalf);
#ifndef USE_GL_STEREO
  else if (RENDERER.getViewType() == SceneRenderer::Stereo)
    mainWindow.setQuadrant(MainWindow::UpperRight);
#endif

  // set server list URL
  if (BZDB.isSet("list"))
    startupInfo.listServerURL = BZDB.get("list");

  // setup silence list
  std::vector<std::string>& list = getSilenceList();

  // search for entries silencedPerson0 silencedPerson1 etc..
  // to the database. Stores silenceList
  // By only allowing up to a certain # of people can prevent
  // the vague chance of buffer overrun.
  const int bufferLength = 30;
  const int maxListSize = 1000000; // do even that many play bzflag?
  char buffer [bufferLength];
  bool keepGoing = true;

  for (int s = 0; keepGoing && (s < maxListSize); s++) {
    sprintf(buffer,"silencedPerson%d",s); // could do %-10d

    if (BZDB.isSet(buffer)) {
      list.push_back(BZDB.get(buffer));
      // remove the value from the database so when we save
      // it saves the list's new values in order
      BZDB.unset(buffer);
    } else {
      keepGoing = false;
    }
  }

  if (BZDB.isSet("serverCacheAge")) {
    (ServerListCache::get())->setMaxCacheAge(atoi(BZDB.get("serverCacheAge").c_str()));
  }

  // start playing
  startPlaying(display, RENDERER, &startupInfo);

  // save resources
  dumpResources(display, RENDERER);
  if (alternateConfig == "")
    CFGMGR.write(getCurrentConfigFileName());
  else
    CFGMGR.write(alternateConfig);

  // shut down
  if (filter != NULL)
    delete filter;
  filter = NULL;
  display->setDefaultResolution();
  delete pmainWindow;
  delete joystick;
  delete window;
  delete visual;
  closeSound();
  delete display;
  delete platformFactory;
  delete bm;

#ifdef _WIN32
  // clean up
  WSACleanup();
#endif

  // clean up singletons
  //  delete FILEMGR;
  //  delete CMDMGR;
  //  delete BZDB;

  return 0;
}
//
#if defined(_WIN32) && !defined(HAVE_SDL)

//
// WinMain()
//	windows entry point.  forward to main()
//

int WINAPI		WinMain(HINSTANCE instance, HINSTANCE, LPSTR _cmdLine, int)
{
  // convert command line to argc and argv.  note that it's too late
  // to do this right because spaces that were embedded in a single
  // argument now look like like normal spaces.  not much we can do
  // about that.
  // FIXME -- argc and argv can be accessible;  use them instead of this.
  char* cmdLine = strdup(_cmdLine);

  // count number of arguments
  int argc = 1;
  char* scan = cmdLine;
  while (isspace(*scan) && *scan != 0) scan++;
  while (*scan) {
    argc++;
    while (!isspace(*scan) && *scan != 0) scan++;
    while (isspace(*scan) && *scan != 0) scan++;
  }

  // get path to application.  this is ridiculously simple.
  char appName[MAX_PATH];

  GetModuleFileName(instance,appName,MAX_PATH);

  if (strrchr(appName,'\\'))
    *strrchr(appName,'\\') = '\0';

  // make argument list and assign arguments
  char** argv = new char*[argc];
  argc = 0;
  argv[argc++] = appName;
  scan = cmdLine;
  while (isspace(*scan) && *scan != 0) scan++;
  while (*scan) {
    argv[argc++] = scan;
    while (!isspace(*scan) && *scan != 0) scan++;
    if (*scan) *scan++ = 0;
    while (isspace(*scan) && *scan != 0) scan++;
  }

  const int exitCode = myMain(argc, argv);

  // clean up and return exit code
  delete[] argv;
  free(cmdLine);
  return exitCode;
}

#endif /* defined(_WIN32) */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

