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

static const char copyright[] = "Copyright (c) 1993 - 2001 Tim Riker";

#include <stdio.h>
#include <stdlib.h>
#include "bzsignal.h"
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

// yikes! that's a lotsa includes!
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

#include "AList.h"
BZF_DEFINE_ALIST(ExplosionList, BillboardSceneNode*);

static const float	FlagHelpDuration = 60.0f;

static StartupInfo	startupInfo;
static BzfKeyMap		keymap;
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
static ControlPanel*	controlPanel = NULL;
static RadarRenderer*	radar = NULL;
static HUDRenderer*	hud = NULL;
static SceneDatabaseBuilder* sceneBuilder = NULL;
static Team*		teams = NULL;
static int		maxPlayers = 0;		// not including me
static RemotePlayer**	player = NULL;
static int		numFlags = 0;
static JoinGameCallback	joinGameCallback = NULL;
static void*		joinGameUserData = NULL;
static boolean		serverError = False;
static boolean		serverDied = False;
static boolean		fireButton = False;
static boolean		restartOnBase = False;
static boolean		firstLife = False;
static boolean		showFPS = False;
static boolean		showDrawTime = False;
static boolean		pausedByUnmap = False;
static boolean		unmapped = False;
static int		preUnmapFormat = -1;
static double		epochOffset;
static double		lastEpochOffset;
static float		clockAdjust = 0.0f;
static float		pauseCountdown = 0.0f;
//static float		maxPauseCountdown = 0.0f;
static float		testVideoFormatTimer = 0.0f;
static int		testVideoPrevFormat = -1;
static PlayingCallbackList	playingCallbacks;
boolean			gameOver = False;
static OpenGLTexture*	tankTexture = NULL;
static ExplosionList	explosions;
static ExplosionList	prototypeExplosions;
static int		savedVolume = -1;
static boolean		grabMouseAlways = False;

static char		messageMessage[PlayerIdPLen + 2 + MessageLen];

static void		restartPlaying();
static void		setTarget();
static void		handleFlagDropped(Player* tank);
static void		handlePlayerMessage(uint16_t, uint16_t, void*);
extern void		dumpResources(BzfDisplay*, SceneRenderer&);

enum BlowedUpReason {
			GotKilledMsg,
			GotShot,
			GotRunOver,
			GotCaptured,
			GenocideEffect
};
static const char*	blowedUpMessage[] = {
			  NULL,
			  "Got hit by shot",
			  "Got run over by Steamroller",
			  "Team flag was captured",
			  "Teammate hit by Genocide"
			};
static boolean		gotBlowedUp(BaseLocalPlayer* tank,
					BlowedUpReason reason,
					const PlayerId& killer,
					int shotId = -1);

#ifdef ROBOT
static void		handleMyTankKilled();
static ServerLink*	robotServer[20];
static RobotPlayer*	robots[20];
static int		numRobots = 0;
#endif

extern struct tm	userTime;
static double		userTimeEpochOffset;

StartupInfo::StartupInfo() : hasConfiguration(False),
				autoConnect(False),
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
  joystick = False;
}

//
// should we grab the mouse?
//

static void		setGrabMouse(boolean grab)
{
  grabMouseAlways = grab;
}

static boolean		shouldGrabMouse()
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
  if (sceneRenderer->useZBuffer()) {
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

boolean			setVideoFormat(int index, boolean test)
{
#if defined(_WIN32)
  // give windows extra time to test format (context reloading takes a while)
  static const float testDuration = 10.0f;
#else
  static const float testDuration = 5.0f;
#endif

  // ignore bad formats or when the format test timer is running
  if (testVideoFormatTimer != 0.0f || !display->isValidResolution(index))
    return False;

  // ignore if no change
  if (display->getResolution() == index) return True;

  // change it
  testVideoPrevFormat = display->getResolution();
  if (!display->setResolution(index)) return False;

  // handle resize
  mainWindow->setFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  mainWindow->warpMouse();
  if (test) testVideoFormatTimer = testDuration;
  else if (shouldGrabMouse()) mainWindow->grabMouse();
  return True;
}

void			addPlayingCallback(PlayingCallback cb, void* data)
{
  PlayingCallbackItem item;
  item.cb = cb;
  item.data = data;
  playingCallbacks.append(item);
}

void			removePlayingCallback(PlayingCallback _cb, void* data)
{
  const int count = playingCallbacks.getLength();
  for (int i = 0; i < count; i++) {
    const PlayingCallbackItem& cb = playingCallbacks[i];
    if (cb.cb == _cb && cb.data == data) {
      playingCallbacks.remove(i);
      break;
    }
  }
}

static void		callPlayingCallbacks()
{
  const int count = playingCallbacks.getLength();
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

void			joinGameHandler(boolean okay, void*)
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
  if (mainWindow) mainWindow->setQuit();
}

//
// handle signals that should disconnect me from the server
//

static void		hangup(int sig)
{
  bzSignal(sig, SIG_PF(hangup));
  serverDied = True;
  serverError = True;
}

//
// ui control default key handler classes
//

class ComposeDefaultKey : public HUDuiDefaultKey {
  public:
    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);
};

boolean			ComposeDefaultKey::keyPress(const BzfKeyEvent& key)
{
  boolean sendIt;
  switch (key.ascii) {
    case 3:	// ^C
    case 27:	// escape
//    case 127:	// delete
      sendIt = False;			// finished composing -- don't send
      break;

    case 4:	// ^D
    case 13:	// return
      sendIt = True;
      break;

    default:
      return False;
  }

  if (sendIt) {
    BzfString message = hud->getComposeString();
    if (message.getLength() > 0) {
      char messageBuffer[MessageLen];
      memset(messageBuffer, 0, MessageLen);
      strncpy(messageBuffer, message, MessageLen);
      nboPackString(messageMessage + PlayerIdPLen + 2,
					messageBuffer, MessageLen);
      serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
    }
  }

  hud->setComposing(NULL);
  HUDui::setDefaultKey(NULL);
  return True;
}

boolean			ComposeDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  return keyPress(key);
}

//
// user input handling
//

#if defined(DEBUG)
#define FREEZING
#define ROAMING
#define SNAPPING
#endif
#if defined(FREEZING)
static boolean		motionFreeze = False;
#endif
#if defined(ROAMING)
static boolean		roaming = False, roamTrack = False;
static float		roamPos[3] = { 0.0f, 0.0f, MuzzleHeight }, roamDPos[3];
static float		roamTheta = 0.0f, roamDTheta;
static float		roamPhi = 0.0f, roamDPhi;
static float		roamZoom = 60.0f, roamDZoom;
#endif

static void		doMotion()
{
#if defined(FREEZING)
  if (motionFreeze) return;
#endif

  // get mouse position
  int mx, my;
  if (mainWindow->joystick())
    mainWindow->getJoyPosition(mx, my);
  else
    mainWindow->getMousePosition(mx, my);

  // calculate desired rotation
  const int noMotionSize = hud->getNoMotionSize();
  const int maxMotionSize = hud->getMaxMotionSize();
  float rotation = 0.0f;
  if (mx < -noMotionSize) {
    rotation = float(-mx - noMotionSize) / float(maxMotionSize);
    if (rotation > 1.0f) rotation = 1.0f;
  }
  else if (mx > noMotionSize) {
    rotation = -float(mx - noMotionSize) / float(maxMotionSize);
    if (rotation < -1.0f) rotation = -1.0f;
  }
  myTank->setDesiredAngVel(rotation);

  // calculate desired speed
  float speed = 0.0f;
  if (my < -noMotionSize) {
    speed = float(-my - noMotionSize) / float(maxMotionSize);
    if (speed > 1.0f) speed = 1.0f;
  }
  else if (my > noMotionSize) {
    speed = -float(my - noMotionSize) / float(maxMotionSize);
    if (speed < -0.5f) speed = -0.5f;
  }
  myTank->setDesiredSpeed(speed);
}

static boolean		doKeyCommon(const BzfKeyEvent& key, boolean pressed)
{
  if (keymap.isMappedTo(BzfKeyMap::TimeForward, key)) {
    // plus five minutes
    if (pressed) clockAdjust += 5.0f * 60.0f;
    return True;
  }

  else if (keymap.isMappedTo(BzfKeyMap::TimeBackward, key)) {
    // minus five minutes
    if (pressed) clockAdjust -= 5.0f * 60.0f;
    return True;
  }

  else if (key.ascii == 27) {
    if (pressed) HUDDialogStack::get()->push(mainMenu);
    return True;
  }

  else if (keymap.isMappedTo(BzfKeyMap::Quit, key)) {
    getMainWindow()->setQuit();
    return True;
  }

  else {
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
	  return True;
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
	  return True;
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
	  return True;
	}
	break;

      case '[':
      case '{':
	// minus 30 seconds
	if (keymap.isMapped(key.ascii) == BzfKeyMap::LastKey) {
	  if (pressed) clockAdjust -= 30.0f;
	  return True;
	}
	break;
    }
  }

  return False;
}

static void		doKeyNotPlaying(const BzfKeyEvent& key, boolean pressed)
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

static void		doKeyPlaying(const BzfKeyEvent& key, boolean pressed)
{
  static ComposeDefaultKey composeKeyHandler;

  if (HUDui::getFocus())
    if ((pressed && HUDui::keyPress(key)) ||
	(!pressed && HUDui::keyRelease(key)))
      return;

  if (doKeyCommon(key, pressed)) return;

#if defined(SNAPPING)
  static int snap = 0;
  if (key.button == BzfKeyEvent::F11 && pressed) {
    // snapshot
    char filename[80];
    sprintf(filename, "bzfi%04d.raw", snap++);
    FILE* f = fopen(filename, "w");
    if (f) {
      int w = mainWindow->getWidth();
      int h = mainWindow->getHeight();
      unsigned char* b = (unsigned char*)malloc(w * h * 3);
      glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, b + 0 * w * h);
      glReadPixels(0, 0, w, h, GL_GREEN, GL_UNSIGNED_BYTE, b + 1 * w * h);
      glReadPixels(0, 0, w, h, GL_BLUE, GL_UNSIGNED_BYTE, b + 2 * w * h);
      fwrite(b, 3 * w * h, 1, f);
      free(b);
      fclose(f);
      fprintf(stderr, "%s: %dx%d\n", filename, w, h);
    }
  }
#endif
#if defined(FREEZING)
  if (key.ascii == '`' && pressed) {
    // toggle motion freeze
    motionFreeze = !motionFreeze;
  }
#endif
#if defined(ROAMING)
  if (key.ascii == '~' && pressed) {
    // toggle roaming
    roaming = !roaming;
  }
  else if (roaming && pressed) {
    switch (key.button) {
      case BzfKeyEvent::Left:
	roamDTheta = 60.0f * (roamZoom / 90.0f);
	break;

      case BzfKeyEvent::Right:
	roamDTheta = -60.0f * (roamZoom / 90.0f);
	break;

      case BzfKeyEvent::Up:
	roamDPhi = 60.0f * (roamZoom / 90.0f);
	break;

      case BzfKeyEvent::Down:
	roamDPhi = -60.0f * (roamZoom / 90.0f);
	break;

      case BzfKeyEvent::End:
	roamDPos[0] = -4.0f * TankSpeed;
	break;

      case BzfKeyEvent::Home:
	roamDPos[0] =  4.0f * TankSpeed;
	break;

      case BzfKeyEvent::Delete:
	roamDPos[1] =  4.0f * TankSpeed;
	break;

      case BzfKeyEvent::PageDown:
	roamDPos[1] = -4.0f * TankSpeed;
	break;

      case BzfKeyEvent::Insert:
	roamDPos[2] = -4.0f * TankSpeed;
	break;

      case BzfKeyEvent::PageUp:
	roamDPos[2] =  4.0f * TankSpeed;
	break;

      case BzfKeyEvent::F9:
	roamDZoom =  30.0;
	break;

      case BzfKeyEvent::F10:
	roamDZoom = -30.0;
	break;

      case BzfKeyEvent::F8:
	roamTrack = !roamTrack;
	break;
    }
  }
#endif

  if (keymap.isMappedTo(BzfKeyMap::FireShot, key)) {
    fireButton = pressed;
    if (pressed && myTank->isAlive())
      myTank->fireShot();
  }

  else if (keymap.isMappedTo(BzfKeyMap::DropFlag, key)) {
    if (pressed) {
      FlagId flagId = myTank->getFlag();
      if (flagId != NoFlag && !myTank->isPaused() &&
	  Flag::getType(flagId) != FlagSticky &&
	  !(flagId == PhantomZoneFlag && myTank->isFlagActive()) &&
	  !(flagId == OscOverthrusterFlag &&
	  myTank->getLocation() == LocalPlayer::InBuilding)) {
	serverLink->sendDropFlag(myTank->getPosition());
		// changed: on windows it may happen the MsgDropFlag
		// never comes back to us, so we drop it right away
	    	handleFlagDropped(myTank);
	  }
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::Identify, key)) {
    if (pressed && !gameOver && !myTank->isAlive() && !myTank->isExploding()) {
      restartPlaying();
    }

    else {
      // set target (for guided missile lock on) or get target info
      if (pressed && myTank->isAlive() && !myTank->isPaused())
	setTarget();
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::Jump, key)) {
    if (pressed) myTank->jump();
  }

  else if (keymap.isMappedTo(BzfKeyMap::Binoculars, key)) {
    if (pressed) {
      if (myTank->getFlag() != WideAngleFlag)
	myTank->setMagnify(1 - myTank->getMagnify());
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::SendAll, key) ||
	   keymap.isMappedTo(BzfKeyMap::SendTeam, key)) {
    // start composing a message
    if (pressed) {
      const char* composePrompt;
      if (keymap.isMappedTo(BzfKeyMap::SendAll, key)) {
	void* buf = messageMessage;
	buf = nboPackUInt(buf, 0);
	buf = nboPackShort(buf, 0);
	buf = nboPackShort(buf, 0);
	buf = nboPackUShort(buf, uint16_t(RogueTeam));
	composePrompt = "Send to all: ";
      }
      else {
	void* buf = messageMessage;
	buf = nboPackUInt(buf, 0);
	buf = nboPackShort(buf, 0);
	buf = nboPackShort(buf, 0);
	buf = nboPackUShort(buf, uint16_t(myTank->getTeam()));
	composePrompt = "Send to teammates: ";
      }
	
      // to send to a player use:
      //   buf = myTank->getId().pack(buf);
      //   buf = nboPackUShort(buf, uint16_t(RogueTeam));
      hud->setComposing(composePrompt);
      HUDui::setDefaultKey(&composeKeyHandler);
    }
  }
  else if (keymap.isMappedTo(BzfKeyMap::ScrollBackward, key)) {
    // scroll message list backward
    if (pressed) {
      controlPanel->setMessagesOffset(1,1);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ScrollForward, key)) {
    // scroll message list forward
    if (pressed) {
      controlPanel->setMessagesOffset(-1,1);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::Score, key)) {
    // toggle score board
    if (pressed) {
      sceneRenderer->setScore(!sceneRenderer->getScore());
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::FlagHelp, key)) {
    // toggle flag help
    if (pressed) {
      sceneRenderer->setShowFlagHelp(!sceneRenderer->getShowFlagHelp());
      if (!sceneRenderer->getShowFlagHelp()) hud->setFlagHelp(NoFlag, 0.0);
      else hud->setFlagHelp(myTank->getFlag(), FlagHelpDuration);
    }
  }

  else if (keymap.isMappedTo(BzfKeyMap::ShortRange, key)) {
    // smallest radar range
    if (pressed) radar->setRange(RadarLowRange);
  }

  else if (keymap.isMappedTo(BzfKeyMap::MediumRange, key)) {
    // medium radar range
    if (pressed) radar->setRange(RadarMedRange);
  }

  else if (keymap.isMappedTo(BzfKeyMap::LongRange, key)) {
    // largest radar range
    if (pressed) radar->setRange(RadarHiRange);
  }

  else if (keymap.isMappedTo(BzfKeyMap::Pause, key)) {
    // pause/resume
    if (pressed && !pausedByUnmap) {
      if (myTank->isAlive()) {
	if (myTank->isPaused()) {
	  myTank->setPause(False);
	  controlPanel->addMessage("Resumed");

	  // restore the sound
	  if (savedVolume != -1) {
	    setSoundVolume(savedVolume);
	    savedVolume = -1;
	  }

	  // grab mouse
	  if (shouldGrabMouse())
	    mainWindow->grabMouse();
	}
	else if (pauseCountdown > 0.0f) {
	  pauseCountdown = 0.0f;
	  hud->setAlert(1, "Pause cancelled", 1.5f, True);
	}
	else {
	  pauseCountdown = 3.0f;
	  char msgBuf[40];
	  sprintf(msgBuf, "Pausing in %d", (int)(pauseCountdown + 0.99f));
	  hud->setAlert(1, msgBuf, 1.0f, False);
	}
      }
    }
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
      buf = nboPackUInt(buf, 0);
      buf = nboPackShort(buf, 0);
      buf = nboPackShort(buf, 0);
      if (key.shift == BzfKeyEvent::ControlKey) {
        sprintf(name, "quickTeamMessage%d", msgno);
        buf = nboPackUShort(buf, uint16_t(myTank->getTeam()));
      } else {
        sprintf(name, "quickMessage%d", msgno);
        buf = nboPackUShort(buf, uint16_t(RogueTeam));
      }
      if (resources->hasValue(name)) {
        char messageBuffer[MessageLen];
        memset(messageBuffer, 0, MessageLen);
        strncpy(messageBuffer, 
                resources->getValue(name).getString(),
                MessageLen);
        nboPackString(messageMessage + PlayerIdPLen + 2,
                      messageBuffer, MessageLen);
        serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
      }
    }
  }
}

static void		doEvent(BzfDisplay* display)
{
  BzfEvent event;
  if (!display->getEvent(event)) return;

  switch (event.type) {
    case BzfEvent::Quit:
      mainWindow->setQuit();
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
	pausedByUnmap = False;
	pauseCountdown = 0.0f;
	if (myTank && myTank->isAlive() && myTank->isPaused()) {
	  myTank->setPause(False);
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

      unmapped = False;
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
	pauseCountdown = 3.0f;

	// set this even though we haven't really paused yet
	pausedByUnmap = True;
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

      unmapped = True;
      mainWindow->ungrabMouse();
      break;

    case BzfEvent::KeyUp:
      if (!myTank)
	doKeyNotPlaying(event.keyDown, False);
      else
	doKeyPlaying(event.keyDown, False);
      break;

    case BzfEvent::KeyDown:
      if (!myTank)
	doKeyNotPlaying(event.keyUp, True);
      else
	doKeyPlaying(event.keyUp, True);
      break;

    case BzfEvent::MouseMove:
      break;
  }
}

//
// misc utility routines
//

Player*			lookupPlayer(const PlayerId& id)
{
  // check my tank first
  if (myTank->getId() == id)
    return myTank;

  // check other players
  for (int i = 0; i < maxPlayers; i++)
    if (player[i] && player[i]->getId() == id)
      return player[i];

  // it's nobody we know about
  return NULL;
}

static int		lookupPlayerIndex(const PlayerId& id)
{
  // check my tank first
  if (myTank->getId() == id)
    return -2;

  // check other players
  for (int i = 0; i < maxPlayers; i++)
    if (player[i] && player[i]->getId() == id)
      return i;

  // it's nobody we know about
  return -1;
}

static Player*		getPlayerByIndex(int index)
{
  if (index == -2)
    return myTank;
  if (index == -1)
    return NULL;
  return player[index];
}

static BaseLocalPlayer*	getLocalPlayer(const PlayerId& id)
{
  if (myTank->getId() == id) return myTank;
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->getId() == id)
      return robots[i];
#endif
  return NULL;
}

static ServerLink*	lookupServer(const Player* player)
{
  const PlayerId& id = player->getId();
  if (myTank->getId() == id) return serverLink;
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->getId() == id)
      return robotServer[i];
#endif
  return NULL;
}

static void		addMessage(const Player* player,
				const BzfString& msg,
				const GLfloat* color = NULL)
{
  BzfString fullMessage;
  if (player) {
    fullMessage += player->getCallSign();
#ifndef BWSUPPORT
    if (color == NULL) {
#endif
      fullMessage += " (";
      fullMessage += Team::getName(player->getTeam());
      fullMessage += ")";
#ifndef BWSUPPORT
    }
#endif
    fullMessage += ": ";
  }
  fullMessage += msg;
#if 0
  if (player && color == NULL)
    color = Team::getRadarColor(player->getTeam());
#endif
  controlPanel->addMessage(fullMessage, color);
}

static void		updateNumPlayers()
{
  int i, numPlayers[NumTeams];
  for (i = 0; i < NumTeams; i++)
    numPlayers[i] = 0;
  for (i = 0; i < maxPlayers; i++)
    if (player[i])
      numPlayers[player[i]->getTeam()]++;
  if (myTank)
    numPlayers[myTank->getTeam()]++;
  controlPanel->setTeamCounts(numPlayers);
}

static void		updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
  boolean anyPlayers = False;
  int i;
  for (i = 0; i < maxPlayers; i++)
    if (player[i]) {
      anyPlayers = True;
      break;
    }
#ifdef ROBOT
  if (!anyPlayers) {
    for (i = 0; i < numRobots; i++)
      if (robots[i]) {
	anyPlayers = True;
	break;
      }
  }
#endif
  if (!anyPlayers) {
    hud->setPlayerHasHighScore(False);
    hud->setTeamHasHighScore(False);
    return;
  }

  boolean haveBest = True;
  int bestScore = myTank ? myTank->getScore() : 0;
  for (i = 0; i < maxPlayers; i++)
    if (player[i] && player[i]->getScore() >= bestScore) {
      haveBest = False;
      break;
    }
#ifdef ROBOT
  if (haveBest) {
    for (i = 0; i < numRobots; i++)
      if (robots[i] && robots[i]->getScore() >= bestScore) {
	haveBest = False;
	break;
      }
  }
#endif
  hud->setPlayerHasHighScore(haveBest);

  if (myTank && myTank->getTeam() != RogueTeam) {
    const Team& myTeam = World::getWorld()->getTeam(int(myTank->getTeam()));
    bestScore = myTeam.won - myTeam.lost;
    haveBest = True;
    for (i = 0; i < NumTeams; i++) {
      if (i == int(myTank->getTeam())) continue;
      const Team& team = World::getWorld()->getTeam(i);
      if (team.activeSize > 0 && team.won - team.lost >= bestScore) {
	haveBest = False;
	break;
      }
    }
    hud->setTeamHasHighScore(haveBest);
  }
  else {
    hud->setTeamHasHighScore(False);
  }
}

static void		updateFlag(FlagId id)
{
  if (id == NoFlag) {
    hud->setColor(1.0f, 0.625f, 0.125f);
    hud->setAlert(2, NULL, 0.0f);
  }
  else {
    const float* color = Flag::getColor(id);
    hud->setColor(color[0], color[1], color[2]);
    hud->setAlert(2, Flag::getName(id), 3.0f, Flag::getType(id) == FlagSticky);
  }

  if (sceneRenderer->getShowFlagHelp())
    hud->setFlagHelp(id, FlagHelpDuration);

  if (!radar && !myTank || !World::getWorld()) return;

  radar->setJammed(id == JammingFlag);
  hud->setAltitudeTape(id == JumpingFlag || World::getWorld()->allowJumping());

  // enable/disable display of markers
  hud->setMarker(0, myTank->getTeam() != RogueTeam &&
			int(id) != int(myTank->getTeam()) &&
			World::getWorld()->allowTeamFlags());
  hud->setMarker(1, World::getWorld()->allowAntidote() &&
			id != NoFlag && Flag::getType(id) == FlagSticky);
}

void			notifyBzfKeyMapChanged()
{
  hud->setRestartKeyLabel(BzfKeyMap::getKeyEventString(
					keymap.get(BzfKeyMap::Identify)));
}

//
// server message handling
//

static Player*		addPlayer(const PlayerId& id, void* msg,
							int showMessage)
{
  uint16_t team, type, wins, losses;
  char callsign[CallSignLen];
  char email[EmailLen];
  msg = nboUnpackUShort(msg, type);
  msg = nboUnpackUShort(msg, team);
  msg = nboUnpackUShort(msg, wins);
  msg = nboUnpackUShort(msg, losses);
  msg = nboUnpackString(msg, callsign, CallSignLen);
  msg = nboUnpackString(msg, email, EmailLen);

  // find empty player slot
  int i;
  for (i = 0; i < maxPlayers; i++)
    if (!player[i])
      break;
  if (i == maxPlayers) {
    // if this happens the server has screwed up
    printError("Server error when adding player");
    serverError = True;
    return NULL;
  }

  // add player
  if (PlayerType(type) == TankPlayer || PlayerType(type) == ComputerPlayer) {
    player[i] = new RemotePlayer(id, TeamColor(team), callsign, email);
    player[i]->changeScore(short(wins), short(losses));
  }

  if (showMessage) {
    BzfString message("joining as a");
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
      BzfString name(callsign);
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

static void		handleServerMessage(boolean human, uint16_t code,
						uint16_t, void* msg)
{
  boolean checkScores = False;
  switch (code) {

    case MsgUDPLinkRequest:
      uint16_t portNo;
      msg = nboUnpackUShort(msg, portNo);
	  printError("Server sent downlink endpoint information, port %d",portNo);
	  playerLink->setPortForUPD(portNo);
      break;
 
    case MsgSuperKill:
      printError("Server forced a disconnect");
      serverError = True;
      break;

    case MsgTimeUpdate: {
      uint16_t timeLeft;
      msg = nboUnpackUShort(msg, timeLeft);
      hud->setTimeLeft(timeLeft);
      if (timeLeft == 0) {
	gameOver = True;
	myTank->explodeTank();
	controlPanel->addMessage("Time Expired");
	hud->setAlert(0, "Time Expired", 10.0f, True);
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
      msg = id.unpack(msg);
      msg = nboUnpackUShort(msg, team);
      Player* player = lookupPlayer(id);

      // make a message
      BzfString msg;
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

      gameOver = True;
      hud->setTimeLeft(-1);
      myTank->explodeTank();
      controlPanel->addMessage(msg);
      hud->setAlert(0, msg, 10.0f, True);
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
      msg = id.unpack(msg);
      if (id == myTank->getId()) break;		// that's odd -- it's me!
      addPlayer(id, msg, True);
      updateNumPlayers();
      checkScores = True;
      break;
    }

    case MsgRemovePlayer: {
      PlayerId id;
      msg = id.unpack(msg);
      int playerIndex = lookupPlayerIndex(id);
      if (playerIndex >= 0) {
	addMessage(player[playerIndex], "signing off");
	world->addDeadPlayer(player[playerIndex]);
	delete player[playerIndex];
	player[playerIndex] = NULL;
	updateNumPlayers();
	checkScores = True;
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
      checkScores = True;
      break;
    }

    case MsgAlive: {
      PlayerId id;
      float pos[3], forward[3];
      msg = id.unpack(msg);
      msg = nboUnpackFloat(msg, pos[0]);
      msg = nboUnpackFloat(msg, pos[1]);
      msg = nboUnpackFloat(msg, pos[2]);
      msg = nboUnpackFloat(msg, forward[0]);
      msg = nboUnpackFloat(msg, forward[1]);
      msg = nboUnpackFloat(msg, forward[2]);
      int playerIndex = lookupPlayerIndex(id);
      if (playerIndex >= 0) {
	static const float zero[3] = { 0.0f, 0.0f, 0.0f };
	Player* tank = getPlayerByIndex(playerIndex);
	tank->setStatus(Player::Alive);
	tank->move(pos, atan2f(forward[1], forward[0]));
	tank->setVelocity(zero);
	tank->setAngularVelocity(0.0f);
	tank->setDeadReckoning();
	playWorldSound(SFX_POP, pos[0], pos[1], pos[2], True);
      }
      break;
    }

    case MsgKilled: {
      PlayerId victim, killer;
      int16_t shotId;
      msg = victim.unpack(msg);
      msg = killer.unpack(msg);
      msg = nboUnpackShort(msg, shotId);
      int victimIndex = lookupPlayerIndex(victim);
      int killerIndex = lookupPlayerIndex(killer);
      BaseLocalPlayer* victimLocal = getLocalPlayer(victim);
      BaseLocalPlayer* killerLocal = getLocalPlayer(killer);
      Player* victimPlayer = getPlayerByIndex(victimIndex);
      Player* killerPlayer = getPlayerByIndex(killerIndex);
#ifdef ROBOT
      if (victimPlayer == myTank) {
	// uh oh, i'm dead
	if (myTank->isAlive()) {
	  serverLink->sendDropFlag(myTank->getPosition());
	  handleMyTankKilled();
	}
      }
#endif
      if (victimLocal) {
	// uh oh, local player is dead
	if (victimLocal->isAlive())
	  gotBlowedUp(victimLocal, GotKilledMsg, killer);
      }
      else if (victimPlayer) {
	victimPlayer->setExplode(TimeKeeper::getTick());
	const float* pos = victimPlayer->getPosition();
	playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2],
						killerLocal == myTank);
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
	  killerLocal->endShot(shotId, True);
	}
	if (killerLocal != victimPlayer) {
	  if (victimPlayer->getTeam() == killerLocal->getTeam() &&
	      killerLocal->getTeam() != RogueTeam) {
	    if (killerLocal == myTank)
	      hud->setAlert(1, "Don't shoot teammates!!!", 3.0f, True);
	    // teammate
	    killerLocal->changeScore(0, 1);
	  }
	  else
	    // enemy
	    killerLocal->changeScore(1, 0);
	}
      }
      // handle my personal score against other players
      if ((killerPlayer == myTank || victimPlayer == myTank) &&
	 !(killerPlayer == myTank && victimPlayer == myTank)) {
	if (killerLocal == myTank) {
	  victimPlayer->changeLocalScore(1, 0);
	}
	else {
	  killerPlayer->changeLocalScore(0, 1);
	}
      }

      // add message
      if (human && victimPlayer) {
	if (killerPlayer == victimPlayer)
	  addMessage(victimPlayer, "blew myself up");
	else if (!killerPlayer)
	  addMessage(victimPlayer, "destroyed by <unknown>");
	else if (killerPlayer->getTeam() == victimPlayer->getTeam() &&
					killerPlayer->getTeam() != RogueTeam) {
	  BzfString message("destroyed by teammate ");
	  message += killerPlayer->getCallSign();
	  addMessage(victimPlayer, message);
	}
	else {
	  BzfString message("destroyed by ");
	  message += killerPlayer->getCallSign();
	  addMessage(victimPlayer, message);
	}
      }

      // blow up if killer has genocide flag and i'm on same team as victim
      // (and we're not rogues)
      if (human && killerPlayer && victimPlayer && victimPlayer != myTank &&
		victimPlayer->getTeam() == myTank->getTeam() &&
		myTank->getTeam() != RogueTeam && shotId >= 0) {
	// now see if shot was fired with a GenocideFlag
	const ShotPath* shot = killerPlayer->getShot(int(shotId));
	if (shot && shot->getFlag() == GenocideFlag) {
	  gotBlowedUp(myTank, GenocideEffect, killerPlayer->getId());
	}
      }

#ifdef ROBOT    
      // blow up robots on victim's team if shot was genocide
      if (killerPlayer && victimPlayer && shotId >= 0) {
	const ShotPath* shot = killerPlayer->getShot(int(shotId));
	if (shot && shot->getFlag() == GenocideFlag)
	  for (int i = 0; i < numRobots; i++)
	    if (victimPlayer != robots[i] &&
		victimPlayer->getTeam() == robots[i]->getTeam() &&
		robots[i]->getTeam() != RogueTeam)
	      gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
      }
#endif

      checkScores = True;
      break;
    }

    case MsgGrabFlag: {
// ROBOT -- FIXME -- robots don't grab flag at the moment
      PlayerId id;
      uint16_t flagIndex;
      msg = id.unpack(msg);
      msg = nboUnpackUShort(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      Player* tank = lookupPlayer(id);
      if (!tank) break;

      // player now has flag
      tank->setFlag(world->getFlag(flagIndex).id);
      if (tank == myTank) {
	// not allowed to grab it if not on the ground
	if (myTank->getLocation() != LocalPlayer::OnGround) {
	  serverLink->sendDropFlag(myTank->getPosition());
	}
	else {
	  // grabbed flag
	  playLocalSound(Flag::getType(myTank->getFlag()) != FlagSticky ?
						SFX_GRAB_FLAG : SFX_GRAB_BAD);
	  updateFlag(myTank->getFlag());
	}
      }
      else if (tank && tank->getTeam() != myTank->getTeam() &&
		int(world->getFlag(flagIndex).id) == int(myTank->getTeam())) {
	hud->setAlert(1, "Flag Alert!!!", 3.0f, True);
	playLocalSound(SFX_ALERT);
      }
      if (tank) {
	BzfString message("grabbed ");
	message += Flag::getName(tank->getFlag());
	message += " flag";
	addMessage(tank, message);
      }
      break;
    }

    case MsgDropFlag: {
      PlayerId id;
      uint16_t flagIndex;
      msg = id.unpack(msg);
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
      msg = id.unpack(msg);
      msg = nboUnpackUShort(msg, flagIndex);
      msg = nboUnpackUShort(msg, team);
      Player* capturer = lookupPlayer(id);
      int capturedTeam = int(world->getFlag(int(flagIndex)).id);

      // player no longer has flag
      if (capturer) {
	capturer->setFlag(NoFlag);
	if (capturer == myTank) {
	  updateFlag(NoFlag);
	}

	// add message
	if (int(capturer->getTeam()) == capturedTeam) {
	  BzfString message("took my flag into ");
	  message += Team::getName(TeamColor(team));
	  message += " territory";
	  addMessage(capturer, message);
	}
	else {
	  BzfString message("captured ");
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
	restartOnBase = True;
      }

      // everybody who's alive on capture team will be blowing up
      // but we're not going to get an individual notification for
      // each of them, so add an explosion for each now.  don't
      // include me, though;  I already blew myself up.
      for (int i = 0; i < maxPlayers; i++) {
	if (player[i] &&
	    player[i]->isAlive() &&
	    player[i]->getTeam() == capturedTeam) {
	  const float* pos = player[i]->getPosition();
	  playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2], False);
	  float explodePos[3];
	  explodePos[0] = pos[0];
	  explodePos[1] = pos[1];
	  explodePos[2] = pos[2] + MuzzleHeight;
	  addTankExplosion(explodePos);
	}
      }

      checkScores = True;
      break;
    }

    case MsgShotBegin: {
      FiringInfo firingInfo;
      msg = firingInfo.unpack(msg);
      for (int i = 0; i < maxPlayers; i++)
	if (player[i] && player[i]->getId() == firingInfo.shot.player) {
	  const float* pos = firingInfo.shot.pos;
	  player[i]->addShot(firingInfo);
	  if (human) {
	    if (firingInfo.flag == ShockWaveFlag)
	      playWorldSound(SFX_SHOCK, pos[0], pos[1], pos[2]);
	    else if (firingInfo.flag == LaserFlag)
	      playWorldSound(SFX_LASER, pos[0], pos[1], pos[2]);
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
      msg = id.unpack(msg);
      msg = nboUnpackShort(msg, shotId);
      msg = nboUnpackUShort(msg, reason);
      BaseLocalPlayer* localPlayer = getLocalPlayer(id);

      if (localPlayer)
	localPlayer->endShot(int(shotId), False, reason == 0);
      else for (int i = 0; i < maxPlayers; i++)
	if (player[i] && player[i]->getId() == id) {
	  player[i]->endShot(int(shotId), False, reason == 0);
	  break;
	}
      break;
    }

    case MsgScore: {
      PlayerId id;
      uint16_t wins, losses;
      msg = id.unpack(msg);
      msg = nboUnpackUShort(msg, wins);
      msg = nboUnpackUShort(msg, losses);
      // only update score of remote players (local score is already known)
      for (int i = 0; i < maxPlayers; i++)
	if (player[i] && player[i]->getId() == id) {
	  player[i]->changeScore(wins - player[i]->getWins(),
				losses - player[i]->getLosses());
	  break;
	}
      break;
    }

    case MsgTeleport: {
      PlayerId id;
      uint16_t from, to;
      msg = id.unpack(msg);
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
      uint16_t team;
      msg = src.unpack(msg);
      msg = dst.unpack(msg);
      msg = nboUnpackUShort(msg, team);
      Player* srcPlayer = lookupPlayer(src);
      Player* dstPlayer = lookupPlayer(dst);
      if (dstPlayer == myTank || (!dstPlayer && /*srcPlayer != myTank &&*/
	  (int(team) == int(RogueTeam) ||
	  int(team) == int(myTank->getTeam())))) {
	// message is for me
	BzfString fullMsg;
	if (dstPlayer) 
	  fullMsg = "[direct] ";
	else if (int(team) != int(RogueTeam)) {
#ifdef BWSUPPORT
	  fullMsg = "[to ";
	  fullMsg += Team::getName(TeamColor(team));
	  fullMsg += "] ";
#else
	  fullMsg = "[Team] ";
#endif
	}
	if (!srcPlayer) {
	  /* may unkown not harm us */
	  fullMsg = "(UNKNOWN) ";
	  fullMsg += (const char*)msg;

	  addMessage(NULL, fullMsg, Team::getRadarColor(RogueTeam));
	  break;
	}
	if (!strncmp((char *)msg,"CLIENTQUERY",strlen("CLIENTQUERY"))) {
	  char messageBuffer[MessageLen];
	  memset(messageBuffer, 0, MessageLen);
	  sprintf(messageBuffer,"Version %d.%d%c%d",
	    (VERSION / 10000000) % 100, (VERSION / 100000) % 100,
	    (char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);
	  if (startupInfo.useUDPconnection)
	    strcat(messageBuffer,"+UDP");

	  nboPackString(messageMessage + PlayerIdPLen + 2, messageBuffer, MessageLen);
	  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
	  const GLfloat* msgColor;
	  if (int(team) == int(RogueTeam) || srcPlayer->getTeam() == NoTeam)
	    msgColor = Team::getRadarColor(RogueTeam);
	  else
	    msgColor = Team::getRadarColor(srcPlayer->getTeam());

	  addMessage(srcPlayer,"[Sent versioninfo per request]", msgColor);
	  break;
	}
	fullMsg += (const char*)msg;
	const GLfloat* msgColor;
	if (srcPlayer->getTeam() == NoTeam)
	  msgColor = Team::getRadarColor(RogueTeam);
	else
	  msgColor = Team::getRadarColor(srcPlayer->getTeam());
	addMessage(srcPlayer, fullMsg, msgColor);

	// HUD one line display
	fullMsg = srcPlayer->getCallSign();
#ifdef BWSUPPORT
	fullMsg += " (";
	fullMsg += Team::getName(srcPlayer->getTeam());
	fullMsg += ")";
#endif
	fullMsg += ": ";
	fullMsg += (const char*)msg;
	hud->setAlert(0, fullMsg, 3.0f, False);
      }
      break;
    }

    case MsgAcquireRadio: {
      PlayerId id;
      uint16_t types;
      msg = id.unpack(msg);
      msg = nboUnpackUShort(msg, types);
      Player* tank = lookupPlayer(id);
      if (tank == myTank) {
	// FIXME -- i now have radio
      }
      else if (tank) {
	// FIXME -- start receiving from player
      }
      break;
    }

    case MsgReleaseRadio: {
      PlayerId id;
      msg = id.unpack(msg);
      Player* tank = lookupPlayer(id);
      if (tank == myTank) {
	// FIXME -- i lost the radio, disable transmission
      }
      else if (tank) {
	// FIXME -- stop receiving from player
      }
      break;
    }

    // inter-player relayed message
    case MsgPlayerUpdate:
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
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
      msg = id.unpack(msg);
      Player* tank = lookupPlayer(id);
      if (!tank || tank == myTank) break;
      short oldStatus = tank->getStatus();
      tank->unpack(msg);
      short newStatus = tank->getStatus();
      if ((oldStatus & short(Player::Paused)) !=
				(newStatus & short(Player::Paused)))
	addMessage(tank, (tank->getStatus() & Player::Paused) ?
						"Paused" : "Resumed");
      if ((oldStatus & short(Player::Exploding)) == 0 &&
		(newStatus & short(Player::Exploding)) != 0) {
	// player has started exploding and we haven't gotten killed
	// message yet -- set explosion now, play sound later (when we
	// get killed message).  status is already !Alive so make player
	// alive again, then call setExplode to kill him.
	tank->setStatus(newStatus | short(Player::Alive));
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
      break;
    }
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
      handleServerMessage(True, code, len, msg);
    if (e == -2) {
      printError("Server communication error");
      serverError = True;
      return;
    }
  }

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    while ((e = robotServer[i]->read(code, len, msg, 0)) == 1);
      if (code == MsgKilled || code == MsgShotBegin || code == MsgShotEnd)
	handleServerMessage(False, code, len, msg);
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
  static const float	MinRange = 0.5f * ShotSpeed;	// meters

  // anything beyond this range is okay at any angle
  static const float	MaxRange = 1.5f * ShotSpeed;	// meters

  // if more than SafeAngle off boresight then MinRange is okay
  if (angleCosOffBoresight < SafeAngle) return MinRange;

  // ramp up to MaxRange as target comes to dead center
  const float f = (angleCosOffBoresight - SafeAngle) / (1.0f - SafeAngle);
  return MinRange + f * (MaxRange - MinRange);
}

static void		restartPlaying()
{
  // maximum tries to find a safe place
  static const int	MaxTries = 20;		

  // minimum time before an existing shot can hit us
  static const float	MinShotImpact = 2.0f;		// seconds

  // restart my tank
  float startPoint[3];
  float startAzimuth;
  boolean located = False;
  int locateCount = 0;

  // check for valid starting (no unfair advantage to player or enemies)
  // should find a good location in a few tries... locateCount is a safety
  // check that will probably be invoked when restarting on the team base
  // if the enemy is loitering around waiting for players to reappear.
  // also have to make sure new position isn't in a building;  that must
  // be enforced no matter how many times we need to try new locations.
  startPoint[2] = 0.0f;
  do {
    do {
      if (restartOnBase) {
	const float* base = world->getBase(int(myTank->getTeam()));
	const float x = (base[4] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	const float y = (base[5] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
	startPoint[0] = base[0] + x * cosf(base[3]) - y * sinf(base[3]);
	startPoint[1] = base[1] + x * sinf(base[3]) + y * cosf(base[3]);
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

    // get info on my tank
    const TeamColor myColor = myTank->getTeam();
    const float myCos = cosf(-startAzimuth);
    const float mySin = sinf(-startAzimuth);

    // check each enemy tank
    located = True;
    for (int i = 0; i < maxPlayers; i++) {
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
	  located = False;
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
      if (enemyDist < minSafeRange(enemyCos)) {
	located = False;
	break;
      }

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
      if (myDist < minSafeRange(myCos)) {
	located = False;
	break;
      }
    }
  } while (!located && ++locateCount <= MaxTries);

  // restart the tank
  myTank->restart(startPoint, startAzimuth);
  serverLink->sendAlive(myTank->getPosition(), myTank->getForward());
  restartOnBase = False;
  firstLife = False;
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

boolean			addExplosion(const float* _pos,
				float size, float duration)
{
  // ignore if no prototypes available;
  if (prototypeExplosions.getLength() == 0) return False;

  // don't show explosions if quality isn't high
  if (sceneRenderer->useQuality() < 2) return False;

  // don't add explosion if blending or texture mapping are off
  if (!sceneRenderer->useBlending() || !sceneRenderer->useTexture())
    return False;

  // pick a random prototype explosion
  const int index = (int)(bzfrand() * (float)prototypeExplosions.getLength());

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
  explosions.append(newExplosion);

  if (size < (3.0f * TankLength)) return True; // shot explosion

  int boom = (int) (bzfrand() * 8.0) + 3;
  while (boom--) {
  // pick a random prototype explosion
  const int index = (int)(bzfrand() * (float)prototypeExplosions.getLength());

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
  explosions.append(newExplosion);
  }

  return True;
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

static void		updateExplosions(float dt)
{
  // update time of all explosions
  int i;
  const int count = explosions.getLength();
  for (i = 0; i < count; i++)
    explosions[i]->updateTime(dt);

  // reap expired explosions
  for (i = count - 1; i >= 0; i--)
    if (explosions[i]->isAtEnd()) {
      delete explosions[i];
      explosions.remove(i);
    }
}

static void		addExplosions(SceneDatabase* scene)
{
  const int count = explosions.getLength();
  for (int i = 0; i < count; i++)
    scene->addDynamicNode(explosions[i]);
}

#ifdef ROBOT
static void		handleMyTankKilled()
{
  // blow me up
  myTank->explodeTank();
  playLocalSound(SFX_DIE);

  // i lose a point
  myTank->changeScore(0, 1);
}
#endif

static void		handleFlagDropped(Player* tank)
{
  // skip it if player doesn't actually have a flag
  if (tank->getFlag() == NoFlag) return;

  if (tank == myTank) {
    // update display and play sound effects
    playLocalSound(SFX_DROP_FLAG);
    updateFlag(NoFlag);
  }

  // add message
  BzfString message("dropped ");
  message += Flag::getName(tank->getFlag());
  message += " flag";
  addMessage(tank, message);

  // player no longer has flag
  tank->setFlag(NoFlag);
}

static boolean		gotBlowedUp(BaseLocalPlayer* tank,
					BlowedUpReason reason,
					const PlayerId& killer,
					int shotId)
{
  if (!tank->isAlive())
    return False;

  // you can't take it with you
  const FlagId flag = tank->getFlag();
  if (flag != NoFlag) {
    // tell other players I've dropped my flag
    lookupServer(tank)->sendDropFlag(tank->getPosition());

    // drop it
    handleFlagDropped(tank);
  }

  // take care of explosion business -- don't want to wait for
  // round trip of killed message.  waiting would simplify things,
  // but the delay (2-3 frames usually) can really fool and irritate
  // the player.  we have to be careful to ignore our own Killed
  // message when it gets back to us -- do this by ignoring killed
  // message if we're already dead.
  // don't die if we had the shield flag and we've been shot.
  if (reason != GotShot || flag != ShieldFlag) {
    // blow me up
    tank->explodeTank();
    if (tank == myTank) {
      playLocalSound(SFX_DIE);
    }
    else {
      const float* pos = tank->getPosition();
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
      tank->changeScore(0, 1);

    // tell server I'm dead if it won't already know
    if (reason == GotShot || reason == GotRunOver || reason == GenocideEffect)
      lookupServer(tank)->sendKilled(killer, shotId);
  }

  // print reason if it's my tank
  if (tank == myTank && blowedUpMessage[reason]) {
    controlPanel->addMessage(blowedUpMessage[reason]);
    hud->setAlert(0, blowedUpMessage[reason], 4.0f, True);
  }

  // make sure shot is terminated locally (if not globally) so it can't
  // hit me again if I had the shield flag.  this is important for the
  // shots that aren't stopped by a hit and so may stick around to hit
  // me on the next update, making the shield useless.
  return (reason == GotShot && flag == ShieldFlag && shotId != -1);
}

static void		checkEnvironment()
{
  if (!myTank) return;

  // skip this if i'm dead or paused
  if (!myTank->isAlive() || myTank->isPaused()) return;

  FlagId flagId = myTank->getFlag();
  if (flagId != NoFlag && int(flagId) >= int(FirstTeamFlag) &&
				int(flagId) <= int(LastTeamFlag)) {
    // have I captured a flag?
    TeamColor base = world->whoseBase(myTank->getPosition());
    TeamColor team = myTank->getTeam();
    if (myTank->getLocation() == LocalPlayer::OnGround && base != NoTeam &&
	((int(flagId) == int(team) && base != team) ||
	 (int(flagId) != int(team) && base == team)))
      serverLink->sendCaptureFlag(base);
  }
  else if (flagId == NoFlag && myTank->getLocation() == LocalPlayer::OnGround) {
    // grab any and all flags i'm driving over
    const float* tpos = myTank->getPosition();
    const float radius = myTank->getRadius();
    const float radius2 = (radius + FlagRadius) * (radius + FlagRadius);
    for (int i = 0; i < numFlags; i++) {
      if (world->getFlag(i).id == NoFlag) continue;
      const float* fpos = world->getFlag(i).position;
      if ((tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		(tpos[1] - fpos[1]) * (tpos[1] - fpos[1]) < radius2) {
	serverLink->sendGrabFlag(i);
      }
    }
  }
  else if (flagId == IdentifyFlag) {
    // identify closest flag
    const float* tpos = myTank->getPosition();
    BzfString message("Closest Flag: ");
    float minDist = IdentityRange * IdentityRange;
    int closestFlag = -1;
    for (int i = 0; i < numFlags; i++) {
      if (world->getFlag(i).id == NoFlag ||
	  world->getFlag(i).status != FlagOnGround) continue;
      const float* fpos = world->getFlag(i).position;
      const float dist = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
			 (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]);
      if (dist < minDist) {
	minDist = dist;
	closestFlag = i;
      }
    }
    if (closestFlag != -1) {
      // Set HUD alert about what the flag is
      message += Flag::getName(world->getFlag(closestFlag).id);
      hud->setAlert(2, message, 0.5f,
		Flag::getType(world->getFlag(closestFlag).id) == FlagSticky);
    }
  }

  // see if i've been shot
  const ShotPath* hit = NULL;
  float minTime = Infinity;
  myTank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < maxPlayers; i++)
    if (player[i])
      myTank->checkHit(player[i], hit, minTime);
  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendEndShot(hit->getPlayer(), hit->getShotId(), 1);
    const boolean stopShot =
	gotBlowedUp(myTank, GotShot, hit->getPlayer(), hit->getShotId());
    if (stopShot || hit->isStoppedByHit()) {
      Player* hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }

  // if not dead yet, see if i got run over by the steamroller
  else if (myTank->getLocation() == LocalPlayer::OnGround) {
    const float* myPos = myTank->getPosition();
    const float myRadius = myTank->getRadius();
    for (i = 0; i < maxPlayers; i++)
      if (player[i] &&
	  player[i]->getFlag() == SteamrollerFlag &&
	  !player[i]->isPaused()) {
	const float* pos = player[i]->getPosition();
	if (pos[2] < TankHeight &&
		!(flagId == PhantomZoneFlag && myTank->isFlagActive())) {
	  const float radius = myRadius + SRRadiusMult * player[i]->getRadius();
	  if (hypot(myPos[0] - pos[0], myPos[1] - pos[1]) < radius)
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
  boolean lockedOn = False;

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
	d < bestDistance) {				// is it better?
      bestTarget = player[i];
      bestDistance = d;
      lockedOn = True;
    }
    else if (a < 0.3f &&				// about 17 degrees
	player[i]->getFlag() != StealthFlag &&		// can't "see" stealth
	d < bestDistance && !lockedOn) {		// is it better?
      bestTarget = player[i];
      bestDistance = d;
    }
  }
  if (!lockedOn) myTank->setTarget(NULL);
  if (!bestTarget) return;

  if (lockedOn) {
    myTank->setTarget(bestTarget);
    BzfString msg("Locked on ");
    msg += bestTarget->getCallSign();
    msg += " (";
    msg += Team::getName(bestTarget->getTeam());
    if (bestTarget->getFlag() != NoFlag) {
      msg += ") with ";
      msg += Flag::getName(bestTarget->getFlag());
    }
    else {
      msg += ")";
    }
    addMessage(NULL, msg);
    hud->setAlert(1, msg, 2.0f, 1);
  }
  else if (myTank->getFlag() == ColorblindnessFlag) {
    addMessage(NULL, "Looking at a tank");
    hud->setAlert(1, "Looking at a tank", 2.0f, 0);
  }
  else {
    BzfString msg("Looking at ");
    msg += bestTarget->getCallSign();
    msg += " (";
    msg += Team::getName(bestTarget->getTeam());
    if (bestTarget->getFlag() != NoFlag) {
      msg += ") with ";
      msg += Flag::getName(bestTarget->getFlag());
    }
    else {
      msg += ")";
    }
    addMessage(NULL, msg);
    hud->setAlert(1, msg, 2.0f, 0);
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

static RegionList	obstacleList;

static void		addObstacle(RegionList& list, const Obstacle& obstacle)
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

  int numRegions = list.getLength();
  for (int k = 0; k < numRegions; k++) {
    BzfRegion* region = list[k];
    int side[4];
    if ((side[0] = region->classify(p[0], p[1])) == 1 ||
		(side[1] = region->classify(p[1], p[2])) == 1 ||
		(side[2] = region->classify(p[2], p[3])) == 1 ||
		(side[3] = region->classify(p[3], p[0])) == 1)
      continue;
    if (side[0] == -1 && side[1] == -1 && side[2] == -1 && side[3] == -1) {
      list.swap(k, numRegions-1);
      list.swap(numRegions-1, list.getLength()-1);
      list.remove(list.getLength()-1);
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
      if (region != list[k]) list.append(region);
      region = newRegion;
    }
    if (region != list[k]) delete region;
  }
}

static void		makeObstacleList()
{
  int i;
  const int count = obstacleList.getLength();
  for (i = 0; i < count; i++)
    delete obstacleList[i];
  obstacleList.removeAll();

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
  obstacleList.append(new BzfRegion(4, gameArea));

  const BoxBuildings& boxes = World::getWorld()->getBoxes();
  const int numBoxes = boxes.getLength();
  for (i = 0; i < numBoxes; i++)
    addObstacle(obstacleList, boxes[i]);
  const PyramidBuildings& pyramids = World::getWorld()->getPyramids();
  const int numPyramids = pyramids.getLength();
  for (i = 0; i < numPyramids; i++)
    addObstacle(obstacleList, pyramids[i]);
  const Teleporters& teleporters = World::getWorld()->getTeleporters();
  const int numTeleporters = teleporters.getLength();
  for (i = 0; i < numTeleporters; i++)
    addObstacle(obstacleList, teleporters[i]);
}

static void		setRobotTarget(RobotPlayer* robot)
{
  Player* bestTarget = NULL;
  float bestPriority = 0.0f;
  for (int j = 0; j < maxPlayers; j++)
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
  boolean pickTarget = False;
  int i;

  // see if we should look for new targets
  clock += dt;
  if (clock > newTargetTimeout) {
    while (clock > newTargetTimeout) clock -= newTargetTimeout;
    pickTarget = True;
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
  for (i = 0; i < maxPlayers; i++)
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
    boolean dead = False;
    const float* myPos = tank->getPosition();
    const float myRadius = tank->getRadius();
    if (myTank->getFlag() == SteamrollerFlag && !myTank->isPaused()) {
      const float* pos = myTank->getPosition();
      if (pos[2] < TankHeight) {
	const float radius = myRadius + SRRadiusMult * myTank->getRadius();
	if (hypot(myPos[0] - pos[0], myPos[1] - pos[1]) < radius) {
	  gotBlowedUp(tank, GotRunOver, myTank->getId());
	  dead = True;
	}
      }
    }
    for (i = 0; !dead && i < maxPlayers; i++)
      if (player[i] &&
	  player[i]->getFlag() == SteamrollerFlag &&
	  !player[i]->isPaused()) {
	const float* pos = player[i]->getPosition();
	if (pos[2] < TankHeight) {
	  const float radius = myRadius + SRRadiusMult * player[i]->getRadius();
	  if (hypot(myPos[0] - pos[0], myPos[1] - pos[1]) < radius) {
	    gotBlowedUp(tank, GotRunOver, player[i]->getId());
	    dead = True;
	  }
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

static void		addRobots(boolean useMulticastRelay)
{
  uint16_t code, len;
  char msg[MaxPacketLen];
  char callsign[CallSignLen];

  for (int j = 0; j < numRobots;) {

#if !defined(_WIN32)
	snprintf(callsign, CallSignLen, "%s%d", myTank->getCallSign(), j);
#else
	sprintf(callsign, "%s%d", myTank->getCallSign(), j);
#endif

    robots[j] = new RobotPlayer(robotServer[j]->getId(), callsign, robotServer[j], myTank->getEmailAddress());
    if (world->allowRogues())
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

//
// join/leave a game
//

static World*		makeWorld(ServerLink* serverLink)
{
  uint16_t code, len, size;
  char msg[MaxPacketLen];

  // ask for world and wait for it (ignoring all other messages)
  nboPackUShort(msg, 0);
  serverLink->send(MsgGetWorld, 2, msg);
  if (serverLink->read(code, len, msg, 5000) <= 0) return NULL;
  if (code == MsgNull || code == MsgSuperKill) return NULL;
  if (code != MsgGetWorld) return NULL;

  // get size of entire world database and make space
  nboUnpackUShort(msg, size);
  char* worldDatabase = new char[size];

  // get world database
  uint16_t ptr = 0, bytesLeft = size;
  while (bytesLeft != 0) {
    // ask and wait for next chunk
    nboPackUShort(msg, ptr);
    serverLink->send(MsgGetWorld, 2, msg);
    if (serverLink->read(code, len, msg, 5000) < 0 ||
	code == MsgNull || code == MsgSuperKill) {
      delete[] worldDatabase;
      return NULL;
    }

    // get bytes left
    void* buf = msg;
    buf = nboUnpackUShort(buf, bytesLeft);

    // add chunk to database so far
    ::memcpy(worldDatabase + int(ptr), buf, len - 2);

    // increment pointer
    ptr += len - 2;
  }

  // make world
  WorldBuilder worldBuilder;
  worldBuilder.unpack(worldDatabase);
  delete[] worldDatabase;

  // return world
  return worldBuilder.getWorld();
}

static boolean		enterServer(ServerLink* serverLink, World* world,
						const LocalPlayer* myTank)
{

  time_t timeout=time(0) + 10;  // give us 10 sec

  // tell server we want to join
  serverLink->sendEnter(TankPlayer, myTank->getTeam(),
		myTank->getCallSign(), myTank->getEmailAddress());

  // wait for response
  uint16_t code, len;
  char msg[MaxPacketLen];
  if (serverLink->read(code, len, msg, -1) < 0) {
    printError("Communication error joining game [No immediate respose].");
    return False;
  }
  if (code == MsgSuperKill) {
    printError("Server forced disconnection.");
    return False;
  }
  if (code != MsgAccept && code != MsgReject) {
    printError("Communication error joining game [Wrong Code %04x].",code);
    return False;
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
    return False;
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
	buf = id.unpack(buf);
	if (id == myTank->getId()) {		// it's me!  end of updates
	  // scan through flags and, for flags on
	  // tanks, tell the tank about its flag.
	  const int maxFlags = world->getMaxFlags();
	  for (int i = 0; i < maxFlags; i++) {
	    const Flag& flag = world->getFlag(i);
	    if (flag.status == FlagOnTank)
	      for (int j = 0; j < maxPlayers; j++)
		if (player[j] && player[j]->getId() == flag.owner) {
		  player[j]->setFlag(flag.id);
		  break;
		}
	  }
	  return True;
	}
	addPlayer(id, buf, False);
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
  return False;
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

  const int count = obstacleList.getLength();
  for (i = 0; i < count; i++)
    delete obstacleList[i];
  obstacleList.removeAll();
#endif

  // my tank goes away
  const boolean sayGoodbye = (myTank != NULL);
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
  numFlags = 0;
  player = NULL;

  // update UI
  controlPanel->resetTeamCounts();
  sceneRenderer->getBackground()->notifyWorldChange();
  hud->setPlaying(False);
  hud->setCracks(False);
  hud->setPlayerHasHighScore(False);
  hud->setTeamHasHighScore(False);
  hud->setHeading(0.0f);
  hud->setAltitude(0.0f);
  hud->setAltitudeTape(False);
  hud->setMarker(0, False);
  hud->setMarker(1, False);

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
			1.1f, 1.5f * WorldSize,
			mainWindow->getWidth(), mainWindow->getHeight(),
			mainWindow->getViewHeight());
  sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

  // reset some flags
  gameOver = False;
  serverError = False;
  serverDied = False;
}

static boolean		joinGame(const StartupInfo* info,
				ServerLink* _serverLink,
				PlayerLink* _playerLink)
{
  // assume everything's okay for now
  serverDied = False;
  serverError = False;

  serverLink = _serverLink;
  playerLink = _playerLink;

  if (!serverLink || !playerLink) {
    printError("Memory error");
    leaveGame();
    return False;
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
    return False;
  }

  // check inter-player connection
/* NOTE -- this will be handled later when we try to fallback to TCP
  if (playerLink->getState() == PlayerLink::SocketError) {
    printError("Couldn't make inter-player connection");
    leaveGame();
    return False;
  }
*/

  // set tank textures
  Player::setTexture(*tankTexture);

  // create world
  world = makeWorld(serverLink);
  if (!world) {
    printError("Error downloading world database");
    leaveGame();
    return False;
  }

  ServerLink::setServer(serverLink);
  PlayerLink::setMulticast(playerLink);
  World::setWorld(world);

  // prep teams
  teams = world->getTeams();

  // prep players
  maxPlayers = world->getMaxPlayers();
  player = world->getPlayers();

  // prep flags
  numFlags = world->getMaxFlags();

  // make scene database
  const boolean oldUseZBuffer = sceneRenderer->useZBuffer();
  sceneRenderer->setZBuffer(False);
  bspScene = sceneBuilder->make(world);
  sceneRenderer->setZBuffer(True);
  if (sceneRenderer->useZBuffer())
    zScene = sceneBuilder->make(world);
  sceneRenderer->setZBuffer(oldUseZBuffer);
  setSceneDatabase();

  // make radar
  radar = new RadarRenderer(*sceneRenderer, *world);
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
    return False;
  }

  // check multicast `connection' to server.  if we get no response then
  // we have to assume the network can't do multicasting.  fall back to
  // using the server as a relay.
  boolean multicastOkay = False;
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
	  multicastOkay = True;
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
  }

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
      return False;
    }

    // prepare player link to use the server as a relay
    playerLink->setUseRelay();
    playerLink->setRelay(serverLink);
    printError("Using multicast relay");
	if (startupInfo.useUDPconnection)
		playerLink->enableUDPConIfRelayed();
	else 
		printError("No UDP connection, see Options to enable.");
  }

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
  restartOnBase = world->allowTeamFlags() && myTank->getTeam() != RogueTeam;

  // if server constrains time then adjust it
  if (!world->allowTimeOfDayAdjust()) {
    epochOffset = double(world->getEpochOffset());
    updateDaylight(epochOffset, *sceneRenderer);
    lastEpochOffset = epochOffset;
  }

  // initialize some other stuff
  sceneRenderer->getBackground()->notifyWorldChange();
  updateNumPlayers();
  updateFlag(NoFlag);
  updateHighScores();
  radar->setRange(RadarMedRange);
  hud->setHeading(myTank->getAngle());
  hud->setAltitude(myTank->getPosition()[2]);
  hud->setTimeLeft(-1);
  fireButton = False;
  firstLife = True;

  return True;
}

static boolean		joinInternetGame(const StartupInfo* info)
{
  // open server
  Address serverAddress(info->serverName);
  if (serverAddress.isAny()) return False;
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

static boolean		joinGame()
{
  // assume we have everything we need to join.  figure out how
  // to join by which arguments are set in StartupInfo.
  // currently only support joinInternetGame.
  if (startupInfo.serverName[0])
    return joinInternetGame(&startupInfo);

  // can't figure out how to join
  printError("Can't figure out how to join.");
  return False;
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
    glScissor(ox, oy + mainWindow->getPanelHeight(),
		width, mainWindow->getViewHeight());
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
  const int zoom = atoi(resources->getValue("zoom"));
  if (zoom < 1) return 1;
  if (zoom > 8) return 8;
  return zoom;
}

static void		playingLoop()
{
  static const float	defaultPos[3] = { 0.0f, 0.0f, 0.0f };
  static const float	defaultDir[3] = { 1.0f, 0.0f, 0.0f };
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
  const boolean fakeCursor = resources->hasValue("fakecursor") &&
		strcmp(resources->getValue("fakecursor"), "yes") == 0;
  mainWindow->setZoomFactor(zoomFactor);
  if (fakeCursor)
    mainWindow->getWindow()->hideMouse();

  // start timing
  int frameCount = 0;
  float cumTime = 0.0f;
  TimeKeeper::setTick();
  updateDaylight(epochOffset, *sceneRenderer);

  // main loop
  while (!mainWindow->getQuit()) {
    // get delta time
    TimeKeeper prevTime = TimeKeeper::getTick();
    TimeKeeper::setTick();
    const float dt = TimeKeeper::getTick() - prevTime;

    // handle incoming packets
    doMessages();

    // do dead reckoning on remote players
    for (i = 0; i < maxPlayers; i++)
      if (player[i]) {
	const boolean wasNotResponding = player[i]->isNotResponding();
	player[i]->doDeadReckoning();
	const boolean isNotResponding = player[i]->isNotResponding();
	if (!wasNotResponding && isNotResponding) {
	  addMessage(player[i], BzfString("not responding"));
	}
	else if (wasNotResponding && !isNotResponding) {
	  addMessage(player[i], BzfString("okay"));
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

    // handle events
    clockAdjust = 0.0f;
#if defined(ROAMING)
    roamDPos[0] = roamDPos[1] = roamDPos[2] = 0.0f;
    roamDTheta = roamDPhi = roamDZoom = 0.0f;
#endif
//#ifndef macintosh    
    while (!mainWindow->getQuit() && display->isEventPending())
      doEvent(display);

//#else
//     doEvent(display);
//#endif

    // invoke callbacks
    callPlayingCallbacks();

    // quick out
    if (mainWindow->getQuit()) break;

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

#if defined(ROAMING)
    {
      // move roaming camera
      float c, s;
      c = cosf(roamTheta * M_PI / 180.0f);
      s = sinf(roamTheta * M_PI / 180.0f);
      roamPos[0] += dt * (c * roamDPos[0] - s * roamDPos[1]);
      roamPos[1] += dt * (c * roamDPos[1] + s * roamDPos[0]);
      roamPos[2] += dt * roamDPos[2];
      roamTheta  += dt * roamDTheta;
      roamPhi    += dt * roamDPhi;
      roamZoom   += dt * roamDZoom;
      if (roamZoom < 1.0f) roamZoom = 1.0f;
      else if (roamZoom > 179.0f) roamZoom = 179.0f;
    }
#endif

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
      hud->setAlert(1, NULL, 0.0f, True);
    }
//    if (maxPauseCountdown > 0.0f) {
//     const int oldMaxPauseCountdown = (int)(maxPauseCountdown + 0.99f);
//      maxPauseCountdown -= dt;
//     if (maxPauseCountdown <= 0.0f) {
//			maxPauseCountdown=0.0;
//	  }
//	  if ((int)(maxPauseCountdown + 0.99f) != oldMaxPauseCountdown) {
//		char msgBuf[40];
//		sprintf(msgBuf, "Pause Countdown %d", (int)(maxPauseCountdown + 0.99f));
//		hud->setAlert(1, msgBuf, 1.0f, False);
//	  }
//	}
    if (pauseCountdown > 0.0f) {
      const int oldPauseCountdown = (int)(pauseCountdown + 0.99f);
      pauseCountdown -= dt;
      if (pauseCountdown <= 0.0f) {
	// okay, now we pause.  first drop any team flag we may have.
	const FlagId flagId = myTank->getFlag();
	if (flagId >= FirstTeamFlag && flagId <= LastTeamFlag)
	  serverLink->sendDropFlag(myTank->getPosition());

	// now actually pause
	myTank->setPause(True);
	hud->setAlert(1, NULL, 0.0f, True);
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
	hud->setAlert(1, msgBuf, 1.0f, False);
      }
    }

    // reposition flags
    updateFlags(dt);

    // update explosion animations
    updateExplosions(dt);

    // update other tank's shots
    for (i = 0; i < maxPlayers; i++)
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
#if defined(ROAMING)
      if (roaming) {
#ifdef FOLLOWTANK
        eyePoint[0] = myTankPos[0] - myTankDir[0] * 20;
        eyePoint[1] = myTankPos[1] - myTankDir[1] * 20;
        eyePoint[2] = myTankPos[2] + MuzzleHeight * 3;
        targetPoint[0] = eyePoint[0] + myTankDir[0];
        targetPoint[1] = eyePoint[1] + myTankDir[1];
        targetPoint[2] = eyePoint[2] + myTankDir[2];
#endif
	float dir[3];
	dir[0] = cosf(roamPhi * M_PI / 180.0f) * cosf(roamTheta * M_PI / 180.0f);
	dir[1] = cosf(roamPhi * M_PI / 180.0f) * sinf(roamTheta * M_PI / 180.0f);
	dir[2] = sinf(roamPhi * M_PI / 180.0f);
	eyePoint[0] = roamPos[0];
	eyePoint[1] = roamPos[1];
	eyePoint[2] = roamPos[2];
	if (!roamTrack) {
	  targetPoint[0] = eyePoint[0] + dir[0];
	  targetPoint[1] = eyePoint[1] + dir[1];
	  targetPoint[2] = eyePoint[2] + dir[2];
	}
#endif
	fov = roamZoom * M_PI / 180.0f;
      }
#endif
      sceneRenderer->getViewFrustum().setProjection(fov,
					1.1f, 1.5f * WorldSize,
					mainWindow->getWidth(),
					mainWindow->getHeight(),
					mainWindow->getViewHeight());
      sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

      // add dynamic nodes
      SceneDatabase* scene = sceneRenderer->getSceneDatabase();
      if (scene && myTank) {
	myTank->addPlayer(scene, False, False);	// add my tank
	if (myTank->getFlag() == CloakingFlag)
	  myTank->setInvisible();		// and make it invisible
#if defined(ROAMING)
	else if (roaming)
	  myTank->setHidden(False);
#endif
	else
	  myTank->setHidden();			// or make it hidden
	myTank->addShots(scene, False);		// add my shells
	myTank->addAntidote(scene);		// add antidote flag
	world->addFlags(scene);			// add flags

	// add other tanks and shells
	const boolean colorblind = (myTank->getFlag() == ColorblindnessFlag);
	for (i = 0; i < maxPlayers; i++)
	  if (player[i]) {
	    player[i]->updateSparks(dt);
	    player[i]->addShots(scene, colorblind);
	    player[i]->addPlayer(scene, colorblind, True);
	    player[i]->setInvisible(player[i]->getFlag() == CloakingFlag);
	  }

	// add explosions
	addExplosions(scene);

	// if i'm inside a building then add eighth dimension scene node.
	if (myTank->getContainingBuilding()) {
	  SceneNode* node = world->getInsideSceneNode(myTank->
						getContainingBuilding());
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
		(myTank && !myTank->isAlive() && !myTank->isExploding()));

      // set hud state
      hud->setDim(HUDDialogStack::get()->isActive());
      hud->setPlaying(myTank && (myTank->isAlive() && !myTank->isPaused()));
      hud->setCracks(myTank && !firstLife && !myTank->isAlive());

      // get frame start time
      if (showDrawTime) {
#if defined(DEBUG_RENDERING)
	// get an accurate measure of frame time (at expense of frame rate)
	glFinish();
#endif
	media->stopwatch(True);
      }

      // draw frame
      const boolean blankRadar = myTank && myTank->isPaused();
      if (viewType == SceneRenderer::ThreeChannel) {
	// draw center channel
	sceneRenderer->render(False);
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render();
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
	sceneRenderer->render(False, True, True);

	// set up for drawing right channel
	mainWindow->setQuadrant(MainWindow::LowerRight);
	// FIXME -- this assumes up is along +z
	targetPoint[0] = eyePoint[0] + cFOV*myTankDir[0] + sFOV*myTankDir[1];
	targetPoint[1] = eyePoint[1] + cFOV*myTankDir[1] - sFOV*myTankDir[0];
	targetPoint[2] = eyePoint[2] + myTankDir[2];
	sceneRenderer->getViewFrustum().setView(eyePoint, targetPoint);

	// draw right channel
	sceneRenderer->render(True, True, True);

	// back to center channel
	mainWindow->setQuadrant(MainWindow::UpperRight);
      }
      else if (viewType == SceneRenderer::Stacked) {
       static float EyeDisplacement = 0.25f * TankWidth;
       static float FocalPlane = BoxBase;
       static boolean init = False;
       if (!init) {
	 init = True;
	 if (resources->hasValue("eyesep"))
	   EyeDisplacement = (float)atof(resources->getValue("eyesep"));
	 if (resources->hasValue("focal"))
	   FocalPlane = (float)atof(resources->getValue("focal"));
       }

       // setup view for left eye
       sceneRenderer->getViewFrustum().setOffset(EyeDisplacement, FocalPlane);

       // draw left eye's view
       sceneRenderer->render(False);
       hud->render(*sceneRenderer);
       renderDialog();
       controlPanel->render();
       if (radar) radar->render(*sceneRenderer, blankRadar);

       // set up view for right eye
       mainWindow->setQuadrant(MainWindow::UpperHalf);
       sceneRenderer->getViewFrustum().setOffset(-EyeDisplacement, FocalPlane);

       // draw right eye's view
       sceneRenderer->render(True, True);
       hud->render(*sceneRenderer);
       renderDialog();
       controlPanel->render();
       if (radar) radar->render(*sceneRenderer, blankRadar);

       // draw common stuff

       // back to left channel
       mainWindow->setQuadrant(MainWindow::LowerHalf);
      }
      else if (viewType == SceneRenderer::Stereo) {
	static float EyeDisplacement = 0.25f * TankWidth;
	static float FocalPlane = BoxBase;
	static boolean init = False;
	if (!init) {
	  init = True;
	  if (resources->hasValue("eyesep"))
	    EyeDisplacement = (float)atof(resources->getValue("eyesep"));
	  if (resources->hasValue("focal"))
	    FocalPlane = (float)atof(resources->getValue("focal"));
	}

	// setup view for left eye
#ifdef USE_GL_STEREO
	glDrawBuffer(GL_BACK_LEFT);
#endif
	sceneRenderer->getViewFrustum().setOffset(EyeDisplacement, FocalPlane);

	// draw left eye's view
	sceneRenderer->render(False);
#ifndef USE_GL_STEREO
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render();
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
	sceneRenderer->render(True, True);
#ifndef USE_GL_STEREO
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render();
	if (radar) radar->render(*sceneRenderer, blankRadar);
#endif

	// draw common stuff
#ifdef USE_GL_STEREO
	glDrawBuffer(GL_BACK);
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render();
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
	  const int ph = mainWindow->getPanelHeight();
	  sceneRenderer->getViewFrustum().setProjection(fov,
					1.1f, 1.5f * WorldSize, w, h, vh);
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
	  glRasterPos2i(0, mainWindow->getPanelHeight());
	  glPopMatrix();

	  // zoom small image to entire window
	  glDisable(GL_DITHER);
	  glPixelZoom((float)zoomFactor, (float)zoomFactor);
	  glCopyPixels(x, y + ph, w, vh, GL_COLOR);
	  glPixelZoom(1.0f, 1.0f);
	  if (sceneRenderer->useDithering()) glEnable(GL_DITHER);
	}
	else {
	  // normal rendering
	  sceneRenderer->render();
	}

	// draw other stuff
	hud->render(*sceneRenderer);
	renderDialog();
	controlPanel->render();
	if (radar) radar->render(*sceneRenderer, blankRadar);
      }

      // get frame end time
      if (showDrawTime) {
#if defined(DEBUG_RENDERING)
	// get an accurate measure of frame time (at expense of frame rate)
	glFinish();
#endif
	hud->setDrawTime((float)media->stopwatch(False));
      }

      // draw a fake cursor if requested.  this is mostly intended for
      // pass through 3D cards that don't have cursor support.
      if (fakeCursor) {
	int mx, my;
	const int width = mainWindow->getWidth();
	const int height = mainWindow->getHeight();
	const int panelHeight = mainWindow->getPanelHeight();
	const int ox = mainWindow->getOriginX();
	const int oy = mainWindow->getOriginY();
	mainWindow->getWindow()->getMouse(mx, my);
	my = height - my - 1;
	if (my < panelHeight) my = panelHeight;

	glScissor(ox, oy + panelHeight, width, height - panelHeight);
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
	if (fireButton && myTank->getFlag() == MachineGunFlag)
	  myTank->fireShot();
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
    if (playerLink && myTank->isDeadReckoningWrong()) {
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
  mainWindow->showWindow(False);
}

//
// game initialization
//

static float		timeConfiguration(boolean useZBuffer)
{
  // prepare depth buffer if requested
  sceneRenderer->setZBuffer(useZBuffer);
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
  sceneRenderer->setBlending(False);
  sceneRenderer->setSmoothing(False);
  sceneRenderer->setLighting(False);
  sceneRenderer->setTexture(False);
  sceneRenderer->setQuality(0);
  sceneRenderer->setDithering(True);
  sceneRenderer->setShadows(False);
  OpenGLTexture::setFilter(OpenGLTexture::Off);
  timeConfiguration(True);

  // time lowest quality with and without blending.  some systems
  // stipple very slowly even though everything else is fast.  we
  // don't want to conclude the system is slow because of stippling.
  printError("  lowest quality");
  const float timeNoBlendNoZ = timeConfiguration(False);
  const float timeNoBlendZ   = timeConfiguration(True);
  sceneRenderer->setBlending(True);
  const float timeBlendNoZ   = timeConfiguration(False);
  const float timeBlendZ     = timeConfiguration(True);
  if (timeNoBlendNoZ > MaxFrameTime &&
      timeNoBlendZ   > MaxFrameTime &&
      timeBlendNoZ   > MaxFrameTime &&
      timeBlendZ     > MaxFrameTime) {
    if (timeNoBlendNoZ < timeNoBlendZ &&
	timeNoBlendNoZ < timeBlendNoZ &&
	timeNoBlendNoZ < timeBlendZ) {
      // no depth, no blending definitely fastest
      sceneRenderer->setZBuffer(False);
      sceneRenderer->setBlending(False);
    }
    if (timeNoBlendZ < timeBlendNoZ &&
	timeNoBlendZ < timeBlendZ) {
      // no blending faster than blending
      sceneRenderer->setZBuffer(True);
      sceneRenderer->setBlending(False);
    }
    if (timeBlendNoZ < timeBlendZ) {
      // blending faster than depth
      sceneRenderer->setZBuffer(False);
      sceneRenderer->setBlending(True);
    }
    // blending and depth faster than without either
    sceneRenderer->setZBuffer(True);
    sceneRenderer->setBlending(True);
    return;
  }

  // leave blending on if blending clearly faster than stippling
  if (timeBlendNoZ > timeNoBlendNoZ || timeBlendNoZ > timeNoBlendZ &&
      timeBlendZ   > timeNoBlendNoZ || timeBlendZ   > timeNoBlendZ) {
    sceneRenderer->setBlending(False);
  }

  // try texturing.  if it's too slow then fall back to
  // lowest quality and return.
  sceneRenderer->setTexture(True);
  sceneRenderer->setQuality(1);
  OpenGLTexture::setFilter(OpenGLTexture::Nearest);
  printError("  lowest quality with texture");
  if (timeConfiguration(False) > MaxFrameTime ||
      timeConfiguration(True) > MaxFrameTime) {
    sceneRenderer->setTexture(False);
    OpenGLTexture::setFilter(OpenGLTexture::Off);
    sceneRenderer->setQuality(0);
    return;
  }

  // everything
  printError("  full quality");
  sceneRenderer->setBlending(True);
  sceneRenderer->setSmoothing(True);
  sceneRenderer->setLighting(True);
  sceneRenderer->setTexture(True);
  sceneRenderer->setQuality(2);
  sceneRenderer->setDithering(True);
  sceneRenderer->setShadows(True);
  OpenGLTexture::setFilter(OpenGLTexture::LinearMipmapLinear);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // try it without shadows -- some platforms stipple very slowly
  sceneRenderer->setShadows(False);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no high quality
  printError("  medium quality");
  sceneRenderer->setQuality(1);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;
  printError("  low quality");
  sceneRenderer->setQuality(0);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // lower quality texturing
  printError("  nearest texturing");
  OpenGLTexture::setFilter(OpenGLTexture::Nearest);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no texturing
  printError("  no texturing");
  sceneRenderer->setTexture(False);
  OpenGLTexture::setFilter(OpenGLTexture::Off);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no blending
  printError("  no blending");
  sceneRenderer->setBlending(False);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no smoothing.  shouldn't really affect fill rate too much.
  printError("  no smoothing");
  sceneRenderer->setSmoothing(False);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no lighting.  shouldn't really affect fill rate, either.
  printError("  no lighting");
  sceneRenderer->setLighting(False);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;

  // no dithering
  printError("  no dithering");
  sceneRenderer->setDithering(False);
  if (timeConfiguration(True) < MaxFrameTime) return;
  if (timeConfiguration(False) < MaxFrameTime) return;
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
					sEdge, tEdge, 1.0f, 1.0f, True);
  node->setColor(color);
  node->setModulateColor(color);
  node->setLightedColor(color);
  node->setLightedModulateColor(color);
  node->setTexture(HUDuiControl::getArrow());
  node->setMaterial(OpenGLMaterial(color, color));
  timingScene->addStaticNode(node);
  sceneRenderer->setSceneDatabase(timingScene);
  sceneRenderer->setDim(False);

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
  controlPanel->render();
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
    sceneRenderer->setBlending(False);
    sceneRenderer->setSmoothing(False);
    sceneRenderer->setLighting(False);
    sceneRenderer->setTexture(False);
    sceneRenderer->setQuality(0);
    sceneRenderer->setDithering(False);
    sceneRenderer->setShadows(False);
    OpenGLTexture::setFilter(OpenGLTexture::Off);
  }

  // should we grab the mouse?  yes if fullscreen.
  if (!resources->hasValue("window"))
    setGrabMouse(True);
#if defined(__linux__) && !defined(DEBUG)
  // linux usually has a virtual root window so grab mouse always
  setGrabMouse(True);
#endif

  // show window and clear it immediately
  mainWindow->showWindow(True);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glDisable(GL_SCISSOR_TEST);
  glClear(GL_COLOR_BUFFER_BIT);
  mainWindow->getWindow()->swapBuffers();

  // resize and draw basic stuff
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST);
  controlPanel->resize();
  sceneRenderer->render();
  controlPanel->render();
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
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(suicide));
  bzSignal(SIGILL, SIG_PF(dying));
  bzSignal(SIGABRT, SIG_PF(dying));
  bzSignal(SIGSEGV, SIG_PF(dying));
  bzSignal(SIGTERM, SIG_PF(suicide));
#if !defined(_WIN32)
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
  if (!resources->hasValue("window") && resources->hasValue("resolution")) {
    BzfString videoFormat = resources->getValue("resolution");
    if (!videoFormat.isNull()) {
      const int format = display->findResolution(videoFormat);
      if (display->isValidResolution(format) &&
	  display->getResolution() != format &&
	  display->setResolution(format)) {

	// handle resize
	if (resources->hasValue("geometry")) {
	  int w, h, x, y, count;
	  char xs, ys;
	  count = sscanf(resources->getValue("geometry"),
			"%dx%d%c%d%c%d", &w, &h, &xs, &x, &ys, &y);
	  if (w < 640) w = 640;
	  if (h < 400) h = 400;
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
  controlPanel->render();
  mainWindow->getWindow()->swapBuffers();

  // make heads up display
  HUDRenderer _hud(display, renderer);
  hud = &_hud;

  // initialize control panel and hud
  updateNumPlayers();
  updateFlag(NoFlag);
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
				OpenGLTexture::Linear, False, True);
    if (!tex.isValid()) continue;

    // make explosion scene node
    BillboardSceneNode* explosion = new BillboardSceneNode(zero);
    explosion->setTexture(tex);
    explosion->setTextureAnimation(8, 8);
    explosion->setLight();
    explosion->setLightColor(1.0f, 0.8f, 0.5f);
    explosion->setLightAttenuation(0.04f, 0.0f, 0.01f);

    // add it to list of prototype explosions
    prototypeExplosions.append(explosion);
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

  // print version
  {
    char bombMessage[80];
    sprintf(bombMessage, "BZFlag version %d.%d%c%d",
		(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
		(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);
    controlPanel->addMessage("");
    controlPanel->addMessage(bombMessage);
  }

  // print expiration
  if (timeBombString()) {
    // add message about date of expiration
    char bombMessage[80];
    sprintf(bombMessage, "This release will expire on %s", timeBombString());
    controlPanel->addMessage(bombMessage);
  }

  // print copyright
  controlPanel->addMessage(copyright);
  controlPanel->addMessage("Author: Chris Schoeneman <crs23@bigfoot.com>");
  controlPanel->addMessage("Maintainer: Tim Riker <Tim@Rikers.org>");
  // print OpenGL renderer
  controlPanel->addMessage((const char*)glGetString(GL_RENDERER));

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
  for (i = 0; i < prototypeExplosions.getLength(); i++)
    delete prototypeExplosions[i];
  prototypeExplosions.removeAll();
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
