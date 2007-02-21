/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// get our interface
#include "bzflag.h"

/* system headers */
#include <assert.h>
#include <ctype.h>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <vector>
#if defined(_WIN32)
#  include <shlobj.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#endif /* defined(_WIN32) */

/* common headers */
#include "Address.h"
#include "BZDBCache.h"
#include "BZDBLocal.h"
#include "BundleMgr.h"
#include "BzfMedia.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "CommandManager.h"
#include "CommandsStandard.h"
#include "ConfigFileManager.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "FontManager.h"
#include "GUIOptionsMenu.h"
#include "KeyManager.h"
#include "OSFile.h"
#include "OpenGLGState.h"
#include "ParseColor.h"
#include "PlatformFactory.h"
#include "Protocol.h"
#include "ServerListCache.h"
#include "StateDatabase.h"
#include "Team.h"
#include "TextUtils.h"
#include "TextureManager.h"
#include "TimeBomb.h"
#include "WordFilter.h"
#include "World.h"
#include "bzfSDL.h"
#include "bzfgl.h"
#include "bzfio.h"

/* local headers */
#include "ActionBinding.h"
#include "ServerStartMenu.h"
#include "callbacks.h"
#include "playing.h"
#include "sound.h"
#include "playing.h"

// invoke incessant rebuilding for build versioning
#include "version.h"

// defaults for bzdb
#include "defaultBZDB.h"

// client prefrences
#include "clientConfig.h"

CS_IMPLEMENT_APPLICATION

int beginendCount = 0;

const char*		argv0;
static bool		anonymous = false;
static std::string	anonymousName("anonymous");
std::string		alternateConfig;
static bool		noAudio = false;
struct tm		userTime;
bool			echoToConsole = false;
bool			echoAnsi = false;
int			debugLevel = 0;

#ifdef ROBOT
// ROBOT -- tidy up
int numRobotTanks = 0;
#endif


//
// application initialization
//

// so that Windows can kill the wsa stuff if needed
int bail ( int returnCode )
{
#ifdef _WIN32
	WSACleanup();
#endif
	return returnCode;
}

static void		setTeamColor(TeamColor team, const std::string& str)
{
  float color[4];
  parseColorString(str, color);
  // don't worry about alpha, Team::setColors() doesn't use it
  Team::setColors(team, color, Team::getRadarColor(team));
}

static void		setRadarColor(TeamColor team, const std::string& str)
{
  float color[4];
  parseColorString(str, color);
  // don't worry about alpha, Team::setColors() doesn't use it
  Team::setColors(team, Team::getTankColor(team), color);
}

Bzflag::Bzflag() : filter(NULL), pmainWindow(NULL),
		   window(NULL), visual(NULL), platformFactory(NULL),
		   bm(NULL), playing(NULL)
{
  SetApplicationName ("BZFlag");
}

Bzflag::~Bzflag()
{
}

void Bzflag::OnCommandLineHelp()
{
  csPrintf(" [-3dfx] [-no3dfx]\n");
  csPrintf(" [-anonymous]\n");
  csPrintf(" [-badwords=<filterfile>]\n");
  csPrintf(" [-config=<configfile>]\n");
  csPrintf(" [-configdir=<config dir name>]\n");
  csPrintf(" [-d | -debug]\n");
  csPrintf(" [-date=mm/dd/yyyy]\n");
  csPrintf(" [{-dir | -directory}=<data-directory>]\n");
  csPrintf(" [-e | -echo]\n");
  csPrintf(" [-ea | -echoAnsi]\n");
  csPrintf(" [{-g | -geometry}=<geometry-spec>]\n");
  csPrintf(" [-latitude=<latitude>] [-longitude=<longitude>]\n");
  csPrintf(" [-list=<list-server-url>] [-nolist]\n");
  csPrintf(" [-locale=<locale>]\n");
  csPrintf(" [-m | -mute]\n");
  csPrintf(" [-motd=<motd-url>] [-nomotd]\n");
  csPrintf(" [-multisample]\n");
#ifdef ROBOT
  csPrintf(" [-solo=<num-robots>]\n");
#endif
  csPrintf(" [-team={red|green|blue|purple|rogue|observer}]\n");
  csPrintf(" [-time=hh:mm:ss] [-notime]\n");
  csPrintf(" [-v | -version | --version]\n");
  csPrintf(" [-view={normal|stereo|stacked|three|anaglyph|interlaced}]\n");
  csPrintf(" [-window]\n");
  csPrintf(" [-zoom=<zoom-factor>]\n");
  csPrintf(" [callsign[:password]@]server[:port]\n");
}

void Bzflag::parse()
{
  if (clp->GetOption("anonymous"))
    anonymous = true;
  if (clp->GetOption("e") || clp->GetOption("echo"))
    echoToConsole = true;
  if (clp->GetOption("ea") || clp->GetOption("echoAnsi")) {
    echoToConsole = true;
    echoAnsi = true;
  }
  if (clp->GetOption("nomotd"))
    BZDB.set("disableMOTD", "1");
  if (clp->GetOption("nolist")) {
    startupInfo.listServerURL = "";
    BZDB.set("list", "");
  }
  if (clp->GetOption("m") || clp->GetOption("mute"))
    noAudio = true;
  if (clp->GetOption("multisample"))
    BZDB.set("_multisample", "1");
  if (clp->GetOption("v") || clp->GetOption("version")) {
    ReportInfo("BZFlag client %s (protocol %s) http://BZFlag.org/\n%s",
	       getAppVersion(),
	       getProtocolVersion(),
	       bzfcopyright);
    bail(0);
    exit(0);
  }
  if (clp->GetOption("window"))
    BZDB.set("_window", "1");
  if (clp->GetOption("3dfx") || clp->GetOption("3Dfx")) {
#if !defined(__linux__)
    putenv("MESA_GLX_FX=fullscreen");
#else
    setenv("MESA_GLX_FX", "fullscreen", 1);
#endif
  }
  if (clp->GetOption("no3dfx") || clp->GetOption("no3Dfx")) {
#if !defined(__linux__)
    putenv("MESA_GLX_FX=");
#else
    unsetenv("MESA_GLX_FX");
#endif
  }
  size_t optionCount; 
  for (optionCount = 0; clp->GetOption("d", optionCount); optionCount++)
    debugLevel++;
  for (optionCount = 0; clp->GetOption("debug", optionCount); optionCount++)
    debugLevel++;

  const char *directory = clp->GetOption("dir");
  if (!directory)
    directory = clp->GetOption("directory");
  if (directory)
    if (strlen(directory) == 0)
      BZDB.unset("directory");
    else
      BZDB.set("directory", directory);

  const char *geometry = clp->GetOption("g");
  if (!geometry)
    geometry = clp->GetOption("geometry");
  if (geometry) {
    int w, h, x, y, count;
    char xs = '+', ys = '+';
    if (strcmp(geometry, "default") != 0 &&
	(((count = sscanf(geometry, "%dx%d%c%d%c%d",
			  &w, &h, &xs, &x, &ys, &y)) != 6 && count != 2) ||
	 (xs != '-' && xs != '+') || (ys != '-' && ys != '+')))
      ReportWarning("Invalid argument for geometry.\n"
		    "Correct format is <width>x<height>[+|-]<x>[+|-]<y>.");
    BZDB.set("geometry", geometry);
  }

  const char *latitudeC = clp->GetOption("latitude");
  if (latitudeC) {
    double latitude = atof(latitudeC);
    if (latitude < -90.0 || latitude > 90.0)
      ReportWarning("Invalid argument for latitude.");
    BZDB.set("latitude", latitudeC);
  }

  const char *longitudeC = clp->GetOption("longitude");
  if (longitudeC) {
    double longitude = atof(longitudeC);
    if (longitude < -180.0 || longitude > 180.0)
      ReportWarning("Invalid argument for longitude.");
    BZDB.set("longitude", longitudeC);
  }

  const char *list = clp->GetOption("list");
  if (list)
    if (strcmp(list, "default") == 0) {
      BZDB.set("list", BZDB.getDefault("list"));
    } else {
      startupInfo.listServerURL = list;
      BZDB.set("list", list);
    }

  const char *locale = clp->GetOption("locale");
  if (locale)
    BZDB.set("locale", locale);
  
  const char *motd = clp->GetOption("motd");
  if (motd) {
    if (strcmp(motd, "default") == 0) {
      BZDB.set("motdServer", BZDB.getDefault("motdServer"));
    } else {
      BZDB.set("motdServer", motd);
    }
    BZDB.unset("disableMOTD");
  }

#ifdef ROBOT
  const char *solo = clp->GetOption("solo");
  if (solo) {
    numRobotTanks = atoi(solo);
    if (numRobotTanks < 1 || numRobotTanks > MAX_ROBOTS) {
      ReportWarning("Invalid argument for solo.");
    }
  }
#endif

  const char *team = clp->GetOption("team");
  if (team) {
    if ((strcmp(team, "a") == 0) ||
	(strcmp(team, "auto") == 0) ||
	(strcmp(team, "automatic") == 0)) {
      startupInfo.team = AutomaticTeam;
    } else if (strcmp(team, "r") == 0 || strcmp(team, "red") == 0) {
      startupInfo.team = RedTeam;
    } else if (strcmp(team, "g") == 0 || strcmp(team, "green") == 0) {
      startupInfo.team = GreenTeam;
    } else if (strcmp(team, "b") == 0 || strcmp(team, "blue") == 0) {
      startupInfo.team = BlueTeam;
    } else if (strcmp(team, "p") == 0 || strcmp(team, "purple") == 0) {
      startupInfo.team = PurpleTeam;
    } else if (strcmp(team, "z") == 0 || strcmp(team, "rogue") == 0) {
      startupInfo.team = RogueTeam;
    } else if (strcmp(team, "o") == 0 || strcmp(team, "observer") == 0) {
      startupInfo.team = ObserverTeam;
    } else {
      ReportWarning("Invalid argument for team.");
    }
  }

#ifdef DEBUG
  const char *date = clp->GetOption("date");
  if (date) {
    int month, day, year;
    // FIXME: should use iso yyyy.mm.dd format
    if (sscanf(date, "%d/%d/%d", &month, &day, &year) != 3
	// FIXME -- upper limit loose
	|| day < 1 || day > 31
	|| month < 1 || month > 12
	|| (year < 0 || (year > 100 && (year < 1970 || year > 2100)))) {
      ReportWarning("Invalid argument for date.");
    }
    if (year > 100)
      year  = year - 1900;
    else if (year < 70)
      year += 100;
    userTime.tm_mday = day;
    userTime.tm_mon  = month - 1;
    userTime.tm_year = year;
  }
#endif

  const char *fixedTime = clp->GetOption("time");
  if (fixedTime)
    BZDB.set("fixedTime", fixedTime);
  
  if (clp->GetOption("notime"))
    BZDB.unset("fixedTime");

  const char *view = clp->GetOption("view");
  if (view)
    BZDB.set("view", view);

  const char *displayZoom = clp->GetOption("zoom");
  if (displayZoom) {
    const int zoom = atoi(displayZoom);
    if (zoom < 1 || zoom > 8) {
      ReportWarning("Invalid argument for zoom.");
    }
    BZDB.set("displayZoom", displayZoom);
  }

  const char *zbuffer = clp->GetOption("zbuffer");
  if (zbuffer) {
    if (strcmp(zbuffer, "on") == 0) {
      BZDB.set("zbuffer", "1");
    } else if (strcmp(zbuffer, "off") == 0) {
      BZDB.set("zbuffer", "disable");
    } else {
      ReportWarning("Invalid argument for zbuffer.");
    }
  }

  const char *eyesep = clp->GetOption("eyesep");
  if (eyesep)
    BZDB.set("eyesep", eyesep);

  const char *focal = clp->GetOption("focal");
  if (focal)
    BZDB.set("focal", focal);

  const char *filterFilename = clp->GetOption("badwords");
  if (filterFilename)
    if (strlen(filterFilename))
      BZDB.set("filterFilename", filterFilename, StateDatabase::ReadOnly);
    else
      ReportWarning("Missing bad word filter file.");

  const char *argumentName = clp->GetName();
  if (argumentName) {
    // find the beginning of the server name, parse the callsign
    const char* serverName;
    if ((serverName = strchr(argumentName, '@')) != NULL) {
      strncpy(startupInfo.callsign,
	      argumentName,
	      sizeof(startupInfo.callsign));
      if (serverName >= argumentName + sizeof(startupInfo.callsign)) {
	ReportWarning("Callsign truncated.");
	startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
      } else {
	startupInfo.callsign[serverName - argumentName] = '\0';
      }
      char* password;
      if ((password = strchr(startupInfo.callsign, ':')) != NULL) {
	*(strchr(startupInfo.callsign, ':')) = '\0';
	*password = '\0', ++password;
	if (strlen(argumentName) >= sizeof(startupInfo.password))
	  ReportWarning("Password truncated.");
	strncpy(startupInfo.password,
		password,
		sizeof(startupInfo.password) - 1);
	startupInfo.password[sizeof(startupInfo.password) - 1] = '\0';
      }
      ++serverName;
    } else {
      serverName = argumentName;
    }

    // find the beginning of the port number, parse it
    char* portNumber;
    if ((portNumber = strchr(serverName, ':')) != NULL) {
      *portNumber = '\0';
      ++portNumber;
      startupInfo.serverPort = atoi(portNumber);
      if (startupInfo.serverPort < 1 || startupInfo.serverPort > 65535) {
	startupInfo.serverPort = ServerPort;
	ReportWarning("Bad port, using default %d.", startupInfo.serverPort);
      }
    }
    if (strlen(serverName) >= sizeof(startupInfo.serverName)) {
      ReportWarning("Server name too long.  Ignoring.");
    } else {
      strcpy(startupInfo.serverName, serverName);
      startupInfo.autoConnect = true;
    }
  }
}

void Bzflag::parseConfigName()
{
  const char *configdir = clp->GetOption("configdir");
  if (configdir) {
    setCustomConfigDir(configdir);
    alternateConfig += configdir;
  }
  const char *config = clp->GetOption("config");
  if (config) {
    alternateConfig = getConfigDirName();
    alternateConfig += config;
  }
}

//
// resource database dumping.  used during initial startup to save
// preferences in case anything catastrophic goes wrong afterwards
// (so user won't have to wait through performance testing again).
//

void			dumpResources()
{
  // collect new configuration

  // only dump username and password if we're allowed to, otherwise
  // erase them if they exist
  if (BZDB.eval("saveIdentity") > 0)
    BZDB.set("callsign", startupInfo.callsign);
  else
    BZDB.set("callsign", "");
  if (BZDB.eval("saveIdentity") > 1)
    BZDB.set("password", startupInfo.password);
  else
    BZDB.set("password", "");

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

  if (RENDERER.getWindow().getWindow()->hasGammaControl()) {
    BZDB.set("gamma",
	     TextUtils::format("%f", RENDERER.getWindow().getWindow()->getGamma()));
  }

  BZDB.set("quality", configQualityValues[RENDERER.useQuality()]);
  BZDB.set("startcode", ServerStartMenu::getSettings());

  BZDB.set("panelopacity", TextUtils::format("%f", RENDERER.getPanelOpacity()));

  BZDB.set("radarsize", TextUtils::format("%d", RENDERER.getRadarSize()));

  BZDB.set("mouseboxsize", TextUtils::format("%d", RENDERER.getMaxMotionFactor()));

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
    sprintf(buffer, "silencedPerson%d", i);
    BZDB.set(TextUtils::format("silencedPerson%d", i), list[i]);
  }

  BZDB.set("email", startupInfo.email); // note email of zero length does not stick

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
  for (int i = 1; i < (int)configViewValues.size(); i++)
    if (value == configViewValues[i])
      return true;

  // bogus view, default to normal so no fullscreen
  return false;
}

static void createCacheSignature ()
{
  // This file is to be used by archiving and mirroring tools avoid
  // this directory (and any of its sub-directories). Please see:
  //	 < http://www.brynosaurus.com/cachedir/ >

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
  return csApplicationRunner<Bzflag>::Run (argc, argv);
}

bool Bzflag::OnInitialize(int argc, char *argv[])
{
  if (!RequestPlugins(GetObjectRegistry(),
		      CS_REQUEST_VFS,
		      CS_REQUEST_OPENGL3D,
		      CS_REQUEST_REPORTER,
		      CS_REQUEST_REPORTERLISTENER,
		      CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");
  // Attempt to load a joystick plugin.
  csRef<iStringArray> joystickClasses
    = iSCF::SCF->QueryClassList("crystalspace.device.joystick.");
  if (joystickClasses.IsValid()) {
    csRef<iPluginManager> plugmgr
      = CS_QUERY_REGISTRY(GetObjectRegistry(), iPluginManager);
    for (size_t i = 0; i < joystickClasses->Length(); i++) {
      const char *className = joystickClasses->Get(i);
      iBase *b = plugmgr->LoadPlugin(className);
      if (b != 0)
	b->DecRef();
    }
  }

  csBaseEventHandler::Initialize(GetObjectRegistry());
  if (!RegisterQueue(GetObjectRegistry(),
		     csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  argv0 = argv[0];

  return true;
}

bool Bzflag::Application()
{
  unsigned int i;

#ifdef _WIN32
  // startup winsock
  static const int major = 2, minor = 2;
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
    return ReportError("Failed to initialize winsock.\n");
  }
  if (LOBYTE(wsaData.wVersion) != major ||
      HIBYTE(wsaData.wVersion) != minor) {
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("Version mismatch in winsock;"
		       "  got %d.%d, expected %d.%d.\n",
		       (int)LOBYTE(wsaData.wVersion),
		       (int)HIBYTE(wsaData.wVersion),
		       major, minor);
  }
#endif

  clp = CS_QUERY_REGISTRY(GetObjectRegistry(), iCommandLineParser);

  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  g2d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics2D);

  filter = (WordFilter *)NULL;

  // init libs

  //init_packetcompression();

  // check time bomb
  if (timeBombBoom()) {
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("This release expired on %s.\n"
		       "Please upgrade to the latest release.\n",
		       timeBombString());
  }

#if defined(_WIN32)
  {
    /* write HKEY_CURRENT_USER\Software\BZFlag\CurrentRunningPath with the
     * current path.  this lets Xfire know that this bzflag.exe running from
     * here really is bzflag, not some imposter.
     * since it may be useful to someone else, it's not protected by USE_XFIRE
     */

    // get our path
    char temppath[MAX_PATH], temppath2[MAX_PATH];
    char tempdrive[10];
    GetModuleFileName(NULL, temppath, MAX_PATH);
    // strip filename/extension
    _splitpath(temppath, tempdrive, temppath2, NULL, NULL);
    _makepath(temppath, tempdrive, temppath2, NULL, NULL);

    // write the registry key in question
    HKEY key = NULL;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\BZFlag", 
        0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, 
	&key, NULL) == ERROR_SUCCESS) {
      RegSetValueEx(key, "CurrentRunningPath", 0, REG_SZ, (LPBYTE)temppath,
		   (DWORD)strlen(temppath));
    }
  }
#endif

  createCacheSignature();

  // initialize global objects and classes
  bzfsrand(time(0));

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
  BZDBLOCAL.init();
  
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

  initConfigData();
  loadBZDBDefaults();

  // parse for the config filename
  // the rest of the options are parsed after the config file
  // has been loaded to allow for command line overrides
  parseConfigName();

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
    if (BZDB.isSet("password")) {
      strncpy(startupInfo.password, BZDB.get("password").c_str(),
					sizeof(startupInfo.password) - 1);
      startupInfo.password[sizeof(startupInfo.password) - 1] = '\0';
    }

    if (BZDB.isSet("team")) {
      std::string value = BZDB.get("team");
      startupInfo.team = Team::getTeam(value);
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


    // however, if the "__window" setting is enabled, let it through
    if (BZDB.isSet("__window"))
      if (BZDB.isTrue("__window"))
        BZDB.set("_window", "1");
  }

  // use UDP? yes
  startupInfo.useUDPconnection=true;

  // parse arguments
  parse();

#ifdef _WIN32
  // this is cheap but it will work on windows
  // clear out the stdout file
  if (echoToConsole){
	  FILE	*fp = fopen ("stdout.txt","w");
	  if (fp) {
		  fprintf(fp,"stdout started\r\n" );
		  fclose(fp);
	  }
  }
#endif

  if (BZDB.isSet("directory")) {
    //Convert to unix paths so that escaping isn't an issue
    std::string directory = BZDB.get("directory");
    OSFileOSToStdDir(directory);
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
      ReportWarning("Invalid argument for fixedTime = %s", dbTime);
    }
    userTime.tm_sec = seconds;
    userTime.tm_min = minutes;
    userTime.tm_hour = hours;
  }

  // see if there is a _default_ badwords file
  if (!BZDB.isSet("filterFilename")) {
    std::string name;
    name = getConfigDirName();
    name += "badwords.txt";

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
  platformFactory = PlatformFactory::getInstance();

  // choose visual
  visual = platformFactory->createVisual(NULL);

  // make the window
  window = platformFactory->createWindow(NULL, visual);
  if (!window->isValid()) {
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("Can't create window.\n");
  }
  window->setTitle("bzflag");

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
    BzfMedia *media = PlatformFactory::getMedia();
    if (media)
      media->setMediaDirectory(exePath);
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
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("No fonts found  (the -directory option may help).");
  }

  if (!Open())
    return false;

  // initialize locale system

  bm = new BundleMgr(PlatformFactory::getMedia()->getMediaDirectory(), "bzflag");
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
	  x = g3d->GetWidth() - x - w;
	if (ys == '-')
	  y = g3d->GetHeight() - y - h;
	setPosition = true;
      }
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
  }
  if (setPosition)
    window->setPosition(x, y);

  // now make the main window wrapper.  this'll cause the OpenGL context
  // to be bound for the first time.
  pmainWindow = new MainWindow(window, this);
  if (pmainWindow->isInFault()) {
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("Error creating window.\n");
  }

  std::string videoFormat;

  // get sound files.  must do this after creating the window because
  // DirectSound is a bonehead API.
  if (!noAudio) {
    openSound("bzflag");
    if (startupInfo.hasConfiguration && BZDB.isSet("volume"))
      setSoundVolume(static_cast<int>(BZDB.eval("volume")));
  }

  // set main window's minimum size (arbitrary but should be big enough
  // to see stuff in control panel)
  pmainWindow->setMinSize(256, 192);

  // initialize graphics state
  pmainWindow->getWindow()->makeCurrent();
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
  if (!OpenGLGState::haveGLContext()) {
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("ERROR: Unable to initialize an OpenGL context");
  }
  OpenGLGState::init();

  // sanity check - make sure OpenGL was actually initialized or
  // there's no sense in continuing.
  const char* const glRenderer = (const char*)glGetString(GL_RENDERER);
  if (!glRenderer) {
    // bad code, no donut for you

    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR) {
      switch (error) {
	case GL_INVALID_ENUM:
	  std::cerr << "ERROR: GL_INVALID_ENUM" << std::endl;
	  break;
	case GL_INVALID_VALUE:
	  std::cerr << "ERROR: GL_INVALID_VALUE" << std::endl;
	  break;
	case GL_INVALID_OPERATION:
	  std::cerr << "ERROR: GL_INVALID_OPERATION" << std::endl;
	  break;
	case GL_STACK_OVERFLOW:
	  std::cerr << "ERROR: GL_STACK_OVERFLOW" << std::endl;
	  break;
	case GL_STACK_UNDERFLOW:
	  std::cerr << "ERROR: GL_STACK_UNDERFLOW" << std::endl;
	  break;
	case GL_OUT_OF_MEMORY:
	  std::cerr << "ERROR: GL_OUT_OF_MEMORY" << std::endl;
	  break;
#ifdef GL_VERSION_1_2
	case GL_TABLE_TOO_LARGE:
	  std::cerr << "ERROR: GL_TABLE_TOO_LARGE" << std::endl;
	  break;
#endif
	case GL_NO_ERROR:
	  // should not reach
	  std::cerr << "ERROR: GL_NO_ERROR" << std::endl;
	  break;
	default:
	  // should not reach
	  std::cerr << "ERROR: UNKNOWN CODE: " << error << std::endl;
      }
    }

    // DIE
#ifdef _WIN32
    WSACleanup();
#endif
    return ReportError("ERROR: Unable to initialize an OpenGL renderer");
  }

  // add the zbuffer callback here, after the OpenGL context is initialized
  BZDB.addCallback("zbuffer", setDepthBuffer, NULL);

  // if we're running on 3Dfx fullscreen add a fake cursor.
  // let the defaults file override this, though.
  if (!BZDB.isSet("fakecursor")) {
    // check that the glrenderer is Mesa Glide
    if ((glRenderer != NULL) && (strncmp(glRenderer, "Mesa Glide", 10) == 0 ||
	strncmp(glRenderer, "3Dfx", 4) == 0))
      BZDB.set("fakecursor", "1");
  }

  // set gamma if set in resources and we have gamma control
  if (BZDB.isSet("gamma")) {
    if (pmainWindow->getWindow()->hasGammaControl())
      g2d->SetGamma((float)atof(BZDB.get("gamma").c_str()));
  }

  // set the scene renderer's window
  RENDERER.setWindow(pmainWindow);

  // restore rendering configuration
  if (startupInfo.hasConfiguration) {
    if (BZDB.isSet("zbuffersplit")) {
      RENDERER.setZBufferSplit(BZDB.isTrue("zbuffersplit"));
    }
    if (BZDB.isSet("quality")) {
      std::string value = BZDB.get("quality");
      const int qualityLevels = (int)configQualityValues.size();
      for (int j = 0; j < qualityLevels; j++) {
	if (value == configQualityValues[j]) {
	  RENDERER.setQuality(j);
	  break;
	}
      }
    }

    TextureManager& tm = TextureManager::instance();
    tm.setMaxFilter(BZDB.get("texture"));
    BZDB.set("texture", tm.getMaxFilterName());

    BZDB.set("texturereplace", (!BZDBCache::lighting &&
	      RENDERER.useQuality() < 2) ? "1" : "0");
    BZDB.setPersistent("texturereplace", false);
    if (BZDB.isSet("view")) {
      RENDERER.setViewType(SceneRenderer::Normal);
      std::string value = BZDB.get("view");
      for (i = 0; i < configViewValues.size(); i++)
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
      RENDERER.setRadarSize(BZDB.getIntClamped("radarsize", 0, GUIOptionsMenu::maxRadarSize));

    if (BZDB.isSet("mouseboxsize"))
      RENDERER.setMaxMotionFactor(atoi(BZDB.get("mouseboxsize").c_str()));
  }

  // set window quadrant
  if (RENDERER.getViewType() == SceneRenderer::ThreeChannel)
    pmainWindow->setQuadrant(MainWindow::UpperRight);
  else if (RENDERER.getViewType() == SceneRenderer::Stacked)
    pmainWindow->setQuadrant(MainWindow::LowerHalf);
#ifndef USE_GL_STEREO
  else if (RENDERER.getViewType() == SceneRenderer::Stereo)
    pmainWindow->setQuadrant(MainWindow::UpperRight);
#endif

  // clear the grid graphics if they are not accessible
#if !defined(DEBUG_RENDERING)
  if (debugLevel <= 0) {
    BZDB.set("showCullingGrid", "0");
    BZDB.set("showCollisionGrid", "0");
  }
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
  playing = new Playing(NULL, RENDERER);

  // start game loop
  Run();
  return true;
}

void Bzflag::Frame()
{
  // main loop
  playing->playingLoop();
}

bool Bzflag::OnKeyboard(iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  utf32_char     code      = csKeyEventHelper::GetCookedCode(&event);
  csKeyModifiers m;

  csKeyEventHelper::GetModifiers(&event, m);

  bool pressed = (eventtype == csKeyEventTypeDown);
  BzfKeyEvent keyEvent;
  keyEvent.ascii  = 0;
  switch (code) {
  case CSKEY_PAUSE:
    keyEvent.button = BzfKeyEvent::Pause;
    break;
  case CSKEY_HOME:
    keyEvent.button = BzfKeyEvent::Home;
    break;
  case CSKEY_END:
    keyEvent.button = BzfKeyEvent::End;
    break;
  case CSKEY_LEFT:
    keyEvent.button = BzfKeyEvent::Left;
    break;
  case CSKEY_RIGHT:
    keyEvent.button = BzfKeyEvent::Right;
    break;
  case CSKEY_UP:
    keyEvent.button = BzfKeyEvent::Up;
    break;
  case CSKEY_DOWN:
    keyEvent.button = BzfKeyEvent::Down;
    break;
  case CSKEY_PGUP:
    keyEvent.button = BzfKeyEvent::PageUp;
    break;
  case CSKEY_PGDN:
    keyEvent.button = BzfKeyEvent::PageDown;
    break;
  case CSKEY_INS:
    keyEvent.button = BzfKeyEvent::Insert;
    break;
  case CSKEY_BACKSPACE:
    keyEvent.button = BzfKeyEvent::Backspace;
    break;
  case CSKEY_DEL:
    keyEvent.button = BzfKeyEvent::Delete;
    break;
  case CSKEY_F1:
    keyEvent.button = BzfKeyEvent::F1;
    break;
  case CSKEY_F2:
    keyEvent.button = BzfKeyEvent::F2;
    break;
  case CSKEY_F3:
    keyEvent.button = BzfKeyEvent::F3;
    break;
  case CSKEY_F4:
    keyEvent.button = BzfKeyEvent::F4;
    break;
  case CSKEY_F5:
    keyEvent.button = BzfKeyEvent::F5;
    break;
  case CSKEY_F6:
    keyEvent.button = BzfKeyEvent::F6;
    break;
  case CSKEY_F7:
    keyEvent.button = BzfKeyEvent::F7;
    break;
  case CSKEY_F8:
    keyEvent.button = BzfKeyEvent::F8;
    break;
  case CSKEY_F9:
    keyEvent.button = BzfKeyEvent::F9;
    break;
  case CSKEY_F10:
    keyEvent.button = BzfKeyEvent::F10;
    break;
  case CSKEY_F11:
    keyEvent.button = BzfKeyEvent::F11;
    break;
  case CSKEY_F12:
    keyEvent.button = BzfKeyEvent::F12;
    break;
  case CSKEY_PAD0:
    keyEvent.button = BzfKeyEvent::Kp0;
    break;
  case CSKEY_PAD1:
    keyEvent.button = BzfKeyEvent::Kp1;
    break;
  case CSKEY_PAD2:
    keyEvent.button = BzfKeyEvent::Kp2;
    break;
  case CSKEY_PAD3:
    keyEvent.button = BzfKeyEvent::Kp3;
    break;
  case CSKEY_PAD4:
    keyEvent.button = BzfKeyEvent::Kp4;
    break;
  case CSKEY_PAD5:
    keyEvent.button = BzfKeyEvent::Kp5;
    break;
  case CSKEY_PAD6:
    keyEvent.button = BzfKeyEvent::Kp6;
    break;
  case CSKEY_PAD7:
    keyEvent.button = BzfKeyEvent::Kp7;
    break;
  case CSKEY_PAD8:
    keyEvent.button = BzfKeyEvent::Kp8;
    break;
  case CSKEY_PAD9:
    keyEvent.button = BzfKeyEvent::Kp9;
    break;
  case CSKEY_PADDECIMAL:
    keyEvent.button = BzfKeyEvent::Kp_Period;
    break;
  case CSKEY_PADDIV:
    keyEvent.button = BzfKeyEvent::Kp_Divide;
    break;
  case CSKEY_PADMULT:
    keyEvent.button = BzfKeyEvent::Kp_Multiply;
    break;
  case CSKEY_PADMINUS:
    keyEvent.button = BzfKeyEvent::Kp_Minus;
    break;
  case CSKEY_PADPLUS:
    keyEvent.button = BzfKeyEvent::Kp_Plus;
    break;
  case CSKEY_PADENTER:
    keyEvent.button = BzfKeyEvent::Kp_Enter;
    break;
  case CSKEY_PRINTSCREEN:
    keyEvent.button = BzfKeyEvent::Print;
    break;
  default:
    keyEvent.button = BzfKeyEvent::NoButton;
    break;
  }
  // When NUM LOCK treat the KP number as numbers and Enter as Enter
  if (m.modifiers[csKeyModifierTypeNumLock])
    if (((keyEvent.button >= BzfKeyEvent::Kp0)
	 && (keyEvent.button <= BzfKeyEvent::Kp9))
	|| (keyEvent.button == BzfKeyEvent::Kp_Enter))
      keyEvent.button = BzfKeyEvent::NoButton;

  if (keyEvent.button == BzfKeyEvent::NoButton) {
    if ((code & 0xFF80))
      return false;
    keyEvent.ascii = code & 0x7F;
  }

  if (code == CSKEY_ENTER)
    keyEvent.ascii = '\r';

  keyEvent.shift = 0;
  if (m.modifiers[csKeyModifierTypeShift])
    keyEvent.shift |= BzfKeyEvent::ShiftKey;
  if (m.modifiers[csKeyModifierTypeCtrl])
    keyEvent.shift |= BzfKeyEvent::ControlKey;
  if (m.modifiers[csKeyModifierTypeAlt])
    keyEvent.shift |= BzfKeyEvent::AltKey;

  playing->doKey(keyEvent, pressed);
  return true;
}

bool Bzflag::OnMouseDown(iEvent &event)
{
  const uint       csmb      = csMouseEventHelper::GetButton(&event);
  csMouseEventType eventtype = csMouseEventHelper::GetEventType(&event);
  csKeyModifiers   m;
  BzfKeyEvent      buttonEvent;

  csMouseEventHelper::GetModifiers(&event, m);
  bool pressed = (eventtype == csMouseEventTypeDown);

  buttonEvent.ascii = 0;
  buttonEvent.shift = 0;
  // With mouse, keyboard modifier do not work great
  //   if (m.modifiers[csKeyModifierTypeShift])
  //     buttonEvent.shift |= BzfKeyEvent::ShiftKey;
  //   if (m.modifiers[csKeyModifierTypeCtrl])
  //     buttonEvent.shift |= BzfKeyEvent::ControlKey;
  //   if (m.modifiers[csKeyModifierTypeAlt])
  //     buttonEvent.shift |= BzfKeyEvent::AltKey;

  switch (csmb) {
  case 0:
    buttonEvent.button = BzfKeyEvent::LeftMouse;
    break;
  case 2:
    buttonEvent.button = BzfKeyEvent::MiddleMouse;
    break;
  case 1:
    buttonEvent.button = BzfKeyEvent::RightMouse;
    break;
  case 3:
    buttonEvent.button = BzfKeyEvent::WheelUp;
    break;
  case 4:
    buttonEvent.button = BzfKeyEvent::WheelDown;
    break;
  case 5:
    buttonEvent.button = BzfKeyEvent::MouseButton6;
    break;
  case 6:
    buttonEvent.button = BzfKeyEvent::MouseButton7;
    break;
  case 7:
    buttonEvent.button = BzfKeyEvent::MouseButton8;
    break;
  case 8:
    buttonEvent.button = BzfKeyEvent::MouseButton9;
    break;
  case 9:
    buttonEvent.button = BzfKeyEvent::MouseButton10;
    break;
  default:
    return false;
  }
  playing->doKey(buttonEvent, pressed);
  return true;
}

bool Bzflag::OnMouseUp(iEvent &event)
{
  return OnMouseDown(event);
}

void Bzflag::OnExit()
{
  delete playing;

  // save resources
  if (BZDB.isTrue("saveSettings")) {
    dumpResources();
    if (alternateConfig == "") {
      CFGMGR.write(getCurrentConfigFileName());
    } else {
      CFGMGR.write(alternateConfig);
    }
  }

  // shut down
  if (filter != NULL)
    delete filter;
  filter = NULL;
  delete pmainWindow;
  delete window;
  delete visual;
  closeSound();
  delete platformFactory;
  delete bm;
  Flags::kill();

#if defined(_WIN32)
  {
    /* clear HKEY_CURRENT_USER\Software\BZFlag\CurrentRunningPath if it 
     * exists */
    HKEY key = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\BZFlag", 
	0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
      RegSetValueEx(key, "CurrentRunningPath", 0, REG_SZ, (LPBYTE)"\0", 1);
    }
  }
#endif

#ifdef _WIN32
  // clean up
  WSACleanup();
#endif

  // clean up singletons
  //  delete FILEMGR;
  //  delete CMDMGR;
  //  delete BZDB;
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
