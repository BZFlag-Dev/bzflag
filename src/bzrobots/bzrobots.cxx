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
#include "BundleMgr.h"
#include "CommandManager.h"
#include "CommandsStandard.h"
#include "ConfigFileManager.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "FontManager.h"
#include "KeyManager.h"
#include "OSFile.h"
#include "ParseColor.h"
#include "Protocol.h"
#include "ServerListCache.h"
#include "StateDatabase.h"
#include "Team.h"
#include "TextUtils.h"
#include "TextureManager.h"
#include "TimeBomb.h"
#include "WordFilter.h"
#include "World.h"
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

// Function in botplaying.cxx:
void botStartPlaying();


// ROBOT -- tidy up
int numRobotTanks = 1;


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

static void		usage()
{
  printFatalError("usage: %s"
	" [-anonymous]"
	" [-badwords <filterfile>]"
	" [-config <configfile>]"
	" [-configdir <config dir name>]"
	" [-d | -debug]"
	" [{-dir | -directory} <data-directory>]"
	" [-e | -echo]"
	" [-ea | -echoAnsi]"
	" [-h | -help | --help]"
	" [-locale <locale>]"
	" [-m | -mute]"
	" [-p | -rcport <remote-control-port>]"
	" [-motd <motd-url>] [-nomotd]"
	" [-solo <num-robots>]"
	" [-team {red|green|blue|purple|rogue|observer}]"
	" [-v | -version | --version]"
	" [callsign[:password]@]server[:port]\n\nExiting.", argv0);
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
  // Defaults
  startupInfo.team = AutomaticTeam;
  strncpy(startupInfo.serverName, "localhost", \
	sizeof(startupInfo.serverName) - 1);
  startupInfo.serverPort = 5154;
  strncpy(startupInfo.callsign, "anonymouscoward", \
	sizeof(startupInfo.callsign) - 1);


  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0 ||
		strcmp(argv[i], "-anonymous") == 0) {
      anonymous = true;
    } else if (strcmp(argv[i], "-config") == 0) {
      checkArgc(i, argc, argv[i]);
      // the setting has already been done in parseConfigName()
	} else if (strcmp(argv[i], "-configdir") == 0) {
		checkArgc(i, argc, argv[i]);
		// the setting has already been done in parseConfigName()
	} else if ((strcmp(argv[i], "-d") == 0) ||
	       (strcmp(argv[i], "-debug") == 0)) {
      debugLevel++;
    } else if ((strcmp(argv[i], "-dir") == 0) ||
	       (strcmp(argv[i], "-directory") == 0)) {
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
    } else if (strcmp(argv[i], "-locale") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("locale", argv[i]);
    } else if (strcmp(argv[i], "-motd") == 0) {
      checkArgc(i, argc, argv[i]);
      if (strcmp(argv[i], "default") == 0) {
	BZDB.set("motdServer", BZDB.getDefault("motdServer"));
      } else {
	BZDB.set("motdServer", argv[i]);
      }
      BZDB.unset("disableMOTD");
    } else if (strcmp(argv[i], "-nomotd") == 0) {
      BZDB.set("disableMOTD", "1");
    } else if (strcmp(argv[i], "-m") == 0 ||
		strcmp(argv[i], "-mute") == 0) {
      noAudio = true;
    } else if (strcmp(argv[i], "-p") == 0 ||
		strcmp(argv[i], "-rcport") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("rcPort", argv[i]);
    } else if (strcmp(argv[i], "-solo") == 0) {
      checkArgc(i, argc, argv[i]);
      numRobotTanks = atoi(argv[i]);
      if (numRobotTanks < 1 || numRobotTanks > MAX_ROBOTS) {
	printFatalError("Invalid argument for %s.", argv[i-1]);
	usage();
      }
    } else if (strcmp(argv[i], "-posnoise") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("bzrcPosNoise", argv[i]);
    } else if (strcmp(argv[i], "-angnoise") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("bzrcAngNoise", argv[i]);
    } else if (strcmp(argv[i], "-velnoise") == 0) {
      checkArgc(i, argc, argv[i]);
      BZDB.set("bzrcVelNoise", argv[i]);
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
      printFatalError("BZFlag client %s (protocol %s) http://BZFlag.org/\n%s",
		getAppVersion(),
		getProtocolVersion(),
		bzfcopyright);
      bail(0);
      exit(0);
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
    char* password;
	  *serverName = '\0';
	  if (strlen(argv[i]) >= sizeof(startupInfo.callsign))
	    printFatalError("Callsign truncated.");
	  strncpy(startupInfo.callsign, argv[i],
		  sizeof(startupInfo.callsign) - 1);
	  startupInfo.callsign[sizeof(startupInfo.callsign) - 1] = '\0';
	  if ((password = strchr(startupInfo.callsign, ':')) != NULL) {
      *(strchr(startupInfo.callsign, ':')) = '\0';
	    *password = '\0', ++password;
	    if (strlen(argv[i]) >= sizeof(startupInfo.password))
	      printFatalError("Password truncated.");
	    strncpy(startupInfo.password, password, sizeof(startupInfo.password) - 1);
	    startupInfo.password[sizeof(startupInfo.password) - 1] = '\0';
    }
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
	  exit(-1);
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
    if (strcmp(argv[i], "-configdir") == 0) {
      checkArgc(i, argc, argv[i]);
      setCustomConfigDir(argv[i]);
      alternateConfig += argv[i];
    }
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-config") == 0) {
      checkArgc(i, argc, argv[i]);
      alternateConfig = getConfigDirName();
      alternateConfig += argv[i];
    }
  }
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

bool checkTimeBomb ( void )
{
	// check time bomb
	if (timeBombBoom()) 
	{
		printFatalError("This release expired on %s. \n"
			"Please upgrade to the latest release. \n"
			"Exiting.", timeBombString());
		bail(0);
		return true;
	}
	return false;
}

void setupBZDB ( void )
{
	// set default DB entries
	for (unsigned int gi = 0; gi < numGlobalDBItems; ++gi) 
	{
		assert(globalDBItems[gi].name != NULL);
		if (globalDBItems[gi].value != NULL) 
		{
			BZDB.set(globalDBItems[gi].name, globalDBItems[gi].value);
			BZDB.setDefault(globalDBItems[gi].name, globalDBItems[gi].value);
		}
		BZDB.setPersistent(globalDBItems[gi].name, globalDBItems[gi].persistent);
		BZDB.setPermission(globalDBItems[gi].name, globalDBItems[gi].permission);
	}

	BZDBCache::init();
}

void setupConfigs ( void )
{
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
		ActionBinding::instance().resetBindings();    // bind default keys

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
    return bail(1);
  }
#endif

  WordFilter *filter = (WordFilter *)NULL;

  argv0 = argv[0];

  // init libs

  //init_packetcompression();

  if (checkTimeBomb())
	return 0;

  createCacheSignature();

  // initialize global objects and classes
  bzfsrand((unsigned int)time(0));

  setupBZDB();

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
  parseConfigName(argc, argv);

  setupConfigs();

  // use UDP? yes
  startupInfo.useUDPconnection=true;

  // parse arguments
  parse(argc, argv);

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

  std::string locale = BZDB.isSet("locale") ? BZDB.get("locale") : "default";
  World::setLocale(locale);

  if (BZDB.isSet("serverCacheAge")) {
    (ServerListCache::get())->setMaxCacheAge(atoi(BZDB.get("serverCacheAge").c_str()));
  }

  // start playing
  botStartPlaying();

  // shut down
  if (filter != NULL)
    delete filter;
  filter = NULL;
  Flags::kill();

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
#if defined(_WIN32)

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
