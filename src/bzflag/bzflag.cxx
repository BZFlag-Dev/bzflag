/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#define _WINSOCKAPI_
#endif /* defined(_WIN32) */
#include "ErrorHandler.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include "Address.h"
#include "Protocol.h"
#include "playing.h"
#include "TimeBomb.h"
#include "Team.h"
#include "sound.h"
#include "KeyManager.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "PlatformFactory.h"
#include "PlatformMediaFactory.h"
#include "CommandManager.h"
#include "CommandsStandard.h"
#include "MenuReader.h"
#include "MenuManager.h"
#include "MessageManager.h"
#include "StateDatabase.h"
#include "FileManager.h"
#include "ViewItems.h"
#include "ViewReader.h"
#include "ConfigIO.h"
#include <iostream>

static const char*		argv0;
static BzfDisplay*		display = NULL;
static BzfWindow*		window = NULL;
static bool				echoToConsole = false;
static bool				useFullscreen = false;
static bool				allowResolutionChange = false;
static const char*		commandPrompt = "> ";

//
// error handlers.  fatalErrorCallback() is used when the app is
// about to exit (taking the console with it on win32).
//

static void				initializingErrorCallback(const char* msg)
{
	PLATFORM->writeConsole(msg, true);
	PLATFORM->writeConsole("\n", true);
}

static void				fatalErrorCallback(const char* msg)
{
#if defined(_WIN32)
	MessageBox(NULL, msg, "BZFlag Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
	PLATFORM->createConsole();
	PLATFORM->writeConsole(msg, true);
	PLATFORM->writeConsole("\n", true);
#endif
}


//
// (external) console echo
//

static bool				onConsoleEcho(const BzfString& msg,
								const float*, void*)
{
	PLATFORM->writeConsole(msg.c_str());
	PLATFORM->writeConsole("\n");
	return true;
}


//
// message buffer callbacks
//

static bool				onMessageCB(const BzfString& msg,
								const float* /*color*/, void* dst)
{
	// discard color
	reinterpret_cast<MessageBuffer*>(dst)->insert(msg, NULL);
	return true;
}


//
// game console functions
//

static void				startComposing();

static void				onRunCommand(const BzfString& cmd, void*)
{
	// echo command with prompt
	printError("%s%s", commandPrompt, cmd.c_str());

	// handle command
	if (!cmd.empty()) {
		BzfString result = CMDMGR->run(cmd);
		if (!result.empty())
			printError(result.c_str());
	}
}

static void				onStopCommand(MessageBuffer::StopReason reason, void*)
{
	// if showing console then begin composing again unless user
	// pressed escape (which is the only way to dismiss the console)
	if (reason != MessageBuffer::kEscape && BZDB->isTrue("displayConsole"))
		startComposing();
	else
		BZDB->unset("displayConsole");
}

static void				startComposing()
{
	if (!MSGMGR->get("console")->isComposing())
		MSGMGR->get("console")->startComposing(commandPrompt,
								&onRunCommand, &onStopCommand, NULL);
}


//
// state database change handlers
//

static void				onConsole(const BzfString& name, void*)
{
	// always enter compose mode when showing the console
	if (BZDB->isTrue(name))
		startComposing();
}

static void				onGammaChanged(const BzfString& name, void*)
{
	if (window != NULL && BZDB->isTrue("featuresGamma") && !BZDB->isEmpty(name))
		window->setGamma((float)atof(BZDB->get(name).c_str()));
}

static void				onResolutionChanged(const BzfString& name, void*)
{
	if (display != NULL &&
		display->getNumResolutions() > 1 &&
		window != NULL &&
		allowResolutionChange) {
		const int index = display->findResolution(BZDB->get(name).c_str());
		if (display->isValidResolution(index) &&
		display->getResolution() != index)
			if (display->setResolution(index)) {
				// resize the window to match the resolution
				const BzfDisplay::ResInfo* resInfo = display->getResolution(index);
				window->setPosition(0, 0);
				window->setSize(resInfo->width, resInfo->height);
			}
	}
}

static void				onCursorChanged(const BzfString& name, void*)
{
	if (window != NULL) {
		if (BZDB->isTrue(name))
			window->showMouse();
		else
			window->hideMouse();
	}
}

static void				onTexturingChanged(const BzfString& name, void*)
{
	OpenGLTexture::Filter filter;
	const BzfString& value = BZDB->get(name);
	if (value == "nearest")
		filter = OpenGLTexture::Nearest;
	else if (value == "linear")
		filter = OpenGLTexture::Linear;
	else if (value == "nearestmipmapnearest")
		filter = OpenGLTexture::NearestMipmapNearest;
	else if (value == "linearmipmapnearest")
		filter = OpenGLTexture::LinearMipmapNearest;
	else if (value == "nearestmipmaplinear")
		filter = OpenGLTexture::NearestMipmapLinear;
	else if (value == "linearmipmaplinear")
		filter = OpenGLTexture::LinearMipmapLinear;
	else
		filter = OpenGLTexture::Off;
	OpenGLTexture::setFilter(filter);
}

static void				onColorChangeCB(const BzfString&, void*)
{
	// update view colors
	ViewColor::setColor(ViewColor::Red,    Team::getRadarColor(RedTeam));
	ViewColor::setColor(ViewColor::Green,  Team::getRadarColor(GreenTeam));
	ViewColor::setColor(ViewColor::Blue,   Team::getRadarColor(BlueTeam));
	ViewColor::setColor(ViewColor::Purple, Team::getRadarColor(PurpleTeam));
	ViewColor::setColor(ViewColor::Rogue,  Team::getRadarColor(RogueTeam));
}


//
// initialize the visual
//

static void				setVisual(BzfVisual* visual)
{
	// sine qua non
	visual->setLevel(0);
	visual->setDoubleBuffer(true);
	visual->setRGBA(1, 1, 1, 0);
	visual->setDepth(16);	// FIXME -- will we get bigger if available?

	// optional
#if defined(DEBUG_RENDERING)
	visual->setStencil(4);
#endif
	if (BZDB->isSet("multisample"))
		visual->setMultisample(4);
#ifdef USE_GL_STEREO
	// FIXME -- do this some other way
	if (BZDB->get("displayView") == "stereo")
		visual->setStereo(true);
#endif
}


//
// command line parsing and usage
//

// FIXME -- use a generic (un)set-the-DB command line option for
// several of the current options
static void				usage()
{
	printError("usage: %s"
		" [-anonymous]"
		" [-callsign <call-sign>]"
		" [-directory <data-directory>]"
		" [-echo]"
		" [-geometry {fullscreen|<geometry-spec>}]"
		" [-interface <interface>]"
		" [-joystick {yes|no}]"
		" [-joystickname <name>]"
		" [-list <server-list-url>] [-nolist]"
		" [-multisample]"
		" [-mute]"
		" [-port <server-port>]"
		" [-team {red|green|blue|purple|rogue}]"
		" [-ttl <time-to-live>]"
		" [-version]"
		" [-view <view-name>]"
		" server\n\nExiting.", argv0);
	exit(1);
}

static void				parse(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0 ||
				strcmp(argv[i], "-anonymous") == 0) {
			BZDB->unset("infoEmail");
		}
		else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-callsign") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			BZDB->set("infoCallsign", argv[i]);
		}
		else if (strcmp(argv[i], "-d") == 0 ||
				strcmp(argv[i], "-directory") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			if (strlen(argv[i]) == 0)
				BZDB->unset("directory");
			else
				BZDB->set("directory", argv[i]);
		}
		else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "-echo") == 0) {
			echoToConsole = true;
		}
		else if (strcmp(argv[i], "-h") == 0 ||
		     strcmp(argv[i], "-help") == 0 ||
		     strcmp(argv[i], "--help") == 0) {
			usage();
		}
		else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "-geometry") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			int w, h, x, y, count;
			char xs = '+', ys = '+';
			if (strcmp(argv[i], "fullscreen") != 0 &&
					(((count = sscanf(argv[i], "%dx%d%c%d%c%d",
					&w, &h, &xs, &x, &ys, &y)) != 6 && count != 2) ||
					(xs != '-' && xs != '+') || (ys != '-' && ys != '+'))) {
				printError("Invalid argument for %s.\n"
						"Correct format is <width>x<height>{+|-}<x>{+|-}<y>.",
						argv[i-1]);
				usage();
			}
			BZDB->set("windowGeometry", argv[i]);
		}
		else if (strcmp(argv[i], "-i") == 0 ||
				strcmp(argv[i], "-interface") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			BZDB->set("infoMulticastInterface", argv[i]);
		}
		else if (strcmp(argv[i], "-list") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			if (strcmp(argv[i], "default") == 0) {
				BZDB->unset("infoListServerURL");
			}
			else {
				BZDB->set("infoListServerURL", argv[i]);
			}
		}
		else if (strcmp(argv[i], "-nolist") == 0) {
			BZDB->unset("infoListServerURL");
		}
		else if (strcmp(argv[i], "-m") == 0 ||
				strcmp(argv[i], "-mute") == 0) {
			BZDB->set("featuresAudio", "0");
		}
		else if (strcmp(argv[i], "-multisample") == 0) {
			BZDB->set("multisample", "1");
		}
		else if (strcmp(argv[i], "-port") == 0) {
			if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
			}
			int serverPort = atoi(argv[i]);
			if (serverPort < 1 || serverPort > 65535) {
				printError("Invalid port (must be between 1 and 65535).");
				usage();
			}
			else {
				BZDB->set("infoPort", argv[i]);
			}
		}
    	else if (strcmp(argv[i], "-team") == 0) {
    		if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
    		}
    		if (strcmp(argv[i], "r") == 0 || strcmp(argv[i], "red") == 0)
				BZDB->set("infoTeam", "red");
    		else if (strcmp(argv[i], "g") == 0 || strcmp(argv[i], "green") == 0)
				BZDB->set("infoTeam", "green");
    		else if (strcmp(argv[i], "b") == 0 || strcmp(argv[i], "blue") == 0)
				BZDB->set("infoTeam", "blue");
    		else if (strcmp(argv[i], "p") == 0 || strcmp(argv[i], "purple") == 0)
				BZDB->set("infoTeam", "purple");
    		else if (strcmp(argv[i], "z") == 0 || strcmp(argv[i], "rogue") == 0)
				BZDB->set("infoTeam", "rogue");
    		else {
				printError("Invalid argument for %s.", argv[i-1]);
				usage();
    		}
    	}
    	else if (strcmp(argv[i], "-ttl") == 0) {
    		if (++i == argc) {
			  printError("Missing argument for %s.", argv[i-1]);
			  usage();
    		}
    		int ttl = atoi(argv[i]);
    		if (ttl < 0) {
				ttl = 0;
				printError("Using minimum ttl of %d.", ttl);
    		}
    		else if (ttl > MaximumTTL) {
				ttl = MaximumTTL;
				printError("Using maximum ttl of %d.", ttl);
    		}
    		BZDB->set("infoNetworkTTL", BzfString::format("%d", ttl));
    	}
    	else if (strcmp(argv[i], "-joystick") == 0) {
    		if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
    		}
    		if (strcmp(argv[i], "no") != 0 && strcmp(argv[i], "yes") != 0) {
				printError("Invalid argument for %s.", argv[i-1]);
				usage();
    		}
    		BZDB->set("infoJoystick", strcmp(argv[i], "yes") ? "0" : "1");
    	}
    	else if (strcmp(argv[i], "-joystickname") == 0) {
    		if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
    		}
    		BZDB->set("infoJoystickName", argv[i]);
    	}
    	else if (strcmp(argv[i], "-v") == 0 ||
		    	 strcmp(argv[i], "-version") == 0 ||
		    	 strcmp(argv[i], "--version") == 0) {
			printError("BZFlag client, version %d.%d%c%d\n"
					"  protocol %c.%d%c",
					(VERSION / 10000000) % 100,
					(VERSION / 100000) % 100,
					(char)('a' - 1 + (VERSION / 1000) % 100),
					VERSION % 1000,
					ServerVersion[4],
					atoi(ServerVersion + 5),
					ServerVersion[7]);
			exit(0);
    	}
    	else if (strcmp(argv[i], "-date") == 0) {
    		if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
    		}
    		int month, day, year, hours, minutes, seconds;
    		if (sscanf(argv[i], "%d/%d/%d %d:%d:%d",
						&month, &day, &year,
						&hours, &minutes, &seconds) != 3 ||
					day < 1 || day > 31 ||		// FIXME -- upper limit loose
					month < 1 || month > 12 ||
					(year < 0 || (year > 100 && (year < 1970 || year > 2100))) ||
					hours < 0 || hours > 23 ||
					minutes < 0 || minutes > 59 ||
					seconds < 0 || seconds > 59) {
			  printError("Invalid argument for %s.", argv[i-1]);
			  usage();
    		}
    		if (year > 100)
				year = year - 1900;
    		else if (year < 70)
				year += 100;

    		struct tm userTime;
    		userTime.tm_sec   = seconds;
    		userTime.tm_min   = minutes;
    		userTime.tm_hour  = hours;
    		userTime.tm_mday  = day;
    		userTime.tm_mon   = month - 1;
    		userTime.tm_year  = year;
    		userTime.tm_isdst = -1;
    		time_t t = mktime(&userTime);
    		if (t == (time_t)-1)
				printError("Cannot convert date;  ignoring.");
    		else
				BZDB->set("timeClock", BzfString::format("%f", (double)t));
    	}
    	else if (strcmp(argv[i], "-view") == 0) {
    		if (++i == argc) {
				printError("Missing argument for %s.", argv[i-1]);
				usage();
    		}
    		BZDB->set("displayView", argv[i]);
    	}
    	else if (i == argc-1) {
    		BZDB->set("infoServer", argv[i]);
    		BZDB->set("connect", "1");
    	}
    	else {
    		usage();
    	}
	}
}

//
// config file read callbacks
//

static bool				readDBOpenCommand(ConfigReader* reader,
								const BzfString&,
								const ConfigReader::Values&,
								void*)
{
	// ignore tags in the command section
	reader->push(NULL, NULL, NULL);
	return true;
}

static bool				readDBDataCommand(ConfigReader*,
								const BzfString& cmd,
								void*)
{
	// if there's nothing but whitespace then skip it
	if (strspn(cmd.c_str(), " \t\r\n") != cmd.size()) {
		BzfString result = CMDMGR->run(cmd);
		if (!result.empty())
			printError(result.c_str());
	}
	return true;
}

static bool				readDBOpenTop(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values&,
								void*)
{
	if (tag == "command") {
		// start reading commands
		reader->push(readDBOpenCommand, NULL, readDBDataCommand);
		return true;
	}
	else {
		// other sections aren't valid
		printError("unexpected configuration section %s", tag.c_str());
		return false;
	}
}


//
// config file write callbacks
//

static void				writeDBEntry(const BzfString& name, void* _stream)
{
	// get the value, escaped for quoting
	BzfString value = ConfigReader::escape(BZDB->get(name));

	// write it
	ostream* stream = reinterpret_cast<ostream*>(_stream);
	(*stream) << "  set " << name << " \"" << value << "\"" << std::endl;
}

static void				writeKeys(const BzfString& name, bool press,
								const BzfString& cmd, void* _stream)
{
	ostream* stream = reinterpret_cast<ostream*>(_stream);
	(*stream) << "  bind \"" << name << "\" " <<
								(press ? "down" : "up") << " \"" <<
								ConfigReader::escape(cmd) << "\"" << std::endl;
}


//
// built-in mappings
//

// font id to texture file mapping
struct FontMapEntry {
public:
	const char*			name;
	const char*			filename;
};
static const FontMapEntry fontMap[] = {
	{ "timesBold",				"timesbr" },
	{ "timesBoldItalic",		"timesbi" },
	{ "helveticaBold",			"helvbr"  },
	{ "helveticaBoldItalic",	"helvbi"  },
	{ "fixed",					"fixedmr" },
	{ "fixedBold",				"fixedbr" }
};

// default database entries
struct DefaultDBItem {
public:
	const char*				name;
	const char*				value;
	bool					persistent;
	StateDatabase::Permission	permission;
	StateDatabase::Callback	callback;
};
static DefaultDBItem		defaultDBItems[] = {
	{ "connect",			NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "connectError",		NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "connectStatus",		NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "displayBinoculars",	NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "displayConsole",		"0",		false, StateDatabase::ReadWrite, onConsole },
	{ "displayCrosshair",	NULL,		true,  StateDatabase::ReadWrite, NULL },
	{ "displayCursor",		"1",		true,  StateDatabase::ReadWrite, onCursorChanged },
	{ "displayFlagHelp",	NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "displayGrabCursor",	NULL,		true,  StateDatabase::ReadWrite, NULL },
	{ "displayRadar",		"1",		true,  StateDatabase::ReadWrite, NULL },
	{ "displayRadarRange",	NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "displayScore",		NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "displayView",		"normal",	true,  StateDatabase::ReadWrite, NULL },
	{ "featuresAudio",		"1",		false, StateDatabase::ReadOnly,  NULL },
	{ "featuresGamma",		"1",		false, StateDatabase::ReadOnly,  NULL },
	{ "featuresMouseGrab",	"1",		true,  StateDatabase::ReadWrite, NULL },
	{ "featuresResolutions","",			false, StateDatabase::ReadOnly,  NULL },
	{ "featuresServers",	NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "audioVolume",		"10",		true,  StateDatabase::ReadWrite, NULL },
	{ "audioMute",			"0",		false, StateDatabase::ReadWrite, NULL },
	{ "renderGamma",		"2.0",		true,  StateDatabase::ReadWrite, onGammaChanged },
	{ "renderMaxLOD",		NULL,		true,  StateDatabase::ReadWrite, NULL },
	{ "renderQuality",		"2",		true,  StateDatabase::ReadWrite, NULL },
	{ "renderTexturing",	NULL,		true,  StateDatabase::ReadWrite, onTexturingChanged },
	{ "infoLatitude",		"37.5",		true,  StateDatabase::ReadWrite, NULL },
	{ "infoLongitude",		"122.0",	true,  StateDatabase::ReadWrite, NULL },
	{ "infoListServerURL",	DefaultListServerURL,
										true,  StateDatabase::ReadWrite, NULL },
	{ "infoTeam",			"rogue",	true,  StateDatabase::ReadWrite, NULL },
	{ "msgCompose",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "msgCommand",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "msgSend",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "outputFlag",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "outputScore",		NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "outputStatus",		NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "timeClock",			NULL,		false, StateDatabase::ReadWrite, NULL },
	{ "timeFrame",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "timeFPS",			NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "multisample",		NULL,		false, StateDatabase::ReadOnly,  NULL },
	{ "windowGeometry",		"fullscreen",true,  StateDatabase::ReadWrite, NULL },
	{ "windowResolution",	NULL,		true,  StateDatabase::ReadWrite, onResolutionChanged }
};

// default key bindings
static const char*		bindingList[] = {
		"bind \"F12\" down \"quit\"",
		"bind \"`\" down \"toggle displayConsole\"",
		"bind \"=\" down \"add timeClock 300\"",
		"bind \"-\" down \"add timeClock -300\"",
		"bind \"]\" down \"add timeClock 30\"",
		"bind \"[\" down \"add timeClock -30\"",
		"bind \"Left Mouse\" down \"fire\"",
		"bind \"Enter\" down \"fire\"",
		"bind \"Left Mouse\" up   \"fire stop\"",
		"bind \"Enter\" up   \"fire stop\"",
		"bind \"Middle Mouse\" down \"drop\"",
		"bind \"Space\" down \"drop\"",
		"bind \"Right Mouse\" down \"identify\"",
		"bind \"L\" down \"identify\"",
		"bind \"Tab\" down \"jump\"",
		"bind \"B\" down \"toggle displayBinoculars\"",
		"bind \"N\" down \"send all\"",
		"bind \"M\" down \"send team\"",
		"bind \",\" down \"send nemesis\"",
		"bind \"S\" down \"toggle displayScore\"",
		"bind \"F\" down \"toggle displayFlagHelp\"",
		"bind \"1\" down \"set displayRadarRange 0.25\"",
		"bind \"2\" down \"set displayRadarRange 0.50\"",
		"bind \"3\" down \"set displayRadarRange 1.00\"",
		"bind \"Pause\" down \"pause\"",
		"bind \"P\" down \"pause\"",
//		"bind \"A\" down \"set motionSlow\"",
//		"bind \"A\" up   \"unset motionSlow\"",
};


//
// main()
//		initialize application and enter event loop
//

#if defined(_WIN32)
int						myMain(int argc, char** argv)
#else /* defined(_WIN32) */
int						main(int argc, char** argv)
#endif /* defined(_WIN32) */
{
	unsigned int i;

	argv0 = argv[0];

	// set hook for initialization errors
	setErrorCallback(fatalErrorCallback);

	// init libs
	//init_packetcompression();

	// check time bomb
	if (timeBombBoom()) {
		printError("This release expired on %s. \n"
				"Please upgrade to the latest release. \n"
				"Exiting.", timeBombString());
		exit(0);
	}

	// initialize global objects and classes
	bzfsrand(time(NULL));

	// initialize some classes
	Team::init();

	// prepare message buffers
	MSGMGR->create("console", 100);
	MSGMGR->create("messages", 100);
	MSGMGR->create("alertGameOver", 1);
	MSGMGR->create("alertInfo", 1);
	MSGMGR->create("alertFlag", 1);
	MSGMGR->create("alertStatus", 1);
	MSGMGR->create("flagHelp", 1);

	// echo some stuff to other buffers
	MSGMGR->get("messages")->addCallback(&onMessageCB,
								MSGMGR->get("console"));
	MSGMGR->get("alertGameOver")->addCallback(&onMessageCB,
								MSGMGR->get("messages"));

	// register some commands
	CommandsStandard::add();

	// bind default keys
	for (i = 0; i < countof(bindingList); ++i)
		CMDMGR->run(bindingList[i]);

	// prepare DB entries
	for (i = 0; i < countof(defaultDBItems); ++i) {
		assert(defaultDBItems[i].name != NULL);
		if (defaultDBItems[i].value != NULL) {
			BZDB->set(defaultDBItems[i].name, defaultDBItems[i].value);
			BZDB->setDefault(defaultDBItems[i].name, defaultDBItems[i].value);
		}
		BZDB->setPersistent(defaultDBItems[i].name, defaultDBItems[i].persistent);
		BZDB->setPermission(defaultDBItems[i].name, defaultDBItems[i].permission);
		if (defaultDBItems[i].callback != NULL)
			BZDB->addCallback(defaultDBItems[i].name,
								defaultDBItems[i].callback, NULL);
	}

	// other default DB entries
	BZDB->set("timeClock", BzfString::format("%f", (double)time(NULL)));
	BzfString callsign = PLATFORM->getEnv("BZFLAGID");
	if (callsign.empty())
		callsign = PLATFORM->getEnv("BZID");
	if (!callsign.empty())
		BZDB->set("infoCallsign", callsign);
	{
		const char* hostname = Address::getHostName();
		BzfString username = PLATFORM->getUserName();
		if (!username.empty() && hostname != NULL) {
			BzfString email = username;
			email = username;
			email += "@";
			email += hostname;
			BZDB->set("infoEmail", email);
		}
	}
	BZDB->set("infoNetworkTTL", BzfString::format("%d", DefaultTTL));

	// read resources
	{
		istream* stream = PLATFORM->createConfigInStream();
		if (stream != NULL) {
			setErrorCallback(initializingErrorCallback);
			ConfigReader reader;
			reader.push(readDBOpenTop, NULL, NULL);
			reader.read(*stream, NULL);
			delete stream;
			setErrorCallback(fatalErrorCallback);
		}
	}

	// parse arguments
	parse(argc, argv);

	// hook up console echo if requested
	if (echoToConsole) {
		PLATFORM->createConsole();
		MSGMGR->get("console")->addCallback(&onConsoleEcho, NULL);
	}

	// open display
	display = MPLATFORM->createDisplay(NULL, NULL);
	if (!display) {
		printError("Can't open display.  Exiting.");
		return 1;
	}

	// choose visual
	BzfVisual* visual = MPLATFORM->createVisual(display);
	setVisual(visual);

	// make the window
	window = MPLATFORM->createWindow(display, visual);
	if (!window->isValid()) {
		printError("Can't create window.  Exiting.");
		delete display;
		return 1;
	}
	window->setTitle("BZFlag");

	// remaining messages to the console
	setErrorCallback(initializingErrorCallback);

	/* initialize the joystick */
	if (BZDB->isTrue("infoJoystick") && !BZDB->isEmpty("joystickName"))
		window->initJoystick(BZDB->get("joystickName").c_str());

	// if the windowResolution isn't set and resolution changing is
	// available then set it to the current (default) resolution.
	if (display->getNumResolutions() > 0 && BZDB->get("windowResolution").empty())
		BZDB->set("windowResolution", display->getResolution(
								display->getDefaultResolution())->name);

	// about to set window size.  it's important to do this before the
	// OpenGL context is first bound to the window because passthrough
	// cards will probably use the window size when bound to choose
	// the passthrough resolution.

	// set passthrough stuff
	if (BZDB->isSet("windowPassthrough") || display->isPassthrough()) {
		// force passthrough
		display->setPassthrough(true);

		// hack for 3Dfx/Mesa combination
		PLATFORM->setEnv("MESA_GLX_FX", "fullscreen");

		// parse value, which should be a size in the form <width>x<height>
		int w, h;
		int count = sscanf(BZDB->get("windowPassthrough").c_str(),
														"%dx%d", &w, &h);
		if (count == 2) {
			// set the passthrough and window size
			display->setPassthroughSize(w, h);
			window->setSize(w, h);
		}
		else {
			// use default passthrough size
			window->setSize(display->getPassthroughWidth(),
						display->getPassthroughHeight());
		}
		useFullscreen = true;
	}

	// set the non-passthrough window position and size
	else {
		int x, y, w, h;
		bool setSize = false, setPosition = false;
		BzfString geometry = BZDB->get("windowGeometry");
		if (geometry != "fullscreen") {
			// parse geometry specification.  on error use fullscreen.
			char xs, ys;
			int count = sscanf(geometry.c_str(), "%dx%d%c%d%c%d",
										&w, &h, &xs, &x, &ys, &y);
			if (w > 0 && h > 0) {
				if (count == 6 && (xs == '-' || xs == '+') &&
								(ys == '-' || ys == '+')) {
					// setting size and position
					setSize     = true;
					setPosition = true;

					// flip negative coordinates
					if (xs == '-')
						x = display->getWidth()  - x - w;
					if (ys == '-')
						y = display->getHeight() - y - h;
				}
				else if (count == 2) {
					// setting size only
					setSize = true;
				}
			}
		}

		// if no size was set then use full screen and allow the user
		// to change video formats
		if (!setSize) {
			useFullscreen = true;
			allowResolutionChange = true;
			window->setFullscreen();
		}

		// otherwise set window position and size
		else {
			window->setSize(w, h);
			if (setPosition)
				window->setPosition(x, y);
		}
	}

	// set main window's minimum size
	window->setMinSize(256, 192);

	// bind the rendering context
	window->makeCurrent();

	// open audio device and read sound files.  must do this after
	// creating the window because DirectSound is a bonehead API.
	// if we can't open the audio then mark the feature as missing.
	if (BZDB->isTrue("featuresAudio")) {
		openSound("bzflag");
		if (!isSoundOpen())
			BZDB->set("featuresAudio", "0");
	}

	// grab the cursor if full screen
	if (useFullscreen)
		BZDB->set("displayGrabCursor", "1");

	// set gamma if we have gamma control
	if (BZDB->isTrue("featuresGamma")) {
		if (!window->hasGammaControl())
			BZDB->set("featuresGamma", "0");
		else
			onGammaChanged("renderGamma", NULL);
	}

	// force update of cursor visibility
	onCursorChanged("displayCursor", NULL);

	// map font names to font files
	for (i = 0; i < countof(fontMap); ++i)
		OpenGLTexFont::mapFont(fontMap[i].name, fontMap[i].filename);

	// prepare menus
	{
		istream* stream = FILEMGR->createDataInStream("menu.bzc");
		if (stream) {
			MenuReader reader;
			reader.read(*stream);
			delete stream;
		}
	}

	// add custom view items
	ViewItems::init();

	// prepare views
	{
		istream* stream = FILEMGR->createDataInStream("view.bzc");
		if (stream) {
			ViewReader reader;
			reader.read(*stream);
			delete stream;
		}
	}

	// register color change callbacks and update colors now
	BZDB->addCallback("colorRadarRogue",  onColorChangeCB, NULL);
	BZDB->addCallback("colorRadarRed",    onColorChangeCB, NULL);
	BZDB->addCallback("colorRadarGreen",  onColorChangeCB, NULL);
	BZDB->addCallback("colorRadarBlue",   onColorChangeCB, NULL);
	BZDB->addCallback("colorRadarPurple", onColorChangeCB, NULL);
	onColorChangeCB(BzfString(), NULL);

	// start playing
	startPlaying(display, window);

	// save resources
	{
		ostream* resourceStream = PLATFORM->createConfigOutStream();
		if (resourceStream) {
			// open command section
			(*resourceStream) << "<command>" << std::endl;

			// write state database
			BZDB->write(writeDBEntry, resourceStream);

			// write key bindings
			KEYMGR->iterate(writeKeys, resourceStream);

			// close command section
			(*resourceStream) << "</command>" << std::endl;

			// done
			delete resourceStream;
		}
	}

	// shut down
	delete window;
	window = NULL;
	delete visual;
	closeSound();
	delete display;
	delete MPLATFORM;
	delete PLATFORM;
	delete FILEMGR;
	delete MENUMGR;
	delete CMDMGR;
	delete BZDB;
	// FIXME -- clean up other singletons

	return 0;
}

#if defined(_WIN32)

#include <stdlib.h>

//
// WinMain()
//		windows entry point.  forward to main()
//

int WINAPI				WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// startup winsock
	{
		static const int major = 2, minor = 2;
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
			printError("Failed to initialize winsock.  Terminating.\n");
			return 1;
		}
		if (LOBYTE(wsaData.wVersion) != major ||
		HIBYTE(wsaData.wVersion) != minor) {
			printError("Version mismatch in winsock;"
						"  got %d.%d.  Terminating.\n",
						(int)LOBYTE(wsaData.wVersion),
						(int)HIBYTE(wsaData.wVersion));
			WSACleanup();
			return 1;
		}
	}

	const int exitCode = myMain(__argc, __argv);

	// clean up and return exit code
	WSACleanup();
	return exitCode;
}

#endif /* defined(_WIN32) */
