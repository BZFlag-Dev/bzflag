/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#if defined(_WIN32)
#define _WINSOCKAPI_
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

#elif !defined(macintosh)
#include <pwd.h>
#include <dirent.h>
#endif /* defined(_WIN32) */
#include <stdarg.h>
#include "common.h"
#include "bzfio.h"
#include <fstream>
#include "bzfgl.h"
#include "ErrorHandler.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "SceneRenderer.h"
#include "resources.h"
#include "MainWindow.h"
#include "Address.h"
#include "Protocol.h"
#include "playing.h"
#include "TimeBomb.h"
#include "Team.h"
#include "sound.h"
#include "KeyMap.h"
#include "menus.h"
#include "ConfigFileManager.h"
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

extern std::vector<std::string>& getSilenceList();
const char*		argv0;
static bool		anonymous = false;
static std::string	anonymousName("anonymous");
static bool		noAudio = false;
struct tm		userTime;
static StartupInfo	startupInfo;
bool			echoToConsole = false;
bool			echoClean = false;
ResourceDatabase	db;

static BzfDisplay*	display = NULL;

float		WorldSize =	DEFAULT_WORLD;					// meters


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
  { "team",			"Rogue",		true,	StateDatabase::ReadWrite,	NULL },
  { "list",			DefaultListServerURL,	true,	StateDatabase::ReadWrite,	NULL },
  { "volume",			"10",			true,	StateDatabase::ReadWrite,	NULL },
  { "latitude",			"37.5",			true,	StateDatabase::ReadWrite,	NULL },
  { "longitude",		"122",			true,	StateDatabase::ReadWrite,	NULL },
  { "joystick",			"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "enhancedRadar",		"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "coloredradarshots",	"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "linedradarshots",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "panelopacity",		"0.3",			true,	StateDatabase::ReadWrite,	NULL },
  { "radarsize",		"4",			true,	StateDatabase::ReadWrite,	NULL },
  { "mouseboxsize",		"5",			true,	StateDatabase::ReadWrite,	NULL },
  { "bigfont",			"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "colorful",			"1",			true,	StateDatabase::ReadWrite,	NULL },
  { "underline",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "killerhighlight",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "serverCacheAge",		"0",			true,	StateDatabase::ReadWrite,	NULL },
  { "displayRadarFlags",	"1",			true,	StateDatabase::ReadWrite,	NULL }
};

// default key bindings
static const char*	bindingList[] = {
  "bind F12 down quit",
  "bind \"Left Mouse\" down fire",
  "bind Enter down fire",
  "bind \"Middle Mouse\" down drop",
  "bind Space down drop",
  "bind \"Right Mouse\" down identify",
  "bind I down identify",
  "bind Tab down jump",
  "bind N down \"send all\"",
  "bind M down \"send team\"",
  "bind , down \"send nemesis\"",
  "bind . down \"send recipient\"",
  "bind S down \"toggle showscore\"",
  "bind Pause down pause",
  "bind P down pause",
#ifdef SNAPPING
  "bind F5 down screenshot",
#endif
  "bind - down \"time backward\"",
  "bind + down \"time forward\""
};

#ifdef ROBOT
// ROBOT -- tidy up
int numRobotTanks = 0;
#endif

//
// special error handler.  shows a message box on Windows.
//

void			printFatalError(const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
#if defined(_WIN32)
  MessageBox(NULL, buffer, "BZFlag Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
  cerr << buffer << endl;
#endif
  delete display;
  display = NULL;
}

//
// application initialization
//

static const char*	configQualityValues[] = {
				"low",
				"medium",
				"high"
			};
static const char*	configViewValues[] = {
				"normal",
				"stereo",
				"stacked",
				"three"
			};

static std::string	getConfigFileName()
{
#if !defined(_WIN32) & !defined(macintosh)

  std::string name;
  struct passwd* pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += std::string(pwent->pw_dir);
    name += "/";
  }
  name += ".bzflag";

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
  std::string name("C:");
  char dir[MAX_PATH];
  ITEMIDLIST* idl;
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
    if (SHGetPathFromIDList(idl, dir)) {
      struct stat statbuf;
      if (stat(dir, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) != 0)
	name = dir;
    }

    IMalloc* shalloc;
    if (SUCCEEDED(SHGetMalloc(&shalloc))) {
      shalloc->Free(idl);
      shalloc->Release();
    }
  }

  // append the config file name
  name += "\\bzflag.bzc";
  return name;

#elif defined(macintosh)
  return "bzflag.bzc";
#endif /* !defined(_WIN32) & !defined(macintosh) */
}

#if !defined(_WIN32) & !defined(macintosh)
static std::string	getConfigFileName2()
{
  std::string name;
  struct passwd* pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += std::string(pwent->pw_dir);
    name += "/";
  }
  name += ".bzflag";
  return name;
}
#endif

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

static void		setVisual(BzfVisual* visual,
				const ResourceDatabase& resources)
{
  // sine qua non
  visual->setLevel(0);
  visual->setDoubleBuffer(true);
  visual->setRGBA(1, 1, 1, 0);

  // ask for a zbuffer if not disabled.  we might choose not to use it
  // if we do ask for it.
  if (!resources.hasValue("zbuffer") ||
	resources.getValue("zbuffer") != "disable")
    visual->setDepth(16);

  // optional
#if defined(DEBUG_RENDERING)
  visual->setStencil(4);
#endif
  if (BZDB->isTrue("multisample"))
    visual->setMultisample(4);
#ifdef USE_GL_STEREO
  if (resources.hasValue("view") &&
	resources.getValue("view") == configViewValues[1])
    visual->setStereo(true);
#endif
}

static void		usage()
{
  printFatalError("usage: %s"
	" [-3dfx]"
	" [-no3dfx]"
	" [-anonymous]"
	" [-callsign <call-sign>]"
	" [-directory <data-directory>]"
	" [-echo]"
	" [-echoClean]"
	" [-geometry <geometry-spec>]"
	" [-interface <interface>]"
	" [-joystick {yes|no}]"
	" [-joystickname <name>]"
	" [-latitude <latitude>] [-longitude <longitude>]"
	" [-list <server-list-url>] [-nolist]"
	" [-locale <locale>]"
	" [-multisample]"
	" [-mute]"
	" [-port <server-port>]"
#ifdef ROBOT
	" [-solo <num-robots>]"
#endif
	" [-team {red|green|blue|purple|rogue}]"
	" [-ttl <time-to-live>]"
	" [-version]"
	" [-view {normal|stereo|stacked|three}]"
	" [-window]"
	" [-zoom <zoom-factor>]"
	" server\n\nExiting.", argv0);
  exit(1);
}

static void		parse(int argc, char** argv,
				ResourceDatabase& resources)
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0 ||
		strcmp(argv[i], "-anonymous") == 0) {
      anonymous = true;
    }
    else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-callsign") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strlen(argv[i]) >= sizeof(startupInfo.callsign))
	printFatalError("Callsign truncated.");
      strncpy(startupInfo.callsign, argv[i], sizeof(startupInfo.callsign) - 1);
      startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
    }
    else if (strcmp(argv[i], "-d") == 0 ||
		strcmp(argv[i], "-directory") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strlen(argv[i]) == 0)
	resources.removeValue("directory");
      else
	resources.addValue("directory", argv[i]);
    }
    else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "-echo") == 0) {
      echoToConsole = true;
    }
    else if (strcmp(argv[i], "-ec") == 0 || strcmp(argv[i], "-echoClean") == 0) {
      echoToConsole = true;
      echoClean = true;
    }
    else if (strcmp(argv[i], "-h") == 0 ||
	     strcmp(argv[i], "-help") == 0 ||
	     strcmp(argv[i], "--help") == 0) {
      usage();
    }
    else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "-geometry") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
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
      resources.addValue("geometry", argv[i]);
    }
    else if (strcmp(argv[i], "-i") == 0 ||
		strcmp(argv[i], "-interface") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strlen(argv[i]) >= sizeof(startupInfo.multicastInterface))
	printFatalError("Interface name too long.");
      else
	strcpy(startupInfo.multicastInterface, argv[i]);
    }
    else if (strcmp(argv[i], "-latitude") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      double latitude = atof(argv[i]);
      if (latitude < -90.0 || latitude > 90.0) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("latitude", argv[i]);
    }
    else if (strcmp(argv[i], "-longitude") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      double longitude = atof(argv[i]);
      if (longitude < -180.0 || longitude > 180.0) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("longitude", argv[i]);
    }
    else if (strcmp(argv[i], "-list") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strcmp(argv[i], "default") == 0) {
	BZDB->unset("list");
      }
      else {
	startupInfo.listServerURL = argv[i];
	BZDB->set("list", argv[i]);
      }
    }
    else if (strcmp(argv[i], "-locale") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("locale", argv[i]);
    }
    else if (strcmp(argv[i], "-nolist") == 0) {
      startupInfo.listServerURL = "";
      BZDB->set("list", "");
    }
    else if (strcmp(argv[i], "-m") == 0 ||
		strcmp(argv[i], "-mute") == 0) {
      noAudio = true;
    }
    else if (strcmp(argv[i], "-multisample") == 0) {
      BZDB->set("_multisample", "1");
    }
    else if (strcmp(argv[i], "-port") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      startupInfo.serverPort = atoi(argv[i]);
      if (startupInfo.serverPort < 1 || startupInfo.serverPort > 65535) {
	startupInfo.serverPort = ServerPort;
	printFatalError("Bad port, using default %d.", startupInfo.serverPort);
      }
    }
#ifdef ROBOT
    else if (strcmp(argv[i], "-solo") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      numRobotTanks = atoi(argv[i]);
      if (numRobotTanks < 1 || numRobotTanks > MAX_ROBOTS) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    }
#endif
    else if (strcmp(argv[i], "-team") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strcmp(argv[i], "r") == 0 || strcmp(argv[i], "red") == 0)
	startupInfo.team = RedTeam;
      else if (strcmp(argv[i], "g") == 0 || strcmp(argv[i], "green") == 0)
	startupInfo.team = GreenTeam;
      else if (strcmp(argv[i], "b") == 0 || strcmp(argv[i], "blue") == 0)
	startupInfo.team = BlueTeam;
      else if (strcmp(argv[i], "p") == 0 || strcmp(argv[i], "purple") == 0)
	startupInfo.team = PurpleTeam;
      else if (strcmp(argv[i], "z") == 0 || strcmp(argv[i], "rogue") == 0)
	startupInfo.team = RogueTeam;
      else {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    }
    else if (strcmp(argv[i], "-ttl") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      startupInfo.ttl = atoi(argv[i]);
      if (startupInfo.ttl < 0) {
	startupInfo.ttl = 0;
	printFatalError("Using minimum ttl of %d.", startupInfo.ttl);
      }
      else if (startupInfo.ttl > MaximumTTL) {
	startupInfo.ttl = MaximumTTL;
	printFatalError("Using maximum ttl of %d.", startupInfo.ttl);
      }
    }
    else if (strcmp(argv[i], "-joystick") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strcmp(argv[i], "0") != 0 && strcmp(argv[i], "1") != 0) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      BZDB->set("joystick", argv[i]);
    }
    else if (strcmp(argv[i], "-joystickname") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      BZDB->set("joystickname", argv[i]);
    }
    else if (strcmp(argv[i], "-v") == 0 ||
	     strcmp(argv[i], "-version") == 0 ||
	     strcmp(argv[i], "--version") == 0) {
      printFatalError("BZFlag client, version %s\n"
		"  protocol %c.%d%c",
		VERSION,
		ServerVersion[4],
		atoi(ServerVersion + 5),
		ServerVersion[7]);
      exit(0);
    }
    else if (strcmp(argv[i], "-window") == 0) {
      BZDB->set("_window", "1");
    }
    else if (strcmp(argv[i], "-3dfx") == 0 || strcmp(argv[i], "-3Dfx") == 0) {
#if !defined(__linux__)
      putenv("MESA_GLX_FX=fullscreen");
#else
      setenv("MESA_GLX_FX", "fullscreen", 1);
#endif
    }
    else if (strcmp(argv[i], "-no3dfx") == 0 || strcmp(argv[i], "-no3Dfx") == 0) {
#if !defined(__linux__)
      putenv("MESA_GLX_FX=");
#else
      unsetenv("MESA_GLX_FX");
#endif
    }
#ifdef DEBUG
    else if (strcmp(argv[i], "-date") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
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
    }
    else if (strcmp(argv[i], "-time") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      int hours, minutes, seconds;
      if (sscanf(argv[i], "%d:%d:%d", &hours, &minutes, &seconds) != 3 ||
		hours < 0 || hours > 23 ||
		minutes < 0 || minutes > 59 ||
		seconds < 0 || seconds > 59) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      userTime.tm_sec = seconds;
      userTime.tm_min = minutes;
      userTime.tm_hour = hours;
    }
#endif
    else if (strcmp(argv[i], "-view") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("view", argv[i]);
    }
    else if (strcmp(argv[i], "-zoom") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      const int zoom = atoi(argv[i]);
      if (zoom < 1 || zoom > 8) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("zoom", argv[i]);
    }
    else if (strcmp(argv[i], "-zbuffer") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      if (strcmp(argv[i], "on") == 0) {
	resources.addValue("zbuffer", "1");
      }
      else if (strcmp(argv[i], "off") == 0) {
	resources.addValue("zbuffer", "disable");
      }
      else {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    }
    else if (strcmp(argv[i], "-eyesep") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("eyesep", argv[i]);
    }
    else if (strcmp(argv[i], "-focal") == 0) {
      if (++i == argc) {
	printFatalError("Missing argument for %s.", argv[i-1]);
	usage();
      }
      resources.addValue("focal", argv[i]);
    }
    else if (strncmp(argv[i], "-psn", 3) == 0) {
	std::vector<std::string> args;
	args.push_back(argv[i]);
	printError("Ignoring Finder argument \"{1}\"", &args);
	// ignore process serial number argument for MacOS X
    }
    else if (i == argc-1) {
      if (strlen(argv[i]) >= sizeof(startupInfo.serverName)) {
	printFatalError("Server name too long.  Ignoring.");
      }
      else {
	strcpy(startupInfo.serverName, argv[i]);
	startupInfo.autoConnect = true;
      }
    }
    else {
      usage();
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
  BZDB->set("callsign", startupInfo.callsign);
  BZDB->set("team", Team::getName(startupInfo.team));
  BZDB->set("server", startupInfo.serverName);
  if (startupInfo.serverPort != ServerPort) {
    BZDB->set("port", string_util::format("%d", startupInfo.serverPort));
  }
  else {
    BZDB->unset("port");
  }
  if (strlen(startupInfo.multicastInterface) != 0)
    BZDB->set("interface", startupInfo.multicastInterface);
  BZDB->set("list", startupInfo.listServerURL);
  if (isSoundOpen()) {
    BZDB->set("volume", string_util::format("%d", getSoundVolume()));
  }
  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0) {
    BZDB->set("zbuffer", "0");
  }

  if (renderer.getWindow().getWindow()->hasGammaControl()) {
    BZDB->set("gamma", string_util::format("%f", renderer.getWindow().getWindow()->getGamma()));
  }

  BZDB->set("quality", configQualityValues[renderer.useQuality()]);
  if (display->getResolution() != -1 &&
      display->getResolution(display->getResolution())) {
    BZDB->set("resolution", display->getResolution(display->getResolution())->name);
  }
  {
    BzfKeyMap& map = getBzfKeyMap();
    for (int i = 0; i < (int)BzfKeyMap::LastKey; i++) {
      // get value string
      const BzfKeyEvent& key1 = map.get((BzfKeyMap::Key)i);
      std::string value;
      if (key1.ascii != 0 || key1.button != 0) {
	value = BzfKeyMap::getKeyEventString(key1);
	const BzfKeyEvent& key2 = map.getAlternate((BzfKeyMap::Key)i);
	if (key2.ascii != 0 || key2.button != 0) {
	  value += "/";
	  value += BzfKeyMap::getKeyEventString(key2);
	}
      }

      // add it
      const std::string name = BzfKeyMap::getKeyName((BzfKeyMap::Key)i);
      db.addValue(name, value);
    }
  }
  BZDB->set("startcode", ServerStartMenu::getSettings());
  BZDB->set("showflaghelp", renderer.getShowFlagHelp() ? "1" : "0");
  BZDB->set("showlabels", renderer.getLabels() ? "1" : "0");

  BZDB->set("panelopacity", string_util::format("%f", renderer.getPanelOpacity()));

  BZDB->set("radarsize", string_util::format("%d", renderer.getRadarSize()));

  BZDB->set("mouseboxsize", string_util::format("%d", renderer.getMaxMotionFactor()));

  BZDB->set("underline", OpenGLTexFont::getUnderlineColor());

  // don't save these configurations
  BZDB->setPersistent("_window", false);
  BZDB->setPersistent("_multisample", false);

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
    BZDB->set(string_util::format("silencedPerson%d", i), list[i]);
  }

  BZDB->set("serverCacheAge", string_util::format("%1d", (long)ServerMenu::getMaxCacheAge()));

  // save configuration
  {
    ofstream resourceStream(getConfigFileName().c_str());
    if (resourceStream)
      resourceStream << db;
  }

  ServerMenu::saveCache();
}

static bool		needsFullscreen()
{
  // fullscreen if not in a window
  if (!BZDB->isSet("_window")) return true;

  // not fullscreen if view is default (normal)
  if (!db.hasValue("view")) return false;

  // fullscreen if view is not default
  std::string value = db.getValue("view");
  for (int i = 1; i < (int)(sizeof(configViewValues) /
			sizeof(configViewValues[0])); i++)
    if (value == configViewValues[i])
      return true;

  // bogus view, default to normal so no fullscreen
  return false;
}

//
// main()
//	initialize application and enter event loop
//

#if defined(_WIN32)
int			myMain(int argc, char** argv)
#else /* defined(_WIN32) */
int			main(int argc, char** argv)
#endif /* defined(_WIN32) */
{
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

  // initialize global objects and classes
  bzfsrand(time(0));

  if (getenv("BZFLAGID")) {
    BZDB->set("callsign", getenv("BZFLAGID"));
    strncpy(startupInfo.callsign, getenv("BZFLAGID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  }
  else if (getenv("BZID")) {
    BZDB->set("callsign", getenv("BZID"));
    strncpy(startupInfo.callsign, getenv("BZID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  }
  time_t timeNow;
  time(&timeNow);
  userTime = *localtime(&timeNow);

  CommandsStandard::add();
  unsigned int i;

  // bind default keys
  for (i = 0; i < countof(bindingList); ++i) {
    CMDMGR->run(bindingList[i]);
  }

  // prepare DB entries
  for (i = 0; i < countof(defaultDBItems); ++i) {
    assert(defaultDBItems[i].name != NULL);
    if (defaultDBItems[i].value != NULL) {
      BZDB->set(defaultDBItems[i].name, defaultDBItems[i].value);
      BZDB->setDefault(defaultDBItems[i].name, defaultDBItems[i].value);
    }
    BZDB->setPersistent(defaultDBItems[i].name, defaultDBItems[i].persistent);
    BZDB->setPermission(defaultDBItems[i].name, defaultDBItems[i].permission);
  }

  // read resources
  {
    ifstream resourceStream(getConfigFileName().c_str(), ios::in);
    if (resourceStream) {
      startupInfo.hasConfiguration = true;
      resourceStream >> db;
      CFGMGR->read(getConfigFileName());
    }

#if !defined(_WIN32) & !defined(macintosh)
    else {
      ifstream resourceStream2(getConfigFileName2().c_str(), ios::in);
      if (resourceStream2) {
	startupInfo.hasConfiguration = true;
	resourceStream2 >> db;
        CFGMGR->read(getConfigFileName2());
      }
    }
#endif
  }

  ServerMenu::loadCache();

  // restore some configuration (command line overrides these)
  if (startupInfo.hasConfiguration) {
    if (BZDB->isSet("callsign")) {
      strncpy(startupInfo.callsign, BZDB->get("callsign").c_str(),
					sizeof(startupInfo.callsign) - 1);
      startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
    }
    if (BZDB->isSet("team")) {
      std::string value = BZDB->get("team");
      for (int i = 0; i < NumTeams; i++)
	if (value == Team::getName((TeamColor)i)) {
	  startupInfo.team = (TeamColor)i;
	  break;
	}
    }
    if (BZDB->isSet("server")) {
      strncpy(startupInfo.serverName, BZDB->get("server").c_str(),
					sizeof(startupInfo.serverName) - 1);
      startupInfo.serverName[sizeof(startupInfo.serverName) - 1] = '\0';
    }
    if (BZDB->isSet("port")) {
      startupInfo.serverPort = atoi(BZDB->get("port").c_str());
    }
    if (BZDB->isSet("interface")) {
      strncpy(startupInfo.multicastInterface, BZDB->get("interface").c_str(),
				sizeof(startupInfo.multicastInterface) - 1);
      startupInfo.multicastInterface[sizeof(startupInfo.multicastInterface) - 1] = '\0';
    }

    // key mapping
    BzfKeyMap& map = getBzfKeyMap();
    for (int i = 0; i < (int)BzfKeyMap::LastKey; i++) {
      BzfKeyMap::Key key = (BzfKeyMap::Key)i;
      const std::string name = BzfKeyMap::getKeyName(key);
      if (db.hasValue(name)) {
	// get saved value
	const std::string value = db.getValue(name);
	const char* const charValue = value.c_str();

	// find separator (forward slash) and get first and second values
	const char* const sep = strchr(charValue + 1, '/');
	const std::string value1 = sep ? value.substr(0, sep - charValue) : value;
	const std::string value2 = sep ? value.substr(sep - charValue + 1).c_str() : "";

	// lookup values
	BzfKeyEvent event1, event2;
	const bool okay1 = BzfKeyMap::translateStringToEvent(value1, event1);
	const bool okay2 = BzfKeyMap::translateStringToEvent(value2, event2);

	// set values
	if (okay1 || okay2) {
	  map.clear(key);
	  if (okay1) map.set(key, event1);
	  if (okay2) map.set(key, event2);
	}
      }
    }

    // check for reassigned team colors
    if (BZDB->isSet("roguecolor"))
      setTeamColor(RogueTeam, BZDB->get("roguecolor"));
    if (BZDB->isSet("redcolor"))
      setTeamColor(RedTeam, BZDB->get("redcolor"));
    if (BZDB->isSet("greencolor"))
      setTeamColor(GreenTeam, BZDB->get("greencolor"));
    if (BZDB->isSet("bluecolor"))
      setTeamColor(BlueTeam, BZDB->get("bluecolor"));
    if (BZDB->isSet("purplecolor"))
      setTeamColor(PurpleTeam, BZDB->get("purplecolor"));

    // check for reassigned radar colors
    if (BZDB->isSet("rogueradar"))
      setRadarColor(RogueTeam, BZDB->get("rogueradar"));
    if (BZDB->isSet("redradar"))
      setRadarColor(RedTeam, BZDB->get("redradar"));
    if (BZDB->isSet("greenradar"))
      setRadarColor(GreenTeam, BZDB->get("greenradar"));
    if (BZDB->isSet("blueradar"))
      setRadarColor(BlueTeam, BZDB->get("blueradar"));
    if (BZDB->isSet("purpleradar"))
      setRadarColor(PurpleTeam, BZDB->get("purpleradar"));


    // ignore window name in config file (it's used internally)
    BZDB->unset("_window");
    BZDB->unset("_multisample");
  }

  // use UDP? yes
  startupInfo.useUDPconnection=true;

  // parse arguments
  parse(argc, argv, db);

  // get email address if not anonymous
  std::string email;
  if (!anonymous) {
    if (db.hasValue("email"))
      email = db.getValue("email");
    else {
      email = anonymousName;
      const char* hostname = Address::getHostName();
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
      if (username && hostname) {
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
  setVisual(visual, db);

  // make the window
  BzfWindow* window = platformFactory->createWindow(display, visual);
  if (!window->isValid()) {
    printFatalError("Can't create window.  Exiting.");
    return 1;
  }
  window->setTitle("bzflag");

  /* initialize the joystick */
  if (BZDB->isTrue("joystick"))
    window->initJoystick(BZDB->get("joystickname").c_str());

  // set data directory if user specified
  if (db.hasValue("directory"))
    PlatformFactory::getMedia()->setMediaDirectory(db.getValue("directory"));

    else
    {
#if defined(__APPLE__)
	extern char *GetMacOSXDataPath(void);
	PlatformFactory::getMedia()->setMediaDirectory(GetMacOSXDataPath());
#elif (defined(_WIN32) || defined(WIN32))
	// What to put here?
#else
	// It's only checking existence of l10n directory
	DIR *localedir = opendir("data/l10n/");
	if (localedir == NULL)
	  PlatformFactory::getMedia()->setMediaDirectory(INSTALL_DATA_DIR);
	else {
	  closedir(localedir);
	}
#endif
    }

  // set window size (we do it here because the OpenGL context isn't yet bound)

  BundleMgr *bm = new BundleMgr(PlatformFactory::getMedia()->getMediaDirectory(), "bzflag");
  World::setBundleMgr(bm);

  std::string locale = (db.hasValue("locale")) ? db.getValue("locale") : "default";
  World::setLocale(locale);
  bm->getBundle(World::getLocale());

  bool setPosition = false, setSize = false;
  int x = 0, y = 0, w = 0, h = 0;
  if (db.hasValue("geometry")) {
    int count = 0;
    char xs, ys;
    std::string geometry = db.getValue("geometry");
    if (geometry == "default" ||
	((count = sscanf(geometry.c_str(), "%dx%d%c%d%c%d",
		&w, &h, &xs, &x, &ys, &y)) != 6 && count != 2) ||
	w < 0 || h < 0) {
      db.removeValue("geometry");
    }
    else if (count == 6 && ((xs != '-' && xs != '+') ||
				(ys != '-' && ys != '+'))) {
      db.removeValue("geometry");
    }
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
  }
  else if (setSize) {
    window->setSize(w, h);
  }
  else {
    window->setSize(640, 480);
  }
  if (setPosition)
    window->setPosition(x, y);

  // now make the main window wrapper.  this'll cause the OpenGL context
  // to be bound for the first time.
  MainWindow* pmainWindow = new MainWindow(window);
  MainWindow& mainWindow = *pmainWindow;
  // set fullscreen again so MainWindow object knows it's full screen
  if (useFullscreen)
    mainWindow.setFullscreen();

  // get sound files.  must do this after creating the window because
  // DirectSound is a bonehead API.
  if (!noAudio) {
    openSound("bzflag");
    if (startupInfo.hasConfiguration && BZDB->isSet("volume"))
      setSoundVolume(static_cast<int>(BZDB->eval("volume")));
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

  // if we're running on 3Dfx fullscreen add a fake cursor.
  // let the defaults file override this, though.
  if (!db.hasValue("fakecursor")) {
    // check that the renderer is Mesa Glide
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    if ((renderer != NULL) && (strncmp(renderer, "Mesa Glide", 10) == 0 ||
	strncmp(renderer, "3Dfx", 4) == 0))
      db.addValue("fakecursor", "1");
  }

  // set gamma if set in resources and we have gamma control
  if (BZDB->isSet("gamma")) {
    if (mainWindow.getWindow()->hasGammaControl())
      mainWindow.getWindow()->setGamma((float)atof(BZDB->get("gamma").c_str()));
  }

  // make scene renderer
  SceneRenderer renderer(mainWindow);

  // restore rendering configuration
  if (startupInfo.hasConfiguration) {
    if (db.hasValue("zbuffersplit"))
      renderer.setZBufferSplit(db.getValue("zbuffersplit") == "1");
    if (BZDB->isSet("texture")) {
      OpenGLTexture::setFilter(BZDB->get("texture"));
    }
    if (BZDB->isSet("quality")) {
      std::string value = BZDB->get("quality");
      for (int i = 0; i < (int)(sizeof(configQualityValues) /
				sizeof(configQualityValues[0])); i++)
	if (value == configQualityValues[i]) {
	  renderer.setQuality(i);
	  break;
	}
    }
    BZDB->set("_texturereplace", (!BZDB->isTrue("lighting") &&
	      renderer.useQuality() < 2) ? "1" : "0");
    BZDB->setPersistent("_texturereplace", false);
    if (db.hasValue("view")) {
      renderer.setViewType(SceneRenderer::Normal);
      std::string value = db.getValue("view");
      for (int i = 0; i < (int)(sizeof(configViewValues) /
				sizeof(configViewValues[0])); i++)
	if (value == configViewValues[i]) {
	  renderer.setViewType((SceneRenderer::ViewType)i);
	  break;
	}
    }

    if (BZDB->isSet("startcode"))
      ServerStartMenu::setSettings(BZDB->get("startcode").c_str());
    if (BZDB->isSet("showflaghelp"))
      renderer.setShowFlagHelp(BZDB->isTrue("showflaghelp"));
    if (BZDB->isSet("showscore"))
      renderer.setLabels(BZDB->isTrue("showlabels"));

    if (BZDB->isSet("panelopacity"))
      renderer.setPanelOpacity(BZDB->eval("panelopacity"));
    if (BZDB->isSet("radarsize"))
      renderer.setRadarSize(atoi(BZDB->get("radarsize").c_str()));
    if (BZDB->isSet("mouseboxsize"))
      renderer.setMaxMotionFactor(atoi(BZDB->get("mouseboxsize").c_str()));
    if (BZDB->isSet("underline"))
      OpenGLTexFont::setUnderlineColor(atoi(BZDB->get("underline").c_str()));
  }

  // grab the mouse only if allowed
  if (BZDB->isSet("mousegrab") && !BZDB->isTrue("mousegrab"))
    mainWindow.setNoMouseGrab();

  // set window quadrant
  if (renderer.getViewType() == SceneRenderer::ThreeChannel)
    mainWindow.setQuadrant(MainWindow::UpperRight);
  else if (renderer.getViewType() == SceneRenderer::Stacked)
    mainWindow.setQuadrant(MainWindow::LowerHalf);
#ifndef USE_GL_STEREO
  else if (renderer.getViewType() == SceneRenderer::Stereo)
    mainWindow.setQuadrant(MainWindow::UpperRight);
#endif

  // set server list URL
  if (BZDB->isSet("list"))
    startupInfo.listServerURL = BZDB->get("list");

  // setup silence list
  std::vector<std::string>& list = getSilenceList();

  // search for entries silencedPerson0 silencedPerson1 etc..
  // to the database. Stores silenceList
  // By only allowing up to a certain # of people can prevent
  // the vague chance of buffer overrun.
  const int bufferLength = 30;
  int maxListSize = 1000000; // do even that many play bzflag?
  char buffer [bufferLength];
  bool keepGoing = true;

  for (int i = 0; keepGoing && (i < maxListSize); i++) {
    sprintf(buffer,"silencedPerson%d",i); // could do %-10d

    if (BZDB->isSet(buffer)) {
      list.push_back(BZDB->get(buffer));
      // remove the value from the database so when we save
      // it saves the list's new values in order
      BZDB->unset(buffer);
    } else
      keepGoing = false;
  }

  if (BZDB->isSet("serverCacheAge")) {
    ServerMenu::setMaxCacheAge(atoi(BZDB->get("serverCacheAge").c_str()));
  }

  // start playing
  startPlaying(display, renderer, db, &startupInfo);

  // save resources
  dumpResources(display, renderer);
  CFGMGR->write(getConfigFileName());

  // shut down
  display->setDefaultResolution();
  delete pmainWindow;
  delete window;
  delete visual;
  closeSound();
  delete display;
  delete platformFactory;
  delete bm;

  // clean up singletons
  delete FILEMGR;
  delete CMDMGR;
  delete BZDB;

  return 0;
}

#if defined(_WIN32)

//
// WinMain()
//	windows entry point.  forward to main()
//

int WINAPI		WinMain(HINSTANCE, HINSTANCE, LPSTR _cmdLine, int)
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

  // get path to application.  this is ridiculously complex.
  char* appName;
  LPCTSTR cmdLine2 = GetCommandLine();
  if (cmdLine2[0] == '\"') {
    // quoted
    cmdLine2++;
    LPCTSTR argv0End = cmdLine2;
    while (*argv0End && *argv0End != '\"') argv0End++;
    const int len = argv0End - cmdLine2;
    appName = new char[len + 1];
    for (int i = 0; i < len; i++)
      appName[i] = (char)cmdLine2[i];
    appName[len] = '\0';
  }
  else {
    // not quoted
    LPCTSTR argv0End = cmdLine2;
    while (*argv0End && !isspace(*argv0End)) argv0End++;
    const int len = argv0End - cmdLine2;
    appName = new char[len + 1];
    for (int i = 0; i < len; i++)
      appName[i] = (char)cmdLine2[i];
    appName[len] = '\0';
  }

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

  // startup winsock
  {
    static const int major = 2, minor = 2;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
      printFatalError("Failed to initialize winsock.  Terminating.\n");
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      printFatalError("Version mismatch in winsock;"
			"  got %d.%d.  Terminating.\n",
			(int)LOBYTE(wsaData.wVersion),
			(int)HIBYTE(wsaData.wVersion));
      WSACleanup();
      return 1;
    }
  }

  const int exitCode = myMain(argc, argv);

  // clean up and return exit code
  WSACleanup();
  delete[] argv;
  delete[] appName;
  free(cmdLine);
  return exitCode;
}

#endif /* defined(_WIN32) */
// ex: shiftwidth=2 tabstop=8
