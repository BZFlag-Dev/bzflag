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

static const char copyright[] = "Copyright (c) 1993 - 2003 Tim Riker";

#if defined(_WIN32)
	#pragma warning(disable: 4786)
	#pragma warning(disable: 4100)
	#pragma warning(disable: 4511)
#endif

#include <string>
#include <vector>
#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include "bzsignal.h"
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#pragma warning( 4 : 4786 )
#define _WINSOCKAPI_
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#endif
#if defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#endif

// yikes! that's a lotsa includes!
#include "common.h"
#include "playing.h"
#include "BzfDisplay.h"
#include "BzfEvent.h"
#include "BzfWindow.h"
#include "BzfMedia.h"
#include "PlatformFactory.h"
#include "global.h"
#include "Address.h"
#include "Protocol.h"
#include "Pack.h"
#include "ServerLink.h"
#include "PlayerLink.h"
#include "resources.h"
#include "SceneRenderer.h"
#include "SceneBuilder.h"
#include "SceneDatabase.h"
#include "BackgroundRenderer.h"
#include "RadarRenderer.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "World.h"
#include "Team.h"
#include "Flag.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "RobotPlayer.h"
#include "MainWindow.h"
#include "ControlPanel.h"
#include "ShotStrategy.h"
#include "StateDatabase.h"
#include "KeyManager.h"
#include "CommandManager.h"
#include "daylight.h"
#include "sound.h"
#include "TimeBomb.h"
#include "HUDDialog.h"
#include "menus.h"
#include "texture.h"
#include "ErrorHandler.h"
#include "ZSceneDatabase.h"
#include "QuadWallSceneNode.h"
#include "BillboardSceneNode.h"
#include "KeyMap.h"
#include "Intersect.h"
#include "Ping.h"
#include "OpenGLTexture.h"
#include "BundleMgr.h"
#include "Bundle.h"
#include "CommandsStandard.h"

#define MAX_MESSAGE_HISTORY (20)

static const float	FlagHelpDuration = 60.0f;

static StartupInfo	startupInfo;
static BzfKeyMap	keymap;
static MainMenu*	mainMenu;
static ServerLink*	serverLink = NULL;
static PlayerLink*	playerLink = NULL;
static World*		world = NULL;
static LocalPlayer*	myTank = NULL;
static BzfDisplay*	display = NULL;
static MainWindow*	mainWindow = NULL;
static ResourceDatabase *resources = NULL;
static SceneRenderer*	sceneRenderer = NULL;
static SceneDatabase*	zScene = NULL;
static SceneDatabase*	bspScene = NULL;
ControlPanel*	controlPanel = NULL;
static RadarRenderer*	radar = NULL;
static HUDRenderer*	hud = NULL;
static SceneDatabaseBuilder* sceneBuilder = NULL;
static Team*		teams = NULL;
static int		maxPlayers = 0;		// not including me
static int		curMaxPlayers = 0;
static RemotePlayer**	player = NULL;
static int		numFlags = 0;
static JoinGameCallback	joinGameCallback = NULL;
static void*		joinGameUserData = NULL;
static bool		admin = false; // am I an admin?
static bool		serverError = false;
static bool		serverDied = false;
static bool		fireButton = false;
static bool		restartOnBase = false;
static bool		firstLife = false;
static bool		showFPS = false;
static bool		showDrawTime = false;
static bool		pausedByUnmap = false;
static bool		unmapped = false;
static int		preUnmapFormat = -1;
static double		epochOffset;
static double		lastEpochOffset;
static float		clockAdjust = 0.0f;
static float		pauseCountdown = 0.0f;
static float		destructCountdown = 0.0f;
static float		testVideoFormatTimer = 0.0f;
static int		testVideoPrevFormat = -1;
static std::vector<PlayingCallbackItem>	playingCallbacks;
bool			gameOver = false;
static bool		Observer = false;
static OpenGLTexture*	tankTexture = NULL;
static std::vector<BillboardSceneNode*>	explosions;
static std::vector<BillboardSceneNode*>	prototypeExplosions;
static int		savedVolume = -1;
static bool		grabMouseAlways = false;
FlashClock		pulse;

static char		messageMessage[PlayerIdPLen + MessageLen];

static std::deque<std::string> messageHistory;
static unsigned int	messageHistoryIndex = 0;
static std::vector<std::string>	silencePlayers;

static void		restartPlaying();
static void		setTarget();
static void		setHuntTarget();
static void		handleFlagDropped(Player* tank);
static void		handlePlayerMessage(uint16_t, uint16_t, void*);
static Player*		getPlayerByName( const char* name );
static void		addMessage(const Player* player, const std::string& msg,
                                   bool highlight=false, const char* oldColor=NULL);
extern void		dumpResources(BzfDisplay*, SceneRenderer&);

enum BlowedUpReason {
			GotKilledMsg,
			GotShot,
			GotRunOver,
			GotCaptured,
			GenocideEffect,
			SelfDestruct
};
static const char*	blowedUpMessage[] = {
			  NULL,
			  "Got shot by ",
			  "Got flattened by ",
			  "Team flag was captured by ",
			  "Teammate hit with Genocide by ",
			  "Tank Self Destructed",
			};
static bool		gotBlowedUp(BaseLocalPlayer* tank,
					BlowedUpReason reason,
					PlayerId killer,
					int shotId = -1);

#ifdef ROBOT
static void		handleMyTankKilled(int reason);
static ServerLink*	robotServer[MAX_ROBOTS];
static RobotPlayer*	robots[MAX_ROBOTS];
static int		numRobots = 0;
#endif

extern struct tm	userTime;
static double		userTimeEpochOffset;

StartupInfo::StartupInfo() : hasConfiguration(false),
				autoConnect(false),
				serverPort(ServerPort),
				ttl(DefaultTTL),
				team(RogueTeam),
				listServerURL(DefaultListServerURL),
				listServerPort(ServerPort + 1)
{
  strcpy(serverName, "");
  strcpy(multicastInterface, "");
  strcpy(callsign, "");
  strcpy(email, "");
  joystickName = "joystick";
  joystick = false;
}

// access silencePlayers from bzflag.cxx
std::vector<std::string>& getSilenceList()
{
  return silencePlayers;
}

//
// should we grab the mouse?
//

static void		setGrabMouse(bool grab)
{
  grabMouseAlways = grab;
}

static bool		shouldGrabMouse()
{
  return grabMouseAlways && !unmapped &&
			(myTank == NULL || !myTank->isPaused());
}

//
// some simple global functions
//

BzfDisplay*		getDisplay()
{
  return display;
}

MainWindow*		getMainWindow()
{
  return mainWindow;
}

SceneRenderer*		getSceneRenderer()
{
  return sceneRenderer;
}

void			setSceneDatabase()
{
  if (BZDB->isTrue("zbuffer")) {
    sceneRenderer->setSceneDatabase(zScene);
  }
  else {
    sceneRenderer->setSceneDatabase(bspScene);
  }
}

StartupInfo*		getStartupInfo()
{
  return &startupInfo;
}

BzfKeyMap&			getBzfKeyMap()
{
  return keymap;
}

bool			setVideoFormat(int index, bool test)
{
#if defined(_WIN32)
  // give windows extra time to test format (context reloading takes a while)
  static const float testDuration = 10.0f;
#else
  static const float testDuration = 5.0f;
#endif

  // ignore bad formats or when the format test timer is running
  if (testVideoFormatTimer != 0.0f || !display->isValidResolution(index))
    return false;

  // ignore if no change
  if (display->getResolution() == index) return true;

  // change it
  testVideoPrevFormat = display->getResolution();
  if (!display->setResolution(index)) return false;

  // handle resize
  mainWindow->setFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  mainWindow->warpMouse();
  if (test) testVideoFormatTimer = testDuration;
  else if (shouldGrabMouse()) mainWindow->grabMouse();
  return true;
}

void			addPlayingCallback(PlayingCallback cb, void* data)
{
  PlayingCallbackItem item;
  item.cb = cb;
  item.data = data;
  playingCallbacks.push_back(item);
}

void			removePlayingCallback(PlayingCallback _cb, void* data)
{
  std::vector<PlayingCallbackItem>::iterator it = playingCallbacks.begin();
  while(it != playingCallbacks.end()) {
    if(it->cb == _cb && it->data == data) {
      playingCallbacks.erase(it);
      break;
    }
    it++;
  }
}

static void		callPlayingCallbacks()
{
  const int count = playingCallbacks.size();
  for (int i = 0; i < count; i++) {
    const PlayingCallbackItem& cb = playingCallbacks[i];
    (*cb.cb)(cb.data);
  }
}

void			joinGame(JoinGameCallback cb, void* data)
{
  joinGameCallback = cb;
  joinGameUserData = data;
}

//
// handle joining status when server provided on command line
//

void			joinGameHandler(bool okay, void*)
{
  if (!okay) printError("Connection failed.");
}

//
// handle signals that should kill me quickly
//

static void		dying(int sig)
{
  bzSignal(sig, SIG_DFL);
  display->setDefaultResolution();
  raise(sig);
}

//
// handle signals that should kill me nicely
//

static void		suicide(int sig)
{
  bzSignal(sig, SIG_PF(suicide));
  CommandsStandard::quit();
}

//
// handle signals that should disconnect me from the server
//

static void		hangup(int sig)
{
  bzSignal(sig, SIG_PF(hangup));
  serverDied = true;
  serverError = true;
}

//
// misc utility routines
//

Player*			lookupPlayer(PlayerId id)
{
  // check my tank first
  if (myTank->getId() == id)
    return myTank;

  if (id < curMaxPlayers && player[id] && player[id]->getId() == id)
    return player[id];

  // it's nobody we know about
  return NULL;
}

static int		lookupPlayerIndex(PlayerId id)
{
  // check my tank first
  if (myTank->getId() == id)
    return -2;

  if (id < curMaxPlayers && player[id] && player[id]->getId() == id)
    return id;

  // it's nobody we know about
  return -1;
}

static Player*		getPlayerByIndex(int index)
{
  if (index == -2)
    return myTank;
  if (index == -1 || index >= curMaxPlayers)
    return NULL;
  return player[index];
}

static Player*		getPlayerByName(const char* name)
{
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i] && strcmp( player[i]->getCallSign(), name ) == 0)
      return player[i];
  return NULL;
}

static BaseLocalPlayer*	getLocalPlayer(PlayerId id)
{
  if (myTank->getId() == id) return myTank;
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->getId() == id)
      return robots[i];
#endif
  return NULL;
}

static TeamColor	PlayerIdToTeam(PlayerId id)
{
	if (id >= 245 && id<=250)
		return TeamColor(250 - id);
	else
		return NoTeam;
}

static PlayerId		TeamToPlayerId(TeamColor team)
{
	if (team == NoTeam)
		return NoPlayer;
	else
		return 250-team;
}


static ServerLink*	lookupServer(const Player* player)
{
  PlayerId id = player->getId();
  if (myTank->getId() == id) return serverLink;
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->getId() == id)
      return robotServer[i];
#endif
  return NULL;
}

//
// ui control default key handler classes
//

class ComposeDefaultKey : public HUDuiDefaultKey {
  public:
    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);
};

void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB->get(name) << std::endl;
}

bool			ComposeDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  if (keymap.isMappedTo(BzfKeyMap::Jump, key)) {
    // jump while typing
    myTank->jump();
  }

  if (!myTank->isKeyboardMoving()) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down))
      return true;
  }
  
  switch (key.ascii) {
    case 3:	// ^C
    case 27:	// escape
//    case 127:	// delete
      sendIt = false;			// finished composing -- don't send
      break;

    case 4:	// ^D
    case 13:	// return
      sendIt = true;
      break;

    default:
      return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();
    if (message.length() > 0) {
      const char* silence = message.c_str();
      if (strncmp(silence, "SILENCE", 7) == 0) {
	Player *loudmouth = getPlayerByName(silence + 8);
	if (loudmouth) {
	  silencePlayers.push_back(silence + 8);
	  std::string message = "Silenced ";
	  message += (silence + 8);
	  addMessage(NULL, message);
	}
      } else if (strncmp(silence, "DUMP", 4) == 0) {
	BZDB->iterate(printout, NULL);
      } else if (strncmp(silence, "UNSILENCE", 9) == 0) {
	Player *loudmouth = getPlayerByName(silence + 10);
	if (loudmouth) {
	  std::vector<std::string>::iterator it = silencePlayers.begin();
	  for (; it != silencePlayers.end(); it++) {
	    if (*it == silence + 10) {
	      silencePlayers.erase(it);
	      std::string message = "Unsilenced ";
	      message += (silence + 10);
	      addMessage(NULL, message);
	      break;
	    }
	  }
	}
      } else {
	int i, mhLen = messageHistory.size();
	for (i = 0; i < mhLen; i++) {
	  if (messageHistory[i] == message) {
	    messageHistory.erase(messageHistory.begin() + i);
	    messageHistory.push_front(message);
	    break;
	  }
	}
	if (i == mhLen) {
	  if (mhLen >= MAX_MESSAGE_HISTORY) {
	    messageHistory.pop_back();
	  }
	  messageHistory.push_front(message);
	}

	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
      }
    }
  }

  messageHistoryIndex = 0;
  hud->setComposing(std::string());
  HUDui::setDefaultKey(NULL);
  return true;
}

bool			ComposeDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (!myTank->isKeyboardMoving()) {
    if (key.button == BzfKeyEvent::Up) {
      if (messageHistoryIndex < messageHistory.size()) {
	hud->setComposeString(messageHistory[messageHistoryIndex]);
	messageHistoryIndex++;
      }
      else
	hud->setComposeString(std::string());
      return true;
    }
    else if (key.button == BzfKeyEvent::Down) {
      if (messageHistoryIndex > 0){
	messageHistoryIndex--;
	hud->setComposeString(messageHistory[messageHistoryIndex]);
      }
      else
	hud->setComposeString(std::string());
      return true;
    }
    else if ((key.shift == BzfKeyEvent::ShiftKey || (hud->getComposeString().length() == 0)) &&
	     (key.button == BzfKeyEvent::Left || key.button == BzfKeyEvent::Right)) {
      const Player *recipient = myTank->getRecipient();
      if(!recipient) {
	for (int i = 0; i < curMaxPlayers; i++) {
	  if (player[i]) {
	    myTank->setRecipient(player[i]);
	    break;
	  }
	}
      }
      else {
	const PlayerId id = recipient->getId();
	int rindex = lookupPlayerIndex(id);
	if (key.button == BzfKeyEvent::Left) {
	  for (int i = rindex-1; i >= 0; i--) {
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = curMaxPlayers-1; i >=0; i--) {
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	}
	else
	{
	  for (int i = rindex+1; i < curMaxPlayers; i++) {
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = 0; i < curMaxPlayers; i++) {
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	}
      }
      recipient = myTank->getRecipient();
      if (recipient) {
        void* buf = messageMessage;
	buf = nboPackUByte(buf, recipient->getId());
	std::string composePrompt = "Send to ";
	composePrompt += recipient->getCallSign();
	composePrompt += ": ";
	hud->setComposing(composePrompt);
      }
      return false;
    }
  }
  return keyPress(key);
}

//
// Choose person to silence

class SilenceDefaultKey : public HUDuiDefaultKey {
  public:
    SilenceDefaultKey();
	bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);
};

SilenceDefaultKey::SilenceDefaultKey()
{

}
bool			SilenceDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  if (keymap.isMappedTo(BzfKeyMap::Jump, key)) {
    // jump while typing
    myTank->jump();
  }

  if (!myTank->isKeyboardMoving()) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down) ||
	(key.button == BzfKeyEvent::Left) ||
	(key.button == BzfKeyEvent::Right))

      return true;
  }

  switch (key.ascii) {
    case 3:	// ^C
    case 27:	// escape
//    case 127:	// delete
      sendIt = false;			// finished composing -- don't send
      break;

    case 4:	// ^D
    case 13:	// return
      sendIt = true;
      break;

    default:

      return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();

    // find the name of the person to silence,
    // either by picking through arrow keys or by compose
    const char* name = NULL;

    if (message.size() == 0) {
      // silence just by picking arrowkeys
      const Player * silenceMe = myTank->getRecipient();
      if (silenceMe) {
	name = silenceMe->getCallSign();
      }
    }
    else if (message.size() > 0) {
      // typed in name
      name = message.c_str();
    }

    // if name is NULL we skip
    if (name != NULL) {
      // bad indent :)
      int inListPos = -1;
      for (unsigned int i = 0; i < silencePlayers.size(); i++) {
	if (strcmp(silencePlayers[i].c_str(),name) == 0) {
	  inListPos = i;
	}
      }

      bool isInList = (inListPos != -1);

      Player *loudmouth = getPlayerByName(name);
      if (loudmouth) {
	// we know this person exists
	if (!isInList) {
	  // exists and not in silence list
	  silencePlayers.push_back(name);
	  std::string message = "Silenced ";
	  message += (name);
	  addMessage(NULL, message);
	} else {
	  // exists and in list --> remove from list
	  silencePlayers.erase(silencePlayers.begin() + inListPos);
	  std::string message = "Unsilenced ";
	  message += (name);
	  addMessage(NULL, message);
	}
      } else {
	// person does not exist, but may be in silence list
	if (isInList) {
	  // does not exist but is in list --> remove
	  silencePlayers.erase(silencePlayers.begin() + inListPos);
	  std::string message = "Unsilenced ";
	  message += (name);
	  if (strcmp (name, "*") == 0) {
	    // to make msg fancier
	    message = "Unblocked Msgs";
	  }
	  addMessage(NULL, message);
	} else {
	  // does not exist and not in list -- duh
	  if (name != NULL) {
	    if (strcmp (name,"*") == 0) {
	      // check for * case
	      silencePlayers.push_back(name);
	      std::string message = "Silenced All Msgs";
	      addMessage(NULL, message);
	    } else {
	      std::string message = name;
	      message += (" Does not exist");
	      addMessage(NULL, message);
	    }
	  }
	}
      }
    }
  }

  hud->setComposing(std::string());

  HUDui::setDefaultKey(NULL);
  return true;
}

bool			SilenceDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (!myTank->isKeyboardMoving()) {

    if (key.button == BzfKeyEvent::Up || key.button==BzfKeyEvent::Down
		||key.button==BzfKeyEvent::Left||key.button==BzfKeyEvent::Right) {
      const Player *recipient = myTank->getRecipient();
      if (!recipient) {
	for (int i = 0; i < curMaxPlayers; i++) {
	  if (player[i]) {
	    myTank->setRecipient(player[i]);
	    break;
	  }
	}
      }
      else {
	const PlayerId id = recipient->getId();
	int rindex = lookupPlayerIndex(id);
	if (key.button == BzfKeyEvent::Up ||
		key.button== BzfKeyEvent::Right) {
	  for (int i = rindex-1; i >= 0; i--) {
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = curMaxPlayers-1; i >=0; i--) {
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	}
	else
	{
	  for (int i = rindex+1; i < curMaxPlayers; i++) {
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = 0; i < curMaxPlayers; i++) {
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	}
      }
      recipient = myTank->getRecipient();
      if (recipient) {
	// FIXME change prompt to show current silence state
	std::string composePrompt = "[Un]Silence -->";
	composePrompt += recipient->getCallSign();

	// Set the prompt and disable editing/composing
	hud->setComposing(composePrompt, false);
      }
      return false;
    }
  }
  return keyPress(key);
}

class ServerCommandKey : public HUDuiDefaultKey {
  public:
    ServerCommandKey();
    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);
    void		init();
    void		adminInit();
    void		nonAdminInit();
  private:
    std::string		makePattern(const InAddr& address);
    void		updatePrompt();

  private:
      enum Mode {
      Kick,
      BanIp,
      Ban1,
      Ban2,
      Ban3,
      Unban,
      Showgroup,
      Setgroup,
      Removegroup,
      Ghost,
      Banlist,
      Playerlist,
      FlagReset,
      FlagUnusedReset,
      FlagUp,
      FlagShow,
      FlagHistory,
      IdleStats,
      LagStats,
      Report,
      LagWarn,
      GameOver,
      CountDown,
      SuperKill,
      Shutdown,
      Register,
      Identify,
      Setpass,
      Grouplist,
      Groupperms,
      Password, // leave this as the last item
    };

    Mode mode;
    int startIndex;
    const int numModes;
    const int numNonAdminModes;
    static const Mode nonAdminModes[7];


};

const ServerCommandKey::Mode ServerCommandKey::nonAdminModes [7]= {LagStats, IdleStats, FlagHistory, Report, Password, Register, Identify};

// note the important numModes and numNonAdminModes values inited here
ServerCommandKey::ServerCommandKey(): mode(Kick), startIndex(-1), numModes(30), numNonAdminModes(7)
{
  if (!admin) mode = nonAdminModes[0];
}

void			ServerCommandKey::nonAdminInit()
{
  mode = nonAdminModes[0];
  updatePrompt();
}
void			ServerCommandKey::adminInit()
{
  mode = Kick;
  updatePrompt();
}

void			ServerCommandKey::init()
{
  updatePrompt();
}

void			ServerCommandKey::updatePrompt()
{
  std::string composePrompt, banPattern;
  // decide what should be on the composing prompt
  const Player * recipient = myTank->getRecipient();
  if (mode >= Kick && mode <= Ghost){ // more complicated modes here
    if (recipient) {

      switch (mode){
      case Kick:
	composePrompt = "Kick -> ";
	composePrompt += recipient->getCallSign();
	hud->setComposing(composePrompt, false);
	break;
      case BanIp: case Ban1: case Ban2: case Ban3:
	// Set the prompt and enable editing/composing --> allows to enter ban time
	/* FIXME FIXME FIXME
	 * temporarily breaking bans for playerid->ubyte
	banPattern = makePattern(recipient->id.serverHost);
	composePrompt = "Ban " + banPattern + " -> " + recipient->getCallSign() + " :";
	hud->setComposing(composePrompt, true);
	*/
	break;

      case Setgroup: composePrompt = "Set players group ";
	composePrompt = composePrompt +  " -> " + recipient->getCallSign() + " :";
	hud->setComposing(composePrompt, true);
	break;
      case Removegroup: composePrompt = "Remove player from group ";
	composePrompt = composePrompt +  " -> " + recipient->getCallSign() + " :";
	hud->setComposing(composePrompt, true);
	break;
      case Ghost: composePrompt = "Ghost player [enter your pass] ";
	composePrompt = composePrompt +  " -> " + recipient->getCallSign() + " :";
	hud->setComposing(composePrompt, true);
	break;
      case Showgroup: composePrompt = "Show player's groups ";
	composePrompt = composePrompt +  " -> " + recipient->getCallSign();
	hud->setComposing(composePrompt, false);
	break;

      default : /* shouldn't happen */ break;

      }


    } else { // no recipient -- we are in editing mode -- kick or ban

      switch (mode){
      case Kick:
	hud->setComposing("Kick :", true);
	break;
      case BanIp: case Ban1: case Ban2: case Ban3:
	hud->setComposing("Ban :", true);
	break;
      case Setgroup: composePrompt = "Set player's group :";
	hud->setComposing(composePrompt, true);
	break;
      case Removegroup: composePrompt = "Remove player from group :";
	hud->setComposing(composePrompt, true);
	break;
      case Ghost: composePrompt = "Ghost :";
	hud->setComposing(composePrompt, true);
	break;
      case Showgroup: composePrompt = "Show players group :";
	hud->setComposing(composePrompt, true);
      break;

      default: /* shouldn't happen */ break;
      }

    }

  } else { // not kick or ban stuff -- should be less complicated
    switch (mode){

    case Unban: hud->setComposing("Unban :", true); break;
    case Banlist: hud->setComposing("Show ban list", false); break;
    case Playerlist: hud->setComposing("Show player list", false); break;
    case FlagReset: hud->setComposing("Reset Flags", false); break;
    case FlagUnusedReset: hud->setComposing("Reset Unused Flags", false); break;
    case FlagUp: hud->setComposing("Remove Flags", false); break;
    case GameOver: hud->setComposing("End Game", false); break;
    case CountDown: hud->setComposing("Restart Countdown:", true); break;
    case FlagShow: hud->setComposing("Show Flag Info", false); break;
    case Shutdown: hud->setComposing("Shut Down Server", false); break;
    case SuperKill: hud->setComposing("Disconnect all Players", false); break;
    case LagWarn: hud->setComposing("Lag Warning threshold:", true); break;
    case IdleStats: hud->setComposing("Idle Stats", false); break;
    case LagStats: hud->setComposing("Lag / Ping Stats", false); break;
    case FlagHistory: hud->setComposing("Flag History", false); break;
    case Password: hud->setComposing("Admin Password:", true); break;
    case Report: hud->setComposing("Send Report to Server:", true); break;
    case Register: hud->setComposing("Register your nick [enter pass]:", true); break;
    case Identify: hud->setComposing("Login [enter pass]:", true); break;
    case Setpass: hud->setComposing("Set your password [enter pass]:", true); break;
    case Grouplist :  hud->setComposing("List Groups", false); break;
    case Groupperms :  hud->setComposing("List Permissions", false); break;



    default: /* shouldn't happen */ break;
    }


  }

}

// return the right ban pattern 123.32.12.* for example depending on the
// mode of the class. Returns an empty string on errors.
std::string		ServerCommandKey::makePattern(const InAddr& address)
{
  const char *  c = inet_ntoa(address);
  if (c == NULL) return "";
  std::string dots  = c;
  std::vector<std::string> dotChunks = string_util::tokenize(dots,".");
  if (dotChunks.size() != 4) return "";

  switch (mode){
  case BanIp:
    return dots;
  case Ban1:
    return dotChunks[0] +"."+ dotChunks[1] + "." + dotChunks[2] + ".*";
  case Ban2:
    return dotChunks[0] +"."+ dotChunks[1] + ".*.*";
  case Ban3:
    return dotChunks[0] +".*.*.*";
  default:
    ;
  }

  return "";
}

bool			ServerCommandKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  if (keymap.isMappedTo(BzfKeyMap::Jump, key)) {
    // jump while typing
    myTank->jump();
  }

  if (!myTank->isKeyboardMoving()) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down) ||
	(key.button == BzfKeyEvent::Left) ||
	(key.button == BzfKeyEvent::Right))

      return true;
  }

  switch (key.ascii) {
    case 3:	// ^C
    case 27:	// escape
//    case 127:	// delete
      sendIt = false;			// finished composing -- don't send
      break;

    case 4:	// ^D
    case 13:	// return
      sendIt = true;
      break;

    default:

      return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();
    std::string banPattern,sendMsg,displayMsg,name;

    const Player * troll = myTank->getRecipient();
    if (mode >= Kick && mode <=Ghost){ // handle more complicated modes
      if (troll) { // cases where we select recipient with keys

	name = troll->getCallSign();

	switch (mode){

	case Kick:
	  sendMsg="/kick " + name;
	  break;
	case BanIp: case Ban1: case Ban2: case Ban3:

	  /* FIXME FIXME FIXME
	   * temporarily break ban-by-name for playerid->ubyte
	  banPattern = makePattern(troll->id.serverHost);
	  sendMsg="/ban " + banPattern;

	  if (message != ""){ // add ban length if something is there
	    sendMsg = sendMsg + " " + message;
	  }
	  */
	  break;

	case Setgroup:
	  sendMsg = "/setgroup";
	  sendMsg = sendMsg + " \"" +name+ "\"" +" " + message;
	  break;
	case Removegroup:
	  sendMsg = "/removegroup";
	  sendMsg = sendMsg + " \"" +name+ "\"" +" " + message;
	  break;
	case Ghost:
	  sendMsg = "/ghost";
	  sendMsg = sendMsg + " \"" +name+ "\"" +" " + message;
	  break;
	case Showgroup:
	  sendMsg = "/showgroup";
	  sendMsg = sendMsg + " \"" +name+ "\"";
	  break;

	default: /* shouldn't happen */ break;

	}

      } else { // no recipient -- editing mode

	switch (mode){
	  case Kick:  sendMsg="/kick"; break;
	  case BanIp: sendMsg="/ban"; break;
	  case Setgroup: sendMsg = "/setgroup"; break;
	  case Removegroup: sendMsg = "/removegroup"; break;
	  case Ghost: sendMsg = "/ghost"; break;
	  case Showgroup: sendMsg = "/showgroup"; break;

	  default: /* shouldn't happen */ break;
	}
	if (message != "") sendMsg = sendMsg + " " + message;

      }
    } else { // handle less complicated messages
      switch (mode){
	case Unban: sendMsg="/unban " + message; break;
	case Banlist: sendMsg="/banlist";  break;
	case Playerlist: sendMsg="/playerlist";  break;
	case FlagReset:  sendMsg="/flag reset"; break;
	case FlagUnusedReset: sendMsg="/flag reset unused"; break;
	case FlagUp: sendMsg="/flag up"; break;
	case GameOver: sendMsg="/gameover"; break;
	case CountDown: sendMsg="/countdown "+ message; break;
	case FlagShow: sendMsg="/flag show"; break;
	case Shutdown: sendMsg="/shutdownserver"; break;
	case SuperKill: sendMsg="/superkill"; break;
	case LagWarn: sendMsg="/lagwarn "+ message; break;
	case IdleStats: sendMsg="/idlestats"; break;
	case LagStats: sendMsg="/lagstats"; break;
	case FlagHistory: sendMsg="/flaghistory"; break;
	case Password: sendMsg = "/password "+ message; break;
	case Report: sendMsg = "/report "+ message; break;
	case Register: sendMsg = "/register "+ message; break;
	case Identify: sendMsg = "/identify "+ message; break;
	case Setpass: sendMsg = "/setpass "+ message; break;
	case Grouplist: sendMsg = "/grouplist"; break;
	case Groupperms: sendMsg = "/groupperms"; break;

	default: /* shouldn't happen */ break;
      }

    }

    // send the message on its way if it isn't empty
    if (sendMsg != ""){
      displayMsg = "-> \"" + sendMsg + "\"";
      if (sendMsg.find("/password",0) == std::string::npos)
	addMessage(NULL, displayMsg);

      void* buf = messageMessage;
      buf = nboPackUByte(buf, ServerPlayer);

      char messageBuffer[MessageLen];
      memset(messageBuffer, 0, MessageLen);
      strncpy(messageBuffer, sendMsg.c_str(), MessageLen);
      buf = nboPackString(buf, messageBuffer, MessageLen);
      serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
    }
  }

  hud->setComposing(std::string());
  //myTank->setRecipient(NULL);
  HUDui::setDefaultKey(NULL);
  return true;
}

bool			ServerCommandKey::keyRelease(const BzfKeyEvent& key)
{
  if (!myTank->isKeyboardMoving()) {

    if (key.button == BzfKeyEvent::Up || key.button==BzfKeyEvent::Down
		||key.button==BzfKeyEvent::Left||key.button==BzfKeyEvent::Right) {
      const Player *recipient = myTank->getRecipient();

      if (recipient) {  // handle selecting another player if <-- or --> hit
	const PlayerId id = recipient->getId();
	int rindex = lookupPlayerIndex(id);

	if (key.button== BzfKeyEvent::Left) {
	  for (int i = rindex-1; i >= 0; i--) {
	    if (i == startIndex && startIndex != -1){
	      myTank->setRecipient(NULL);
	      startIndex = -1;
	      break;
	    }
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = curMaxPlayers-1; i >=0; i--) {
	      if (i == startIndex && startIndex != -1){
		myTank->setRecipient(NULL);
		startIndex = -1;
		break;
	      }
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	} else if (key.button == BzfKeyEvent::Right) {
	  for (int i = rindex+1; i < curMaxPlayers; i++) {
	    if (i == startIndex && startIndex != -1){
	      myTank->setRecipient(NULL);
	      startIndex = -1;
	      break;
	    }
	    if (player[i]) {
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	  if (recipient == myTank->getRecipient()) {
	    for (int i = 0; i < curMaxPlayers; i++) {
	      if (i == startIndex && startIndex != -1){
	      myTank->setRecipient(NULL);
	      startIndex = -1;
	      break;
	      }
	      if (player[i]) {
		myTank->setRecipient(player[i]);
		break;
	      }
	    }
	  }
	}

      } else { // there is no recipient so choose one if <-- or --> hit
	if (!recipient && (key.button == BzfKeyEvent::Left ||key.button == BzfKeyEvent::Right) ) {
	  for (int i = 0; i < curMaxPlayers; i++) {
	    if (player[i]) {
	      startIndex = i; // we pretend a null player is at this pos
	      myTank->setRecipient(player[i]);
	      break;
	    }
	  }
	}
      }


      recipient = myTank->getRecipient();

      // choose which mode we are in
      int maxModes;
      if (admin){
	maxModes = numModes;
      } else {
	maxModes = numNonAdminModes;
      }

      if (key.button == BzfKeyEvent::Down){
	int newMode = mode;
	if (!admin){
	  bool foundIt = false;
	  for (int i = 0; i < numNonAdminModes; i ++){
	    if (mode == nonAdminModes[i]) {
	      newMode = i;
	      foundIt = true;
	    }
	  }
	  if (!foundIt) newMode = 0;
	}

	newMode ++;
	if (newMode >= maxModes) newMode =0;
	mode = (admin? ((Mode)newMode): nonAdminModes[newMode]);
	// if no recipient skip Ban1,2,3 -- applies to admin mode
	if (!recipient && (mode >= Ban1 && mode <= Ban3))
	  mode = Unban;

      } else if (key.button == BzfKeyEvent::Up){
	int newMode = (int) mode;

	bool foundIt = false;
	if (!admin){
	  for (int i = 0; i < numNonAdminModes; i ++){
	    if (mode == nonAdminModes[i]) {
	      newMode = i;
	      foundIt = true;
	    }
	  }
	  if (!foundIt) newMode = 0;
	}

	newMode--;
	if (newMode < 0) newMode = maxModes -1;
	mode = (admin? ((Mode) newMode): nonAdminModes[newMode]);
	// if no recipient skip Ban1,2,3 -- applies to admin mode
	if (!recipient && (mode >= Ban1 && mode <= Ban3))
	  mode = BanIp;
      }

      //update composing prompt
      updatePrompt();
      return false;
    }
  }
  return keyPress(key);
}



//
// user input handling
//

#if defined(DEBUG)
#define FREEZING
#endif
#if defined(FREEZING)
static bool		motionFreeze = false;
#endif
static bool		roaming = false;
enum roamingView {
  roamViewFree = 0,
  roamViewTrack,
  roamViewFollow,
  roamViewFP,
  roamViewFlag,
  roamViewCount
} roamView = roamViewFP;
static int		roamTrackTank = -1, roamTrackWinner = -1, roamTrackFlag = 0;
static float		roamPos[3] = { 0.0f, 0.0f, MuzzleHeight },
			roamDPos[3] = {0.0f, 0.0f, 0.0f};
static float		roamTheta = 0.0f, roamDTheta = 0.0f;
static float		roamPhi = 0.0f, roamDPhi = 0.0f;
static float		roamZoom = 60.0f, roamDZoom = 0.0f;

static void setRoamingLabel(bool force)
{
  if (!player)
    return;
  char *winner;
  if (roamTrackTank == -1) {
    int oldWinner = roamTrackWinner;
    if (roamTrackWinner == -1) {
      // in case we don't find one
      roamTrackWinner = 0;
    }
    // FIXME find the current living winner alive
    int bestScore = -65536; // nobody should be this bad, should they?
    for (int i = 0; i < curMaxPlayers; i++) {
      if (player[i] && player[i]->isAlive() && player[i]->getScore() >= bestScore) {
	roamTrackWinner = i;
	bestScore = player[i]->getScore();
      }
    }
    if (!force && roamTrackWinner == oldWinner)
      return;
    winner="Winner ";
  } else {
    winner="";
  }
  if (player[roamTrackWinner]) {
    switch (roamView) {
      case roamViewTrack:
	hud->setRoamingLabel(std::string("Tracking ") + winner +
			     player[roamTrackWinner]->getCallSign());
	break;

      case roamViewFollow:
	hud->setRoamingLabel(std::string("Following ") + winner +
			     player[roamTrackWinner]->getCallSign());
	break;

      case roamViewFP:
	hud->setRoamingLabel(std::string("Driving with ") + winner +
			     player[roamTrackWinner]->getCallSign());
	break;

      case roamViewFlag:
	hud->setRoamingLabel(std::string("Tracking ") +
			     world->getFlag(roamTrackFlag).desc->flagName +
			     " Flag");
	break;

      default:
	hud->setRoamingLabel(std::string("Roaming"));
	break;
    }
  }
  else
    hud->setRoamingLabel("Roaming");
}

static void		showKeyboardStatus()
{
  if (myTank->isKeyboardMoving())
    controlPanel->addMessage("Keyboard movement");
  else if (mainWindow->joystick())
    controlPanel->addMessage("Joystick movement");
  else
    controlPanel->addMessage("Mouse movement");
}

static bool		doKeyCommon(const BzfKeyEvent& key, bool pressed)
{
  const std::string cmd = KEYMGR->get(key, pressed);
  if (!cmd.empty()) {
    std::string result = CMDMGR->run(cmd);
    if (!result.empty())
      std::cerr << result << std::endl;
    return true;
  }
  if (key.ascii == 27) {
    if (pressed) HUDDialogStack::get()->push(mainMenu);
    return true;
  } else if (keymap.isMappedTo(BzfKeyMap::Hunt, key)) {
    if (pressed) {
      if (hud->getHunting())
	hud->setHunting(false);
      else {
        playLocalSound(SFX_HUNT);
        hud->setHunt(!hud->getHunt());
        hud->setHuntPosition(0);
	if (!BZDB->isTrue("showscore"))
	  BZDB->set("showscore", "1");
      }
    }
    return true;
  } else if (hud->getHunt()) {
    if (key.button == BzfKeyEvent::Down ||
        keymap.isMappedTo(BzfKeyMap::Identify, key)) {
      if (pressed) {
        hud->setHuntPosition(hud->getHuntPosition()+1);
      }
      return true;
    } else if (key.button == BzfKeyEvent::Up ||
               keymap.isMappedTo(BzfKeyMap::DropFlag, key)) {
      if (pressed) {
        hud->setHuntPosition(hud->getHuntPosition()-1);
      }
      return true;
    } else if (keymap.isMappedTo(BzfKeyMap::FireShot, key)) {
      if (pressed) {
        hud->setHuntSelection(true);
        playLocalSound(SFX_HUNT_SELECT);
      }
      return true;
    }
  } else {
    // built-in unchangeable keys.  only perform if not masked.
    switch (key.ascii) {
      case 'T':
      case 't':
	// toggle frames-per-second display
	if (keymap.isMapped(key.ascii) == BzfKeyMap::LastKey) {
	  if (pressed) {
	    showFPS = !showFPS;
	    if (!showFPS) hud->setFPS(-1.0);
	  }
	  return true;
	}
	break;

      case 'Y':
      case 'y':
	// toggle milliseconds for drawing
	if (keymap.isMapped(key.ascii) == BzfKeyMap::LastKey) {
	  if (pressed) {
	    showDrawTime = !showDrawTime;
	    if (!showDrawTime) hud->setDrawTime(-1.0);
	  }
	  return true;
	}
	break;

/* XXX -- for testing forced recreation of OpenGL context
      case 'o':
	if (pressed) {
	  // destroy OpenGL context
	  getMainWindow()->getWindow()->freeContext();

	  // recreate OpenGL context
	  getMainWindow()->getWindow()->makeContext();

	  // force a redraw (mainly for control panel)
	  getMainWindow()->getWindow()->callExposeCallbacks();

	  // cause sun/moon to be repositioned immediately
	  lastEpochOffset = epochOffset - 5.0;

	  // reload display lists and textures and initialize other state
	  OpenGLGState::initContext();
	}
	break;
*/

      case ']':
      case '}':
	// plus 30 seconds
	if (keymap.isMapped(key.ascii) == BzfKeyMap::LastKey) {
	  if (pressed) clockAdjust += 30.0f;
	  return true;
	}
	break;

      case '[':
      case '{':
	// minus 30 seconds
	if (keymap.isMapped(key.ascii) == BzfKeyMap::LastKey) {
	  if (pressed) clockAdjust -= 30.0f;
	  return true;
	}
	break;
    }
  }

  return false;
}

static void		doKeyNotPlaying(const BzfKeyEvent& key, bool pressed)
{
  // handle key
  if (HUDDialogStack::get()->isActive()) {
    if (pressed) HUDui::keyPress(key);
    else HUDui::keyRelease(key);
  }
  else {
    doKeyCommon(key, pressed);
  }
}

static void		doKeyPlaying(const BzfKeyEvent& key, bool pressed)
{
  static ComposeDefaultKey composeKeyHandler;
  static SilenceDefaultKey silenceKeyHandler;
  static ServerCommandKey serverCommandKeyHandler;

  if (HUDui::getFocus())
    if ((pressed && HUDui::keyPress(key)) ||
	(!pressed && HUDui::keyRelease(key)))
      return;

  if (doKeyCommon(key, pressed)) return;

#if defined(FREEZING)
  if (key.ascii == '`' && pressed) {
    // toggle motion freeze
    motionFreeze = !motionFreeze;
  }
  //  else
#endif
  if (roaming) {
    bool roamingkey = true;
    switch (key.button) {
      case BzfKeyEvent::Left:
	if (pressed) {
	  if (key.shift == BzfKeyEvent::ShiftKey)
	    roamDPos[1] =  4.0f * TankSpeed;
	  else
	    roamDTheta = 90.0f * (roamZoom / 90.0f);
	}
	else
	  roamDTheta = roamDPos[1] = 0.0f;
	break;

      case BzfKeyEvent::Right:
	if (pressed) {
	  if (key.shift == BzfKeyEvent::ShiftKey)
	    roamDPos[1] = -4.0f * TankSpeed;
	  else
	    roamDTheta = -90.0f * (roamZoom / 90.0f);
	}
	else
	  roamDTheta = roamDPos[1] = 0.0f;
	break;

      case BzfKeyEvent::Up:
	if (pressed) {
	  if (key.shift == BzfKeyEvent::ShiftKey)
	    roamDPos[0] =  4.0f * TankSpeed;
	  else if (key.shift == BzfKeyEvent::AltKey)
	    roamDPos[2] =  4.0f * TankSpeed;
	  else
	    roamDPhi = -60.0f * (roamZoom / 90.0f);
	}
	else
	  roamDPhi = roamDPos[0] = roamDPos[2] = 0.0f;
	break;

      case BzfKeyEvent::Down:
	if (pressed)
	{
	  if (key.shift == BzfKeyEvent::ShiftKey)
	    roamDPos[0] = -4.0f * TankSpeed;
	  else if (key.shift == BzfKeyEvent::AltKey)
	    roamDPos[2] = -4.0f * TankSpeed;
	  else
	    roamDPhi = 60.0f * (roamZoom / 90.0f);
	}
	else
	  roamDPhi = roamDPos[0] = roamDPos[2] = 0.0f;
	break;

      case BzfKeyEvent::F6:
	if (pressed) {
	  if (roamView == roamViewFree)
	    break;
	  if (roamView == roamViewFlag) {
	    // search next team flag
	    const int maxFlags = world->getMaxFlags();
	    for (int i = 1; i < maxFlags; i++) {
	      int j = (roamTrackFlag - i + maxFlags) % maxFlags;
	      const Flag& flag = world->getFlag(j);
	      if (flag.desc->flagTeam != NoTeam) {
		roamTrackFlag = j;
		break;
	      }
	    }
	  }
	  else {
	    for (int i = 2; i <= curMaxPlayers; i++) {
	      int j = (roamTrackTank - i + curMaxPlayers) % (curMaxPlayers + 1) - 1;
	      if ((j == -1) || (player[j] && player[j]->isAlive())) {
		roamTrackTank = roamTrackWinner = j;
		break;
	      }
	    }
	  }
	  setRoamingLabel(true);
	 }
	 break;

      case BzfKeyEvent::F7:
	if (pressed) {
	  if (roamView == roamViewFree)
	    break;
	  if (roamView == roamViewFlag) {
	    // search previous team flag
	    const int maxFlags = world->getMaxFlags();
	    for (int i = 1; i < maxFlags; i++) {
	      int j = (roamTrackFlag + i) % maxFlags;
	      const Flag& flag = world->getFlag(j);
	      if (flag.desc->flagTeam != NoTeam) {
		roamTrackFlag = j;
		break;
	      }
	    }
	  }
	  else {
	    int i, j;
	    for (i = 2; i <= curMaxPlayers; i++) {
	      j = (roamTrackTank + i) % (curMaxPlayers + 1) - 1;
	      if ((j == -1) || (player[j] && player[j]->isAlive())) {
		roamTrackTank = roamTrackWinner = j;
		break;
	      }
	    }
	  }
	  setRoamingLabel(true);
	}
	break;

      case BzfKeyEvent::F8:
	if (pressed) {
	  roamView = roamingView((roamView + 1) % roamViewCount);
	  if (roamView == roamViewFlag) {
	    const int maxFlags = world->getMaxFlags();
	    bool found = false;
	    for(int i = 0; i < maxFlags; i++) {
	      const Flag& flag = world->getFlag(i);
	      if (flag.desc->flagTeam != NoTeam) {
		roamTrackFlag = i;
		found = true;
		break;
	      }
	    }
	    if(!found)
	      roamView = roamViewFree;
	  }
	  else if ((roamTrackTank != -1) && (roamView == roamViewTrack ||
		roamView == roamViewFollow || roamView == roamViewFP)) {
	    if ((player[roamTrackTank] != NULL) && (!player[roamTrackTank]->isAlive())) {
	      bool found = false;
	      for(int i = 0; i < curMaxPlayers; i++) {
		if(player[i] && player[i]->isAlive()) {
		  roamTrackTank = roamTrackWinner = i;
		  found = true;
		  break;
		}
	      }
	      if(!found)
		roamTrackTank = -1;
	    }
	  }
	  setRoamingLabel(true);
	}
	break;

      case BzfKeyEvent::F9:
	if (pressed)
	  roamDZoom =  50.0;
	else
	  roamDZoom = 0.0;
	break;

      case BzfKeyEvent::F10:
	if (pressed)
	  roamDZoom = -50.0;
	else
	  roamDZoom = 0.0;
	break;

      case BzfKeyEvent::F11:
	if (pressed)
	  roamZoom = 60.0;

      default:
	roamingkey = false;
	break;
    }
    if (roamingkey)
      return;
  }
  //  else

  if (keymap.isMappedTo(BzfKeyMap::Binoculars, key)) {
    if (pressed) {
      if (myTank->getFlag() != Flags::WideAngle)
	myTank->setMagnify(1 - myTank->getMagnify());
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ChooseSilence, key)) {
    if (pressed) {
      messageHistoryIndex = 0;
      hud->setComposing("[Un]Silence  :");
      HUDui::setDefaultKey(&silenceKeyHandler);
    }
  }
  else if (keymap.isMappedTo(BzfKeyMap::ServerCommand, key)) {
    if (pressed) {
      static bool prevAdmin = admin;
      if (prevAdmin == false && admin == true) serverCommandKeyHandler.adminInit();
      if (prevAdmin == true && admin == false) serverCommandKeyHandler.nonAdminInit();
      prevAdmin = admin;

      messageHistoryIndex = 0;
      serverCommandKeyHandler.init();
      HUDui::setDefaultKey(&serverCommandKeyHandler);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ScrollBackward, key)) {
    // scroll message list backward
    if (pressed) {
      controlPanel->setMessagesOffset(2,1);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ScrollForward, key)) {
    // scroll message list forward
    if (pressed) {
      controlPanel->setMessagesOffset(-2,1);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::Labels, key)) {
    // toggle tank labels in roaming mode
    if (pressed) {
      sceneRenderer->setLabels(!sceneRenderer->getLabels());
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::FlagHelp, key)) {
    // toggle flag help
    if (pressed) {
      sceneRenderer->setShowFlagHelp(!sceneRenderer->getShowFlagHelp());
      if (!sceneRenderer->getShowFlagHelp()) hud->setFlagHelp(Flags::Null, 0.0);
      else hud->setFlagHelp(myTank->getFlag(), FlagHelpDuration);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ShortRange, key)) {
    // smallest radar range
    if (pressed) radar->setRange(RadarLowRangeFactor*WorldSize);
  }

  else if (keymap.isMappedTo(BzfKeyMap::MediumRange, key)) {
    // medium radar range
    if (pressed) radar->setRange(RadarMedRangeFactor*WorldSize);
  }

  else if (keymap.isMappedTo(BzfKeyMap::LongRange, key)) {
    // largest radar range
    if (pressed) radar->setRange(RadarHiRangeFactor*WorldSize);
  }

  else if (key.ascii == 0 &&
	   key.button >= BzfKeyEvent::F1 &&
	   key.button <= BzfKeyEvent::F10 &&
	   (key.shift & (BzfKeyEvent::ControlKey +
			 BzfKeyEvent::AltKey)) != 0) {
    // [Ctrl]-[Fx] is message to team
    // [Alt]-[Fx] is message to all
    if (pressed) {
      char name[32];
      int msgno = (key.button - BzfKeyEvent::F1) + 1;
      void* buf = messageMessage;
      if (key.shift == BzfKeyEvent::ControlKey) {
        sprintf(name, "quickTeamMessage%d", msgno);
        buf = nboPackUByte(buf, TeamToPlayerId(myTank->getTeam()));
      } else {
        sprintf(name, "quickMessage%d", msgno);
        buf = nboPackUByte(buf, AllPlayers);
      }
      if (resources->hasValue(name)) {
	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer,
		resources->getValue(name).c_str(),
		MessageLen);
	nboPackString(buf, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
      }
    }
  }
  else if (keymap.isMappedTo(BzfKeyMap::SlowKeyboardMotion, key)) {
    if (myTank->isKeyboardMoving())
      myTank->setSlowKeyboard(pressed);
  }
  // Might be a direction key. Save it for later.
  else if (myTank->isAlive()) {
    if (!myTank->isKeyboardMoving() && pressed)
      switch (key.button)
      {
      case BzfKeyEvent::Left:
      case BzfKeyEvent::Right:
      case BzfKeyEvent::Up:
      case BzfKeyEvent::Down:
	myTank->setKeyboardMoving(true);
	showKeyboardStatus();
	break;
      }
    if (myTank->isKeyboardMoving())
      myTank->setKey(key.button, pressed);
  }
}

static float getKeyValue(bool pressed)
{
  if (pressed)
    return 1;
  return 0;
}

static void		doMotion()
{
#if defined(FREEZING)
  if (motionFreeze) return;
#endif

  float rotation, speed;

  if (myTank->isKeyboardMoving()) {
    rotation = myTank->getKeyboardAngVel();
    speed = myTank->getKeyboardSpeed();

    switch (myTank->getKeyButton())
    {
    case BzfKeyEvent::Left:
      rotation = getKeyValue(myTank->getKeyPressed());
      break;
    case BzfKeyEvent::Right:
      rotation = - getKeyValue(myTank->getKeyPressed());
      break;
    case BzfKeyEvent::Up:
      speed = getKeyValue(myTank->getKeyPressed());
      break;
    case BzfKeyEvent::Down:
      speed = - getKeyValue(myTank->getKeyPressed()) / 2.0f;
      break;
    }

    myTank->setKeyboardAngVel(rotation);
    myTank->setKeyboardSpeed(speed);
    myTank->resetKey();

    if (myTank->getMagnify())
      rotation *= 0.2f;
    if (myTank->hasSlowKeyboard()) {
      rotation /= 2.0f;
      speed /= 2.0f;
    }
  }
  else {
    // get mouse position
    int mx, my;
    if (mainWindow->joystick()) {
      mainWindow->getJoyPosition(mx, my);

      static const BzfKeyEvent::Button button_map[] = { BzfKeyEvent::LeftMouse,
				BzfKeyEvent::MiddleMouse,
				BzfKeyEvent::RightMouse,
				BzfKeyEvent::BZ_Mouse_Button_4,
				BzfKeyEvent::BZ_Mouse_Button_5,
				BzfKeyEvent::BZ_Mouse_Button_6,
				BzfKeyEvent::BZ_Mouse_Button_7,
				BzfKeyEvent::BZ_Mouse_Button_8,
				BzfKeyEvent::BZ_Mouse_Button_9,
				BzfKeyEvent::F1,
				BzfKeyEvent::F2,
				BzfKeyEvent::F3,
				BzfKeyEvent::F4,
				BzfKeyEvent::F5,
				BzfKeyEvent::F6,
				BzfKeyEvent::F7,
				BzfKeyEvent::F8,
				BzfKeyEvent::F9
      };

      static unsigned long old_buttons = 0;
      const int button_count = sizeof(button_map) / sizeof(button_map[0]);
      unsigned long new_buttons = mainWindow->getJoyButtonSet();
      if (old_buttons != new_buttons)
	for (int j = 0; j<button_count; j++)
	  if ((old_buttons & (1<<j)) != (new_buttons & (1<<j))) {
	    BzfKeyEvent ev;
	    ev.button = button_map[j];
	    ev.ascii = 0;
	    ev.shift = 0;
	    doKeyPlaying(ev, (new_buttons&(1<<j)) != 0);
	  }
      old_buttons = new_buttons;
    } else
      mainWindow->getMousePosition(mx, my);

    // calculate desired rotation
    const int noMotionSize = hud->getNoMotionSize();
    const int maxMotionSize = hud->getMaxMotionSize();
    rotation = 0.0f;
    if (mx < -noMotionSize) {
      rotation = float(-mx - noMotionSize) / float(maxMotionSize);
      if (rotation > 1.0f) rotation = 1.0f;
    }
    else if (mx > noMotionSize) {
      rotation = -float(mx - noMotionSize) / float(maxMotionSize);
      if (rotation < -1.0f) rotation = -1.0f;
    }

    // calculate desired speed
    speed = 0.0f;
    if (my < -noMotionSize) {
      speed = float(-my - noMotionSize) / float(maxMotionSize);
      if (speed > 1.0f) speed = 1.0f;
    }
    else if (my > noMotionSize) {
      speed = -float(my - noMotionSize) / float(maxMotionSize);
      if (speed < -0.5f) speed = -0.5f;
    }
  }
  myTank->setDesiredAngVel(rotation);
  myTank->setDesiredSpeed(speed);
}

static std::string cmdJump(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: jump";
  if (myTank != NULL)
    myTank->jump();
  return std::string();
}

static std::string cmdFire(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: fire";
  if (myTank != NULL && myTank->isAlive() && !Observer)
    myTank->fireShot();
  return std::string();
}

static std::string cmdDrop(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: drop";
  if (myTank != NULL) {
    FlagDesc* flag = myTank->getFlag();
    if (flag != Flags::Null && !myTank->isPaused() &&
        flag->flagType != FlagSticky &&
        !(flag == Flags::PhantomZone && myTank->isFlagActive()) &&
	!(flag == Flags::OscillationOverthruster &&
	myTank->getLocation() == LocalPlayer::InBuilding)) {
      serverLink->sendDropFlag(myTank->getPosition());
      // changed: on windows it may happen the MsgDropFlag
      // never comes back to us, so we drop it right away
      handleFlagDropped(myTank);
    }
  }
  return std::string();
}

static std::string cmdIdentify(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: identify";
  if (myTank != NULL) {
    if (!gameOver && !Observer && !myTank->isAlive() && !myTank->isExploding())
      restartPlaying();
    else if (myTank->isAlive() && !myTank->isPaused())
      setTarget();
  }
  return std::string();
}

static std::string cmdDestruct(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: destruct";
  if (myTank != NULL) {
    if (destructCountdown > 0.0f) {
      destructCountdown = 0.0f;
       hud->setAlert(1, "Self Destruct cancelled", 1.5f, true);
    } else {
      destructCountdown = 5.0f;
      char msgBuf[40];
      sprintf(msgBuf, "Self Destructing in %d", (int)(destructCountdown + 0.99f));
      hud->setAlert(1, msgBuf, 1.0f, false);
    }
  }
  return std::string();
}

static std::string cmdPause(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: pause";
  if (!pausedByUnmap && myTank->isAlive()) {
    if (myTank->isPaused()) {
      myTank->setPause(false);
      controlPanel->addMessage("Resumed");

      // restore the sound
      if (savedVolume != -1) {
	setSoundVolume(savedVolume);
	savedVolume = -1;
      }

      // grab mouse
      if (shouldGrabMouse())
	mainWindow->grabMouse();
    } else if (pauseCountdown > 0.0f) {
      pauseCountdown = 0.0f;
      hud->setAlert(1, "Pause cancelled", 1.5f, true);
    } else {
      pauseCountdown = 5.0f;
      char msgBuf[40];
      sprintf(msgBuf, "Pausing in %d", (int) (pauseCountdown + 0.99f));
      hud->setAlert(1, msgBuf, 1.0f, false);
    }
  }
  return std::string();
}

static std::string cmdSend(const std::string&, const CommandManager::ArgList& args)
{
  static ComposeDefaultKey composeKeyHandler;
  if (args.size() != 1)
    return "usage: send {all|team|nemesis|recipient}";
  std::string composePrompt;
  if (args[0] == "all") {
    void* buf = messageMessage;
    buf = nboPackUByte(buf, AllPlayers);
    composePrompt = "Send to all: ";
  } else if (args[0] == "team") {
    void* buf = messageMessage;
    buf = nboPackUByte(buf, TeamToPlayerId(myTank->getTeam()));
    composePrompt = "Send to teammates: ";
  } else if (args[0] == "nemesis") {
    const Player* nemesis = myTank->getNemesis();
    if (!nemesis) return std::string();

    void* buf = messageMessage;
    buf = nboPackUByte(buf, nemesis->getId());
    composePrompt = "Send to ";
    composePrompt += nemesis->getCallSign();
    composePrompt += ": ";
  } else if (args[0] == "recipient") {
    const Player* recipient = myTank->getRecipient();
    if (!recipient) {
      for (int i = 0; i < curMaxPlayers; i++) {
	if (player[i]) {
	  myTank->setRecipient(player[i]);
	  break;
	}
      }
    }
    recipient = myTank->getRecipient();
    if (recipient) {
      void* buf = messageMessage;
      buf = nboPackUByte(buf, recipient->getId());
      composePrompt = "Send to ";
      composePrompt += recipient->getCallSign();
      composePrompt += ": ";
    }
  } else {
    return "usage: send {all|team|nemesis|recipient}";
  }
  messageHistoryIndex = 0;
  hud->setComposing(composePrompt);
  HUDui::setDefaultKey(&composeKeyHandler);
  return std::string();
}

#ifdef SNAPPING
static std::string cmdScreenshot(const std::string&, const CommandManager::ArgList& args)
{
  static int snap = 0;
  if (args.size() != 0)
    return "usage: screenshot";

  std::fstream f;
  std::string filename = string_util::format("bzfi%04d.raw", snap++);
  f.open(filename.c_str(), std::ios::out | std::ios::binary);
  if (f.is_open()) {
    int w = mainWindow->getWidth(), h = mainWindow->getHeight();
    // use something like netpbm and the following command to get usable images
    // rawtoppm -rgb 640 480 bzfi0000.raw | pnmflip -tb | pnmtopng -compression 9 > test.png
    unsigned char* b = new unsigned char[w * h * 3];
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, b);
    // apply gamma correction - FIXME
/*    unsigned char *ptr = b;
    for(int i = 0; i < w * h * 3; i++) {
      *ptr = gammaTable[*ptr];
      ptr++;
    }*/
    f.write(reinterpret_cast<char*>(b), w * h * 3);
    delete [] b;
    f.close();
    char notify[128];
#ifdef _WIN32
    _snprintf(notify, 128, "%s: %dx%d", filename.c_str(), w, h);
#else
    snprintf(notify, 128, "%s: %dx%d", filename.c_str(), w, h);
#endif
    controlPanel->addMessage(notify);
  }
  return std::string();
}
#endif

static std::string cmdTime(const std::string&, const CommandManager::ArgList& args)
{
  // FIXME - time should be moved into BZDB
  if (args.size() != 1)
    return "usage: time {forward|backward}";
  if (args[0] == "forward") {
    clockAdjust += 5.0f * 60.0f;
  } else if (args[0] == "backward") {
    clockAdjust -= 5.0f * 60.0f;
  } else {
    return "usage: time {forward|backward}";
  }
  return std::string();
}

struct CommandListItem {
  const char* name;
  CommandManager::CommandFunction func;
  const char* help;
};

static const CommandListItem commandList[] = {
  { "fire",	&cmdFire,	"fire:  fire a shot" },
  { "jump",	&cmdJump,	"jump:  make player jump" },
  { "drop",	&cmdDrop,	"drop:  drop the current flag" },
  { "identify",	&cmdIdentify,	"identify:  identify/lock-on-to player in view" },
  { "destruct", &cmdDestruct,	"destruct:  self destruct" },
  { "pause",	&cmdPause,	"pause:  pause/resume" },
  { "send",	&cmdSend,	"send {all|team|nemesis|recipient}:  start composing a message" },
#ifdef SNAPPING
  { "screenshot", &cmdScreenshot, "screenshot:  take a screenshot" },
#endif
  { "time",	&cmdTime,	"time {forward|backward}: adjust the current time" }
};

static void		doEvent(BzfDisplay* display)
{
  BzfEvent event;
  if (!display->getEvent(event)) return;

  switch (event.type) {
    case BzfEvent::Quit:
      CommandsStandard::quit();
      break;

    case BzfEvent::Redraw:
      mainWindow->getWindow()->callExposeCallbacks();
      sceneRenderer->setExposed();
      break;

    case BzfEvent::Resize:
      mainWindow->getWindow()->callResizeCallbacks();
      break;

    case BzfEvent::Map:
      // window has been mapped.  this normally occurs when the game
      // is uniconified.  if the player was paused because of an unmap
      // then resume.
      if (pausedByUnmap) {
	pausedByUnmap = false;
	pauseCountdown = 0.0f;
	if (myTank && myTank->isAlive() && myTank->isPaused()) {
	  myTank->setPause(false);
	  controlPanel->addMessage("Resumed");
	}
      }

      // restore the resolution we want if full screen
      if (mainWindow->getFullscreen()) {
	if (preUnmapFormat != -1) {
	  display->setResolution(preUnmapFormat);
	  mainWindow->warpMouse();
	}
      }

      // restore the sound
      if (savedVolume != -1) {
	setSoundVolume(savedVolume);
	savedVolume = -1;
      }

      unmapped = false;
      if (shouldGrabMouse())
	mainWindow->grabMouse();
      break;

    case BzfEvent::Unmap:
      // begin pause countdown when unmapped if:  we're not already
      // paused because of an unmap (shouldn't happen), we're not
      // already counting down to pausing, we're alive, and we're not
      // already paused.
      if (!pausedByUnmap && pauseCountdown == 0.0f &&
	  myTank && myTank->isAlive() && !myTank->isPaused()) {
	// get ready to pause (no cheating through instantaneous pausing)
	pauseCountdown = 5.0f;

	// set this even though we haven't really paused yet
	pausedByUnmap = true;
      }

      // ungrab the mouse if we're running full screen
      if (mainWindow->getFullscreen()) {
	preUnmapFormat = -1;
	if (display->getNumResolutions() > 1) {
	  preUnmapFormat = display->getResolution();
	  display->setDefaultResolution();
	}
      }

      // turn off the sound
      if (savedVolume == -1) {
	savedVolume = getSoundVolume();
	setSoundVolume(0);
      }

      unmapped = true;
      mainWindow->ungrabMouse();
      break;

    case BzfEvent::KeyUp:
      if (!myTank)
	doKeyNotPlaying(event.keyDown, false);
      else
	doKeyPlaying(event.keyDown, false);
      break;

    case BzfEvent::KeyDown:
      if (!myTank)
	doKeyNotPlaying(event.keyUp, true);
      else
	doKeyPlaying(event.keyUp, true);
      break;

    case BzfEvent::MouseMove:
      if (myTank && myTank->isAlive() && myTank->isKeyboardMoving()) {
	myTank->setKeyboardMoving(false);
	showKeyboardStatus();
      }
      break;
  }
}

static void		addMessage(const Player* player,
				const std::string& msg, bool highlight,
				const char* oldColor)
{
  std::string fullMessage;

  if (BZDB->isTrue("colorful")) {
    if (player) {
      if (highlight) {
	if (BZDB->get("killerhighlight") == "0")
          fullMessage += ColorStrings[BlinkColor];
	else if (BZDB->get("killerhighlight") == "1")
         fullMessage += ColorStrings[UnderlineColor];
      }
      int color = player->getTeam();
      if (color < 0 || color > 4) color = 5;

      fullMessage += ColorStrings[color];
      fullMessage += player->getCallSign();

      if (highlight)
        fullMessage += ColorStrings[ResetColor];
#ifdef BWSUPPORT
      fullMessage += " (";
      fullMessage += Team::getName(player->getTeam());
      fullMessage += ")";
#endif
      fullMessage += ColorStrings[DefaultColor];
      fullMessage += ": ";
    }
    fullMessage += msg;
  } else {
    std::string cleanMsg;
    char *tmpstr;

    tmpstr = strdup(msg.c_str());
    OpenGLTexFont::stripAnsiCodes(tmpstr, strlen(tmpstr));
    cleanMsg = tmpstr;
    free(tmpstr);

    if (oldColor != NULL)
      fullMessage = oldColor;

    if (player) {
      fullMessage += player->getCallSign();

#ifdef BWSUPPORT
      fullMessage += " (";
      fullMessage += Team::getName(player->getTeam());
      fullMessage += ")";
#endif
      fullMessage += ": ";
    }
    fullMessage += cleanMsg;
  }
  controlPanel->addMessage(fullMessage);
}

static void		updateNumPlayers()
{
  int i, numPlayers[NumTeams];
  for (i = 0; i < NumTeams; i++)
    numPlayers[i] = 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i])
      numPlayers[player[i]->getTeam()]++;
  if (myTank)
    numPlayers[myTank->getTeam()]++;
}

static void		updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
  bool anyPlayers = false;
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i]) {
      anyPlayers = true;
      break;
    }
#ifdef ROBOT
  if (!anyPlayers) {
    for (i = 0; i < numRobots; i++)
      if (robots[i]) {
	anyPlayers = true;
	break;
      }
  }
#endif
  if (!anyPlayers) {
    hud->setPlayerHasHighScore(false);
    hud->setTeamHasHighScore(false);
    return;
  }

  bool haveBest = true;
  int bestScore = myTank ? myTank->getScore() : 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i] && player[i]->getScore() >= bestScore) {
      haveBest = false;
      break;
    }
#ifdef ROBOT
  if (haveBest) {
    for (i = 0; i < numRobots; i++)
      if (robots[i] && robots[i]->getScore() >= bestScore) {
	haveBest = false;
	break;
      }
  }
#endif
  hud->setPlayerHasHighScore(haveBest);

  if (myTank && myTank->getTeam() != RogueTeam) {
    const Team& myTeam = World::getWorld()->getTeam(int(myTank->getTeam()));
    bestScore = myTeam.won - myTeam.lost;
    haveBest = true;
    for (i = 0; i < NumTeams; i++) {
      if (i == int(myTank->getTeam())) continue;
      const Team& team = World::getWorld()->getTeam(i);
      if (team.activeSize > 0 && team.won - team.lost >= bestScore) {
	haveBest = false;
	break;
      }
    }
    hud->setTeamHasHighScore(haveBest);
  }
  else {
    hud->setTeamHasHighScore(false);
  }
}

static void		updateFlag(FlagDesc* flag)
{
  if (flag == Flags::Null) {
    hud->setColor(1.0f, 0.625f, 0.125f);
    hud->setAlert(2, NULL, 0.0f);
  }
  else {
    const float* color = flag->getColor();
    hud->setColor(color[0], color[1], color[2]);
    hud->setAlert(2, flag->flagName, 3.0f, flag->flagType == FlagSticky);
  }

  if (sceneRenderer->getShowFlagHelp())
    hud->setFlagHelp(flag, FlagHelpDuration);

  if (!radar && !myTank || !World::getWorld()) return;

  radar->setJammed(flag == Flags::Jamming);
  hud->setAltitudeTape(flag == Flags::Jumping || World::getWorld()->allowJumping());

  // enable/disable display of markers
  hud->setMarker(0, myTank->getTeam() != RogueTeam &&
			flag->flagTeam != myTank->getTeam() &&
			World::getWorld()->allowTeamFlags());
  hud->setMarker(1, World::getWorld()->allowAntidote() &&
			flag != Flags::Null && flag->flagType == FlagSticky);
}

void			notifyBzfKeyMapChanged()
{
  hud->setRestartKeyLabel(BzfKeyMap::getKeyEventString(
					keymap.get(BzfKeyMap::Identify)));
}

//
// server message handling
//

static Player*		addPlayer(PlayerId id, void* msg, int showMessage)
{
  uint16_t team, type, wins, losses, tks;
  char callsign[CallSignLen];
  char email[EmailLen];
  msg = nboUnpackUShort(msg, type);
  msg = nboUnpackUShort(msg, team);
  msg = nboUnpackUShort(msg, wins);
  msg = nboUnpackUShort(msg, losses);
  msg = nboUnpackUShort(msg, tks);
  msg = nboUnpackString(msg, callsign, CallSignLen);
  msg = nboUnpackString(msg, email, EmailLen);

  // Strip any ANSI color codes
  OpenGLTexFont::stripAnsiCodes (callsign, strlen (callsign));

  // id is slot, check if it's empty
  const int i = id;
  if (player[i]) {
    // we're not in synch with server -> help!
    printError("Server error when adding player");
    serverError = true;
    return NULL;
  }

  if (i >= curMaxPlayers) {
    curMaxPlayers = i+1;
    World::getWorld()->setCurMaxPlayers(curMaxPlayers);
  }
  // add player
  if (PlayerType(type) == TankPlayer || PlayerType(type) == ComputerPlayer) {
    player[i] = new RemotePlayer(id, TeamColor(team), callsign, email);
    player[i]->changeScore(short(wins), short(losses), short(tks));
  }

  if (showMessage) {
    std::string message("joining as a");
    switch (PlayerType(type)) {
      case TankPlayer:
	message += " tank";
	break;
      case JAFOPlayer:
	message += "n observer";
	break;
      case ComputerPlayer:
	message += " robot tank";
	break;
      default:
	message += "n unknown type";
	break;
    }
    if (!player[i]) {
      std::string name(callsign);
      name += ": ";
      name += message;
      message = name;
    }
    addMessage(player[i], message);
  }

  // restore player's local score if player had been playing earlier
  world->reviveDeadPlayer(player[i]);

  return player[i];
}

static void		handleServerMessage(bool human, uint16_t code,
						uint16_t, void* msg)
{
  std::vector<std::string> args;
  char buf[50];
  bool checkScores = false;
  switch (code) {

    case MsgUDPLinkRequest:
      uint16_t portNo;
      msg = nboUnpackUShort(msg, portNo);
	  sprintf(buf,"%d", portNo);
	  args.push_back(buf);
	  printError("Server sent downlink endpoint information, port {1}",&args);
	  playerLink->setPortForUPD(portNo);
      break;

    case MsgSuperKill:
      printError("Server forced a disconnect");
      serverError = true;
      break;

    case MsgTimeUpdate: {
      uint16_t timeLeft;
      msg = nboUnpackUShort(msg, timeLeft);
      hud->setTimeLeft(timeLeft);
      if (timeLeft == 0) {
	gameOver = true;
	myTank->explodeTank();
	controlPanel->addMessage("Time Expired");
	hud->setAlert(0, "Time Expired", 10.0f, true);
#ifdef ROBOT
	for (int i = 0; i < numRobots; i++)
	  robots[i]->explodeTank();
#endif
      }
      break;
    }

    case MsgScoreOver: {
      // unpack packet
      PlayerId id;
      uint16_t team;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, team);
      Player* player = lookupPlayer(id);

      // make a message
      std::string msg;
      if (team == (uint16_t)NoTeam) {
	// a player won
	if (player) {
	  msg = player->getCallSign();
	  msg += " (";
	  msg += Team::getName(player->getTeam());
	  msg += ")";
	}
	else {
	  msg = "[unknown player]";
	}
      }
      else {
	// a team won
	msg = Team::getName(TeamColor(team));
      }
      msg += " won the game";

      gameOver = true;
      hud->setTimeLeft(-1);
      myTank->explodeTank();
      controlPanel->addMessage(msg);
      hud->setAlert(0, msg.c_str(), 10.0f, true);
#ifdef ROBOT
      for (int i = 0; i < numRobots; i++)
	robots[i]->explodeTank();
#endif
      break;
    }

    case MsgSetTTL: {
      uint16_t ttl;
      nboUnpackUShort(msg, ttl);
      if ((int)ttl > playerLink->getTTL())
	playerLink->setTTL((int)ttl);
      break;
    }

    case MsgNetworkRelay: {
      // server is telling us that we must use multicast relaying
      playerLink->setUseRelay();
      printError("Now using server as relay");
      break;
    }

    case MsgAddPlayer: {
      PlayerId id;
      msg = nboUnpackUByte(msg, id);
#if defined(FIXME) && defined(ROBOT)
      for (int i = 0; i < numRobots; i++) {
	void *tmpbuf = msg;
	uint16_t team, type, wins, losses, tks;
	char callsign[CallSignLen];
	char email[EmailLen];
	tmpbuf = nboUnpackUShort(tmpbuf, type);
	tmpbuf = nboUnpackUShort(tmpbuf, team);
	tmpbuf = nboUnpackUShort(tmpbuf, wins);
	tmpbuf = nboUnpackUShort(tmpbuf, losses);
	tmpbuf = nboUnpackUShort(tmpbuf, tks);
	tmpbuf = nboUnpackString(tmpbuf, callsign, CallSignLen);
	tmpbuf = nboUnpackString(tmpbuf, email, EmailLen);
	fprintf(stderr, "id %d:%u:%s %d:%u:%s\n",
	    id.port,
	    id.number,
	    callsign,
	    robots[i]->getId().port,
	    robots[i]->getId().number,
	    robots[i]->getCallSign());
	if (strncmp(robots[i]->getCallSign(), callsign, CallSignLen)) {
	  // check for real robot id
	  fprintf(stderr, "id test %p %p %p %8.8x %8.8x\n",
	      robots[i], tmpbuf, msg, *(int *)tmpbuf, *((int *)tmpbuf + 1));
	  if (tmpbuf < (char *)msg + len) {
	    PlayerId id;
	    tmpbuf = nboUnpackUByte(tmpbuf, id);
	    robots[i]->id.serverHost = id.serverHost;
	    robots[i]->id.port = id.port;
	    robots[i]->id.number = id.number;
	    robots[i]->server->send(MsgIdAck, 0, NULL);
	  }
	}
      }
#endif
      if (id == myTank->getId()) break;		// that's odd -- it's me!
      addPlayer(id, msg, true);
      updateNumPlayers();
      checkScores = true;
      break;
    }

    case MsgRemovePlayer: {
      PlayerId id;
      msg = nboUnpackUByte(msg, id);
      int playerIndex = lookupPlayerIndex(id);
      if (playerIndex >= 0) {
	addMessage(player[playerIndex], "signing off");
	world->addDeadPlayer(player[playerIndex]);
	if (myTank->getRecipient() == player[playerIndex])
	  myTank->setRecipient(0);
	if (myTank->getNemesis() == player[playerIndex])
	  myTank->setNemesis(0);
	delete player[playerIndex];
	player[playerIndex] = NULL;

	while ((playerIndex >= 0)
	&&     (playerIndex+1 == curMaxPlayers)
	&&     (player[playerIndex] == NULL))
	{
	  playerIndex--;
	  curMaxPlayers--;
	}
	World::getWorld()->setCurMaxPlayers(curMaxPlayers);

	updateNumPlayers();
	checkScores = true;
      }
      break;
    }

    case MsgFlagUpdate: {
      uint16_t flagIndex;
      msg = nboUnpackUShort(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      world->initFlag(int(flagIndex));
      break;
    }

    case MsgTeamUpdate: {
      uint16_t team;
      msg = nboUnpackUShort(msg, team);
      msg = teams[int(team)].unpack(msg);
      updateNumPlayers();
      checkScores = true;
      break;
    }

    case MsgAlive: {
      PlayerId id;
      float pos[3], forward[3];
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackVector(msg, pos);
      msg = nboUnpackVector(msg, forward);
      int playerIndex = lookupPlayerIndex(id);
      if (playerIndex >= 0) {
	static const float zero[3] = { 0.0f, 0.0f, 0.0f };
	Player* tank = getPlayerByIndex(playerIndex);
	tank->setStatus(PlayerState::Alive);
	tank->move(pos, atan2f(forward[1], forward[0]));
	tank->setVelocity(zero);
	tank->setAngularVelocity(0.0f);
	tank->setDeadReckoning();
	playWorldSound(SFX_POP, pos[0], pos[1], pos[2], true);
      }
      break;
    }

    case MsgKilled: {
      PlayerId victim, killer;
      int16_t shotId, reason;
      msg = nboUnpackUByte(msg, victim);
      msg = nboUnpackUByte(msg, killer);
      msg = nboUnpackShort(msg, reason);
      msg = nboUnpackShort(msg, shotId);
      BaseLocalPlayer* victimLocal = getLocalPlayer(victim);
      BaseLocalPlayer* killerLocal = getLocalPlayer(killer);
      Player* victimPlayer = lookupPlayer(victim);
      Player* killerPlayer = lookupPlayer(killer);
#ifdef ROBOT
      if (victimPlayer == myTank) {
	// uh oh, i'm dead
	if (myTank->isAlive()) {
	  serverLink->sendDropFlag(myTank->getPosition());
	  handleMyTankKilled(reason);
	}
      }
#endif
      if (victimLocal) {
	// uh oh, local player is dead
	if (victimLocal->isAlive()){
	  gotBlowedUp(victimLocal, GotKilledMsg, killer);
	}
      }
      else if (victimPlayer) {
	victimPlayer->setExplode(TimeKeeper::getTick());
	const float* pos = victimPlayer->getPosition();
	if (reason == GotRunOver)
	  playWorldSound(SFX_RUNOVER, pos[0], pos[1], pos[2], killerLocal == myTank);
	else
	  playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2], killerLocal == myTank);
	float explodePos[3];
	explodePos[0] = pos[0];
	explodePos[1] = pos[1];
	explodePos[2] = pos[2] + MuzzleHeight;
	addTankExplosion(explodePos);
      }
      if (killerLocal) {
	// local player did it
	if (shotId >= 0) {
	  // terminate the shot
	  killerLocal->endShot(shotId, true);
	}
	if (victimPlayer && killerLocal != victimPlayer) {
	  if (victimPlayer->getTeam() == killerLocal->getTeam() &&
		  ((killerLocal->getTeam() != RogueTeam) || (World::getWorld()->allowRabbit()))) {
	    if (killerLocal == myTank) {
		 hud->setAlert(1, "Don't shoot teammates!!!", 3.0f, true);
		 playLocalSound( SFX_KILL_TEAM );
	    }
	    // teammate
	    killerLocal->changeScore(0, 1, 1);
	  }
	  else
	    // enemy
	    killerLocal->changeScore(1, 0, 0);
	}
      }

      // handle my personal score against other players
      if ((killerPlayer == myTank || victimPlayer == myTank) &&
	 !(killerPlayer == myTank && victimPlayer == myTank)) {
	if (killerLocal == myTank) {
	  if (victimPlayer)
	    victimPlayer->changeLocalScore(1, 0, 0);
	  myTank->setNemesis(victimPlayer);
	}
	else {
	  if (killerPlayer)
	    killerPlayer->changeLocalScore(0, 1, killerPlayer->getTeam() == victimPlayer->getTeam() ? 1 : 0);
	  myTank->setNemesis(killerPlayer);
	}
      }

      // add message
      if (human && victimPlayer) {
	if (killerPlayer == victimPlayer) {
	  std::string message(ColorStrings[WhiteColor]);
	  message += "blew myself up";
	  addMessage(victimPlayer, message);
	}
	else if (!killerPlayer) {
	  addMessage(victimPlayer, "destroyed by (UNKNOWN)");
	}
	else if ((shotId == -1) || (killerPlayer->getShot(int(shotId)) == NULL)) {
	  std::string message(ColorStrings[WhiteColor]);
	  message += "destroyed by ";
	  if (killerPlayer->getTeam() == victimPlayer->getTeam() &&
	      killerPlayer->getTeam() != RogueTeam)
	    message += "teammate ";
	  message += ColorStrings[killerPlayer->getTeam()];
	  message += killerPlayer->getCallSign();
	  addMessage(victimPlayer, message);
	}
	else {
	  const ShotPath* shot = killerPlayer->getShot(int(shotId));
	  std::string message (ColorStrings[WhiteColor]);
	  std::string playerStr;
	  if (killerPlayer->getTeam() == victimPlayer->getTeam() &&
	      killerPlayer->getTeam() != RogueTeam)
	    playerStr += "teammate ";

	  if (victimPlayer == myTank) {
	    if (BZDB->get("killerhighlight") == "0")
	      playerStr += ColorStrings[BlinkColor];
	    else if (BZDB->get("killerhighlight") == "1")
	      playerStr += ColorStrings[UnderlineColor];
	  }
	  playerStr += ColorStrings[killerPlayer->getTeam()];
	  playerStr += killerPlayer->getCallSign();

	  if (victimPlayer == myTank)
	    playerStr += ColorStrings[ResetColor];
	  playerStr += ColorStrings[WhiteColor];

	  // Give more informative kill messages
	  FlagDesc* shotFlag = shot->getFlag();
	  if (shotFlag == Flags::Laser) {
	    message += "was fried by ";
	    message += playerStr;
	    message += "'s laser";
	  }
	  else if (shotFlag == Flags::GuidedMissile) {
	    message += "was destroyed by ";
	    message += playerStr;
	    message += "'s guided missile";
	  }
	  else if (shotFlag == Flags::ShockWave) {
	    message += "felt the effects of ";
	    message += playerStr;
	    message += "'s shockwave";
	  }
	  else if (shotFlag == Flags::InvisibleBullet) {
	    message += "didn't see ";
	    message += playerStr;
	    message += "'s bullet";
	  }
	  else if (shotFlag == Flags::MachineGun) {
	    message += "was turned into swiss cheese by ";
	    message += playerStr;
	    message += "'s machine gun";
	  }
	  else if (shotFlag == Flags::SuperBullet) {
	    message += "got skewered by ";
	    message += playerStr;
	    message += "'s super bullet";
	  }
	  else {
	    message += "killed by ";
	    message += playerStr;
	  }
	  addMessage(victimPlayer, message, killerPlayer==myTank);
	}
      }

      // blow up if killer has genocide flag and i'm on same team as victim
      // (and we're not rogues, unless in rabbit mode)
      if (human && killerPlayer && victimPlayer && victimPlayer != myTank &&
		victimPlayer->getTeam() == myTank->getTeam() &&
		((myTank->getTeam() != RogueTeam) || World::getWorld()->allowRabbit()) && shotId >= 0) {
	// now see if shot was fired with a GenocideFlag
	const ShotPath* shot = killerPlayer->getShot(int(shotId));
	if (shot && shot->getFlag() == Flags::Genocide) {
	  gotBlowedUp(myTank, GenocideEffect, killerPlayer->getId());
	}
      }

#ifdef ROBOT
      // blow up robots on victim's team if shot was genocide
      if (killerPlayer && victimPlayer && shotId >= 0) {
	const ShotPath* shot = killerPlayer->getShot(int(shotId));
	if (shot && shot->getFlag() == Flags::Genocide)
	  for (int i = 0; i < numRobots; i++)
	    if (victimPlayer != robots[i] &&
		victimPlayer->getTeam() == robots[i]->getTeam() &&
		robots[i]->getTeam() != RogueTeam)
	      gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
      }
#endif

      checkScores = true;
      break;
    }

    case MsgGrabFlag: {
// ROBOT -- FIXME -- robots don't grab flag at the moment
      PlayerId id;
      uint16_t flagIndex;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      Player* tank = lookupPlayer(id);
      if (!tank) break;

      // player now has flag
      tank->setFlag(world->getFlag(flagIndex).desc);
      if (tank == myTank) {
	// not allowed to grab it if not on the ground
	if (myTank->getLocation() != LocalPlayer::OnGround &&
	  myTank->getLocation() != LocalPlayer::OnBuilding) {
	  serverLink->sendDropFlag(myTank->getPosition());
	}
	else {
	  // grabbed flag
	  playLocalSound(myTank->getFlag()->flagType != FlagSticky ?
	      SFX_GRAB_FLAG : SFX_GRAB_BAD);
	  updateFlag(myTank->getFlag());
	}
      }
      else if (myTank->getTeam() != RabbitTeam && tank &&
	       tank->getTeam() != myTank->getTeam() &&
	       world->getFlag(flagIndex).desc->flagTeam == myTank->getTeam()) {
	hud->setAlert(1, "Flag Alert!!!", 3.0f, true);
	playLocalSound(SFX_ALERT);
      }
      else {
	FlagDesc* fd = world->getFlag(flagIndex).desc;
	if ( fd->flagTeam != NoTeam
	     && fd->flagTeam != tank->getTeam()
	     && ((tank && (tank->getTeam() == myTank->getTeam())))) {
	  hud->setAlert(1, "Team Grab!!!", 3.0f, false);
	  const float* pos = tank->getPosition();
	  playWorldSound(SFX_TEAMGRAB, pos[0], pos[1], pos[2], false);
	}
      }
      std::string message("grabbed ");
      message += tank->getFlag()->flagName;
      message += " flag";
      addMessage(tank, message);
      break;
    }

    case MsgDropFlag: {
      PlayerId id;
      uint16_t flagIndex;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      Player* tank = lookupPlayer(id);
      if (!tank) break;
      handleFlagDropped(tank);
      break;
    }

    case MsgCaptureFlag: {
      PlayerId id;
      uint16_t flagIndex, team;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, flagIndex);
      msg = nboUnpackUShort(msg, team);
      Player* capturer = lookupPlayer(id);
      int capturedTeam = world->getFlag(int(flagIndex)).desc->flagTeam;

      // player no longer has flag
      if (capturer) {
	capturer->setFlag(Flags::Null);
	if (capturer == myTank) {
	  updateFlag(Flags::Null);
	}

	// add message
	if (int(capturer->getTeam()) == capturedTeam) {
	  std::string message("took my flag into ");
	  message += Team::getName(TeamColor(team));
	  message += " territory";
	  addMessage(capturer, message);
	}
	else {
	  std::string message("captured ");
	  message += Team::getName(TeamColor(capturedTeam));
	  message += "'s flag";
	  addMessage(capturer, message);
	}
      }

      // play sound -- if my team is same as captured flag then my team lost,
      // but if I'm on the same team as the capturer then my team won.
      if (capturedTeam == int(myTank->getTeam()))
	playLocalSound(SFX_LOSE);
      else if (capturer->getTeam() == myTank->getTeam())
	playLocalSound(SFX_CAPTURE);

      // blow up if my team flag captured
      if (capturedTeam == int(myTank->getTeam())) {
	gotBlowedUp(myTank, GotCaptured, id);
	restartOnBase = true;
      }

      // everybody who's alive on capture team will be blowing up
      // but we're not going to get an individual notification for
      // each of them, so add an explosion for each now.  don't
      // include me, though;  I already blew myself up.
      for (int i = 0; i < curMaxPlayers; i++) {
	if (player[i] &&
	    player[i]->isAlive() &&
	    player[i]->getTeam() == capturedTeam) {
	  const float* pos = player[i]->getPosition();
	  playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2], false);
	  float explodePos[3];
	  explodePos[0] = pos[0];
	  explodePos[1] = pos[1];
	  explodePos[2] = pos[2] + MuzzleHeight;
	  addTankExplosion(explodePos);
	}
      }

      checkScores = true;
      break;
    }

    case MsgNewRabbit: {
      PlayerId id;
      msg = nboUnpackUByte(msg, id);
      Player *rabbit = lookupPlayer(id);

      for (int i = 0; i < curMaxPlayers; i++) {
	if (player[i])
	  player[i]->setHunted(false);
        if (i != id && player[i] && player[i]->getTeam() != RogueTeam) {
	   player[i]->changeTeam(RogueTeam);
	}
      }

      if (rabbit != NULL) {
	rabbit->changeTeam(RabbitTeam);
	if (rabbit == myTank) {
	  if (myTank->isPaused())
	    serverLink->sendNewRabbit();
          else {
            hud->setAlert(0, "You are now the rabbit.", 10.0f, false);
            playLocalSound(SFX_HUNT_SELECT);
          }
	  hud->setHunting(false);
	} else {
	  myTank->changeTeam(RogueTeam);
	  rabbit->setHunted(true);
	  hud->setHunting(true);
	}

	addMessage(rabbit, "is now the rabbit", true);
      }

      break;
    }

    case MsgShotBegin: {
      FiringInfo firingInfo;
      msg = firingInfo.unpack(msg);
      for (int i = 0; i < curMaxPlayers; i++)
	if (player[i] && player[i]->getId() == firingInfo.shot.player) {
	  const float* pos = firingInfo.shot.pos;
	  player[i]->addShot(firingInfo);
	  if (human) {
	    if (firingInfo.flag == Flags::ShockWave)
	      playWorldSound(SFX_SHOCK, pos[0], pos[1], pos[2]);
	    else if (firingInfo.flag == Flags::Laser)
	      playWorldSound(SFX_LASER, pos[0], pos[1], pos[2]);
	    else if (firingInfo.flag == Flags::GuidedMissile)
	      playWorldSound(SFX_MISSILE, pos[0], pos[1], pos[2]);
	    else
	      playWorldSound(SFX_FIRE, pos[0], pos[1], pos[2]);
	  }
	  break;
	}
      break;
    }

    case MsgShotEnd: {
      PlayerId id;
      int16_t shotId;
      uint16_t reason;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackShort(msg, shotId);
      msg = nboUnpackUShort(msg, reason);
      BaseLocalPlayer* localPlayer = getLocalPlayer(id);

      if (localPlayer)
	localPlayer->endShot(int(shotId), false, reason == 0);
      else {
        Player *pl = lookupPlayer(id);
        if (pl)
	  pl->endShot(int(shotId), false, reason == 0);
	}
      break;
    }

    case MsgScore: {
      PlayerId id;
      uint16_t wins, losses, tks;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, wins);
      msg = nboUnpackUShort(msg, losses);
      msg = nboUnpackUShort(msg, tks);

      int i = lookupPlayerIndex(id);
      if (i >= 0)
	player[i]->changeScore(wins - player[i]->getWins(),
                               losses - player[i]->getLosses(),
			       tks - player[i]->getTeamKills());
      break;
    }

    case MsgTeleport: {
      PlayerId id;
      uint16_t from, to;
      msg = nboUnpackUByte(msg, id);
      msg = nboUnpackUShort(msg, from);
      msg = nboUnpackUShort(msg, to);
      Player* tank = lookupPlayer(id);
      if (tank && tank != myTank) {
	int face;
	const Teleporter* teleporter = world->getTeleporter(int(to), face);
	const float* pos = teleporter->getPosition();
	tank->setTeleport(TimeKeeper::getTick(), short(from), short(to));
	playWorldSound(SFX_TELEPORT, pos[0], pos[1], pos[2]);
      }
      break;
    }

    case MsgMessage: {
      PlayerId src;
      PlayerId dst;
      msg = nboUnpackUByte(msg, src);
      msg = nboUnpackUByte(msg, dst);
      Player* srcPlayer = lookupPlayer(src);
      Player* dstPlayer = lookupPlayer(dst);
      TeamColor dstTeam = PlayerIdToTeam(dst);
      bool toAll = (dst == AllPlayers);
      bool fromServer = (src == ServerPlayer);

      const std::string srcName = srcPlayer ?
        srcPlayer->getCallSign() :
        fromServer ? "SERVER" : "(UNKNOWN)";
      const std::string dstName = dstPlayer ?
        dstPlayer->getCallSign() : "(UNKNOWN)";

      std::string fullMsg;

      bool ignore = false;
      unsigned int i;
      for (i = 0; i < silencePlayers.size(); i++) {
	const char *silenceCallSign = silencePlayers[i].c_str();
	if ((strcmp(srcName.c_str(), silenceCallSign) == 0)
	    || (strcmp( "*", silenceCallSign) == 0)) {
	  ignore = true;
	  break;
	}
      }
      if (ignore) {
	// to verify working
	std::string msg = "Ignored Msg";
	if (silencePlayers[i] != "*") {
	  msg = msg + " from " + silencePlayers[i];
	} else {
	  //if * just echo a generic Ignored
	}
	addMessage(NULL,msg);
	break;
      }

      // CLIENTQUERY hack
      if (!strncmp((char*)msg,"CLIENTQUERY",strlen("CLIENTQUERY"))) {
	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	sprintf(messageBuffer,"Version %s", VERSION);
	if (startupInfo.useUDPconnection)
	  strcat(messageBuffer,"+UDP");

	char response[PlayerIdPLen + MessageLen];
	void* buf = response;
	buf = nboPackUByte(buf, src);
	nboPackString(buf, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(response), response);
	const char *oldcolor = NULL;
        if (dstTeam == RogueTeam || srcPlayer->getTeam() == NoTeam)
          oldcolor = ColorStrings[RogueTeam];
        else
          oldcolor = ColorStrings[srcPlayer->getTeam()];

        addMessage(srcPlayer,"[Sent versioninfo per request]", false, oldcolor);
	break;
      }

      OpenGLTexFont::stripAnsiCodes((char*) msg, strlen ((char*) msg));

      std::string text = BundleMgr::getCurrentBundle()->getLocalString(std::string((char*)msg));

      if (toAll || srcPlayer == myTank || dstPlayer == myTank ||
          dstTeam == myTank->getTeam()) {
	// message is for me
	std::string colorStr;

	if (srcPlayer && srcPlayer->getTeam() != NoTeam)
	  colorStr += ColorStrings[srcPlayer->getTeam()];
	else
	  colorStr += ColorStrings[RogueTeam];

	fullMsg += colorStr;

	// direct message to or from me
        if (dstPlayer) {
          if (fromServer && (text == "You are now an administrator!" || text == "Password Accepted, welcome back."))
	      admin = true;
	  // talking to myself? that's strange
	  if (dstPlayer==myTank && srcPlayer==myTank) {
	    fullMsg=text;
	  }
	  else {
	    if (BZDB->get("kilerhighlight") == "0")
	      fullMsg += ColorStrings[BlinkColor];
	    else if (BZDB->get("killerhighlight") == "1")
	      fullMsg += ColorStrings[UnderlineColor];
	    fullMsg += "[";
	    if (srcPlayer == myTank) {
	      fullMsg += "->";
	      fullMsg += dstName;
	      fullMsg += colorStr;
	    }
	    else {
	      fullMsg += srcName;
	      fullMsg += colorStr;
	      fullMsg += "->";
	      if (srcPlayer)
		myTank->setRecipient(srcPlayer);
	    }
	    fullMsg += "]";
	    fullMsg += ColorStrings[ResetColor];
	    fullMsg += " ";
	    fullMsg += ColorStrings[CyanColor];
	    fullMsg += text;
	  }
	}
	else {
	  // team message
	  if (dstTeam != NoTeam) {
#ifdef BWSUPPORT
	    fullMsg = "[to ";
	    fullMsg += Team::getName(TeamColor(dstTeam));
	    fullMsg += "] ";
#else
	    fullMsg += "[Team] ";
#endif
	  }
	  fullMsg += srcName;
	  fullMsg += colorStr;
	  fullMsg += ": ";
	  fullMsg += ColorStrings[CyanColor];
	  fullMsg += text;
	}
        const char *oldcolor = NULL;
        if (srcPlayer && srcPlayer->getTeam() != NoTeam)
          oldcolor = ColorStrings[srcPlayer->getTeam()];
        else
          oldcolor = ColorStrings[RogueTeam];
        addMessage(NULL, fullMsg, false, oldcolor);

	if (!srcPlayer || srcPlayer!=myTank)
	  hud->setAlert(0, fullMsg.c_str(), 3.0f, false);
      }
      break;
    }

    // inter-player relayed message
    case MsgPlayerUpdate:
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
    case MsgLagPing:
      handlePlayerMessage(code, 0, msg);
      break;
  }

  if (checkScores) updateHighScores();
}

//
// player message handling
//

static void		handlePlayerMessage(uint16_t code, uint16_t,
								void* msg)
{
  switch (code) {
    case MsgPlayerUpdate: {
      PlayerId id;
      msg = nboUnpackUByte(msg, id);
      Player* tank = lookupPlayer(id);
      if (!tank || tank == myTank) break;
      short oldStatus = tank->getStatus();
      tank->unpack(msg);
      short newStatus = tank->getStatus();
      if ((oldStatus & short(PlayerState::Paused)) !=
				(newStatus & short(PlayerState::Paused)))
	addMessage(tank, (tank->getStatus() & PlayerState::Paused) ?
						"Paused" : "Resumed");
      if ((oldStatus & short(PlayerState::Exploding)) == 0 &&
		(newStatus & short(PlayerState::Exploding)) != 0) {
	// player has started exploding and we haven't gotten killed
	// message yet -- set explosion now, play sound later (when we
	// get killed message).  status is already !Alive so make player
	// alive again, then call setExplode to kill him.
	tank->setStatus(newStatus | short(PlayerState::Alive));
	tank->setExplode(TimeKeeper::getTick());
	// ROBOT -- play explosion now
      }
      break;
    }

    case MsgGMUpdate: {
      ShotUpdate shot;
      msg = shot.unpack(msg);
      Player* tank = lookupPlayer(shot.player);
      if (!tank || tank == myTank) break;
      RemotePlayer* remoteTank = (RemotePlayer*)tank;
      RemoteShotPath* shotPath =
		(RemoteShotPath*)remoteTank->getShot(shot.id);
      if (shotPath) shotPath->update(shot, code, msg);
      PlayerId targetId;
      msg = nboUnpackUByte(msg, targetId);
      Player* targetTank = lookupPlayer(targetId);
      if (targetTank && (targetTank == myTank)) {
	static TimeKeeper lastLockMsg;
	if (TimeKeeper::getTick() - lastLockMsg > 0.75) {
	  playWorldSound(SFX_LOCK, shot.pos[0], shot.pos[1], shot.pos[2]);
	  lastLockMsg=TimeKeeper::getTick();
	  addMessage(tank, "locked on me");
	}
      }
      break;
    }

    // just echo lag ping message
    case MsgLagPing:
      playerLink->send(MsgLagPing,2,msg);
      break;
  }
}

//
// message handling
//

static void		doMessages()
{
  char msg[MaxPacketLen];
  uint16_t code, len;
  int e = 0;

  // handle server messages
  if (serverLink) {
    while (!serverError && (e = serverLink->read(code, len, msg, 0)) == 1)
      handleServerMessage(true, code, len, msg);
    if (e == -2) {
      printError("Server communication error");
      serverError = true;
      return;
    }
  }

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    while ((e = robotServer[i]->read(code, len, msg, 0)) == 1);
      if (code == MsgKilled || code == MsgShotBegin || code == MsgShotEnd)
	handleServerMessage(false, code, len, msg);
  }
#endif

  // handle player messages
  if (playerLink) {
    while ((e = playerLink->read(code, len, msg, 0)) == 1)
      handlePlayerMessage(code, len, msg);
    if (e == -2) {
      printError("Player communication error");
      return;
    }
  }
}

//
// local update utility functions
//

static float		minSafeRange(float angleCosOffBoresight)
{
  // anything farther than this much from dead-center is okay to
  // place at MinRange
  static const float	SafeAngle = 0.5f;		// cos(angle)

  // don't ever place within this range
  static const float	MinRange = 2.0f * ShotSpeed;	// meters

  // anything beyond this range is okay at any angle
  static const float	MaxRange = 4.0f * ShotSpeed;	// meters

  // if more than SafeAngle off boresight then MinRange is okay
  if (angleCosOffBoresight < SafeAngle) return MinRange;

  // ramp up to MaxRange as target comes to dead center
  const float f = (angleCosOffBoresight - SafeAngle) / (1.0f - SafeAngle);
  return (float)(MinRange + f * (MaxRange - MinRange));
}

static void		restartPlaying()
{
  // maximum tries to find a safe place
  static const int	MaxTries = 1000;

  // minimum time before an existing shot can hit us
  static const float	MinShotImpact = 2.0f;		// seconds

  // restart my tank
  float startPoint[3];
  float startAzimuth;
  int locateCount = 0;
  startPoint[2] = 0.0f;
  float bestStartPoint[3], bestDist = -1e6;
  bool located;

  // check for valid starting (no unfair advantage to player or enemies)
  // should find a good location in a few tries... locateCount is a safety
  // check that will probably be invoked when restarting on the team base
  // if the enemy is loitering around waiting for players to reappear.
  // also have to make sure new position isn't in a building;  that must
  // be enforced no matter how many times we need to try new locations.
  // If I can't find a safe spot, try to use the best of the unsafe ones.
  // The best one is that which violates the minimum safe distance by the
  // smallest amount.
  do {
    do {
      if (restartOnBase) {
	const float* base = world->getBase(int(myTank->getTeam()));
	const float x = (base[4] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	const float y = (base[5] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	startPoint[0] = base[0] + x * cosf(base[3]) - y * sinf(base[3]);
	startPoint[1] = base[1] + x * sinf(base[3]) + y * cosf(base[3]);
	if(base[2] != 0) {
	  startPoint[2] = base[2] + 1;
	} else {
	  startPoint[2] = base[2];
	}
      }
      else {
	if (world->allowTeamFlags()) {
	  startPoint[0] = 0.4f * WorldSize * ((float)bzfrand() - 0.5f);
	  startPoint[1] = 0.4f * WorldSize * ((float)bzfrand() - 0.5f);
	}
	else {
	  startPoint[0] = (WorldSize - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	  startPoint[1] = (WorldSize - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	}
      }
      startAzimuth = 2.0f * M_PI * (float)bzfrand();
    } while (world->inBuilding(startPoint, 2.0f * TankRadius));

    // use first point as best point, so we'll have a fallback
    if (locateCount == 0) {
      bestStartPoint[0] = startPoint[0];
      bestStartPoint[1] = startPoint[1];
      bestStartPoint[2] = startPoint[2];
    }

    // get info on my tank
    const TeamColor myColor = myTank->getTeam();
    const float myCos = cosf(-startAzimuth);
    const float mySin = sinf(-startAzimuth);

    // check each enemy tank
    located = true;
    float worstDist = 1e6;
    for (int i = 0; i < curMaxPlayers; i++) {
      // ignore missing player
      if (!player[i]) continue;

      // test against all existing shots of all players except mine
      // (mine don't count because I can't come alive before all my
      // shots have expired anyway)
      const int maxShots = World::getWorld()->getMaxShots();
      for (int j = 0; j < maxShots; j++) {
	// get shot and ignore non-existent ones
	ShotPath* shot = player[i]->getShot(j);
	if (!shot) continue;

	// get shot's current position and velocity and see if it'll
	// hit my tank earlier than MinShotImpact.  use something
	// larger than the actual tank size to give some leeway.
	const Ray ray(shot->getPosition(), shot->getVelocity());
	const float t = timeRayHitsBlock(ray, startPoint, startAzimuth,
				4.0f * TankLength, 4.0f * TankWidth,
				2.0f * TankHeight);
	if (t >= 0.0f && t < MinShotImpact) {
	  located = false;
	  break;
	}
      }
      if (!located) break;

      // test against living enemy tanks
      if (!player[i]->isAlive() ||
	  (myColor != RogueTeam  && player[i]->getTeam() == myColor)) continue;

      // compute enemy position in my local coordinate system
      const float* enemyPos = player[i]->getPosition();
      const float enemyX = myCos * (enemyPos[0] - startPoint[0]) -
			   mySin * (enemyPos[1] - startPoint[1]);
      const float enemyY = mySin * (enemyPos[0] - startPoint[0]) +
			   myCos * (enemyPos[1] - startPoint[1]);

      // get distance and angle of enemy from boresight
      const float enemyDist = hypotf(enemyX, enemyY);
      const float enemyCos = enemyX / enemyDist;

      // don't allow tank placement if enemy tank is +/- 30 degrees of
      // my boresight and in firing range (our unfair advantage)
      float safeDist = enemyDist - minSafeRange(enemyCos);
      if (safeDist < worstDist)
	worstDist = safeDist;

      // compute my position in enemy coordinate system
      // cos = enemyUnitVect[0], sin = enemyUnitVect[1]
      const float* enemyUnitVect = player[i]->getForward();
      const float myX = enemyUnitVect[0] * (startPoint[0] - enemyPos[0]) -
			enemyUnitVect[1] * (startPoint[1] - enemyPos[1]);
      const float myY = enemyUnitVect[1] * (startPoint[0] - enemyPos[0]) +
			enemyUnitVect[0] * (startPoint[1] - enemyPos[1]);

      // get distance and angle of enemy from boresight
      const float myDist = hypotf(myX, myY);
      const float myCos = myX / myDist;

      // don't allow tank placement if my tank is +/- 30 degrees of
      // the enemy's boresight and in firing range (enemy's unfair advantage)
      safeDist = myDist - minSafeRange(myCos);
      if (safeDist < worstDist)
	worstDist = safeDist;
    }
    if (located && worstDist > bestDist) {
      bestDist = worstDist;
      bestStartPoint[0] = startPoint[0];
      bestStartPoint[1] = startPoint[1];
      bestStartPoint[2] = startPoint[2];
    }
    if (bestDist < 0.0f)
      located = false;
  } while (!located && ++locateCount <= MaxTries);

  // restart the tank
  myTank->restart(bestStartPoint, startAzimuth);
  if (!Observer)
    serverLink->sendAlive(myTank->getPosition(), myTank->getForward());
  restartOnBase = false;
  firstLife = false;
  mainWindow->warpMouse();
  playLocalSound(SFX_POP);

  // make sure altitude tape is correctly on/off
  hud->setAltitudeTape(World::getWorld()->allowJumping());
}

static void		updateFlags(float dt)
{
  for (int i = 0; i < numFlags; i++) {
    Flag& flag = world->getFlag(i);
    if (flag.status == FlagOnTank) {
      // position flag on top of tank
      Player* tank = lookupPlayer(flag.owner);
      if (tank) {
	const float* pos = tank->getPosition();
	flag.position[0] = pos[0];
	flag.position[1] = pos[1];
	flag.position[2] = pos[2] + TankHeight;
      }
    }
    world->updateFlag(i, dt);
  }
}

bool			addExplosion(const float* _pos,
				float size, float duration)
{
  // ignore if no prototypes available;
  if (prototypeExplosions.size() == 0) return false;

  // don't show explosions if quality isn't high
  if (sceneRenderer->useQuality() < 2) return false;

  // don't add explosion if blending or texture mapping are off
  if (!BZDB->isTrue("blend") || !BZDB->isTrue("texture"))
    return false;

  // pick a random prototype explosion
  const int index = (int)(bzfrand() * (float)prototypeExplosions.size());

  // make a copy and initialize it
  BillboardSceneNode* newExplosion = prototypeExplosions[index]->copy();
  GLfloat pos[3];
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  newExplosion->move(pos);
  newExplosion->setSize(size);
  newExplosion->setDuration(duration);
  newExplosion->setAngle(2.0f * M_PI * (float)bzfrand());
  newExplosion->setLightScaling(size / TankLength);
  newExplosion->setLightFadeStartTime(0.7f * duration);

  // add copy to list of current explosions
  explosions.push_back(newExplosion);

  if (size < (3.0f * TankLength)) return true; // shot explosion

  int boom = (int) (bzfrand() * 8.0) + 3;
  while (boom--) {
  // pick a random prototype explosion
  const int index = (int)(bzfrand() * (float)prototypeExplosions.size());

  // make a copy and initialize it
  BillboardSceneNode* newExplosion = prototypeExplosions[index]->copy();
  GLfloat pos[3];
  pos[0] = _pos[0]+(float)(bzfrand()*12.0 - 6.0);
  pos[1] = _pos[1]+(float)(bzfrand()*12.0 - 6.0);
  pos[2] = _pos[2]+(float)(bzfrand()*10.0);
  newExplosion->move(pos);
  newExplosion->setSize(size);
  newExplosion->setDuration(duration);
  newExplosion->setAngle(2.0f * M_PI * (float)bzfrand());
  newExplosion->setLightScaling(size / TankLength);
  newExplosion->setLightFadeStartTime(0.7f * duration);

  // add copy to list of current explosions
  explosions.push_back(newExplosion);
  }

  return true;
}

void			addTankExplosion(const float* pos)
{
  addExplosion(pos, 3.5f * TankLength, 1.2f);
}

void			addShotExplosion(const float* pos)
{
  // only play explosion sound if you see an explosion
  if (addExplosion(pos, 1.2f * TankLength, 0.8f))
    playWorldSound(SFX_SHOT_BOOM, pos[0], pos[1], pos[2]);
}

void			addShotPuff(const float* pos)
{
  addExplosion(pos, 0.3f * TankLength, 0.8f);
}

// update events from outside if they should be checked
void                   updateEvents()
{
  if (mainWindow && display) {
    while (display->isEventPending() && !CommandsStandard::isQuit())
      doEvent(display);
  }
}

static void		updateExplosions(float dt)
{
  // update time of all explosions
  int i;
  const int count = explosions.size();
  for (i = 0; i < count; i++)
    explosions[i]->updateTime(dt);

  // reap expired explosions
  for (i = count - 1; i >= 0; i--)
    if (explosions[i]->isAtEnd()) {
      delete explosions[i];
      std::vector<BillboardSceneNode*>::iterator it = explosions.begin();
      for(int j = 0; j < i; j++) it++;
      explosions.erase(it);
    }
}

static void		addExplosions(SceneDatabase* scene)
{
  const int count = explosions.size();
  for (int i = 0; i < count; i++)
    scene->addDynamicNode(explosions[i]);
}

#ifdef ROBOT
static void		handleMyTankKilled(int reason)
{
  // blow me up
  myTank->explodeTank();
  if (reason == GotRunOver)
	playLocalSound(SFX_RUNOVER);
  else
	playLocalSound(SFX_DIE);

  // i lose a point
  myTank->changeScore(0, 1, 0);
}
#endif

static void		handleFlagDropped(Player* tank)
{
  // skip it if player doesn't actually have a flag
  if (tank->getFlag() == Flags::Null) return;

  if (tank == myTank) {
    // update display and play sound effects
    playLocalSound(SFX_DROP_FLAG);
    updateFlag(Flags::Null);
  }

  // add message
  std::string message("dropped ");
  message += tank->getFlag()->flagName;
  message += " flag";
  addMessage(tank, message);

  // player no longer has flag
  tank->setFlag(Flags::Null);
}

static bool		gotBlowedUp(BaseLocalPlayer* tank,
					BlowedUpReason reason,
					PlayerId killer,
					int shotId)
{
  if (Observer || !tank->isAlive())
    return false;

  // you can't take it with you
  const FlagId flag = tank->getFlag();
  if (flag != Flags::Null) {
    // tell other players I've dropped my flag
    lookupServer(tank)->sendDropFlag(tank->getPosition());

    // drop it
    handleFlagDropped(tank);
  }

  // restore the sound, this happens when paused tank dies
  // (genocide or team flag captured)
  if (savedVolume != -1) {
    setSoundVolume(savedVolume);
    savedVolume = -1;
  }

  // take care of explosion business -- don't want to wait for
  // round trip of killed message.  waiting would simplify things,
  // but the delay (2-3 frames usually) can really fool and irritate
  // the player.  we have to be careful to ignore our own Killed
  // message when it gets back to us -- do this by ignoring killed
  // message if we're already dead.
  // don't die if we had the shield flag and we've been shot.
  if (reason != GotShot || flag != Flags::Shield) {
    // blow me up
    tank->explodeTank();
    if (tank == myTank) {
      if (reason == GotRunOver)
	playLocalSound( SFX_RUNOVER );
      else
        playLocalSound(SFX_DIE);
    }
    else {
      const float* pos = tank->getPosition();
      if (reason == GotRunOver)
        playWorldSound(SFX_RUNOVER, pos[0], pos[1], pos[2],
				getLocalPlayer(killer) == myTank);
      else
        playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2],
				getLocalPlayer(killer) == myTank);

      float explodePos[3];
      explodePos[0] = pos[0];
      explodePos[1] = pos[1];
      explodePos[2] = pos[2] + MuzzleHeight;
      addTankExplosion(explodePos);
    }

    // i lose a point
    if (reason != GotCaptured)
      tank->changeScore(0, 1, 0);

    // tell server I'm dead if it won't already know
    if (reason == GotShot || reason == GotRunOver || reason == GenocideEffect)
      lookupServer(tank)->sendKilled(killer, reason, shotId);
  }

  // print reason if it's my tank
  if (tank == myTank && blowedUpMessage[reason]) {
    std::string blowedUpNotice = blowedUpMessage[reason];
    // first, check if i'm the culprit
    if (reason == GotShot && getLocalPlayer(killer) == myTank)
      blowedUpNotice = "Shot myself";
    else {
      // 1-4 are messages sent when the player dies because of someone else
      if (reason >= GotShot && reason <= GenocideEffect) {
	// matching the team-display style of other kill messages
	if (myTank->getTeam() == lookupPlayer(killer)->getTeam() && myTank->getTeam() != RogueTeam) {
	  blowedUpNotice += "teammate " ;
	  blowedUpNotice += lookupPlayer(killer)->getCallSign();
	}
	else {
	  blowedUpNotice += lookupPlayer(killer)->getCallSign();
	  blowedUpNotice += " (";
	  blowedUpNotice += Team::getName(lookupPlayer(killer)->getTeam());
	  blowedUpNotice += ")";
	}
      }
    }
    hud->setAlert(0, blowedUpNotice.c_str(), 4.0f, true);
    controlPanel->addMessage(blowedUpNotice);
  }

  // make sure shot is terminated locally (if not globally) so it can't
  // hit me again if I had the shield flag.  this is important for the
  // shots that aren't stopped by a hit and so may stick around to hit
  // me on the next update, making the shield useless.
  return (reason == GotShot && flag == Flags::Shield && shotId != -1);
}

static void		checkEnvironment()
{
  if (!myTank || Observer) return;

  // skip this if i'm dead or paused
  if (!myTank->isAlive() || myTank->isPaused()) return;

  FlagId flagId = myTank->getFlag();
  if (flagId != Flags::Null && int(flagId) >= int(FirstTeamFlag) &&
      int(flagId) <= int(LastTeamFlag)) {
    // have I captured a flag?
    TeamColor base = world->whoseBase(myTank->getPosition());
    TeamColor team = myTank->getTeam();
    if ((base != NoTeam) &&
	((int(flagId) == int(team) && base != team) ||
	(int(flagId) != int(team) && base == team)))
      serverLink->sendCaptureFlag(base);
  }
  else if (flagId == Flags::Null && (myTank->getLocation() == LocalPlayer::OnGround ||
      myTank->getLocation() == LocalPlayer::OnBuilding)) {
    // Don't grab too fast
    static TimeKeeper lastGrabSent;
    if (TimeKeeper::getTick()-lastGrabSent > 0.2) {
      // grab any and all flags i'm driving over
      const float* tpos = myTank->getPosition();
      const float radius = myTank->getRadius();
      const float radius2 = (radius + FlagRadius) * (radius + FlagRadius);
      for (int i = 0; i < numFlags; i++) {
	if (world->getFlag(i).id == Flags::Null || world->getFlag(i).status != FlagOnGround)
	  continue;
	if (world->getFlag(i).id == NullFlag)
	  continue;
	const float* fpos = world->getFlag(i).position;
	if ((fabs(tpos[2] - fpos[2]) < 0.1f) && ((tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
	    (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]) < radius2)) {
	  serverLink->sendGrabFlag(i);
	  lastGrabSent=TimeKeeper::getTick();
	}
      }
    }
  }
  else if (flagId == IdentifyFlag) {
    // identify closest flag
    const float* tpos = myTank->getPosition();
    std::string message("Closest Flag: ");
    float minDist = IdentityRange * IdentityRange;
    int closestFlag = -1;
    for (int i = 0; i < numFlags; i++) {
      if (world->getFlag(i).id == Flags::Null ||
	  world->getFlag(i).status != FlagOnGround) continue;
      const float* fpos = world->getFlag(i).position;
      const float dist = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
			 (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]) +
			 (tpos[2] - fpos[2]) * (tpos[2] - fpos[2]);
      if (dist < minDist) {
	minDist = dist;
	closestFlag = i;
      }
    }
    if (closestFlag != -1) {
      // Set HUD alert about what the flag is
      message += Flag::getName(world->getFlag(closestFlag).id);
      hud->setAlert(2, message.c_str(), 0.5f,
		Flag::getType(world->getFlag(closestFlag).id) == FlagSticky);
    }
  }

  // see if i've been shot
  const ShotPath* hit = NULL;
  float minTime = Infinity;
  myTank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i])
      myTank->checkHit(player[i], hit, minTime);
  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendEndShot(hit->getPlayer(), hit->getShotId(), 1);
    const bool stopShot =
	gotBlowedUp(myTank, GotShot, hit->getPlayer(), hit->getShotId());
    if (stopShot || hit->isStoppedByHit()) {
      Player* hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }

  // if not dead yet, see if i got run over by the steamroller
  else {
    const float* myPos = myTank->getPosition();
    const float myRadius = myTank->getRadius();
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i] &&
	  player[i]->getFlag() == SteamrollerFlag &&
	  !player[i]->isPaused()) {
	const float* pos = player[i]->getPosition();
	if (!(flagId == PhantomZoneFlag && myTank->isFlagActive())) {
	  const float radius = myRadius + SRRadiusMult * player[i]->getRadius();
	  if (hypot(hypot(myPos[0] - pos[0], myPos[1] - pos[1]), myPos[2] - pos[2]) < radius)
	    gotBlowedUp(myTank, GotRunOver, player[i]->getId());
	}
      }
  }
}

static void		setTarget()
{
  // get info about my tank
  const float c = cosf(-myTank->getAngle());
  const float s = sinf(-myTank->getAngle());
  const float x0 = myTank->getPosition()[0];
  const float y0 = myTank->getPosition()[1];

  // initialize best target
  Player* bestTarget = NULL;
  float bestDistance = Infinity;
  bool lockedOn = false;

  // figure out which tank is centered in my sights
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!player[i] || !player[i]->isAlive()) continue;

    // compute position in my local coordinate system
    const float* pos = player[i]->getPosition();
    const float x = c * (pos[0] - x0) - s * (pos[1] - y0);
    const float y = s * (pos[0] - x0) + c * (pos[1] - y0);

    // ignore things behind me
    if (x < 0.0f) continue;

    // get distance and sin(angle) from directly forward
    const float d = hypotf(x, y);
    const float a = fabsf(y / d);

    // see if it's inside lock-on angle (if we're trying to lock-on)
    if (a < 0.15f &&					// about 8.5 degrees
	myTank->getFlag() == GuidedMissileFlag &&	// am i locking on?
	player[i]->getFlag() != StealthFlag &&		// can't lock on stealth
	!player[i]->isPaused() &&			// can't lock on paused
	!player[i]->isNotResponding() &&		// can't lock on not responding
	d < bestDistance) {				// is it better?
      bestTarget = player[i];
      bestDistance = d;
      lockedOn = true;
    }
    else if (a < 0.3f &&				// about 17 degrees
	((player[i]->getFlag() != StealthFlag) || (myTank->getFlag() == SeerFlag)) &&	// can't "see" stealth unless have seer
	d < bestDistance && !lockedOn) {		// is it better?
      bestTarget = player[i];
      bestDistance = d;
    }
  }
  if (!lockedOn) myTank->setTarget(NULL);
  if (!bestTarget) return;

  if (lockedOn) {
    myTank->setTarget(bestTarget);
    myTank->setNemesis(bestTarget);

    std::string msg("Locked on ");
    msg += bestTarget->getCallSign();
    msg += " (";
    msg += Team::getName(bestTarget->getTeam());
    if (bestTarget->getFlag() != Flags::Null) {
      msg += ") with ";
      msg += Flag::getName(bestTarget->getFlag());
    }
    else {
      msg += ")";
    }
    hud->setAlert(1, msg.c_str(), 2.0f, 1);
    msg = ColorStrings[DefaultColor] + msg;
    addMessage(NULL, msg);
  }
  else if (myTank->getFlag() == ColorblindnessFlag) {
    std::string msg("Looking at a tank");
    hud->setAlert(1, msg.c_str(), 2.0f, 0);
    msg = ColorStrings[DefaultColor] + msg;
    addMessage(NULL, msg);
  }
  else {
    std::string msg("Looking at ");
    msg += bestTarget->getCallSign();
    msg += " (";
    msg += Team::getName(bestTarget->getTeam());
    if (bestTarget->getFlag() != Flags::Null) {
      msg += ") with ";
      msg += Flag::getName(bestTarget->getFlag());
    }
    else {
      msg += ")";
    }
    hud->setAlert(1, msg.c_str(), 2.0f, 0);
    msg = ColorStrings[DefaultColor] + msg;
    addMessage(NULL, msg);
    myTank->setNemesis(bestTarget);
  }
}

static void		setHuntTarget()
{
  // get info about my tank
  const float c = cosf(-myTank->getAngle());
  const float s = sinf(-myTank->getAngle());
  const float x0 = myTank->getPosition()[0];
  const float y0 = myTank->getPosition()[1];

  // initialize best target
  Player* bestTarget = NULL;
  float bestDistance = Infinity;
  bool lockedOn = false;

  // figure out which tank is centered in my sights
  for (int i = 0; i < maxPlayers; i++) {
    if (!player[i] || !player[i]->isAlive()) continue;

    // compute position in my local coordinate system
    const float* pos = player[i]->getPosition();
    const float x = c * (pos[0] - x0) - s * (pos[1] - y0);
    const float y = s * (pos[0] - x0) + c * (pos[1] - y0);

    // ignore things behind me
    if (x < 0.0f) continue;

    // get distance and sin(angle) from directly forward
    const float d = hypotf(x, y);
    const float a = fabsf(y / d);

    // see if it's inside lock-on angle (if we're trying to lock-on)
    if (a < 0.15f &&					// about 8.5 degrees
	myTank->getFlag() == GuidedMissileFlag &&	// am i locking on?
	player[i]->getFlag() != StealthFlag &&		// can't lock on stealth
	!player[i]->isPaused() &&			// can't lock on paused
	!player[i]->isNotResponding() &&		// can't lock on not responding
	d < bestDistance) {				// is it better?
      bestTarget = player[i];
      bestDistance = d;
      lockedOn = true;
    }
    else if (a < 0.3f &&				// about 17 degrees
	((player[i]->getFlag() != StealthFlag) || (myTank->getFlag() == SeerFlag)) &&	// can't "see" stealth unless have seer
	d < bestDistance && !lockedOn) {		// is it better?
      bestTarget = player[i];
      bestDistance = d;
    }
  }
  if (!bestTarget) return;


  if (bestTarget->isHunted()  && myTank->getFlag() != BlindnessFlag) {
    if (myTank->getTarget() == NULL) { // Don't interfere with GM lock display
      std::string msg("SPOTTED: ");
      msg += bestTarget->getCallSign();
      msg += " (";
      msg += Team::getName(bestTarget->getTeam());
      if (bestTarget->getFlag() != Flags::Null) {
        msg += ") with ";
        msg += Flag::getName(bestTarget->getFlag());
      } else {
        msg += ")";
      }
    hud->setAlert(1, msg.c_str(), 2.0f, 0);
    }
    if (!pulse.isOn()) {
      const float* bestTargetPosition = bestTarget->getPosition();
      playWorldSound(SFX_HUNT, bestTargetPosition[0], bestTargetPosition[1], bestTargetPosition[2]);
      pulse.setClock(1.0f);
    }
  }
}

static void		updateDaylight(double offset, SceneRenderer& renderer)
{
  static const double SecondsInDay = 86400.0;

  // update sun, moon & sky
  renderer.setTimeOfDay(unixEpoch + offset / SecondsInDay);
}

#ifdef ROBOT

//
// some robot stuff
//

static std::vector<BzfRegion*>	obstacleList;

static void		addObstacle(std::vector<BzfRegion*>& rgnList, const Obstacle& obstacle)
{
  float p[4][2];
  const float* c = obstacle.getPosition();
  const float a = obstacle.getRotation();
  // FIXME -- this is too generous;  robots will be able to come within
  //	0.49*TankWidth of a building at any orientation which means they
  //	could penetrate buildings.  it's either this or have robots go
  //	dead when they (or the target) moves within a dead-zone.
  const float w = obstacle.getWidth() + 0.49f * TankWidth;
  const float h = obstacle.getBreadth() + 0.49f * TankWidth;
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);
  p[0][0] = c[0] - xx - yx;
  p[0][1] = c[1] - xy - yy;
  p[1][0] = c[0] + xx - yx;
  p[1][1] = c[1] + xy - yy;
  p[2][0] = c[0] + xx + yx;
  p[2][1] = c[1] + xy + yy;
  p[3][0] = c[0] - xx + yx;
  p[3][1] = c[1] - xy + yy;

  int numRegions = rgnList.size();
  for (int k = 0; k < numRegions; k++) {
    BzfRegion* region = rgnList[k];
    int side[4];
    if ((side[0] = region->classify(p[0], p[1])) == 1 ||
		(side[1] = region->classify(p[1], p[2])) == 1 ||
		(side[2] = region->classify(p[2], p[3])) == 1 ||
		(side[3] = region->classify(p[3], p[0])) == 1)
      continue;
    if (side[0] == -1 && side[1] == -1 && side[2] == -1 && side[3] == -1) {
      BzfRegion* temp = rgnList[k];
      rgnList[k] = rgnList[numRegions-1];
      rgnList[numRegions-1] = temp;
      temp = rgnList[numRegions-1];
      rgnList[numRegions-1] = rgnList[rgnList.size()-1];
      rgnList[rgnList.size()-1] = temp;
      rgnList.pop_back();
      numRegions--;
      k--;
      delete region;
      continue;
    }
    for (int j = 0; j < 4; j++) {
      if (side[j] == -1) continue;		// to inside
      // split
      const float* p1 = p[j];
      const float* p2 = p[(j+1)&3];
      BzfRegion* newRegion = region->orphanSplitRegion(p2, p1);
      if (!newRegion) continue;		// no split
      if (region != rgnList[k]) rgnList.push_back(region);
      region = newRegion;
    }
    if (region != rgnList[k]) delete region;
  }
}

static void		makeObstacleList()
{
  int i;
  const int count = obstacleList.size();
  for (i = 0; i < count; i++)
    delete obstacleList[i];
  obstacleList.clear();

  // FIXME -- shouldn't hard code game area
  float gameArea[4][2];
  gameArea[0][0] = -0.5f * WorldSize;
  gameArea[0][1] = -0.5f * WorldSize;
  gameArea[1][0] =  0.5f * WorldSize;
  gameArea[1][1] = -0.5f * WorldSize;
  gameArea[2][0] =  0.5f * WorldSize;
  gameArea[2][1] =  0.5f * WorldSize;
  gameArea[3][0] = -0.5f * WorldSize;
  gameArea[3][1] =  0.5f * WorldSize;
  obstacleList.push_back(new BzfRegion(4, gameArea));

  const std::vector<BoxBuilding>& boxes = World::getWorld()->getBoxes();
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++)
    addObstacle(obstacleList, boxes[i]);
  const std::vector<PyramidBuilding>& pyramids = World::getWorld()->getPyramids();
  const int numPyramids = pyramids.size();
  for (i = 0; i < numPyramids; i++)
    addObstacle(obstacleList, pyramids[i]);
  const std::vector<Teleporter>& teleporters = World::getWorld()->getTeleporters();
  const int numTeleporters = teleporters.size();
  for (i = 0; i < numTeleporters; i++)
    addObstacle(obstacleList, teleporters[i]);
}

static void		setRobotTarget(RobotPlayer* robot)
{
  Player* bestTarget = NULL;
  float bestPriority = 0.0f;
  for (int j = 0; j < curMaxPlayers; j++)
    if (player[j] && player[j]->getId() != robot->getId() &&
	player[j]->isAlive() && (robot->getTeam() == RogueTeam ||
	player[j]->getTeam() != robot->getTeam())) {
      const float priority = robot->getTargetPriority(player[j]);
      if (priority > bestPriority) {
	bestTarget = player[j];
	bestPriority = priority;
      }
    }
  if (myTank->isAlive() && (robot->getTeam() == RogueTeam ||
				myTank->getTeam() != robot->getTeam())) {
    const float priority = robot->getTargetPriority(myTank);
    if (priority > bestPriority) {
      bestTarget = myTank;
      bestPriority = priority;
    }
  }
  robot->setTarget(obstacleList, bestTarget);
}

static void		updateRobots(float dt)
{
  static float newTargetTimeout = 2.0f;
  static float clock = 0.0f;
  bool pickTarget = false;
  int i;

  // see if we should look for new targets
  clock += dt;
  if (clock > newTargetTimeout) {
    while (clock > newTargetTimeout) clock -= newTargetTimeout;
    pickTarget = true;
  }

  // start dead robots and retarget
  for (i = 0; i < numRobots; i++)
    if (!gameOver && !robots[i]->isAlive() && !robots[i]->isExploding()) {
      robots[i]->restart();
      robotServer[i]->sendAlive(robots[i]->getPosition(), robots[i]->getForward());
      setRobotTarget(robots[i]);
    }
    else if (pickTarget || (robots[i]->getTarget() &&
				!robots[i]->getTarget()->isAlive())) {
      setRobotTarget(robots[i]);
    }

  // do updates
  for (i = 0; i < numRobots; i++)
    robots[i]->update();
}


static void		checkEnvironment(RobotPlayer* tank)
{
  // skip this if i'm dead or paused
  if (!tank->isAlive() || tank->isPaused()) return;

  // see if i've been shot
  const ShotPath* hit = NULL;
  float minTime = Infinity;
  tank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i] && player[i]->getId() != tank->getId())
      tank->checkHit(player[i], hit, minTime);
  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      lookupServer(tank)->sendEndShot(hit->getPlayer(), hit->getShotId(), 1);
    gotBlowedUp(tank, GotShot, hit->getPlayer(), hit->getShotId());
    if (hit->isStoppedByHit()) {
      Player* hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }

  // if not dead yet, see if i got run over by the steamroller
  else if (tank->getPosition()[2] == 0.0) {
    bool dead = false;
    const float* myPos = tank->getPosition();
    const float myRadius = tank->getRadius();
    if (myTank->getFlag() == SteamrollerFlag && !myTank->isPaused()) {
      const float* pos = myTank->getPosition();
      const float radius = myRadius + SRRadiusMult * myTank->getRadius();
      if (hypot(hypot(myPos[0] - pos[0], myPos[1] - pos[1]), myPos[2] - pos[2]) < radius) {
	gotBlowedUp(tank, GotRunOver, myTank->getId());
	dead = true;
      }
    }
    for (i = 0; !dead && i < curMaxPlayers; i++)
      if (player[i] &&
	  player[i]->getFlag() == SteamrollerFlag &&
	  !player[i]->isPaused()) {
	  const float* pos = player[i]->getPosition();
	  const float radius = myRadius + SRRadiusMult * player[i]->getRadius();
	  if (hypot(hypot(myPos[0] - pos[0], myPos[1] - pos[1]), myPos[2] - pos[2]) < radius) {
	    gotBlowedUp(tank, GotRunOver, player[i]->getId());
	    dead = true;
	  }
      }
  }
}

static void		checkEnvironmentForRobots()
{
  for (int i = 0; i < numRobots; i++)
    checkEnvironment(robots[i]);
}

static void		sendRobotUpdates()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->isDeadReckoningWrong()) {
      playerLink->setRelay(robotServer[i]);
      playerLink->sendPlayerUpdate(robots[i]);
    }
}

static void		addRobots(bool useMulticastRelay)
{
  uint16_t code, len;
  char msg[MaxPacketLen];
  char callsign[CallSignLen];

  for (int j = 0; j < numRobots;) {

#if !defined(_WIN32)
	snprintf(callsign, CallSignLen, "%s%2.2d", myTank->getCallSign(), j);
#else
	sprintf(callsign, "%s%2.2d", myTank->getCallSign(), j);
#endif

    robots[j] = new RobotPlayer(robotServer[j]->getId(), callsign, robotServer[j], myTank->getEmailAddress());
    if (world->allowRabbit())
      robots[j]->setTeam(RogueTeam);
    else if (world->allowRogues())
      robots[j]->setTeam((TeamColor)((int)RogueTeam + (int)(bzfrand() *
					(int)(PurpleTeam - RogueTeam + 1))));
    else
      robots[j]->setTeam((TeamColor)((int)RedTeam + (int)(bzfrand() *
					(int)(PurpleTeam - RedTeam + 1))));
    robotServer[j]->sendEnter(ComputerPlayer, robots[j]->getTeam(),
		robots[j]->getCallSign(), robots[j]->getEmailAddress());

    // wait for response
    if (robotServer[j]->read(code, len, msg, -1) < 0 || code != MsgAccept) {
      delete robots[j];
      delete robotServer[j];
      robotServer[j] = robotServer[--numRobots];
      continue;
    }

    // use multicast relay if required
    if (useMulticastRelay) {
      robotServer[j]->send(MsgNetworkRelay, 0, NULL);
      if (robotServer[j]->read(code, len, msg, 1000) <= 0 || code == MsgReject) {
	delete robots[j];
	delete robotServer[j];
	robotServer[j] = robotServer[--numRobots];
	continue;
      }
    }

    j++;
  }
  makeObstacleList();
}

#endif

std::string		getCacheDirectoryName()
{
#if defined(_WIN32)
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

  name += "\\bzflag-cache";
  mkdir(name.c_str());
  return name;

#elif defined(__APPLE__)
  std::string cacheName;
  ::FSRef libraryFolder;
  ::OSErr err;
  err = ::FSFindFolder(::kUserDomain, ::kApplicationSupportFolderType, true, &libraryFolder);
  if(err == ::noErr) {
    char buff[1024];
    err = ::FSRefMakePath(&libraryFolder, (UInt8*)buff, sizeof(buff));
    if(err == ::noErr) {
      std::strcat(buff, "/BZFlag");
      struct stat statbuf;
      if (!(stat(buff, &statbuf) == 0 && (S_ISDIR(statbuf.st_mode)))) {
	if(mkdir(buff, 0777) != 0) {
	  return NULL;
	}
      }
      std::strcat(buff, "/cache");
      if (!(stat(buff, &statbuf) == 0 && (S_ISDIR(statbuf.st_mode)))) {
	if(mkdir(buff, 0777) != 0) {
	  return NULL;
	}
      }
      fprintf(stderr, "cache dir is \"%s\"\n", buff);
      cacheName = buff;
    }
  }
  return cacheName;
#else
  std::string name;
  struct passwd *pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += std::string(pwent->pw_dir);
    name += "/";
  }
  name += ".bzflag-cache";

  // add in hostname on UNIX
  if (getenv("HOST")) {
    name += ".";
    name += getenv("HOST");
  }

  struct stat statbuf;
  if (!(stat(name.c_str(), &statbuf) == 0 && (S_ISDIR(statbuf.st_mode)))) {
    if(mkdir(name.c_str(), 0777) != 0) {
      return "bzflag-cache";
    }
  }

  return name;
#endif
}

static void cleanWorldCache()
{
  char buffer[10];
  int cacheLimit = 100L * 1024L;
  if (resources->hasValue("worldCacheLimit"))
    cacheLimit = atoi(resources->getValue("worldCacheLimit").c_str());
  else {
#ifndef _WIN32
    snprintf(buffer, 10, "%d", cacheLimit);
#else
    sprintf(buffer, "%d", cacheLimit);
#endif
    resources->addValue("worldCacheLimit", buffer);
  }

  std::string worldPath = getCacheDirectoryName();

  char *oldestFile = NULL;
  int oldestSize = 0;
  int totalSize = 0;

  do {
    oldestFile = 0;
    totalSize = 0;
#ifdef _WIN32
	  std::string pattern = worldPath + "/*.bwc";

	  WIN32_FIND_DATA findData;
	  HANDLE h = FindFirstFile(pattern.c_str(), &findData);
	  if (h != INVALID_HANDLE_VALUE) {
	    FILETIME oldestTime = findData.ftLastAccessTime;
	    oldestFile = strdup(findData.cFileName);
	    oldestSize = findData.nFileSizeLow;
	    totalSize = findData.nFileSizeLow;

	    while (FindNextFile(h, &findData)) {
		if (CompareFileTime( &oldestTime, &findData.ftLastAccessTime ) > 0) {
		  oldestTime = findData.ftLastAccessTime;
		  if (oldestFile)
		    free(oldestFile);
		  oldestFile = strdup(findData.cFileName);
		  oldestSize = findData.nFileSizeLow;
		}
		totalSize += findData.nFileSizeLow;
	    }
	    FindClose(h);
	  }
#else
	  DIR *directory = opendir(worldPath.c_str());
	  if (directory) {
	    struct dirent* contents;
	    struct stat statbuf;
	    time_t oldestTime = time(NULL);
	    while ((contents = readdir(directory))) {
	      stat((worldPath + "/" + contents->d_name).c_str(), &statbuf);
	      if (statbuf.st_atime < oldestTime) {
		if (oldestFile)
		  free(oldestFile);
		oldestFile = strdup(contents->d_name);
		oldestSize = statbuf.st_size;
	      }
	      totalSize += statbuf.st_size;
	    }
	    closedir(directory);

	  }
#endif

	  if (totalSize < cacheLimit) {
	    if (oldestFile != NULL) {
		free(oldestFile);
		oldestFile = NULL;
	    }
	    return;
	  }

	  if (oldestFile != NULL)
	    remove((worldPath + "/" + oldestFile).c_str());

	  if (oldestFile != NULL)
	    free(oldestFile);
	  totalSize -= oldestSize;
  } while (oldestFile && (totalSize > cacheLimit));
}

static void markOld(std::string &fileName)
{
#ifdef _WIN32
  FILETIME ft;
  HANDLE h = CreateFile(fileName.c_str(), FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h != INVALID_HANDLE_VALUE) {

    SYSTEMTIME st;
    memset( &st, 0, sizeof(st));
    st.wYear = 1900;
    st.wMonth = 1;
    st.wDay = 1;
    SystemTimeToFileTime( &st, &ft );
    BOOL b = SetFileTime(h, &ft, &ft, &ft);
    int i = GetLastError();
    CloseHandle(h);
  }
#else
  struct utimbuf times;
  times.actime = 0;
  times.modtime = 0;
  utime(fileName.c_str(), &times);
#endif
}

static int negotiateFlags(ServerLink* serverLink)
{
  uint16_t code, len;
  char msg[MaxPacketLen];
  char *buf = msg;
  buf = (char *) nboPackUShort(buf, LastFlag - FirstFlag + 1);
  for (int i = FirstFlag; i <= LastFlag; i++) {
	const char *abbv = Flag::getAbbreviation((FlagId)i);
	buf = (char *) nboPackString( buf, abbv, 2);
  }
  serverLink->send( MsgNegotiateFlags, buf - msg, msg );

  if (serverLink->read(code, len, msg, 5000) <= 0) return MsgNull;
  return code;
}

//
// join/leave a game
//

static World*		makeWorld(ServerLink* serverLink)
{
  FILE *cachedWorld = NULL;
  uint16_t code, len;
  uint32_t size;
  char msg[MaxPacketLen];
  std::string worldPath;
  bool isTemp = false;

  //ask for the hash of the world (ignoring all other messages)
  serverLink->send( MsgWantWHash, 0, NULL );
  if (serverLink->read(code, len, msg, 5000) > 0) {
	  if (code != MsgWantWHash) return NULL;

	  char *hexDigest = new char[len];
	  nboUnpackString( msg, hexDigest, len );
	  isTemp = hexDigest[0] == 't';

	  worldPath = getCacheDirectoryName();
	  worldPath += "/";
	  worldPath += hexDigest;
	  worldPath += ".bwc";
	  cachedWorld = fopen( worldPath.c_str(), "rb" );
  }

  char* worldDatabase;
  if (cachedWorld == NULL) {
	  // ask for world and wait for it (ignoring all other messages)
	  nboPackUInt(msg, 0);
	  serverLink->send(MsgGetWorld, sizeof(uint32_t), msg);
	  if (serverLink->read(code, len, msg, 5000) <= 0) return NULL;
	  if (code == MsgNull || code == MsgSuperKill) return NULL;
	  if (code != MsgGetWorld) return NULL;

	  // get size of entire world database and make space
	  void *buf = nboUnpackUInt(msg, size);
	  worldDatabase = new char[size];

	  // get world database
	  uint32_t ptr = 0, bytesLeft = size;
	  while (bytesLeft != 0) {
		// add chunk to database so far
		::memcpy(worldDatabase + int(ptr), buf, len - sizeof(uint32_t));

		// increment pointer
		ptr += len - sizeof(uint32_t);
		// ask and wait for next chunk
		nboPackUInt(msg, ptr);
		serverLink->send(MsgGetWorld, sizeof(uint32_t), msg);
		if (serverLink->read(code, len, msg, 5000) < 0 ||
		code == MsgNull || code == MsgSuperKill) {
		  delete[] worldDatabase;
		  return NULL;
		}
		// get bytes left
		buf = nboUnpackUInt(msg, bytesLeft);
	  }
	  //add final chunk
	  ::memcpy(worldDatabase + int(ptr), buf, len - sizeof(uint32_t));

	  if (worldPath.length() > 0) {
		  cleanWorldCache();
		  cachedWorld = fopen(worldPath.c_str(), "wb");
		  if (cachedWorld != NULL) {
			  fwrite(worldDatabase, size, 1, cachedWorld);
			  fclose(cachedWorld);
			  if (isTemp)
			    markOld(worldPath);
		  }
	  }
  }
  else
  {

	  fseek( cachedWorld, 0, SEEK_END );
	  long size = ftell( cachedWorld );
	  fseek( cachedWorld, 0, SEEK_SET );
	  worldDatabase = new char[size];
	  fread( worldDatabase, size, 1, cachedWorld );
	  fclose( cachedWorld );
  }

  // make world
  WorldBuilder worldBuilder;
  worldBuilder.unpack(worldDatabase);
  delete[] worldDatabase;

  // return world
  return worldBuilder.getWorld();
}

static bool		enterServer(ServerLink* serverLink, World* world,
						LocalPlayer* myTank)
{

  time_t timeout=time(0) + 10;  // give us 10 sec

  if (world->allowRabbit())
    myTank->setTeam(RogueTeam);

  // tell server we want to join
  serverLink->sendEnter(TankPlayer, myTank->getTeam(),
		myTank->getCallSign(), myTank->getEmailAddress());

  // @ as first lettter of callsign is observer
  Observer = myTank->getCallSign()[0] == '@';
  roaming = Observer;


  controlPanel->setControlColor(Team::getRadarColor(myTank->getTeam()));
  radar->setControlColor(Team::getRadarColor(myTank->getTeam()));

  // wait for response
  uint16_t code, len;
  char msg[MaxPacketLen];
  if (serverLink->read(code, len, msg, -1) < 0) {
    printError("Communication error joining game [No immediate respose].");
    return false;
  }
  if (code == MsgSuperKill) {
    printError("Server forced disconnection.");
    return false;
  }
  if (code != MsgAccept && code != MsgReject) {
    char buf[10];
    std::vector<std::string> args;
    sprintf(buf, "%04x", code);
    args.push_back(buf);
    printError("Communication error joining game [Wrong Code {1}].",&args);
    return false;
  }
  if (code == MsgReject) {
    uint16_t rejectCode;
    nboUnpackUShort(msg, rejectCode);
    switch (rejectCode) {
      default:
      case RejectBadRequest:
      case RejectBadTeam:
      case RejectBadType:
	printError("Communication error joining game [Rejected].");
	break;

      case RejectNoRogues:
	printError("Rogues not allowed.  Try another team.");
	break;

      case RejectTeamFull:
	printError("This team is full.  Try another team.");
	break;

      case RejectServerFull:
	printError("This game is full.  Try again later.");
	break;
    }
    return false;
  }

  // get updates
  if (serverLink->read(code, len, msg, -1) < 0) {
	goto failed;
  }
  while (code == MsgAddPlayer || code == MsgTeamUpdate ||
	 code == MsgFlagUpdate || code == MsgNetworkRelay ||
	 code == MsgUDPLinkRequest) {
    void* buf = msg;
    switch (code) {
      case MsgAddPlayer: {
	PlayerId id;
	buf = nboUnpackUByte(buf, id);
        if (id == myTank->getId()) {
	  // it's me!  end of updates
	  // scan through flags and, for flags on
	  // tanks, tell the tank about its flag.
	  const int maxFlags = world->getMaxFlags();
	  for (int i = 0; i < maxFlags; i++) {
	    const Flag& flag = world->getFlag(i);
	    if (flag.status == FlagOnTank)
	      for (int j = 0; j < curMaxPlayers; j++)
		if (player[j] && player[j]->getId() == flag.owner) {
		  player[j]->setFlag(flag.id);
		  break;
		}
	  }
	  return true;
	}
	addPlayer(id, buf, false);
	break;
      }
      case MsgTeamUpdate: {
	uint16_t team;
	buf = nboUnpackUShort(buf, team);
	buf = teams[int(team)].unpack(buf);
	break;
      }
      case MsgFlagUpdate: {
	uint16_t flag;
	buf = nboUnpackUShort(buf, flag);
	buf = world->getFlag(int(flag)).unpack(buf);
	world->initFlag(int(flag));
	break;
      }
      case MsgNetworkRelay: {
	playerLink->setUseRelay();
	playerLink->setRelay(serverLink);
	printError("Using server as relay");
	break;
      }
      case MsgUDPLinkRequest:
	printError("*** Received UDP Link Granted");
	// internally
	break;
    }

    if (time(0)>timeout) goto failed;

    if (serverLink->read(code, len, msg, -1) < 0) goto failed;
  }

failed:
  printError("Communication error joining game");
  return false;
}

static void		leaveGame()
{
  // delete scene database
  sceneRenderer->setSceneDatabase(NULL);
  delete zScene;
  delete bspScene;
  zScene = NULL;
  bspScene = NULL;

  // no more radar
  controlPanel->setRadarRenderer(NULL);
  delete radar;
  radar = NULL;

#if defined(ROBOT)
  // shut down robot connections
  int i;
  for (i = 0; i < numRobots; i++) {
    if (robots[i] && robotServer[i])
      robotServer[i]->send(MsgExit, 0, NULL);
    delete robots[i];
    delete robotServer[i];
    robots[i] = NULL;
    robotServer[i] = NULL;
  }
  numRobots = 0;

  const int count = obstacleList.size();
  for (i = 0; i < count; i++)
    delete obstacleList[i];
  obstacleList.clear();
#endif

  // my tank goes away
  const bool sayGoodbye = (myTank != NULL);
  LocalPlayer::setMyTank(NULL);
  delete myTank;
  myTank = NULL;

  // time goes back to current time if previously constrained by server
  if (world && !world->allowTimeOfDayAdjust()) {
    epochOffset = userTimeEpochOffset;
    updateDaylight(epochOffset, *sceneRenderer);
    lastEpochOffset = epochOffset;
  }

  // delete world
  World::setWorld(NULL);
  delete world;
  world = NULL;
  teams = NULL;
  maxPlayers = 0;
  curMaxPlayers = 0;
  numFlags = 0;
  player = NULL;

  // update UI
  hud->setPlaying(false);
  hud->setCracks(false);
  hud->setPlayerHasHighScore(false);
  hud->setTeamHasHighScore(false);
  hud->setHeading(0.0f);
  hud->setAltitude(0.0f);
  hud->setAltitudeTape(false);
  hud->setMarker(0, false);
  hud->setMarker(1, false);

  // shut down player channel
  PlayerLink::setMulticast(NULL);
  delete playerLink;
  playerLink = NULL;

  // shut down server connection
  if (sayGoodbye) serverLink->send(MsgExit, 0, NULL);
  ServerLink::setServer(NULL);
  delete serverLink;
  serverLink = NULL;

  // reset viewpoint
  float eyePoint[3], targetPoint[3];
  eyePoint[0] = 0.0f;
  eyePoint[1] = 0.0f;
  eyePoint[2] = 0.0f + MuzzleHeight;
  targetPoint[0] = eyePoint[0] - 1.0f;
  targetPoint[1] = eyePoint[1] + 0.0f;
  targetPoint[2] = eyePoint[2] + 0.0f;
  sceneRenderer->getViewFrustum().setProjection(60.0f * M_PI / 180.0f,
      1.1f, 1.5f * WorldSize, mainWindow->getWidth(),
      mainWindow->getHeight(), mainWindow->getViewHeight());
  sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

  // reset some flags
  gameOver = false;
  serverError = false;
  serverDied = false;
}

static bool		joinGame(const StartupInfo* info,
				ServerLink* _serverLink,
				PlayerLink* _playerLink)
{
  // assume everything's okay for now
  serverDied = false;
  serverError = false;
  admin = false;

  serverLink = _serverLink;
  playerLink = _playerLink;

  if (!serverLink || !playerLink) {
    printError("Memory error");
    leaveGame();
    return false;
  }

  // printError("Join Game");
  // check server
  if (serverLink->getState() != ServerLink::Okay) {
    switch (serverLink->getState()) {
      case ServerLink::BadVersion: {
	static char versionError[] = "Incompatible server version XXXXXXXX";
	strncpy(versionError + strlen(versionError) - 8,
					serverLink->getVersion(), 8);
	printError(versionError);
	break;
      }

      case ServerLink::Rejected:
	// the server is probably full or the game is over.  if not then
	// the server is having network problems.
	printError("Game is full or over.  Try again later.");
	break;

      case ServerLink::SocketError:
	printError("Error connecting to server.");
	break;

      case ServerLink::CrippledVersion:
	// can't connect to (otherwise compatible) non-crippled server
	printError("Cannot connect to full version server.");
	break;

      default:
	printError("Internal error connecting to server.");
	break;
    }

    leaveGame();
    return false;
  }

  // check inter-player connection
/* NOTE -- this will be handled later when we try to fallback to TCP
  if (playerLink->getState() == PlayerLink::SocketError) {
    printError("Couldn't make inter-player connection");
    leaveGame();
    return false;
  }
*/

  // set tank textures
  Player::setTexture(*tankTexture);

  int code = negotiateFlags(serverLink);
  switch (code) {
    case MsgSuperKill:
	printError("Your tank is not capable of carrying flags found in this world");
	leaveGame();
	return false;
    break;
    case MsgNull:
        printError("Communication error joining game [No immediate respose].");
        leaveGame();
	return false;
    break;

  }

  // create world
  world = makeWorld(serverLink);
  if (!world) {
    printError("Error downloading world database");
    leaveGame();
    return false;
  }

  ServerLink::setServer(serverLink);
  PlayerLink::setMulticast(playerLink);
  World::setWorld(world);

  // prep teams
  teams = world->getTeams();

  // prep players
  maxPlayers = world->getMaxPlayers();
  curMaxPlayers = 0;
  player = world->getPlayers();

  // prep flags
  numFlags = world->getMaxFlags();

  // make scene database
  const bool oldUseZBuffer = BZDB->isTrue("zbuffer");
  BZDB->set("zbuffer", "0");
  bspScene = sceneBuilder->make(world);
  BZDB->set("zbuffer", "1");
  // FIXME - test the zbuffer here
  if (BZDB->isTrue("zbuffer"))
    zScene = sceneBuilder->make(world);
  BZDB->set("zbuffer", oldUseZBuffer ? "1" : "0");
  setSceneDatabase();

  mainWindow->getWindow()->yieldCurrent();
  // make radar
  radar = new RadarRenderer(*sceneRenderer, *world);
  mainWindow->getWindow()->yieldCurrent();

  controlPanel->setRadarRenderer(radar);
  controlPanel->resize();

  // make local player
  myTank = new LocalPlayer(serverLink->getId(), info->callsign, info->email);
  myTank->setTeam(info->team);
  LocalPlayer::setMyTank(myTank);

  // enter server
  if (!enterServer(serverLink, world, myTank)) {
    delete myTank;
    myTank = NULL;
    leaveGame();
    return false;
  }

  // check multicast `connection' to server.  if we get no response then
  // we have to assume the network can't do multicasting.  fall back to
  // using the server as a relay.
  bool multicastOkay = false;
  /* FIXME - commented out by davidtrowbridge for ubyte playerid
   * id.port is the issue
  if (playerLink->getState() == PlayerLink::Okay) {
    // send 5 pings, one every 2/10ths of a second.  wait up to two
    // seconds after the last ping for a reply.  that's kinda long but
    // let's be generous.  first open the sockets.
    Address multicastAddress(BroadcastAddress);
    struct sockaddr_in pingOutAddr, pingInAddr;
    const int pingOutSocket = openMulticast(multicastAddress,
				ServerPort, NULL,
				info->ttl, info->multicastInterface,
				"w", &pingOutAddr);
    const int pingInSocket = openMulticast(multicastAddress,
				ServerPort, NULL,
				info->ttl, info->multicastInterface,
				"r", &pingInAddr);
    if (pingOutSocket != -1 && pingInSocket != -1) {
      Address myAddress("");

      // now send pings and wait for an echo
      int count = 5;
      PingPacket ping;
      while (count-- > 0) {
	// send another ping
	ping.sendRequest(pingOutSocket, &pingOutAddr, info->ttl);

	// wait for a response.  if the ping didn't originate with me
	// or if it's for a server on a different port than the server
	// I want then ignore.  should check server's address but the
	// server could be multihomed so we might not get the address
	// we expect.  problems can only occur if another process on
	// this host is trying to contact another server using the
	// same port at the same time we are.
	if (ping.waitForReply(pingInSocket, myAddress, 200) &&
	    ping.serverId.port == htons(info->serverPort)) {
	  multicastOkay = true;
	  break;
	}
      }

      // if no reply yet, wait another couple of seconds
      if (!multicastOkay)
	multicastOkay = ping.waitForReply(pingInSocket, myAddress, 2000) &&
		ping.serverId.port == htons(info->serverPort);
    }

    // close sockets
    closeMulticast(pingOutSocket);
    closeMulticast(pingInSocket);
  }*/

  // if multicast isn't okay then ask server if it can relay for us.
  // give up after waiting for one second.  if we don't get a valid
  // reply or our request is rejected then quit game because we won't
  // be able to talk to other players.
  if (!multicastOkay && playerLink->getState() != PlayerLink::ServerRelay) {
    char msgbuf[MaxPacketLen];
    uint16_t code, len;
    serverLink->send(MsgNetworkRelay, 0, NULL);
    if (serverLink->read(code, len, msgbuf, 1000) <= 0 || code == MsgReject) {
      printError("Couldn't make inter-player connection");
      leaveGame();
      return false;
    }

    // prepare player link to use the server as a relay
    playerLink->setUseRelay();
    playerLink->setRelay(serverLink);
    printError("Using multicast relay");
  }

  // use parallel UDP if desired and using server relay
  if (startupInfo.useUDPconnection && (playerLink->getState() == PlayerLink::ServerRelay))
    playerLink->enableUDPConIfRelayed();
  else
    printError("No UDP connection, see Options to enable.");

  // set marker colors -- team color and antidote flag color
  const float* myTeamColor = Team::getTankColor(myTank->getTeam());
  hud->setMarkerColor(0, myTeamColor[0], myTeamColor[1], myTeamColor[2]);
  hud->setMarkerColor(1, 1.0f, 1.0f, 0.0f);

  // add robot tanks
#if defined(ROBOT)
  addRobots(!multicastOkay);
#endif

  // tell server what ttl I need
  char msg[2];
  nboPackUShort(msg, playerLink->getTTL());
  serverLink->send(MsgSetTTL, sizeof(msg), msg);

  // decide how start for first time
  restartOnBase = world->allowTeamFlags() && myTank->getTeam() != RogueTeam &&
		  !Observer;

  // if server constrains time then adjust it
  if (!world->allowTimeOfDayAdjust()) {
    epochOffset = double(world->getEpochOffset());
    updateDaylight(epochOffset, *sceneRenderer);
    lastEpochOffset = epochOffset;
  }

  // initialize some other stuff
  updateNumPlayers();
  updateFlag(Flags::Null);
  updateHighScores();
  radar->setRange(RadarMedRangeFactor*WorldSize);
  hud->setHeading(myTank->getAngle());
  hud->setAltitude(myTank->getPosition()[2]);
  hud->setTimeLeft(-1);
  fireButton = false;
  firstLife = true;

  return true;
}

static bool		joinInternetGame(const StartupInfo* info)
{
  // open server
  Address serverAddress(info->serverName);
  if (serverAddress.isAny()) return false;
  ServerLink* serverLink = new ServerLink(serverAddress, info->serverPort);

  Address multicastAddress(BroadcastAddress);
  PlayerLink* playerLink = new PlayerLink(multicastAddress, BroadcastPort,
				info->ttl, info->multicastInterface);

#if defined(ROBOT)
  extern int numRobotTanks;
  int i, j;
  for (i = 0, j = 0; i < numRobotTanks; i++) {
    robotServer[j] = new ServerLink(serverAddress, info->serverPort, j + 1);
    if (!robotServer[j] || robotServer[j]->getState() != ServerLink::Okay) {
      delete robotServer[j];
      continue;
    }
    j++;
  }
  numRobots = j;
#endif

  return joinGame(info, serverLink, playerLink);
}

static bool		joinGame()
{
  // assume we have everything we need to join.  figure out how
  // to join by which arguments are set in StartupInfo.
  // currently only support joinInternetGame.
  if (startupInfo.serverName[0])
    return joinInternetGame(&startupInfo);

  // can't figure out how to join
  printError("Can't figure out how to join.");
  return false;
}

//
// main playing loop
//

static void		renderDialog()
{
  if (HUDDialogStack::get()->isActive()) {
    const int width = mainWindow->getWidth();
    const int height = mainWindow->getHeight();
    const int ox = mainWindow->getOriginX();
    const int oy = mainWindow->getOriginY();
    glScissor(ox, oy, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    OpenGLGState::resetState();
    HUDDialogStack::get()->render();
    glPopMatrix();
  }
}

static int		getZoomFactor()
{
  if (!resources->hasValue("zoom")) return 1;
  const int zoom = atoi(resources->getValue("zoom").c_str());
  if (zoom < 1) return 1;
  if (zoom > 8) return 8;
  return zoom;
}

static void		playingLoop()
{
  static const float	defaultPos[3] = { 0.0f, 0.0f, 0.0f };
  static const float	defaultDir[3] = { 1.0f, 0.0f, 0.0f };
  static const GLfloat  colorblindColor[3] = { 0.25f, 0.25f, 0.25f };
  const float* myTankPos;
  const float* myTankDir;
  GLfloat eyePoint[3];
  GLfloat targetPoint[3];
  GLfloat fov;
  int i;

  // get media object
  BzfMedia* media = PlatformFactory::getMedia();

  // get view type (constant for entire game)
  const SceneRenderer::ViewType viewType = sceneRenderer->getViewType();
  const int zoomFactor = getZoomFactor();
  const bool fakeCursor = resources->hasValue("fakecursor") &&
		strcmp(resources->getValue("fakecursor").c_str(), "1") == 0;
  mainWindow->setZoomFactor(zoomFactor);
  if (fakeCursor)
    mainWindow->getWindow()->hideMouse();

  // start timing
  int frameCount = 0;
  float cumTime = 0.0f;
  TimeKeeper::setTick();
  updateDaylight(epochOffset, *sceneRenderer);

  // main loop
  while (!CommandsStandard::isQuit()) {
    // get delta time
    TimeKeeper prevTime = TimeKeeper::getTick();
    TimeKeeper::setTick();
    const float dt = TimeKeeper::getTick() - prevTime;

    mainWindow->getWindow()->yieldCurrent();

    // handle incoming packets
    doMessages();

    mainWindow->getWindow()->yieldCurrent();

    // do dead reckoning on remote players
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i]) {
	const bool wasNotResponding = player[i]->isNotResponding();
	player[i]->doDeadReckoning();
	const bool isNotResponding = player[i]->isNotResponding();
	if (!wasNotResponding && isNotResponding) {
	  addMessage(player[i], "not responding");
	}
	else if (wasNotResponding && !isNotResponding) {
	  addMessage(player[i], "okay");
	}
      }

    // try to join a game if requested.  do this *before* handling
    // events so we do a redraw after the request is posted and
    // before we actually try to join.
    if (joinGameCallback) {
      // if already connected to a game then first sign off
      if (myTank) leaveGame();

      // now try connecting
      (*joinGameCallback)(joinGame(), joinGameUserData);

      // don't try again
      joinGameCallback = NULL;
    }

    mainWindow->getWindow()->yieldCurrent();

    // handle events
    clockAdjust = 0.0f;
    while (!CommandsStandard::isQuit() && display->isEventPending())
      doEvent(display);

    mainWindow->getWindow()->yieldCurrent();

    // invoke callbacks
    callPlayingCallbacks();

    mainWindow->getWindow()->yieldCurrent();

    // quick out
    if (CommandsStandard::isQuit()) break;

    // if server died then leave the game (note that this may cause
    // further server errors but that's okay).
    if (serverError ||
	(serverLink && serverLink->getState() == ServerLink::Hungup)) {
      // if we haven't reported the death yet then do so now
      if (serverDied ||
	  (serverLink && serverLink->getState() == ServerLink::Hungup))
	printError("Server has unexpectedly disconnected");
      leaveGame();
    }

    // update time of day -- update sun and sky every few seconds
    epochOffset += (double)dt;
    if (!world || world->allowTimeOfDayAdjust())
      epochOffset += (double)(50.0f * dt * clockAdjust);
    if (fabs(epochOffset - lastEpochOffset) >= 4.0) {
      updateDaylight(epochOffset, *sceneRenderer);
      lastEpochOffset = epochOffset;
    }

    // move roaming camera
    if (roaming) {
      float c, s;
      c = cosf(roamTheta * M_PI / 180.0f);
      s = sinf(roamTheta * M_PI / 180.0f);
      roamPos[0] += dt * (c * roamDPos[0] - s * roamDPos[1]);
      roamPos[1] += dt * (c * roamDPos[1] + s * roamDPos[0]);
      roamPos[2] += dt * roamDPos[2];
      if (roamPos[2] < MuzzleHeight)
	roamPos[2] = MuzzleHeight;
      roamTheta  += dt * roamDTheta;
      roamPhi    += dt * roamDPhi;
      roamZoom   += dt * roamDZoom;
      if (roamZoom < 1.0f)
	roamZoom = 1.0f;
      else if (roamZoom > 179.0f)
	roamZoom = 179.0f;
    }
    setRoamingLabel(false);

    // update test video format timer
    if (testVideoFormatTimer > 0.0f) {
      testVideoFormatTimer -= dt;
      if (testVideoFormatTimer <= 0.0f) {
	testVideoFormatTimer = 0.0f;
	setVideoFormat(testVideoPrevFormat);
      }
    }

    // update pause countdown
    if (!myTank) pauseCountdown = 0.0f;
    if (pauseCountdown > 0.0f && !myTank->isAlive()) {
      pauseCountdown = 0.0f;
      hud->setAlert(1, NULL, 0.0f, true);
    }
    if (pauseCountdown > 0.0f) {
      const int oldPauseCountdown = (int)(pauseCountdown + 0.99f);
      pauseCountdown -= dt;
      if (pauseCountdown <= 0.0f) {
	// okay, now we pause.  first drop any team flag we may have.
	const FlagId flagId = myTank->getFlag();
	if (flagId >= FirstTeamFlag && flagId <= LastTeamFlag)
	  serverLink->sendDropFlag(myTank->getPosition());

	if (World::getWorld()->allowRabbit() && (myTank->getTeam() == RabbitTeam))
	  serverLink->sendNewRabbit();

	// now actually pause
	myTank->setPause(true);
	hud->setAlert(1, NULL, 0.0f, true);
	controlPanel->addMessage("Paused");

	// turn off the sound
	if (savedVolume == -1) {
	  savedVolume = getSoundVolume();
	  setSoundVolume(0);
	}

	// ungrab mouse
	mainWindow->ungrabMouse();
      }
      else if ((int)(pauseCountdown + 0.99f) != oldPauseCountdown &&
							!pausedByUnmap) {
	// update countdown alert
	char msgBuf[40];
	sprintf(msgBuf, "Pausing in %d", (int)(pauseCountdown + 0.99f));
	hud->setAlert(1, msgBuf, 1.0f, false);
      }
    }

    // update destruct countdown
    if (!myTank) destructCountdown = 0.0f;
    if (destructCountdown > 0.0f && !myTank->isAlive()) {
      destructCountdown = 0.0f;
      hud->setAlert(1, NULL, 0.0f, true);
    }
    if (destructCountdown > 0.0f) {
      const int oldDestructCountdown = (int)(destructCountdown + 0.99f);
      destructCountdown -= dt;
      if (destructCountdown <= 0.0f) {

	// now actually destruct
	gotBlowedUp( myTank, SelfDestruct, myTank->getId() );

	hud->setAlert(1, NULL, 0.0f, true);
      }
      else if ((int)(destructCountdown + 0.99f) != oldDestructCountdown) {
	// update countdown alert
	char msgBuf[40];
	sprintf(msgBuf, "Self Destructing in %d", (int)(destructCountdown + 0.99f));
	hud->setAlert(1, msgBuf, 1.0f, false);
      }
    }

    // reposition flags
    updateFlags(dt);

    // update explosion animations
    updateExplosions(dt);

    // update other tank's shots
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i])
	player[i]->updateShots(dt);

    // stuff to draw a frame
    if (!unmapped) {
      // compute fps
      frameCount++;
      cumTime += float(dt);
      if (cumTime >= 2.0) {
	if (showFPS) hud->setFPS(float(frameCount) / cumTime);
	cumTime = 0.0;
	frameCount = 0;
      }

      // drift clouds
      sceneRenderer->getBackground()->addCloudDrift(1.0f * dt, 0.731f * dt);

      // get tank camera info
      if (!myTank) {
	myTankPos = defaultPos;
	myTankDir = defaultDir;
	fov = 60.0f;
      }
      else {
	myTankPos = myTank->getPosition();
	myTankDir = myTank->getForward();

	if (viewType == SceneRenderer::ThreeChannel) {
	  if (myTank->getFlag() == WideAngleFlag) fov = 90.0f;
	  else fov = (myTank->getMagnify() == 1 ? 12.0f : 45.0f);
	}
	else {
	  if (myTank->getFlag() == WideAngleFlag) fov = 120.0f;
	  else fov = (myTank->getMagnify() == 1 ? 15.0f : 60.0f);
	}
      }
      fov *= M_PI / 180.0f;

      // set projection and view
      eyePoint[0] = myTankPos[0];
      eyePoint[1] = myTankPos[1];
      eyePoint[2] = myTankPos[2] + MuzzleHeight;
      targetPoint[0] = eyePoint[0] + myTankDir[0];
      targetPoint[1] = eyePoint[1] + myTankDir[1];
      targetPoint[2] = eyePoint[2] + myTankDir[2];
      if (roaming) {
	hud->setAltitude(-1.0f);
	float roamViewAngle;
#ifdef FOLLOWTANK
	eyePoint[0] = myTankPos[0] - myTankDir[0] * 20;
	eyePoint[1] = myTankPos[1] - myTankDir[1] * 20;
	eyePoint[2] = myTankPos[2] + MuzzleHeight * 3;
	targetPoint[0] = eyePoint[0] + myTankDir[0];
	targetPoint[1] = eyePoint[1] + myTankDir[1];
	targetPoint[2] = eyePoint[2] + myTankDir[2];
#else
	setRoamingLabel(false);
	if (player && (roamView != roamViewFree) && player[roamTrackWinner]) {
	  RemotePlayer *target = player[roamTrackWinner];
	  const float *targetTankDir = target->getForward();
	  // fixed camera tracking target
	  if (roamView == roamViewTrack) {
	    eyePoint[0] = roamPos[0];
	    eyePoint[1] = roamPos[1];
	    eyePoint[2] = roamPos[2];
	    targetPoint[0] = target->getPosition()[0];
	    targetPoint[1] = target->getPosition()[1];
	    targetPoint[2] = target->getPosition()[2];
	  }
	  // camera following target
	  else if (roamView == roamViewFollow) {
	    eyePoint[0] = target->getPosition()[0] - targetTankDir[0] * 40;
	    eyePoint[1] = target->getPosition()[1] - targetTankDir[1] * 40;
	    eyePoint[2] = target->getPosition()[2] + MuzzleHeight * 6;
	    targetPoint[0] = target->getPosition()[0];
	    targetPoint[1] = target->getPosition()[1];
	    targetPoint[2] = target->getPosition()[2];
	  }
	  // target's view
	  else if (roamView == roamViewFP) {
	    eyePoint[0] = target->getPosition()[0];
	    eyePoint[1] = target->getPosition()[1];
	    eyePoint[2] = target->getPosition()[2] + MuzzleHeight;
	    targetPoint[0] = eyePoint[0] + targetTankDir[0];
	    targetPoint[1] = eyePoint[1] + targetTankDir[1];
	    targetPoint[2] = eyePoint[2] + targetTankDir[2];
	    hud->setAltitude(target->getPosition()[2]);
	  }
	  // track team flag
	  else if (roamView == roamViewFlag) {
	    Flag &targetFlag = world->getFlag(roamTrackFlag);
	    eyePoint[0] = roamPos[0];
	    eyePoint[1] = roamPos[1];
	    eyePoint[2] = roamPos[2];
	    targetPoint[0] = targetFlag.position[0];
	    targetPoint[1] = targetFlag.position[1];
	    targetPoint[2] = targetFlag.position[2];
	  }
	  roamViewAngle = (float) (atan2(targetPoint[1]-eyePoint[1],
	      targetPoint[0]-eyePoint[0]) * 180.0f / M_PI);
	}
	// free Roaming
	else {
	  float dir[3];
	  dir[0] = cosf(roamPhi * M_PI / 180.0f) * cosf(roamTheta * M_PI / 180.0f);
	  dir[1] = cosf(roamPhi * M_PI / 180.0f) * sinf(roamTheta * M_PI / 180.0f);
	  dir[2] = sinf(roamPhi * M_PI / 180.0f);
	  eyePoint[0] = roamPos[0];
	  eyePoint[1] = roamPos[1];
	  eyePoint[2] = roamPos[2];
	  targetPoint[0] = eyePoint[0] + dir[0];
	  targetPoint[1] = eyePoint[1] + dir[1];
	  targetPoint[2] = eyePoint[2] + dir[2];
	  roamViewAngle = roamTheta;
	}
	float virtPos[]={eyePoint[0], eyePoint[1], 0};
	if (myTank)
	  myTank->move(virtPos, roamViewAngle * M_PI / 180.0f);
#endif
	fov = roamZoom * M_PI / 180.0f;
	moveSoundReceiver(eyePoint[0], eyePoint[1], eyePoint[2], 0.0, false);
      }
      sceneRenderer->getViewFrustum().setProjection(fov,
	  1.1f, 1.5f * WorldSize,
	  mainWindow->getWidth(),
	  mainWindow->getHeight(),
	  mainWindow->getViewHeight());
      sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

      // add dynamic nodes
      SceneDatabase* scene = sceneRenderer->getSceneDatabase();
      if (scene && myTank) {
	// add my tank
	myTank->addPlayer(scene, 0, false);
	if (myTank->getFlag() == CloakingFlag) {
	  // and make it invisible
	  myTank->setInvisible();
	} else if (roaming)
	  myTank->setHidden(false);
	else {
	  // or make it hidden
	  myTank->setHidden();
	}
	// add my shells
	myTank->addShots(scene, false);
	// add antidote flag
	myTank->addAntidote(scene);
	// add flags
	world->addFlags(scene);

	const GLfloat *override = NULL;
	if (myTank->getFlag() == ColorblindnessFlag)
	  override = colorblindColor;

	// add other tanks and shells
	const bool colorblind = (myTank->getFlag() == ColorblindnessFlag);
	for (i = 0; i < curMaxPlayers; i++)
	  if (player[i]) {
	    player[i]->updateSparks(dt);
	    player[i]->addShots(scene, colorblind);
	    if (!colorblind && (player[i]->getFlag() == MasqueradeFlag) && (myTank->getFlag() != SeerFlag)) {
	       override = Team::getTankColor(myTank->getTeam());
	    }
	    player[i]->addPlayer(scene, override, true);
	    if ((player[i]->getFlag() == CloakingFlag) && (myTank->getFlag() != SeerFlag))
	      player[i]->setInvisible();
	    else
	      player[i]->setHidden(roaming && roamView == roamViewFP && roamTrackWinner == i);
	  }

	// add explosions
	addExplosions(scene);

	// if i'm inside a building then add eighth dimension scene node.
	if (myTank->getContainingBuilding()) {
	  SceneNode* node = world->getInsideSceneNode(myTank->getContainingBuilding());
	  if (node) scene->addDynamicNode(node);
	}
      }

      // turn blanking and inversion on/off as appropriate
      sceneRenderer->setBlank(myTank && (myTank->isPaused() ||
	  myTank->getFlag() == BlindnessFlag));
      sceneRenderer->setInvert(myTank &&
	  myTank->getFlag() == PhantomZoneFlag &&
	  myTank->isFlagActive());

      // turn on scene dimming when showing menu or when
      // we're dead and no longer exploding.
      sceneRenderer->setDim(HUDDialogStack::get()->isActive() ||
	  (myTank && !roaming && !myTank->isAlive() && !myTank->isExploding()));

      // set hud state
      hud->setDim(HUDDialogStack::get()->isActive());
      hud->setPlaying(myTank && (myTank->isAlive() && !myTank->isPaused()));
      hud->setRoaming(roaming);
      hud->setCracks(myTank && !firstLife && !myTank->isAlive());

      // get frame start time
      if (showDrawTime) {
#if defined(DEBUG_RENDERING)
	// get an accurate measure of frame time (at expense of frame rate)
	glFinish();
#endif
	media->stopwatch(true);
      }

      // draw frame
      const bool blankRadar = myTank && myTank->isPaused();
      if (viewType == SceneRenderer::ThreeChannel) {
	// draw center channel
	sceneRenderer->render(false);
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render(*sceneRenderer);
	if (radar) radar->render(*sceneRenderer, blankRadar);

	// set up for drawing left channel
	mainWindow->setQuadrant(MainWindow::LowerLeft);
	// FIXME -- this assumes up is along +z
	const float cFOV = cosf(fov);
	const float sFOV = sinf(fov);
	targetPoint[0] = eyePoint[0] + cFOV*myTankDir[0] - sFOV*myTankDir[1];
	targetPoint[1] = eyePoint[1] + cFOV*myTankDir[1] + sFOV*myTankDir[0];
	targetPoint[2] = eyePoint[2] + myTankDir[2];
	sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

	// draw left channel
	sceneRenderer->render(false, true, true);

	// set up for drawing right channel
	mainWindow->setQuadrant(MainWindow::LowerRight);
	// FIXME -- this assumes up is along +z
	targetPoint[0] = eyePoint[0] + cFOV*myTankDir[0] + sFOV*myTankDir[1];
	targetPoint[1] = eyePoint[1] + cFOV*myTankDir[1] - sFOV*myTankDir[0];
	targetPoint[2] = eyePoint[2] + myTankDir[2];
	sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

	// draw right channel
	sceneRenderer->render(true, true, true);

#if defined(DEBUG_RENDERING)
	// set up for drawing rear channel
	mainWindow->setQuadrant(MainWindow::UpperLeft);
	// FIXME -- this assumes up is along +z
	targetPoint[0] = eyePoint[0] - myTankDir[0];
	targetPoint[1] = eyePoint[1] - myTankDir[1];
	targetPoint[2] = eyePoint[2] + myTankDir[2];
	sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

	// draw rear channel
	sceneRenderer->render(true, true, true);
#endif
	// back to center channel
	mainWindow->setQuadrant(MainWindow::UpperRight);
      }
      else if (viewType == SceneRenderer::Stacked) {
       static float EyeDisplacement = 0.25f * TankWidth;
       static float FocalPlane = BoxBase;
       static bool init = false;
       if (!init) {
	 init = true;
	 if (resources->hasValue("eyesep"))
	   EyeDisplacement = (float)atof(resources->getValue("eyesep").c_str());
	 if (resources->hasValue("focal"))
	   FocalPlane = (float)atof(resources->getValue("focal").c_str());
       }

       // setup view for left eye
       sceneRenderer->getViewFrustum().setOffset(EyeDisplacement, FocalPlane);

       // draw left eye's view
       sceneRenderer->render(false);
       hud->render(*sceneRenderer);
       renderDialog();
       controlPanel->render(*sceneRenderer);
       if (radar) radar->render(*sceneRenderer, blankRadar);

       // set up view for right eye
       mainWindow->setQuadrant(MainWindow::UpperHalf);
       sceneRenderer->getViewFrustum().setOffset(-EyeDisplacement, FocalPlane);

       // draw right eye's view
       sceneRenderer->render(true, true);
       hud->render(*sceneRenderer);
       renderDialog();
       controlPanel->render(*sceneRenderer);
       if (radar) radar->render(*sceneRenderer, blankRadar);

       // draw common stuff

       // back to left channel
       mainWindow->setQuadrant(MainWindow::LowerHalf);
      }
      else if (viewType == SceneRenderer::Stereo) {
	static float EyeDisplacement = 0.25f * TankWidth;
	static float FocalPlane = BoxBase;
	static bool init = false;
	if (!init) {
	  init = true;
	  if (resources->hasValue("eyesep"))
	    EyeDisplacement = (float)atof(resources->getValue("eyesep").c_str());
	  if (resources->hasValue("focal"))
	    FocalPlane = (float)atof(resources->getValue("focal").c_str());
	}

	// setup view for left eye
#ifdef USE_GL_STEREO
	glDrawBuffer(GL_BACK_LEFT);
#endif
	sceneRenderer->getViewFrustum().setOffset(EyeDisplacement, FocalPlane);

	// draw left eye's view
	sceneRenderer->render(false);
#ifndef USE_GL_STEREO
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render(*sceneRenderer);
	if (radar) radar->render(*sceneRenderer, blankRadar);
#endif

	// set up view for right eye
#ifdef USE_GL_STEREO
	glDrawBuffer(GL_BACK_RIGHT);
#else
	mainWindow->setQuadrant(MainWindow::UpperLeft);
#endif
	sceneRenderer->getViewFrustum().setOffset(-EyeDisplacement, FocalPlane);

	// draw right eye's view
	sceneRenderer->render(true, true);
#ifndef USE_GL_STEREO
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render(*sceneRenderer);
	if (radar) radar->render(*sceneRenderer, blankRadar);
#endif

	// draw common stuff
#ifdef USE_GL_STEREO
	glDrawBuffer(GL_BACK);
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render(*sceneRenderer);
	if (radar) radar->render(*sceneRenderer, blankRadar);
#endif

#ifndef USE_GL_STEREO
	// back to left channel
	mainWindow->setQuadrant(MainWindow::UpperRight);
#endif
      }
      else {
	if (zoomFactor != 1) {
	  // draw small out-the-window view
	  mainWindow->setQuadrant(MainWindow::ZoomRegion);
	  const int x = mainWindow->getOriginX();
	  const int y = mainWindow->getOriginY();
	  const int w = mainWindow->getWidth();
	  const int h = mainWindow->getHeight();
	  const int vh = mainWindow->getViewHeight();
	  sceneRenderer->getViewFrustum().setProjection(fov, 1.1f, 1.5f * WorldSize, w, h, vh);
	  sceneRenderer->render();

	  // set entire window
	  mainWindow->setQuadrant(MainWindow::FullWindow);
	  glScissor(mainWindow->getOriginX(), mainWindow->getOriginY(),
		    mainWindow->getWidth(), mainWindow->getHeight());

	  // set pixel copy destination
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  glOrtho(-0.25, (GLdouble)mainWindow->getWidth() - 0.25,
		-0.25, (GLdouble)mainWindow->getHeight() - 0.25, -1.0, 1.0);
	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	  glLoadIdentity();
	  glRasterPos2i(0, 0);
	  glPopMatrix();

	  // zoom small image to entire window
	  glDisable(GL_DITHER);
	  glPixelZoom((float)zoomFactor, (float)zoomFactor);
	  glCopyPixels(x, y, w, h, GL_COLOR);
	  glPixelZoom(1.0f, 1.0f);
	  if (BZDB->isTrue("dither")) glEnable(GL_DITHER);
	}
	else {
	  // normal rendering
	  sceneRenderer->render();
	}

	// draw other stuff
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render(*sceneRenderer);
	if (radar) radar->render(*sceneRenderer, blankRadar);
      }

      // get frame end time
      if (showDrawTime) {
#if defined(DEBUG_RENDERING)
	// get an accurate measure of frame time (at expense of frame rate)
	glFinish();
#endif
	hud->setDrawTime((float)media->stopwatch(false));
      }

      // draw a fake cursor if requested.  this is mostly intended for
      // pass through 3D cards that don't have cursor support.
      if (fakeCursor) {
	int mx, my;
	const int width = mainWindow->getWidth();
	const int height = mainWindow->getHeight();
	const int ox = mainWindow->getOriginX();
	const int oy = mainWindow->getOriginY();
	mainWindow->getWindow()->getMouse(mx, my);
	my = height - my - 1;

	glScissor(ox, oy, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0f, 0.0f, 0.0f);
	glRecti(mx - 8, my - 2, mx - 2, my + 2);
	glRecti(mx + 2, my - 2, mx + 8, my + 2);
	glRecti(mx - 2, my - 8, mx + 2, my - 2);
	glRecti(mx - 2, my + 2, mx + 2, my + 8);

	glColor3f(1.0f, 1.0f, 1.0f);
	glRecti(mx - 7, my - 1, mx - 3, my + 1);
	glRecti(mx + 3, my - 1, mx + 7, my + 1);
	glRecti(mx - 1, my - 7, mx + 1, my - 3);
	glRecti(mx - 1, my + 3, mx + 1, my + 7);

	glPopMatrix();
      }

      mainWindow->getWindow()->swapBuffers();

      // remove dynamic nodes from this frame
      if (scene) scene->removeDynamicNodes();
    }
    else {
      // wait around a little to avoid spinning the CPU when iconified
      media->sleep(0.05f);
    }

    updateSound();

    // do motion
    if (myTank) {
      if (myTank->isAlive() && !myTank->isPaused()) {
	doMotion();
	if (hud->getHunting()) setHuntTarget(); //spot hunt target
	if (fireButton && myTank->getFlag() == MachineGunFlag && !Observer)
	  myTank->fireShot();
      }
      else {
	int mx, my;
	mainWindow->getMousePosition(mx, my);
      }
      myTank->update();
    }

#ifdef ROBOT
    updateRobots(dt);
#endif

    // prep the HUD
    if (myTank) {
      const float* myPos = myTank->getPosition();
      hud->setHeading(myTank->getAngle());
      hud->setAltitude(myPos[2]);
      if (world->allowTeamFlags() &&
		int(myTank->getFlag()) != int(myTank->getTeam())) {
	// marker for my team flag
	for (i = 0; i < numFlags; i++) {
	  Flag& flag = world->getFlag(i);
	  if (int(flag.id) == int(myTank->getTeam())) {
	    const float* flagPos = flag.position;
	    hud->setMarkerHeading(0, atan2f(flagPos[1] - myPos[1],
					flagPos[0] - myPos[0]));
	    break;
	  }
	}
      }
      if (myTank->getAntidoteLocation()) {
	// marker for my antidote flag
	const GLfloat* antidotePos = myTank->getAntidoteLocation();
	hud->setMarkerHeading(1, atan2f(antidotePos[1] - myPos[1],
					antidotePos[0] - myPos[0]));
      }
    }

    // check for flags and hits
    checkEnvironment();

#ifdef ROBOT
    checkEnvironmentForRobots();
#endif

    // send my data
    if (playerLink && myTank->isDeadReckoningWrong() && !Observer) {
      playerLink->setRelay(serverLink);
      playerLink->sendPlayerUpdate(myTank);
    }

#ifdef ROBOT
    sendRobotUpdates();
#endif
  }

  // restore the sound.  if we don't do this then we'll save the
  // wrong volume when we dump out the configuration file if the
  // app exits when the game is paused.
  if (savedVolume != -1) {
    setSoundVolume(savedVolume);
    savedVolume = -1;
  }

  // hide window
  mainWindow->showWindow(false);
}

//
// game initialization
//

static float		timeConfiguration(bool useZBuffer)
{
  // prepare depth buffer if requested
  BZDB->set("zbuffer", useZBuffer ? "1" : "0");
  if (useZBuffer) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  // use glFinish() to get accurate timings
  glFinish();
  TimeKeeper startTime = TimeKeeper::getCurrent();
  sceneRenderer->setExposed();
  sceneRenderer->render();
  glFinish();
  TimeKeeper endTime = TimeKeeper::getCurrent();

  // turn off depth buffer
  if (useZBuffer) glDisable(GL_DEPTH_TEST);

  return endTime - startTime;
}

static void		timeConfigurations()
{
  static const float MaxFrameTime = 0.050f;	// seconds

  // ignore results of first test.  OpenGL could be doing lazy setup.
  BZDB->set("blend", "0");
  BZDB->set("smooth", "0");
  BZDB->set("lighting", "0");
  BZDB->set("texture", "0");
  sceneRenderer->setQuality(0);
  BZDB->set("dither", "1");
  BZDB->set("shadows", "0");
  BZDB->set("enhancedradar", "0");
  OpenGLTexture::setFilter(OpenGLTexture::Off);
  timeConfiguration(true);

  // time lowest quality with and without blending.  some systems
  // stipple very slowly even though everything else is fast.  we
  // don't want to conclude the system is slow because of stippling.
  printError("  lowest quality");
  const float timeNoBlendNoZ = timeConfiguration(false);
  const float timeNoBlendZ   = timeConfiguration(true);
  BZDB->set("blend", "1");
  const float timeBlendNoZ   = timeConfiguration(false);
  const float timeBlendZ     = timeConfiguration(true);
  if (timeNoBlendNoZ > MaxFrameTime &&
      timeNoBlendZ   > MaxFrameTime &&
      timeBlendNoZ   > MaxFrameTime &&
      timeBlendZ     > MaxFrameTime) {
    if (timeNoBlendNoZ < timeNoBlendZ &&
	timeNoBlendNoZ < timeBlendNoZ &&
	timeNoBlendNoZ < timeBlendZ) {
      // no depth, no blending definitely fastest
      BZDB->set("zbuffer", "0");
      BZDB->set("blend", "0");
    }
    if (timeNoBlendZ < timeBlendNoZ &&
	timeNoBlendZ < timeBlendZ) {
      // no blending faster than blending
      BZDB->set("zbuffer", "0");
      BZDB->set("blend", "0");
    }
    if (timeBlendNoZ < timeBlendZ) {
      // blending faster than depth
      BZDB->set("zbuffer", "0");
      BZDB->set("blend", "1");
    }
    // blending and depth faster than without either
    BZDB->set("zbuffer", "1");
    BZDB->set("blend", "1");
    return;
  }

  // leave blending on if blending clearly faster than stippling
  if (timeBlendNoZ > timeNoBlendNoZ || timeBlendNoZ > timeNoBlendZ &&
      timeBlendZ   > timeNoBlendNoZ || timeBlendZ   > timeNoBlendZ) {
    BZDB->set("blend", "0");
  }

  // try texturing.  if it's too slow then fall back to
  // lowest quality and return.
  OpenGLTexture::setFilter(OpenGLTexture::Nearest);
  BZDB->set("texture", OpenGLTexture::getFilterName());
  sceneRenderer->setQuality(1);
  printError("  lowest quality with texture");
  if (timeConfiguration(false) > MaxFrameTime ||
      timeConfiguration(true) > MaxFrameTime) {
    BZDB->set("texture", "0");
    OpenGLTexture::setFilter(OpenGLTexture::Off);
    sceneRenderer->setQuality(0);
    return;
  }

  // everything
  printError("  full quality");
  BZDB->set("blend", "1");
  BZDB->set("smooth", "1");
  BZDB->set("lighting", "1");
  OpenGLTexture::setFilter(OpenGLTexture::LinearMipmapLinear);
  BZDB->set("texture", OpenGLTexture::getFilterName());
  sceneRenderer->setQuality(2);
  BZDB->set("dither", "1");
  BZDB->set("shadows", "1");
  BZDB->set("enhancedradar", "1");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // try it without shadows -- some platforms stipple very slowly
  BZDB->set("shadows", "0");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no high quality
  printError("  medium quality");
  sceneRenderer->setQuality(1);
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;
  printError("  low quality");
  sceneRenderer->setQuality(0);
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // lower quality texturing
  printError("  nearest texturing");
  OpenGLTexture::setFilter(OpenGLTexture::Nearest);
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no texturing
  printError("  no texturing");
  BZDB->set("texture", "0");
  OpenGLTexture::setFilter(OpenGLTexture::Off);
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no blending
  printError("  no blending");
  BZDB->set("blend", "0");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no smoothing.  shouldn't really affect fill rate too much.
  printError("  no smoothing");
  BZDB->set("smooth", "0");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no lighting.  shouldn't really affect fill rate, either.
  printError("  no lighting");
  BZDB->set("lighting", "0");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;

  // no dithering
  printError("  no dithering");
  BZDB->set("dither", "0");
  if (timeConfiguration(true) < MaxFrameTime) return;
  if (timeConfiguration(false) < MaxFrameTime) return;
}

static void		findFastConfiguration()
{
  // time the rendering of the background with various rendering styles
  // until we find one fast enough.  these tests assume that we're
  // going to be fill limited.  each test comes in a pair:  with and
  // without the zbuffer.
  //
  // this, of course, is only a rough estimate since we're not drawing
  // a normal frame (no radar, no HUD, no buildings, etc.).  the user
  // can always turn stuff on later and the settings are remembered
  // across invokations.

  // setup projection
  static const GLfloat eyePoint[3] = { 0.0f, 0.0f, MuzzleHeight };
  static const GLfloat targetPoint[3] = { 0.0f, 10.0f, MuzzleHeight };
  sceneRenderer->getViewFrustum().setProjection(45.0f * M_PI / 180.0f,
					1.1f, 1.5f * WorldSize,
					mainWindow->getWidth(),
					mainWindow->getHeight(),
					mainWindow->getViewHeight());
  sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

  // add a big wall in front of where we're looking.  this is important
  // because once textures are off, the background won't draw much of
  // anything.  this will ensure that we continue to test polygon fill
  // rate.  with one polygon it doesn't matter if we use a z or bsp
  // database.
  static const GLfloat base[3]  = { -10.0f, 10.0f,  0.0f };
  static const GLfloat sEdge[3] = {  20.0f,  0.0f,  0.0f };
  static const GLfloat tEdge[3] = {   0.0f,  0.0f, 10.0f };
  static const GLfloat color[4] = { 1.0f, 1.0f, 1.0f, 0.5f };
  SceneDatabase* timingScene = new ZSceneDatabase;
  WallSceneNode* node = new QuadWallSceneNode(base,
					sEdge, tEdge, 1.0f, 1.0f, true);
  node->setColor(color);
  node->setModulateColor(color);
  node->setLightedColor(color);
  node->setLightedModulateColor(color);
  node->setTexture(HUDuiControl::getArrow());
  node->setMaterial(OpenGLMaterial(color, color));
  timingScene->addStaticNode(node);
  sceneRenderer->setSceneDatabase(timingScene);
  sceneRenderer->setDim(false);

  timeConfigurations();

  sceneRenderer->setSceneDatabase(NULL);
  delete timingScene;
}

static void		defaultErrorCallback(const char* msg)
{
  controlPanel->addMessage(msg);
}

static void		startupErrorCallback(const char* msg)
{
  controlPanel->addMessage(msg);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  controlPanel->render(*sceneRenderer);
  mainWindow->getWindow()->swapBuffers();
}

void			startPlaying(BzfDisplay* _display,
					SceneRenderer& renderer,
					ResourceDatabase& _resources,
					StartupInfo* _info)
{
  int i;
  // initalization
  display = _display;
  sceneRenderer = &renderer;
  resources = &_resources;
  mainWindow = &sceneRenderer->getWindow();
  
  // register some commands
  for (unsigned int c = 0; c < countof(commandList); ++c) {
    CMDMGR->add(commandList[c].name, commandList[c].func, commandList[c].help);
  }

  // make control panel
  ControlPanel _controlPanel(*mainWindow, *sceneRenderer);
  controlPanel = &_controlPanel;

  // tell the control panel how many frame buffers there are.  we
  // cheat when drawing the control panel, not drawing it if it
  // hasn't changed.  that only works if we've filled all the
  // frame buffers (e.g. front and back buffers) with the correct
  // data.
  // FIXME -- assuming the contents of any frame buffer except the
  // front buffer are anything but garbage violates the OpenGL
  // spec.  we really should redraw the control panel every frame
  // but this works on every system so far.
  {
    int n = 3;	// assume triple buffering
    switch (sceneRenderer->getViewType()) {
      case SceneRenderer::Stacked:
      case SceneRenderer::Stereo:
#ifndef USE_GL_STEREO
	// control panel drawn twice per frame
	n *= 2;
#endif
	break;

      case SceneRenderer::ThreeChannel:
      default:
	// only one copy of control panel visible
	break;
    }
    controlPanel->setNumberOfFrameBuffers(n);
  }

  // if no configuration, turn off fancy rendering so startup is fast,
  // even on a slow machine.
  if (!_info->hasConfiguration) {
    BZDB->set("blend", "0");
    BZDB->set("smooth", "0");
    BZDB->set("lighting", "0");
    BZDB->set("texture", "0");
    sceneRenderer->setQuality(0);
    BZDB->set("dither", "0");
    BZDB->set("shadows", "0");
    BZDB->set("enhancedradar", "0");
    OpenGLTexture::setFilter(OpenGLTexture::Off);
  }

  // should we grab the mouse?  yes if fullscreen.
  if (!BZDB->isSet("_window"))
    setGrabMouse(true);
#if defined(__linux__) && !defined(DEBUG)
  // linux usually has a virtual root window so grab mouse always
  setGrabMouse(true);
#endif

  // show window and clear it immediately
  mainWindow->showWindow(true);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glDisable(GL_SCISSOR_TEST);
  glClear(GL_COLOR_BUFFER_BIT);
  mainWindow->getWindow()->swapBuffers();

  // resize and draw basic stuff
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST);
  controlPanel->resize();
  sceneRenderer->render();
  controlPanel->render(*sceneRenderer);
  mainWindow->getWindow()->swapBuffers();

  // startup error callback adds message to control panel and
  // forces an immediate redraw.
  setErrorCallback(startupErrorCallback);

  // initialize epoch offset (time)
  userTimeEpochOffset = (double)mktime(&userTime);
  epochOffset = userTimeEpochOffset;
  updateDaylight(epochOffset, *sceneRenderer);
  lastEpochOffset = epochOffset;

  // catch kill signals before changing video mode so we can
  // put it back even if we die.  ignore a few signals.
  bzSignal(SIGILL, SIG_PF(dying));
  bzSignal(SIGABRT, SIG_PF(dying));
  bzSignal(SIGSEGV, SIG_PF(dying));
  bzSignal(SIGTERM, SIG_PF(suicide));
#if !defined(_WIN32)
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(suicide));
  bzSignal(SIGPIPE, SIG_PF(hangup));
  bzSignal(SIGHUP, SIG_IGN);
  if (bzSignal(SIGQUIT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGQUIT, SIG_PF(dying));
  #ifndef GUSI_20
  bzSignal(SIGBUS, SIG_PF(dying));
  #endif
  bzSignal(SIGUSR1, SIG_IGN);
  bzSignal(SIGUSR2, SIG_IGN);
#endif /* !defined(_WIN32) */

  // set the resolution (only if in full screen mode)
  if (!BZDB->isSet("_window") && BZDB->isSet("resolution")) {
    std::string videoFormat = BZDB->get("resolution");
    if (videoFormat.length() != 0) {
      const int format = display->findResolution(videoFormat.c_str());
      if (display->isValidResolution(format) &&
	  display->getResolution() != format &&
	  display->setResolution(format)) {

	// handle resize
	if (resources->hasValue("geometry")) {
	  int w, h, x, y, count;
	  char xs, ys;
	  count = sscanf(resources->getValue("geometry").c_str(),
			"%dx%d%c%d%c%d", &w, &h, &xs, &x, &ys, &y);
	  if (w < 256) w = 256;
	  if (h < 192) h = 192;
	  if (count == 6) {
	    if (xs == '-') x = display->getWidth() - x - w;
	    if (ys == '-') y = display->getHeight() - y - h;
	    mainWindow->setPosition(x, y);
	  }
	  mainWindow->setSize(w, h);
	}
	else {
	  mainWindow->setFullscreen();
	}

	// more resize handling
	mainWindow->getWindow()->callResizeCallbacks();
	mainWindow->warpMouse();
      }
    }
  }

  // grab mouse if we should
  if (shouldGrabMouse())
    mainWindow->grabMouse();

  // draw again
  glClear(GL_COLOR_BUFFER_BIT);
  sceneRenderer->render();
  controlPanel->render(*sceneRenderer);
  mainWindow->getWindow()->swapBuffers();
  mainWindow->getWindow()->yieldCurrent();

  // make heads up display
  HUDRenderer _hud(display, renderer);
  hud = &_hud;

  // initialize control panel and hud
  updateNumPlayers();
  updateFlag(Flags::Null);
  updateHighScores();
  notifyBzfKeyMapChanged();

  // make background renderer
  BackgroundRenderer background(renderer);
  sceneRenderer->setBackground(&background);

  // if no configuration file try to determine rendering settings
  // that yield reasonable performance.
  if (!_info->hasConfiguration) {
    printError("testing performance;  please wait...");
    findFastConfiguration();
    dumpResources(display, renderer);
  }

  // load prototype explosions
  static const char*	explosionNames[] = {
				"explode1",
				"explode2",
				"explode3",
				"explode4"
			};
  static const GLfloat	zero[3] = { 0.0f, 0.0f, 0.0f };
  for (i = 0; i < (int)(sizeof(explosionNames) /
					sizeof(explosionNames[0])); i++) {
    // try loading texture
    OpenGLTexture tex = getTexture(explosionNames[i],
				OpenGLTexture::Linear, false, true);
    if (!tex.isValid()) continue;

    // make explosion scene node
    BillboardSceneNode* explosion = new BillboardSceneNode(zero);
    explosion->setTexture(tex);
    explosion->setTextureAnimation(8, 8);
    explosion->setLight();
    explosion->setLightColor(1.0f, 0.8f, 0.5f);
    explosion->setLightAttenuation(0.04f, 0.0f, 0.01f);

    // add it to list of prototype explosions
    prototypeExplosions.push_back(explosion);
  }

  // get tank textures
  {
    static const char* tankFilename = "flage";
    int width, height;
    tankTexture = new OpenGLTexture;
    *tankTexture = getTexture(tankFilename, &width, &height,
					OpenGLTexture::LinearMipmapLinear);
  }

  // let other stuff do initialization
  sceneBuilder = new SceneDatabaseBuilder(sceneRenderer);
  World::init();
  ShotStrategy::init();

  // prepare dialogs
  mainMenu = new MainMenu;

  // initialize startup info with stuff provided from command line
  startupInfo = *_info;

  // normal error callback (doesn't force a redraw)
  setErrorCallback(defaultErrorCallback);

  std::string tmpString;

  // print version
  {
    char bombMessage[80];
    sprintf(bombMessage, "BZFlag version %s", VERSION);
    controlPanel->addMessage("");
    tmpString = ColorStrings[RedColor];
    tmpString += (const char *) bombMessage;
    controlPanel->addMessage(tmpString);
  }

  // print expiration
  if (timeBombString()) {
    // add message about date of expiration
    char bombMessage[80];
    sprintf(bombMessage, "This release will expire on %s", timeBombString());
    controlPanel->addMessage(bombMessage);
  }

  tmpString = ColorStrings[RogueColor];
  tmpString += copyright;
  controlPanel->addMessage(tmpString);
  // print author
  tmpString = ColorStrings[GreenColor];
  tmpString += "Author: Chris Schoeneman <crs23@bigfoot.com>";
  controlPanel->addMessage(tmpString);
  // print maintainer
  tmpString = ColorStrings[BlueColor];
  tmpString += "Maintainer: Tim Riker <Tim@Rikers.org>";
  controlPanel->addMessage(tmpString);
  // print GL renderer
  tmpString = ColorStrings[PurpleColor];
  tmpString += (const char*)glGetString(GL_RENDERER);
  controlPanel->addMessage(tmpString);

  //inform user of silencePlayers on startup
  for (unsigned int j = 0; j < silencePlayers.size(); j ++){
    std::string aString = silencePlayers[j];
    aString += " Silenced";
    if (silencePlayers[j] == "*") {
      aString = "Silenced All Msgs";
    }
    controlPanel->addMessage(aString);
  }

  // enter game if we have all the info we need, otherwise
  // pop up main menu
  if (startupInfo.autoConnect &&
	startupInfo.callsign[0] && startupInfo.serverName[0]) {
    joinGameCallback = &joinGameHandler;
    controlPanel->addMessage("Trying...");
  }
  else {
    HUDDialogStack::get()->push(mainMenu);
  }

  // start game loop
  playingLoop();

  // clean up
  delete tankTexture;
  for (unsigned int ext = 0; ext < prototypeExplosions.size(); ext++)
    delete prototypeExplosions[ext];
  prototypeExplosions.clear();
  *_info = startupInfo;
  leaveGame();
  setErrorCallback(NULL);
  while (HUDDialogStack::get()->isActive())
    HUDDialogStack::get()->pop();
  delete mainMenu;
  delete sceneBuilder;
  sceneRenderer->setBackground(NULL);
  sceneRenderer->setSceneDatabase(NULL);
  delete zScene;
  delete bspScene;
  ShotStrategy::done();
  World::done();
  zScene = NULL;
  bspScene = NULL;
  mainWindow = NULL;
  sceneRenderer = NULL;
  display = NULL;
}
// ex: shiftwidth=2 tabstop=8
