/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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
#else /* defined(_WIN32) */
#include <pwd.h>
#endif /* defined(_WIN32) */
#include <stdarg.h>
#include "bzfio.h"
#include <fstream.h>
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

#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfMedia.h"
#include "PlatformFactory.h"

const char*		argv0;
static boolean		anonymous = False;
static BzfString	anonymousName("anonymous");
static boolean		noAudio = False;
struct tm		userTime;
static StartupInfo	startupInfo;
static ResourceDatabase	db;
boolean			echoToConsole = False;

static BzfDisplay*	display = NULL;

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
  MessageBox(NULL, buffer, "BZFLAG Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
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
static const char*	configFilterValues[] = {
				"no",
				"nearest",
				"linear",
				"nearestmipmapnearest",
				"linearmipmapnearest",
				"nearestmipmaplinear",
				"linearmipmaplinear"
			};

static BzfString	getConfigFileName()
{
#if !defined(_WIN32)

  BzfString name;
  struct passwd* pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += BzfString(pwent->pw_dir);
    name += "/";
  }
  name += ".bzflag";

  // add in hostname on UNIX
  if (getenv("HOST")) {
    name += ".";
    name += getenv("HOST");
  }

  return name;

#else /* !defined(_WIN32) */

  // get location of personal files from system.  this appears to be
  // the closest thing to a home directory on windows.  use root of
  // C drive as a default in case we can't get the path or it doesn't
  // exist.
  BzfString name("C:");
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

#endif /* !defined(_WIN32) */
}

#if !defined(_WIN32)
static BzfString	getConfigFileName2()
{
  BzfString name;
  struct passwd* pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += BzfString(pwent->pw_dir);
    name += "/";
  }
  name += ".bzflag";
  return name;
}
#endif

static void		setTeamColor(TeamColor team, const BzfString& value)
{
  float color[3];
  if (sscanf((const char*)value, "%f %f %f", color+0, color+1, color+2) != 3)
    return;
  if (color[0] < 0.0f) color[0] = 0.0f;
  else if (color[0] > 1.0f) color[0] = 1.0f;
  if (color[1] < 0.0f) color[1] = 0.0f;
  else if (color[1] > 1.0f) color[1] = 1.0f;
  if (color[2] < 0.0f) color[2] = 0.0f;
  else if (color[2] > 1.0f) color[2] = 1.0f;
  Team::setColors(team, color, Team::getRadarColor(team));
}

static void		setRadarColor(TeamColor team, const BzfString& value)
{
  float color[3];
  if (sscanf((const char*)value, "%f %f %f", color+0, color+1, color+2) != 3)
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
  visual->setDoubleBuffer(True);
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
  if (resources.hasValue("multisample"))
    visual->setMultisample(4);
#ifdef USE_GL_STEREO
  if (resources.hasValue("view") &&
	resources.getValue("view") == configViewValues[1])
    visual->setStereo(True);
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
	" [-geometry <geometry-spec>]"
	" [-interface <interface>]"
	" [-latitude <latitude>] [-longitude <longitude>]"
	" [-list <server-list-url>] [-nolist]"
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
      anonymous = True;
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
      echoToConsole = True;
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
	resources.removeValue("list");
      }
      else {
	startupInfo.listServerURL = argv[i];
	resources.addValue("list", argv[i]);
      }
    }
    else if (strcmp(argv[i], "-nolist") == 0) {
      startupInfo.listServerURL = "";
      resources.addValue("list", "");
    }
    else if (strcmp(argv[i], "-m") == 0 ||
		strcmp(argv[i], "-mute") == 0) {
      noAudio = True;
    }
    else if (strcmp(argv[i], "-multisample") == 0) {
      resources.addValue("multisample", "");
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
      if (numRobotTanks < 1 || numRobotTanks > 20) {
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
    else if (strcmp(argv[i], "-v") == 0 ||
	     strcmp(argv[i], "-version") == 0 ||
	     strcmp(argv[i], "--version") == 0) {
#if defined(ALPHA_RELEASE) || defined(BETA_RELEASE)
      printFatalError("BZFLAG client, version %d.%d%c build %d %s\n"
#else
      printFatalError("BZFLAG client, version %d.%d%c\n"
#endif
		"  protocol %c.%d%c",
		(VERSION / 10000000) % 100,
		(VERSION / 100000) % 100,
		(char)('a' - 1 + (VERSION / 1000) % 100),
#if defined(ALPHA_RELEASE) || defined(BETA_RELEASE)
		VERSION % 1000,
#if defined(ALPHA_RELEASE)
		"Alpha",
#elif defined(BETA_RELEASE)
		"Beta",
#else
		"",
#endif
#endif
		ServerVersion[4],
		atoi(ServerVersion + 5),
		ServerVersion[7]);
#if 0
      cout << "BZFLAG client, version " <<
		(VERSION / 10000000) % 100 << "." <<
		(VERSION / 100000) % 100 <<
		(char)('a' - 1 + (VERSION / 1000) % 100) <<
#if defined(ALPHA_RELEASE) || defined(BETA_RELEASE)
		" build " << VERSION % 1000 <<
#endif
#if defined(ALPHA_RELEASE)
		"Alpha" <<
#elif defined(BETA_RELEASE)
		"Beta" <<
#else
		"" <<
#endif
		endl;

      cout << "  protocol " << ServerVersion[4] << ".";
      if (ServerVersion[5] != '0') cout << ServerVersion[5];
      cout << ServerVersion[6] << (char)tolower(ServerVersion[7]) << endl;
#endif

      exit(0);
    }
    else if (strcmp(argv[i], "-window") == 0) {
      resources.addValue("window", "");
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
	resources.addValue("zbuffer", "yes");
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
    else if (i == argc-1) {
      if (strlen(argv[i]) >= sizeof(startupInfo.serverName)) {
	printFatalError("Server name too long.  Ignoring.");	
      }
      else {
	strcpy(startupInfo.serverName, argv[i]);
	startupInfo.autoConnect = True;
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
  db.addValue("callsign", startupInfo.callsign);
  db.addValue("team", Team::getName(startupInfo.team));
  db.addValue("server", startupInfo.serverName);
  if (startupInfo.serverPort != ServerPort) {
    char buf[20];
    sprintf(buf, "%d", startupInfo.serverPort);
    db.addValue("port", buf);
  }
  else {
    db.removeValue("port");
  }
  if (strlen(startupInfo.multicastInterface) != 0)
    db.addValue("interface", startupInfo.multicastInterface);
  db.addValue("list", startupInfo.listServerURL);
  if (isSoundOpen()) {
    char buf[20];
    sprintf(buf, "%d", getSoundVolume());
    db.addValue("volume", buf);
  }
  db.addValue("dither",   renderer.useDithering() ? "yes" : "no");
  db.addValue("blend",    renderer.useBlending() ? "yes" : "no");
  db.addValue("smooth",   renderer.useSmoothing() ? "yes" : "no");
  db.addValue("lighting", renderer.useLighting() ? "yes" : "no");
  db.addValue("texture",  renderer.useTexture() ?
 		configFilterValues[OpenGLTexture::getFilter()] : "no");
  db.addValue("shadows",  renderer.useShadows() ? "yes" : "no");

  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0)
    db.addValue("zbuffer",  "disable");
  else
    db.addValue("zbuffer",  renderer.useZBuffer() ? "yes" : "no");

  if (renderer.getWindow().getWindow()->hasGammaControl()) {
    char buf[20];
    sprintf(buf, "%f", renderer.getWindow().getWindow()->getGamma());
    db.addValue("gamma", buf);
  }

  db.addValue("quality", configQualityValues[renderer.useQuality()]);
  if (display->getResolution() != -1 &&
      display->getResolution(display->getResolution()))
    db.addValue("resolution",
	display->getResolution(display->getResolution())->name);
  {
    char buf[20];
    sprintf(buf, "%f", renderer.getLatitude());
    db.addValue("latitude", buf);
    sprintf(buf, "%f", renderer.getLongitude());
    db.addValue("longitude", buf);
  }
  {
    KeyMap& map = getKeyMap();
    for (int i = 0; i < (int)KeyMap::LastKey; i++) {
      // get value string
      const BzfKeyEvent& key1 = map.get((KeyMap::Key)i);
      BzfString value;
      if (key1.ascii != 0 || key1.button != 0) {
	value = KeyMap::getKeyEventString(key1);
	const BzfKeyEvent& key2 = map.getAlternate((KeyMap::Key)i);
	if (key2.ascii != 0 || key2.button != 0) {
	  value += "/";
	  value += KeyMap::getKeyEventString(key2);
	}
      }

      // add it
      const BzfString name = KeyMap::getKeyName((KeyMap::Key)i);
      db.addValue(name, value);
    }
  }
  db.addValue("startcode", ServerStartMenu::getSettings());

  // don't save these configurations
  db.removeValue("window");
  db.removeValue("multisample");

  // save configuration
  {
    ofstream resourceStream(getConfigFileName());
    if (resourceStream)
      resourceStream << db;
  }
}

static boolean		needsFullscreen()
{
  // fullscreen if not in a window
  if (!db.hasValue("window")) return True;

  // not fullscreen if view is default (normal)
  if (!db.hasValue("view")) return False;

  // fullscreen if view is not default
  BzfString value = db.getValue("view");
  for (int i = 1; i < (int)(sizeof(configViewValues) /
			sizeof(configViewValues[0])); i++)
    if (value == configViewValues[i])
      return True;

  // bogus view, default to normal so no fullscreen
  return False;
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
    strncpy(startupInfo.callsign, getenv("BZFLAGID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  }
  else if (getenv("BZID")) {
    strncpy(startupInfo.callsign, getenv("BZID"),
					sizeof(startupInfo.callsign) - 1);
    startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
  }
  time_t timeNow;
  time(&timeNow);
  userTime = *localtime(&timeNow);

  // read resources
  {
    ifstream resourceStream(getConfigFileName(), ios::in | ios::nocreate);
    if (resourceStream) {
      startupInfo.hasConfiguration = True;
      resourceStream >> db;
    }

#if !defined(_WIN32)
    else {
      ifstream resourceStream2(getConfigFileName2(), ios::in | ios::nocreate);
      if (resourceStream2) {
	startupInfo.hasConfiguration = True;
	resourceStream2 >> db;
      }
    }
#endif
  }

  // restore some configuration (command line overrides these)
  if (startupInfo.hasConfiguration) {
    if (db.hasValue("callsign")) {
      strncpy(startupInfo.callsign, db.getValue("callsign"),
					sizeof(startupInfo.callsign) - 1);
      startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
    }
    if (db.hasValue("team")) {
      BzfString value = db.getValue("team");
      for (int i = 0; i < NumTeams; i++)
	if (value == Team::getName((TeamColor)i)) {
	  startupInfo.team = (TeamColor)i;
	  break;
	}
    }
    if (db.hasValue("server")) {
      strncpy(startupInfo.serverName, db.getValue("server"),
					sizeof(startupInfo.serverName) - 1);
      startupInfo.serverName[sizeof(startupInfo.serverName) - 1] = '\0';
    }
    if (db.hasValue("port")) {
      startupInfo.serverPort = atoi(db.getValue("port"));
    }
    if (db.hasValue("interface")) {
      strncpy(startupInfo.multicastInterface, db.getValue("interface"),
				sizeof(startupInfo.multicastInterface) - 1);
      startupInfo.multicastInterface[
			sizeof(startupInfo.multicastInterface) - 1] = '\0';
    }

    // key mapping
    KeyMap& map = getKeyMap();
    for (int i = 0; i < (int)KeyMap::LastKey; i++) {
      KeyMap::Key key = (KeyMap::Key)i;
      const BzfString name = KeyMap::getKeyName(key);
      if (db.hasValue(name)) {
	// get saved value
	const BzfString value = db.getValue(name);
	const char* const charValue = value;

	// find separator (forward slash) and get first and second values
	const char* const sep = strchr(charValue + 1, '/');
	const BzfString value1 = sep ? value(0, sep - charValue) : value;
	const BzfString value2 = sep ? value(sep - charValue + 1).getString() : "";

	// lookup values
	BzfKeyEvent event1, event2;
	const boolean okay1 = KeyMap::translateStringToEvent(value1, event1);
	const boolean okay2 = KeyMap::translateStringToEvent(value2, event2);

	// set values
	if (okay1 || okay2) {
	  map.clear(key);
	  if (okay1) map.set(key, event1);
	  if (okay2) map.set(key, event2);
	}
      }
    }

    // check for reassigned team colors
    if (db.hasValue("redcolor"))
      setTeamColor(RedTeam, db.getValue("redcolor"));
    if (db.hasValue("greencolor"))
      setTeamColor(GreenTeam, db.getValue("greencolor"));
    if (db.hasValue("bluecolor"))
      setTeamColor(BlueTeam, db.getValue("bluecolor"));
    if (db.hasValue("purplecolor"))
      setTeamColor(PurpleTeam, db.getValue("purplecolor"));

    // check for reassigned radar colors
    if (db.hasValue("redradar"))
      setRadarColor(RedTeam, db.getValue("redradar"));
    if (db.hasValue("greenradar"))
      setRadarColor(GreenTeam, db.getValue("greenradar"));
    if (db.hasValue("blueradar"))
      setRadarColor(BlueTeam, db.getValue("blueradar"));
    if (db.hasValue("purpleradar"))
      setRadarColor(PurpleTeam, db.getValue("purpleradar"));

    // ignore window name in config file (it's used internally)
    db.removeValue("window");
    db.removeValue("multisample");
  }

  // parse arguments
  parse(argc, argv, db);

  // get email address if not anonymous
  BzfString email = anonymousName;
  if (!anonymous) {
    const char* hostname = Address::getHostName();
#if !defined(_WIN32)
    struct passwd* pwent = getpwuid(getuid());
    const char* username = pwent ? pwent->pw_name : NULL;
#else /* !defined(_WIN32) */
    char username[256];
    DWORD usernameLen = sizeof(username);
    GetUserName(username, &usernameLen);
#endif /* !defined(_WIN32) */
    if (username && hostname) {
      email = username;
      email += "@";
      email += hostname;
    }
  }
  email.truncate(sizeof(startupInfo.email) - 1);
  strcpy(startupInfo.email, email);

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

  // set data directory if user specified
  if (db.hasValue("directory"))
    PlatformFactory::getMedia()->setMediaDirectory(db.getValue("directory"));

  // set window size (we do it here because the OpenGL context isn't yet bound)
  boolean setPosition = False, setSize = False;
  int x = 0, y = 0, w = 0, h = 0;
  if (db.hasValue("geometry")) {
    int count = 0;
    char xs, ys;
    BzfString geometry = db.getValue("geometry");
    if (geometry == "default" ||
	((count = sscanf(geometry, "%dx%d%c%d%c%d",
		&w, &h, &xs, &x, &ys, &y)) != 6 && count != 2) ||
	w < 0 || h < 0) {
      db.removeValue("geometry");
    }
    else if (count == 6 && ((xs != '-' && xs != '+') ||
				(ys != '-' && ys != '+'))) {
      db.removeValue("geometry");
    }
    else {
      setSize = True;
      if (w < 640) w = 640;
      if (h < 400) h = 400;
      if (count == 6) {
	if (xs == '-') x = display->getWidth() - x - w;
	if (ys == '-') y = display->getHeight() - y - h;
	setPosition = True;
      }

      // must call this before setFullscreen() is called
      display->setPassthroughSize(w, h);
    }
  }

  // set window size (we do it here because the OpenGL context isn't yet
  // bound and 3Dfx passthrough cards use the window size to determine
  // the resolution to use)
  const boolean useFullscreen = needsFullscreen();
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
    if (startupInfo.hasConfiguration && db.hasValue("volume"))
      setSoundVolume(atoi(db.getValue("volume")));
  }

  // set main window's minimum size (arbitrary but should be big enough
  // to see stuff in control panel)
  mainWindow.setMinSize(640, 120, 360);

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
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  OpenGLGState::init();

  // if we're running on 3Dfx fullscreen add a fake cursor.
  // let the defaults file override this, though.
  if (!db.hasValue("fakecursor")) {
    // check that the renderer is Mesa Glide
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    if (strncmp(renderer, "Mesa Glide", 10) == 0 ||
	strncmp(renderer, "3Dfx", 4) == 0)
      db.addValue("fakecursor", "yes");
  }

  // set gamma if set in resources and we have gamma control
  if (db.hasValue("gamma")) {
    if (mainWindow.getWindow()->hasGammaControl())
      mainWindow.getWindow()->setGamma((float)atof(db.getValue("gamma")));
  }

  // make scene renderer
  SceneRenderer renderer(mainWindow);

  // restore rendering configuration
  if (startupInfo.hasConfiguration) {
    if (db.hasValue("dither"))
      renderer.setDithering(db.getValue("dither") == "yes");
    if (db.hasValue("blend"))
      renderer.setBlending(db.getValue("blend") == "yes");
    if (db.hasValue("smooth"))
      renderer.setSmoothing(db.getValue("smooth") == "yes");
    if (db.hasValue("lighting"))
      renderer.setLighting(db.getValue("lighting") == "yes");
    if (db.hasValue("shadows"))
      renderer.setShadows(db.getValue("shadows") == "yes");
    if (db.hasValue("zbuffer"))
      renderer.setZBuffer(db.getValue("zbuffer") == "yes");
    if (db.hasValue("zbuffersplit"))
      renderer.setZBufferSplit(db.getValue("zbuffersplit") == "yes");
    if (db.hasValue("texture")) {
      BzfString value = db.getValue("texture");
      for (int i = 0; i < (int)(sizeof(configFilterValues) /
				sizeof(configFilterValues[0])); i++)
	if (value == configFilterValues[i]) {
	  OpenGLTexture::setFilter((OpenGLTexture::Filter)i);
	  break;
	}
      renderer.setTexture(OpenGLTexture::getFilter() != OpenGLTexture::Off);
    }
    if (db.hasValue("quality")) {
      BzfString value = db.getValue("quality");
      for (int i = 0; i < (int)(sizeof(configQualityValues) /
				sizeof(configQualityValues[0])); i++)
	if (value == configQualityValues[i]) {
	  renderer.setQuality(i);
	  break;
	}
    }
    renderer.setTextureReplace(!renderer.useLighting() &&
				renderer.useQuality() < 2);
    if (db.hasValue("view")) {
      renderer.setViewType(SceneRenderer::Normal);
      BzfString value = db.getValue("view");
      for (int i = 0; i < (int)(sizeof(configViewValues) /
				sizeof(configViewValues[0])); i++)
	if (value == configViewValues[i]) {
	  renderer.setViewType((SceneRenderer::ViewType)i);
	  break;
	}
    }

    if (db.hasValue("maxlod"))
      renderer.setMaxLOD(atoi(db.getValue("maxlod")));

    if (db.hasValue("latitude"))
      renderer.setLatitude((float)atof(db.getValue("latitude")));
    if (db.hasValue("longitude"))
      renderer.setLongitude((float)atof(db.getValue("longitude")));

    if (db.hasValue("startcode"))
      ServerStartMenu::setSettings(db.getValue("startcode"));
  }

  // grab the mouse only if allowed
  if (db.hasValue("mousegrab") && db.getValue("mousegrab") == "no")
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
  if (db.hasValue("list"))
    startupInfo.listServerURL = db.getValue("list");

  // start playing
  startPlaying(display, renderer, db, &startupInfo);

  // save resources
  dumpResources(display, renderer);

  // shut down
  display->setDefaultResolution();
  delete pmainWindow;
  delete window;
  delete visual;
  closeSound();
  delete display;
  delete platformFactory;

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

  // run the app
  const int exitCode = myMain(argc, argv);

  // clean up and return exit code
  WSACleanup();
  delete[] argv;
  delete[] appName;
  free(cmdLine);
  return exitCode;
}

#endif /* defined(_WIN32) */
