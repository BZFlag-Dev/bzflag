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

// interface header
#include "playing.h"
#include "guiplaying.h"

// system includes
#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#  include <utime.h>
#endif

// common headers
#include "AccessList.h"
#include "AutoHunt.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "BzfMedia.h"
#include "bzsignal.h"
#include "CacheManager.h"
#include "CollisionManager.h"
#include "CommandsStandard.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "GameTime.h"
#include "KeyManager.h"
#include "LinkManager.h"
#include "MsgStrings.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "PlatformFactory.h"
#include "ServerList.h"
#include "TextUtils.h"
#include "TimeBomb.h"
#include "TimeKeeper.h"
#include "WordFilter.h"
#include "bz_md5.h"
#include "vectors.h"
#include "version.h"

// common client headers
#include "ClientIntangibilityManager.h"
#include "FlashClock.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "RobotPlayer.h"
#include "Roster.h"
#include "ShotStats.h"
#include "GameTime.h"
#include "WorldBuilder.h"
#include "WorldPlayer.h"
#include "World.h"

// gui headers
#include "BackgroundRenderer.h"
#include "BillboardSceneNode.h"
#include "FlagSceneNode.h"
#include "QuadWallSceneNode.h"
#include "SphereSceneNode.h"
#include "TankGeometryMgr.h"
#include "TextureManager.h"
#include "ZSceneDatabase.h"

// local implementation headers
#include "AutoPilot.h"
#include "AnsiCodes.h"
#include "Daylight.h"
#include "Downloads.h"
#include "EffectsRenderer.h"
#include "ExportInformation.h"
#include "ForceFeedback.h"
#include "HUDDialogStack.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "MainMenu.h"
#include "RadarRenderer.h"
#include "SceneBuilder.h"
#include "ScoreboardRenderer.h"
#include "TrackMarks.h"
#include "VerticalSync.h"

#include "bzflag.h"
#include "commands.h"
#include "motd.h"

#include "sound.h"

#include "clientvars.h"


// FIXME: Any code surrounded by "if (!headless)" is unsafely assuming that
// it's operating in a context where graphics and sound are available.
bool headless = false;

static MainMenu *mainMenu;

static RadarRenderer *radar = NULL;
static ScoreboardRenderer *scoreboard = NULL;
static SceneDatabaseBuilder *sceneBuilder = NULL;
static bool firstLife = false;
static BZDB_bool showDrawFPS("showDrawFPS");
static BZDB_bool showDrawTime("showDrawTime");
static bool unmapped = false;
static int preUnmapFormat = -1;
static float testVideoFormatTimer = 0.0f;
static int testVideoPrevFormat = -1;

static std::vector<BillboardSceneNode*> explosions;
static std::vector<BillboardSceneNode*> prototypeExplosions;
static bool wasRabbit = false;

static bool forcedControls = false;
static float forcedSpeed  = 0.0f;
static float forcedAngVel = 0.0f;

static void setHuntTarget();

static ResourceGetter *resourceDownloader = NULL;

// Far and Near Frustum clipping planes
static const float FarPlaneScale = 1.5f; // gets multiplied by BZDB_WORLDSIZE
static const float FarPlaneDefault = FarPlaneScale * 800.0f;
static const float FarDeepPlaneScale = 10.0f;
static const float FarDeepPlaneDefault = FarPlaneDefault * FarDeepPlaneScale;
static const float NearPlaneNormal = 1.0f;
static const float NearPlaneClose = 0.25f; // for drawing in the cockpit
static bool FarPlaneCull = false;
static float FarPlane = FarPlaneDefault;
static float FarDeepPlane = FarDeepPlaneDefault;
static float NearPlane = NearPlaneNormal;

// cache _syncTime BZDB value
static BZDB_float bzdbSyncTime("_syncTime"); // AKA BZDBNAMES.SYNCTIME

#ifdef ROBOT
void handleMyTankKilled(int reason);
#endif


// to simplify code shared between bzrobots and bzflag
// - in bzflag, this shows the error on the HUD
void showError(const char *msg, bool flush)
{
  HUDDialogStack::get()->setFailedMessage(msg);
  if(flush) {
    drawFrame(0.0f);
  }
}

// - in bzflag, this shows the error on the HUD
void showMessage(const std::string& line)
{
  controlPanel->addMessage(line);
}

void showMessage(const std::string& line, ControlPanel::MessageModes mode)
{
  controlPanel->addMessage(line,mode);
}


// access silencePlayers from bzflag.cxx
std::vector<std::string> &getSilenceList()
{
  return silencePlayers;
}


// try to select the next recipient in the specified direction
// eventually avoiding robots
void selectNextRecipient(bool forward, bool robotIn)
{
  LocalPlayer *my = LocalPlayer::getMyTank();
  const Player *recipient = my->getRecipient();
  int rindex;
  if (!recipient) {
    rindex = - 1;
    forward = true;
  } else {
    const PlayerId id = recipient->getId();
    rindex = lookupPlayerIndex(id);
  }
  int i = rindex;
  while (true) {
    if (forward) {
      i++;
      if (i == curMaxPlayers) {
	// if no old rec id we have just ended our search
	if (recipient == NULL)
	  break;
	else
	  // wrap around
	  i = 0;
      }
    } else {
      if (i == 0)
	// wrap around
	i = curMaxPlayers;
      i--;
    }
    if (i == rindex)
      break;
    if (remotePlayers[i] &&
      (robotIn || remotePlayers[i]->getPlayerType() != ComputerPlayer)) {
	my->setRecipient(remotePlayers[i]);
	break;
    }
  }
}

//
// should we grab the mouse?
//

bool shouldGrabMouse()
{
  return (mainWindow->getFullscreen() && !unmapped &&
    (myTank == NULL || !myTank->isPaused() || myTank->isAutoPilot()));
}


//
// some simple global functions
//

void warnAboutMainFlags()
{
  // warning message for hidden flags
  if (!BZDBCache::displayMainFlags) {
    std::vector<std::string> keys = KEYMGR.getKeysFromCommand("toggleFlags main", true);

    if (keys.size() != 0) {
      addMessage(NULL,
	TextUtils::format("%sFlags on field hidden.  To show them, hit \"%s%s%s\"",
	ColorStrings[YellowColor],
	ColorStrings[WhiteColor],
	keys[0].c_str(),
	ColorStrings[YellowColor]));
    } else {
      addMessage(NULL,
	TextUtils::format("%sFlags on field hidden.  To show them, bind a key to 'Toggle Flags on Field'.",
	ColorStrings[YellowColor]));
    }
  }
}


void warnAboutRadarFlags()
{
  if (!BZDB.isTrue("displayRadarFlags")) {
    std::vector<std::string> keys = KEYMGR.getKeysFromCommand("toggleFlags radar", true);

    if (keys.size() != 0) {
      addMessage(NULL,
	TextUtils::format("%sFlags on radar hidden.  To show them, hit \"%s%s%s\"",
	ColorStrings[YellowColor],
	ColorStrings[WhiteColor],
	keys[0].c_str(),
	ColorStrings[YellowColor]));
    } else {
      addMessage(NULL,
	TextUtils::format("%sFlags on radar hidden.  To show them, bind a key to 'Toggle Flags on Radar'.%s",
	ColorStrings[YellowColor],
	ColorStrings[YellowColor]));
    }
  }
}


void warnAboutRadar()
{
  if (!BZDB.isTrue("displayRadar")) {
    std::vector<std::string> keys = KEYMGR.getKeysFromCommand("toggleRadar", true);

    if (keys.size() != 0) {
      addMessage(NULL,
	TextUtils::format("%sTo toggle the radar, hit \"%s%s%s\"",
	ColorStrings[YellowColor],
	ColorStrings[WhiteColor],
	keys[0].c_str(),
	ColorStrings[YellowColor]));
    } else {
      addMessage(NULL,
	TextUtils::format("%sTo toggle the radar, bind a key to 'Toggle Radar'",
	ColorStrings[YellowColor]));
    }
  }
}


void warnAboutConsole()
{
  if (!BZDB.isTrue("displayConsole")) {
    // We can't use a console message for this one.
    std::vector<std::string> keys = KEYMGR.getKeysFromCommand("toggleConsole", true);

    if (keys.size() != 0) {
      hud->setAlert(3, TextUtils::format("%sTo toggle the console, hit \"%s%s%s\"",
	ColorStrings[YellowColor],
	ColorStrings[WhiteColor],
	keys[0].c_str(),
	ColorStrings[YellowColor]).c_str(),
	2.0f, true);
    } else {
      hud->setAlert(3, TextUtils::format("%sTo toggle the console, bind a key to 'Toggle Console'",
	ColorStrings[YellowColor]).c_str(),
	2.0f, true);
    }
  }
}


static void warnAboutSlowMotion()
{
  static bool notified = false;
  const bool slow = BZDB.isTrue("slowMotion");

  /* if it's not slow, then nothing to warn */
  if (!slow) {
    notified = false;
    return;
  }

  /* it's slow, so see if we need to warn */
  if (!notified) {
    std::vector<std::string> keys = KEYMGR.getKeysFromCommand("toggle slowMotion", true);

    if (keys.size() != 0) {
      addMessage(NULL,
	TextUtils::format("%sTo toggle slow tank controls, hit \"%s%s%s\"",
	ColorStrings[YellowColor],
	ColorStrings[WhiteColor],
	keys[0].c_str(),
	ColorStrings[YellowColor]));
      notified = true;
    }
  }
}


inline bool isViewTank(Player *tank)
{
  return (tank &&
    ((tank == LocalPlayer::getMyTank()) ||
    ((ROAM.isRoaming()) &&
    (ROAM.getMode() == Roaming::roamViewFP) &&
    (ROAM.getTargetTank() == tank))));
}


BzfDisplay *getDisplay()
{
  return display;
}


MainWindow *getMainWindow()
{
  return mainWindow;
}


RadarRenderer *getRadarRenderer()
{
  return radar;
}


void forceControls(bool enabled, float speed, float angVel)
{
  forcedControls = enabled;
  if (enabled) {
    forcedSpeed  = speed;
    forcedAngVel = angVel;
  }
}


void setSceneDatabase()
{
  // FIXME - test the zbuffer here
  // delete the old database
  RENDERER.setSceneDatabase(NULL);

  if (world == NULL) {
    return;
  }

  // make the scene, and record the processing time
  TimeKeeper startTime = TimeKeeper::getCurrent();
  SceneDatabase *scene = sceneBuilder->make(world);
  float elapsed = float(TimeKeeper::getCurrent() - startTime);

  // print debugging info
  if (BZDBCache::zbuffer) {
    logDebugMessage(2, "ZSceneDatabase processed in %.3f seconds.\n", elapsed);
  } else {
    logDebugMessage(2, "BSPSceneDatabase processed in %.3f seconds.\n", elapsed);
  }

  // set the scene
  RENDERER.setSceneDatabase(scene);
}




bool setVideoFormat(int index, bool test)
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




void joinGame()
{
  if (joiningGame) {
    if(worldDownLoader) {
      worldDownLoader->stop();
      joiningGame = false;
      showError("Download stopped by user action");
    }
  }
  joinRequested = true;

  thirdPersonVars.load();
}


//
// handle signals that should kill me quickly
//
static void dying(int sig)
{
  bzSignal(sig, SIG_DFL);
  if (display) {
    display->setDefaultResolution();
  }
  raise(sig);
}




//
// user input handling
//

#if defined(DEBUG)
#define FREEZING
#endif
#if defined(FREEZING)
static bool motionFreeze = false;
#endif

static enum { None, Left, Right, Up, Down } keyboardMovement;
static int shiftKeyStatus;


static bool doKeyCommon(const BzfKeyEvent &key, bool pressed)
{
  keyboardMovement = None;
  shiftKeyStatus   = key.shift;

  const std::string cmd = KEYMGR.get(key, pressed);
  if (key.chr == 27) {
    if (pressed) {
      mainMenu->createControls();
      HUDDialogStack::get()->push(mainMenu);
    }
    return true;
  } else if (scoreboard->getHuntState() == ScoreboardRenderer::HUNT_SELECTING) {
    if (key.button == BzfKeyEvent::Down || KEYMGR.get(key, true) == "identify") {
      if (pressed)
	scoreboard->setHuntNextEvent();
      return true;
    } else if (key.button == BzfKeyEvent::Up || KEYMGR.get(key, true) == "drop") {
      if (pressed)
	scoreboard->setHuntPrevEvent();
      return true;
    } else if (KEYMGR.get(key, true) == "fire") {
      if (pressed) {
	scoreboard->setHuntSelectEvent();
      }
      return true;
    }
  }

  // Check for driving keys
  std::string cmdDrive = cmd;
  if (cmdDrive.empty()) {
    BzfKeyEvent cleanKey = key;
    cleanKey.shift = 0;
    cmdDrive = KEYMGR.get(cleanKey, pressed);
  }
  if (cmdDrive == "turn left") {
    keyboardMovement = Left;
  } else if (cmdDrive == "turn right") {
    keyboardMovement = Right;
  } else if (cmdDrive == "drive forward") {
    keyboardMovement = Up;
  } else if (cmdDrive == "drive reverse") {
    keyboardMovement = Down;
  }

  if (myTank && myTank->isAlive() && !HUDDialogStack::get()->isActive() &&
      (myTank->getInputMethod() != LocalPlayer::Keyboard) && pressed &&
      (keyboardMovement != None) && BZDB.isTrue("allowInputChange")) {
    myTank->setInputMethod(LocalPlayer::Keyboard);
  }

  if (myTank && (myTank->getInputMethod() == LocalPlayer::Keyboard)) {
    switch (keyboardMovement) {
      case Left:  { myTank->setKey(BzfKeyEvent::Left,  pressed); break; }
      case Right: { myTank->setKey(BzfKeyEvent::Right, pressed); break; }
      case Up:    { myTank->setKey(BzfKeyEvent::Up,    pressed); break; }
      case Down:  { myTank->setKey(BzfKeyEvent::Down,  pressed); break; }
      case None:  { break; }
    }
  }

  if (!cmd.empty()) {
    if (cmd == "fire") {
      fireButton = pressed;
    }
    roamButton = pressed;
    if (keyboardMovement == None) {
      std::string result = CMDMGR.run(cmd);
      if (!result.empty()) {
	std::cerr << result << std::endl;
      }
    }
    return true;
  }

  // if we don't have a tank, the following key commands don't apply
  if (!myTank) {
    return false;
  }

  // built-in unchangeable keys.  only perform if not masked.
  // TODO: not localizable
  switch (key.chr) {
#if defined(DEBUG_RENDERING)
    // for testing forced recreation of OpenGL context
    case 'X': {
      if (pressed && ((shiftKeyStatus & BzfKeyEvent::AltKey) != 0)) {
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
      return true;
    }
#endif // DEBUG_RENDERING
  } // end switch on key

  // Shot/Accuracy Statistics display
  if (key.button == BzfKeyEvent::Home && pressed) {
    HUDDialogStack::get()->push(new ShotStats);
    return true;
  }

  return false;
}


static void doKeyNotPlaying(const BzfKeyEvent&, bool, bool)
{
}


static void doKeyPlaying(const BzfKeyEvent &key, bool pressed, bool haveBinding)
{
#if defined(FREEZING)
  if ((key.chr == '`' && pressed) && (!haveBinding && key.shift)) {
    // toggle motion freeze
    motionFreeze = !motionFreeze;
    if (motionFreeze) {
      addMessage(NULL, "The tank's motion is now frozen! ... Press Shift+` to unfreeze");
    }
    return;
  }
#endif

  const int altORctrl = (BzfKeyEvent::ControlKey | BzfKeyEvent::AltKey);
  if (!haveBinding &&
      (key.chr == 0) &&
      (key.button >= BzfKeyEvent::F1) &&
      (key.button <= BzfKeyEvent::F10) &&
      ((key.shift & altORctrl) != 0)) {
      // [Ctrl]-[Fx] is message to team
      // [Alt]-[Fx] is message to all
      if (pressed) {
	char name[32];
	int msgno = (key.button - BzfKeyEvent::F1) + 1;
	PlayerId  to;
	if ((key.shift == BzfKeyEvent::ControlKey) && world->allowTeams()) {
	  sprintf(name, "quickTeamMessage%d", msgno);
	  to = myTank->getTeam();
	} else {
	  sprintf(name, "quickMessage%d", msgno);
	  to = AllPlayers;
	}
	if (BZDB.isSet(name)) {
	  char messageBuffer[MessageLen];
	  memset(messageBuffer, 0, MessageLen);
	  strncpy(messageBuffer, BZDB.get(name).c_str(), MessageLen);
	  serverLink->sendMessage(to, messageBuffer);
	}
      }
  }
}


static void doKey(const BzfKeyEvent &key, bool pressed)
{
  if (myTank) {
    const std::string cmd = KEYMGR.get(key, pressed);
    if (cmd == "jump") {
      myTank->setJumpPressed(pressed);
    }
  }

  if (HUDui::getFocus()) {
    if (( pressed && HUDui::keyPress(key)) ||
        (!pressed && HUDui::keyRelease(key)))
      return;
  }

  bool haveBinding = doKeyCommon(key, pressed);

  if (!myTank) {
    doKeyNotPlaying(key, pressed, haveBinding);
  } else {
    doKeyPlaying(key, pressed, haveBinding);
  }
}


static void doMotion()
{
  float speed  = 0.0f;
  float angVel = 0.0f;

  int keyboardSpeed  = myTank->getSpeed();
  int keyboardAngVel = myTank->getRotation();

  // mouse is default steering method; query mouse pos always, not doing so
  // can lead to stuttering movement with X and software rendering (uncertain why)
  int mx = 0, my = 0;
  mainWindow->getMousePosition(mx, my);

  // determine if joystick motion should be used instead of mouse motion
  // when the player bumps the mouse, LocalPlayer::getInputMethod return Mouse;
  // make it return Joystick when the user bumps the joystick
  if (mainWindow->haveJoystick()) {
    int jx = 0, jy = 0;
    mainWindow->getJoyPosition(jx, jy);
    if (myTank->getInputMethod() == LocalPlayer::Joystick) {
      // if we are using the joystick, use its coordinates
      mx = jx; my = jy;
    } else if (BZDB.isTrue("allowInputChange")) {
      // if we aren't using the joystick, but it's moving, start using it
      if ((jx < -100) || (jx > 100) || (jy < -100) || (jy > 100)) {
	myTank->setInputMethod(LocalPlayer::Joystick);
      }
    }
  }

#if defined(FREEZING)
  if (motionFreeze) {
    return;
  }
#endif

  if (forcedControls) {
    speed  = forcedSpeed;
    angVel = forcedAngVel;
  }
  else if (myTank->isAutoPilot()) {
    doAutoPilot(angVel, speed);
  }
  else if (myTank->getInputMethod() == LocalPlayer::Keyboard) {
    angVel = (float)keyboardAngVel;
    speed  = (float)keyboardSpeed;
    if (speed < 0.0f) {
      speed *= 0.5f;
    }
  }
  else { // both mouse and joystick
    int noMotionSize  = hud->getNoMotionSize();
    int maxMotionSize = hud->getMaxMotionSize();

    if (myTank->getInputMethod() == LocalPlayer::Joystick) {
      noMotionSize = 0; // joystick deadzone is specified at platform level
      maxMotionSize = 1000; // joysticks read 0 - 1000
    }

    // calculate desired angular velocity
    if (keyboardAngVel) {
      angVel = float(keyboardAngVel);
    } else if (mx < -noMotionSize) {
      angVel = float(-mx - noMotionSize) / float(maxMotionSize - noMotionSize);
    } else if (mx > noMotionSize) {
      angVel = -float(mx - noMotionSize) / float(maxMotionSize - noMotionSize);
    }

    // calculate desired speed
    if (keyboardSpeed) {
      speed = float(keyboardSpeed);
      if (speed < 0.0f) {
	speed *= 0.5f;
      }
    } else if (my < -noMotionSize) {
      speed = float(-my - noMotionSize) / float(maxMotionSize - noMotionSize);
    } else if (my > noMotionSize) {
      speed = -float(my - noMotionSize) / float(maxMotionSize - noMotionSize);
    } else {
      speed = 0.0f;
    }

    // parabolic control
    static BZDB_float parabolicSlope("parabolicSlope");
    if ((parabolicSlope != 1.0f) && !isnan((float)parabolicSlope)) {
      float s1 = parabolicSlope;
           if (s1 > +1.0e6f) { s1 = +1.0e6f; }
      else if (s1 < -1.0e6f) { s1 = -1.0e6f; }
      const float s2 = (1.0f - s1);
      if (speed >= 0.0f) {
        speed *= ((s2 * speed) + s1);
      } else {
        speed *= -2.0f; // scale to (0.0f,1.0f]  (for the -0.5f clamping)
        speed *= ((s2 * speed) + s1);
        speed *= -0.5f;
      }
      if (angVel >= 0.0f) {
        angVel *= ((+s2 * angVel) + s1);
      } else {
        angVel *= ((-s2 * angVel) + s1);
      }
    }
  }

  // slow motion modifier
  warnAboutSlowMotion();
  if (BZDB.isTrue("slowMotion")) {
    speed  *= 0.5f;
    angVel *= 0.5f;
  }

  // FOV modifier
  if (BZDB.isTrue("slowBinoculars")) {
    angVel *= BZDB.eval("displayFOV") / 60.0f;
  }

  // speed clamp
  if (speed < -0.5f) {
    speed = -0.5f;
  } else if (speed > +1.0f) {
    speed = +1.0f;
  }

  // angVel clamp
  if (angVel < -1.0f) {
    angVel = -1.0f;
  } else if (angVel > +1.0f) {
    angVel = +1.0f;
  }

  /* see if controls are reversed */
  if (myTank->getFlag() == Flags::ReverseControls) {
    speed  = -speed;
    angVel = -angVel;
  }

  myTank->setDesiredSpeed(speed);
  myTank->setDesiredAngVel(angVel);
}


static void mouseClamp(const BzfMotionEvent& event)
{
  // only clamp when it might be useful
  if ((myTank == NULL) || !myTank->isAlive() ||
      myTank->isPaused() || (myTank->getTeam() == ObserverTeam)) {
    return;
  }

  // do not clamp if CTRL is being held down
  bool alt, ctrl, shift;
  display->getModState(alt, ctrl, shift);
  if (ctrl) {
    return;
  }

  // calculate the max motion size in pixels
  const int clampFudge = 2;
  const int w  = mainWindow->getWidth();
  const int vh = mainWindow->getViewHeight();
  const float xScale = (float)w  / (float) MinX;
  const float yScale = (float)vh / (float) MinY;
  const float scale = (xScale < yScale) ? xScale : yScale;
  const float effScale =  scale * (0.7f + RENDERER.getMaxMotionFactor() / 16.667f);
  const int maxMotionSize = (int)((float)MaxMotionSize * effScale);
  const int pixels = maxMotionSize + clampFudge;

  // calculate the clamp extents
  const int xc = (w / 2);
  const int xn = xc - pixels;
  const int xp = xc + pixels;
  const int yc = (vh / 2);
  const int yn = yc - pixels;
  const int yp = yc + pixels;

  // clamp, as required
  if (event.x < xn) { mainWindow->getWindow()->warpMouse(xn, event.y); }
  if (event.x > xp) { mainWindow->getWindow()->warpMouse(xp, event.y); }
  if (event.y < yn) { mainWindow->getWindow()->warpMouse(event.x, yn); }
  if (event.y > yp) { mainWindow->getWindow()->warpMouse(event.x, yp); }
}


static void doEvent(BzfDisplay *disply)
{
  BzfEvent event;
  if (!disply->getEvent(event)) {
    return;
  }

  switch (event.type) {
    case BzfEvent::Quit: {
      CommandsStandard::quit();
      break;
    }
    case BzfEvent::Redraw: {
      mainWindow->getWindow()->callExposeCallbacks();
      RENDERER.setExposed();
      break;
    }
    case BzfEvent::Resize: {
      if ((mainWindow->getWidth()  != event.resize.width) ||
          (mainWindow->getHeight() != event.resize.height)) {
        mainWindow->getWindow()->setSize(event.resize.width, event.resize.height);
        mainWindow->getWindow()->callResizeCallbacks();
      }
      break;
    }
    case BzfEvent::Map: {
      // window has been mapped.  this normally occurs when the game
      // is uniconified.  if the player was paused because of an unmap
      // then resume.
      if (pausedByUnmap && BZDB.isTrue("unpauseForMinimize")) {
        // FIXME -- send a MsgPause to cancle pauseOnMinimize initiated pauses
        //          if not paused, and the counter is running
        pausedByUnmap = false;
        pauseCountdown = 5.0f;
        if (myTank && myTank->isAlive() && myTank->isPaused()) {
          serverLink->sendPaused(false);
        }
      }

      // restore the resolution we want if full screen
      if (mainWindow->getFullscreen()) {
        if (preUnmapFormat != -1) {
          disply->setResolution(preUnmapFormat);
          mainWindow->warpMouse();
          mainWindow->deiconify();
        }
      }

      // restore the sound
      if (savedVolume != -1) {
        SOUNDSYSTEM.setVolume(savedVolume * 0.1f);
        savedVolume = -1;
      }

      unmapped = false;
      if (shouldGrabMouse()) {
        mainWindow->grabMouse();
      }

      if (myTank) {
        // clear the jump polarity
        myTank->setJumpPressed(false);
      }

      break;
    }
    case BzfEvent::Unmap: {
      // begin pause countdown when unmapped if:  we're not already
      // paused because of an unmap (shouldn't happen), we're not
      // already counting down to pausing, we're alive, and we're not
      // already paused.
      if (!pausedByUnmap && (pauseCountdown == 0.0f) &&
          myTank && myTank->isAlive() && !myTank->isPaused() &&
          !myTank->isAutoPilot() && BZDB.isTrue("pauseOnMinimize")) {
        pauseCountdown = 5.0f;
        pausedByUnmap = true;
        serverLink->sendPaused(true);
      }

      // ungrab the mouse if we're running full screen
      if (mainWindow->getFullscreen()) {
        preUnmapFormat = -1;
        if (disply->getNumResolutions() > 1) {
          preUnmapFormat = disply->getResolution();
          disply->setDefaultResolution();
        }
      }

      // turn off the sound
      if (savedVolume == -1) {
        savedVolume = (int)(SOUNDSYSTEM.getVolume() * 10);
        SOUNDSYSTEM.setVolume(0);
      }

      unmapped = true;
      mainWindow->ungrabMouse();
      break;
    }
    case BzfEvent::KeyUp: {
      doKey(event.keyUp, false);
      break;
    }
    case BzfEvent::KeyDown: {
      doKey(event.keyDown, true);
      break;
    }
    case BzfEvent::MouseMove: {
      if (myTank && myTank->isAlive() &&
          (myTank->getInputMethod() != LocalPlayer::Mouse) &&
          (BZDB.isTrue("allowInputChange"))) {
        myTank->setInputMethod(LocalPlayer::Mouse);
      }
      static BZDB_bool bzdbMouseClamp("mouseClamp");
      if (bzdbMouseClamp) {
        mouseClamp(event.mouseMove);
      }
      break;
    }
    default: {
      /* unset */
      break;
    }
  }
}






void updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
  * `>= bestScore' to `> bestScore' if you want to share the number
  * one spot. */
  bool anyPlayers = false;
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i]) {
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
      if (remotePlayers[i] && remotePlayers[i]->getScore() >= bestScore) {
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

      World *_world = World::getWorld();
      if (_world && myTank && Team::isColorTeam(myTank->getTeam())) {
	const Team &myTeam = _world->getTeam(int(myTank->getTeam()));
	bestScore = myTeam.won - myTeam.lost;
	haveBest = true;
	for (i = 0; i < NumTeams; i++) {
	  if (i == int(myTank->getTeam())) continue;
	  const Team &team = _world->getTeam(i);
	  if (team.size > 0 && team.won - team.lost >= bestScore) {
	    haveBest = false;
	    break;
	  }
	}
	hud->setTeamHasHighScore(haveBest);
      } else {
	hud->setTeamHasHighScore(false);
      }
}

static void updateFlag(FlagType *flag)
{
  if (flag == Flags::Null) {
    hud->setColor(1.0f, 0.625f, 0.125f);
    hud->setAlert(2, NULL, 0.0f);
  } else {
    const fvec4& color = flag->getColor();
    hud->setColor(color.r, color.g, color.b);
    hud->setAlert(2, flag->flagName.c_str(), 3.0f, flag->endurance == FlagSticky);
  }

  if (BZDB.isTrue("displayFlagHelp"))
    hud->setFlagHelp(flag, FlagHelpDuration);

  World *_world = World::getWorld();
  if ((!radar && !myTank) || !_world) return;

  radar->setJammed(flag == Flags::Jamming);
  hud->setAltitudeTape(myTank->canJump());
}


void notifyBzfKeyMapChanged()
{
  std::string restartLabel = BundleMgr::getCurrentBundle()->getLocalString("Right Mouse");
  std::vector<std::string> keys = KEYMGR.getKeysFromCommand("restart", false);

  if (keys.size() == 0) {
    // found nothing on down binding, so try up
    keys = KEYMGR.getKeysFromCommand("identify", true);
    if (keys.size() == 0) {
      std::cerr << "There does not appear to be any key bound to enter the game" << std::endl;
    }
  }

  if (keys.size() >= 1) {
    // display single letter keys as a quoted lowercase letter
    if (keys[0].size() == 1) {
      restartLabel = '\"';
      restartLabel += tolower(keys[0][0]);
      restartLabel += '\"';
    } else {
      restartLabel = BundleMgr::getCurrentBundle()->getLocalString(keys[0]);
    }
  }

  // only show the first 2 keys found to keep things simple
  if (keys.size() > 1) {
    restartLabel.append(BundleMgr::getCurrentBundle()->getLocalString(" or "));
    // display single letter keys as quoted lowercase letter
    if (keys[1].size() == 1) {
      restartLabel += '\"';
      restartLabel += tolower(keys[1][0]);
      restartLabel += '\"';
    } else {
      restartLabel.append(keys[1]);
    }
  }

  hud->setRestartKeyLabel(restartLabel);
}






int curlProgressFunc(void * /*clientp*/,
		     double dltotal, double dlnow,
		     double /*ultotal*/, double /*ulnow*/)
{
  // FIXME: beaucoup cheeze here in the aborting style
  // we should be using async dns and multi-curl

  // run an inner main loop to check if we should abort
  BzfEvent event;
  if (display->isEventPending() && display->peekEvent(event)) {
    switch (event.type) {
    case BzfEvent::Quit:
      return 1; // terminate the curl call
    case BzfEvent::KeyDown:
      display->getEvent(event); // flush the event
      if (event.keyDown.chr == 27)
	return 1; // terminate the curl call

      break;
    case BzfEvent::KeyUp:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::MouseMove:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::Unset:
    case BzfEvent::Map:
    case BzfEvent::Unmap:
    case BzfEvent::Redraw:
    case BzfEvent::Resize:
      // leave the event, it might be important
      break;
    }
  }

  // update the status
  double percentage = 0.0;
  if ((int)dltotal > 0)
    percentage = 100.0 * dlnow / dltotal;

  showError(TextUtils::format("%2.1f%% (%i/%i)", percentage, (int) dlnow, (int) dltotal).c_str());

  return 0;
}


static void textureDoneFunc(const trResourceItem& item, bool success)
{
  printf("FIXME -  textureDoneFunc() for %s, %s\n", item.URL.c_str(),
          success ? "true" : "false");
  if (!success) {
    return;
  }
  TextureManager::instance().reloadTextureImage(item.filePath);
}


void handleResourceFetch (void *msg)
{
  if (BZDB.isTrue("_noRemoteFiles")) {
    return;
  }

  uint16_t numItems;
  void *buf;

  buf = nboUnpackUInt16 (msg, numItems);    // the type

  for (int i = 0; i < numItems; i++) {
    uint16_t itemType;
    char buffer[MessageLen];
    uint16_t stringLen;
    uint8_t overwriteChar;

    trResourceItem item;

    buf = nboUnpackUInt8 (buf, overwriteChar);
    item.overwrite = (overwriteChar != 0);

    buf = nboUnpackUInt16 (buf, itemType);
    item.resType = (teResourceType) itemType;
    if (item.resType == eImage) {
      item.doneFunc = textureDoneFunc;
    }

    // URL
    buf = nboUnpackUInt16 (buf, stringLen);
    buf = nboUnpackString (buf, buffer, stringLen);

    buffer[stringLen] = '\0';
    item.URL = buffer;

    if (!CACHEMGR.isCacheFileType(item.URL))
      continue; // not a supported URL type

    item.filePath = CACHEMGR.getLocalName(item.URL);

    std::vector < std::string > temp =
      TextUtils::tokenize (item.URL, std::string ("\\/"));
    item.fileName = temp[temp.size () - 1]; // unused?

    std::string hostname;
    parseHostname (item.URL, hostname);
    if (Downloads::instance().authorizedServer (hostname)) {
      if (!resourceDownloader)
	resourceDownloader = new ResourceGetter;

      resourceDownloader->addResource (item);
      printf("FIXME - fetch started for %s, %s\n", item.URL.c_str(),
             item.doneFunc ? "doneFunc" : "no-doneFunc");
      
    }
  }
}


void handleCustomSound(void *msg)
{
  // bail out if we don't want to do remote sounds
  if (BZDB.isTrue("_noRemoteSounds")) {
    return;
  }

  uint8_t soundType;
  uint8_t bits;
  std::string soundName;

  msg = nboUnpackUInt8(msg, soundType);
  if (soundType != LocalCustomSound) {
    return;
  }

  msg = nboUnpackUInt8(msg, bits);

  msg = nboUnpackStdString(msg, soundName);

  // sound parameters
  float gain  = 1.0f;
  float pitch = 1.0f;
  fvec3 pos, vel, dir;
  const float* posPtr = NULL;
  const float* velPtr = NULL;
  const float* dirPtr = NULL;
  float innerCone = 0.0f;
  float outerCone = 360.0f;
  float outerGain = 1.0;

  if ((bits & SoundGain) != 0) {
    msg = nboUnpackFloat(msg, gain);
  }
  if ((bits & SoundPitch) != 0) {
    msg = nboUnpackFloat(msg, pitch);
  }
  if ((bits & SoundPosition) != 0) {
    posPtr = pos;
    msg = nboUnpackFVec3(msg, pos);
  }
  if ((bits & SoundVelocity) != 0) {
    velPtr = vel;
    msg = nboUnpackFVec3(msg, vel);
  }
  if ((bits & SoundDirection) != 0) {
    dirPtr = dir;
    msg = nboUnpackFVec3(msg, dir);
    msg = nboUnpackFloat(msg, innerCone);
    msg = nboUnpackFloat(msg, outerCone);
    msg = nboUnpackFloat(msg, outerGain);
  }

  std::string localName = soundName;
  if (CACHEMGR.isCacheFileType(soundName)) {
    localName = CACHEMGR.getLocalName(soundName);
  }
  SOUNDSYSTEM.play(localName, posPtr, false, (posPtr == NULL));
}


void handleTimeUpdate(void *msg)
{
  int32_t timeLeft;
  msg = nboUnpackInt32(msg, timeLeft);
  hud->setTimeLeft(timeLeft);
  if (timeLeft == 0) {
    gameOver = true;
    myTank->explodeTank();
    showMessage("Time Expired");
    hud->setAlert(0, "Time Expired", 10.0f, true);
#ifdef ROBOT
    for (int i = 0; i < numRobots; i++)
      if (robots[i])
	robots[i]->explodeTank();
#endif
  } else if (timeLeft < 0) {
    hud->setAlert(0, "Game Paused", 10.0f, true);
  }
}


void handleScoreOver(void *msg)
{
  // unpack packet
  PlayerId id;
  int16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackInt16(msg, team);
  Player *player = lookupPlayer(id);

  // make a message
  std::string msg2;
  if (team == (int16_t)NoTeam) {
    // a player won
    if (player) {
      msg2 = TextUtils::format("%s (%s) won the game",
      player->getCallSign(),
      Team::getName(player->getTeam()));
    } else {
      msg2 = "[unknown player] won the game";
    }
  } else {
    msg2 = TextUtils::format("%s won the game",
      Team::getName(TeamColor(team)));
  }

  gameOver = true;
  hud->setTimeLeft((uint32_t)~0);
  myTank->explodeTank();
  showMessage(msg2);
  hud->setAlert(0, msg2.c_str(), 10.0f, true);

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    if (robots[i])
      robots[i]->explodeTank();
  }
#endif
}


void handleAliveMessage(void *msg)
{
  PlayerId id;
  fvec3 pos;
  float forward;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackFloat(msg, forward);
  int playerIndex = lookupPlayerIndex(id);

  if ((playerIndex < 0) && (playerIndex != -2)) {
    return;
  }
  Player *tank = getPlayerByIndex(playerIndex);
  if (tank == NULL) {
    return;
  }

  if (tank == myTank) {
    wasRabbit = tank->getTeam() == RabbitTeam;
    myTank->restart(pos, forward);
    myTank->setShotType(StandardShot);
    firstLife = false;
    justJoined = false;

    if (!myTank->isAutoPilot())
      mainWindow->warpMouse();

    hud->setAltitudeTape(myTank->canJump());
  }
#ifdef ROBOT
  else if (tank->getPlayerType() == ComputerPlayer) {
    for (int r = 0; r < numRobots; r++) {
      if (robots[r] && robots[r]->getId() == playerIndex) {
        robots[r]->restart(pos, forward);
        setRobotTarget(robots[r]);
        break;
      }
    }
  }
#endif

  if (SceneRenderer::instance().useQuality() >= _MEDIUM_QUALITY) {
    if (((tank != myTank) && ((ROAM.getMode() != Roaming::roamViewFP) ||
                              (tank != ROAM.getTargetTank())))
        || BZDB.isTrue("enableLocalSpawnEffect")) {
      if (myTank->getFlag() != Flags::Colorblindness) {
        static fvec4 cbColor(1.0f, 1.0f, 1.0f, 1.0f);
        EFFECTS.addSpawnEffect(cbColor, pos);
      } else {
        EFFECTS.addSpawnEffect(tank->getColor(), pos);
      }
    }
  }

  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  tank->setStatus(tank->getStatus() | PlayerState::Alive);
  tank->move(pos, forward);
  tank->setVelocity(zero);
  tank->setAngularVelocity(0.0f);
  tank->setDeadReckoning(GameTime::getDRTime());
  tank->spawnEffect();
  if (tank == myTank) {
    myTank->setSpawning(false);
  }

  SOUNDSYSTEM.play(SFX_POP, pos, true, isViewTank(tank));
}


void handleAutoPilot(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  uint8_t autopilot;
  nboUnpackUInt8(msg, autopilot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  tank->setAutoPilot(autopilot != 0);

  if (tank != myTank) {
    addMessage(tank, autopilot ? "Roger taking controls" : "Roger releasing controls");
  }
  else {
    if (autopilot == 0) {
      if (myTank->requestedAutopilot) {
        hud->setAlert(0, "autopilot disabled", 1.0f, true);
      } else {
        hud->setAlert(0, "manual drive enabled", 1.0f, true);
      }

      // grab mouse
      if (shouldGrabMouse()) {
        mainWindow->grabMouse();
      }

      myTank->requestedAutopilot = false;
    }
    else {
      if (myTank->requestedAutopilot) {
        hud->setAlert(0, "autopilot enabled", 1.0f, true);
      } else {
        hud->setAlert(0, "manual drive disabled", 1.0f, true);
      }

      // ungrab mouse
      mainWindow->ungrabMouse();
    }
  }
}




void handleKilledMessage(void *msg, bool human, bool &checkScores)
{
  PlayerId victim, killer;
  FlagType *flagType;
  int16_t shotId, reason;
  int phydrv = -1;
  msg = nboUnpackUInt8(msg, victim);
  msg = nboUnpackUInt8(msg, killer);
  msg = nboUnpackInt16(msg, reason);
  msg = nboUnpackInt16(msg, shotId);
  msg = FlagType::unpack(msg, flagType);
  if (reason == (int16_t)PhysicsDriverDeath) {
    int32_t inPhyDrv;
    msg = nboUnpackInt32(msg, inPhyDrv);
    phydrv = int(inPhyDrv);
  }
  BaseLocalPlayer *victimLocal = getLocalPlayer(victim);
  BaseLocalPlayer *killerLocal = getLocalPlayer(killer);
  Player *victimPlayer = lookupPlayer(victim);
  Player *killerPlayer = lookupPlayer(killer);

  if (victimPlayer == myTank) {
    // uh oh, i'm dead
    if (myTank->isAlive()) {
      serverLink->sendDropFlag(myTank->getPosition());
      myTank->setShotType(StandardShot);
      handleMyTankKilled(reason);
    }
  }

  if (victimLocal) {
    // uh oh, local player is dead
    if (victimLocal->isAlive())
      gotBlowedUp(victimLocal, GotKilledMsg, killer);
  } else if (victimPlayer) {
    victimPlayer->setExplode(TimeKeeper::getTick());
    const fvec3& pos = victimPlayer->getPosition();
    const bool localView = isViewTank(victimPlayer);
    if (reason == GotRunOver)
      SOUNDSYSTEM.play(SFX_RUNOVER, pos, killerLocal == myTank, localView);
    else
      SOUNDSYSTEM.play(SFX_EXPLOSION, pos, killerLocal == myTank, localView);

    const float victimMuzzle = victimPlayer->getMuzzleHeight();
    const fvec3 explodePos(pos.x, pos.y, pos.z + victimMuzzle);
    addTankExplosion(explodePos);

    EFFECTS.addDeathEffect(victimPlayer->getColor(), pos, victimPlayer->getAngle());
  }

  if (killerLocal) {
    // local player did it
    if (shotId >= 0)
      killerLocal->endShot(shotId, true); // terminate the shot

    if (victimPlayer && killerLocal != victimPlayer) {
      if ((victimPlayer->getTeam() == killerLocal->getTeam()) && (killerLocal->getTeam() != RogueTeam) && !(killerPlayer == myTank && wasRabbit) && World::getWorld()->allowTeams()) {
	// teamkill
	if (killerPlayer == myTank) {
	  hud->setAlert(1, "Don't kill teammates!!!", 3.0f, true);
	  SOUNDSYSTEM.play(SFX_KILL_TEAM);
	  if (myTank->isAutoPilot()) {
	    char meaculpa[MessageLen];
	    memset(meaculpa, 0, MessageLen);
	    strncpy(meaculpa,
	      "sorry, i'm just a silly machine",
	      MessageLen);
	    serverLink->sendMessage(victimPlayer->getId(), meaculpa);
	  }
	}
      } else {
	// enemy
	if (myTank->isAutoPilot()) {
	  if (killerPlayer) {
	    const ShotPath *shot = killerPlayer->getShot(int(shotId));
	    if (shot != NULL)
	      teachAutoPilot(shot->getFlag(), 1);
	  }
	}
      }
    }
  }

  // handle my personal score against other players
  if ((killerPlayer == myTank || victimPlayer == myTank) && !(killerPlayer == myTank && victimPlayer == myTank)) {
    if (killerLocal == myTank) {
      if (victimPlayer)
	victimPlayer->changeLocalScore(1, 0, 0);

      myTank->setNemesis(victimPlayer);
    } else {
      if (killerPlayer)
	killerPlayer->changeLocalScore(0, 1, killerPlayer->getTeam() == victimPlayer->getTeam() ? 1 : 0);

      myTank->setNemesis(killerPlayer);
    }
  }

  // add message
  if (human && victimPlayer) {
    std::string message(ColorStrings[WhiteColor]);
    if (killerPlayer == victimPlayer) {
      message += "blew myself up";
      addMessage(victimPlayer, message);
    } else if (killer >= LastRealPlayer) {
      addMessage(victimPlayer, "destroyed by the server");
    } else if (!killerPlayer) {
      addMessage(victimPlayer, "destroyed by a (GHOST)");
    } else if (reason == WaterDeath) {
      message += "fell in the water";
      addMessage(victimPlayer, message);
    } else if (reason == PhysicsDriverDeath) {
      const PhysicsDriver *driver = PHYDRVMGR.getDriver(phydrv);
      if (driver == NULL) {
	message += "Unknown Deadly Obstacle";
      } else {
	message += driver->getDeathMsg();
      }

      addMessage(victimPlayer, message);
    } else {
      std::string playerStr;
      if (World::getWorld()->allowTeams() &&
          (killerPlayer->getTeam() == victimPlayer->getTeam()) &&
          (killerPlayer->getTeam() != RogueTeam) &&
          (killerPlayer->getTeam() != ObserverTeam)) {
        playerStr += "teammate ";
      }

      if (victimPlayer == myTank) {
	if (BZDB.get("killerhighlight") == "1") {
	  playerStr += ColorStrings[PulsatingColor];
	} else if (BZDB.get("killerhighlight") == "2") {
	  playerStr += ColorStrings[UnderlineColor];
	}
      }

      int color = killerPlayer->getTeam();
      playerStr += ColorStrings[color];
      playerStr += killerPlayer->getCallSign();

      if (victimPlayer == myTank)
	playerStr += ColorStrings[ResetColor];

      playerStr += ColorStrings[WhiteColor];

      // Give more informative kill messages
      if (flagType == Flags::Laser)
	message += "was fried by " + playerStr + "'s laser";
      else if (flagType == Flags::GuidedMissile)
	message += "was destroyed by " + playerStr + "'s guided missile";
      else if (flagType == Flags::ShockWave)
	message += "felt the effects of " + playerStr + "'s shockwave";
      else if (flagType == Flags::InvisibleBullet)
	message += "didn't see " + playerStr + "'s bullet";
      else if (flagType == Flags::MachineGun)
	message += "was turned into swiss cheese by " + playerStr + "'s machine gun";
      else if (flagType == Flags::SuperBullet)
	message += "got skewered by " + playerStr + "'s super bullet";
      else
	message += "killed by " + playerStr;

      addMessage(victimPlayer, message, ControlPanel::MessageMisc,
                 (killerPlayer == myTank));
    }
  }

  // geno only works in team games :)
  if (World::getWorld()->allowTeams()) {
    // blow up if killer has genocide flag and i'm on same team as victim
    // (and we're not rogues, unless in rabbit mode)
    if (human && killerPlayer && victimPlayer &&
      victimPlayer != myTank &&
      (victimPlayer->getTeam() == myTank->getTeam()) &&
      (myTank->getTeam() != RogueTeam) &&
      (shotId >= 0)) {
	// now see if shot was fired with a GenocideFlag
	const ShotPath *shot = killerPlayer->getShot(int(shotId));

	//but make sure that if we are not allowing teamkills, the victim was not a suicide
	if (shot && shot->getFlag() == Flags::Genocide &&
	  (killerPlayer != victimPlayer || World::getWorld()->allowTeamKills())) {
	    // go boom
	    gotBlowedUp(myTank, GenocideEffect, killerPlayer->getId());
	}
    }
  }

#ifdef ROBOT
  // blow up robots on victim's team if shot was genocide
  if (killerPlayer && victimPlayer && shotId >= 0) {
    const ShotPath *shot = killerPlayer->getShot(int(shotId));
    if (shot && shot->getFlag() == Flags::Genocide) {
      for (int i = 0; i < numRobots; i++) {
	if (robots[i] && victimPlayer != robots[i] && victimPlayer->getTeam() == robots[i]->getTeam() && robots[i]->getTeam() != RogueTeam)
	  gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
      }
    }
  }
#endif

  checkScores = true;
}


void handleGrabFlag(void *msg)
{
  PlayerId id;
  uint16_t flagIndex;
  unsigned char shot;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  Flag& flag = world->getFlag(int(flagIndex));
  msg = flag.unpack(msg);
  msg = nboUnpackUInt8(msg, shot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  // player now has flag
  tank->setFlag(flag.type);
  tank->setShotType((ShotType)shot);

  if (tank->getPlayerType() == ComputerPlayer) {
    RobotPlayer *robot = lookupRobotPlayer(id);
    if (!robot)
      return;
    robot->setFlag(flag.type);
    robot->setShotType((ShotType)shot);
  }

  if (tank == myTank) {
    SOUNDSYSTEM.play(myTank->getFlag()->endurance != FlagSticky ? SFX_GRAB_FLAG : SFX_GRAB_BAD); // grabbed flag
    updateFlag(myTank->getFlag());
  }
  else if (isViewTank(tank)) {
    SOUNDSYSTEM.play(tank->getFlag()->endurance != FlagSticky ? SFX_GRAB_FLAG : SFX_GRAB_BAD);
  }
  else if ((myTank->getTeam() != RabbitTeam) && tank &&
             (tank->getTeam() != myTank->getTeam()) &&
             (flag.type->flagTeam == myTank->getTeam())) {
    hud->setAlert(1, "Flag Alert!!!", 3.0f, true);
    SOUNDSYSTEM.play(SFX_ALERT);
  }
  else {
    FlagType *fd = world->getFlag(flagIndex).type;
    if (fd->flagTeam != NoTeam
      && fd->flagTeam != tank->getTeam()
      && ((tank && (tank->getTeam() == myTank->getTeam())))
      && (Team::isColorTeam(myTank->getTeam()))) {
	hud->setAlert(1, "Team Grab!!!", 3.0f, false);
	const fvec3& pos = tank->getPosition();
	SOUNDSYSTEM.play(SFX_TEAMGRAB, pos, false, false);
    }
  }

  std::string message("grabbed ");
  message += tank->getFlag()->flagName;
  message += " flag";

  addMessage(tank, message);
}


void handleCaptureFlag(void *msg, bool &checkScores)
{
  PlayerId id;
  uint16_t flagIndex;
  int16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  msg = nboUnpackInt16(msg, team);
  Player *capturer = lookupPlayer(id);

  Flag& capturedFlag = world->getFlag(int(flagIndex));

  if (capturedFlag.type == Flags::Null)
    return;

  int capturedTeam = capturedFlag.type->flagTeam;

  // player no longer has flag
  if (capturer) {
    capturer->setFlag(Flags::Null);
    if (capturer == myTank)
      updateFlag(Flags::Null);

    // add message
    if (int(capturer->getTeam()) == capturedTeam) {
      std::string message("took my flag into ");
      message += Team::getName(TeamColor(team));
      message += " territory";
      addMessage(capturer, message);
      if (capturer == myTank) {
	hud->setAlert(1, "Don't capture your own flag!!!", 3.0f, true);
	SOUNDSYSTEM.play(SFX_KILL_TEAM);
	if (myTank->isAutoPilot()) {
	  char meaculpa[MessageLen];
	  memset(meaculpa, 0, MessageLen);
	  strncpy(meaculpa, "sorry, i'm just a silly machine", MessageLen);
	  serverLink->sendMessage(myTank->getTeam(), meaculpa);
	}
      }
    } else {
      std::string message("captured ");
      message += Team::getName(TeamColor(capturedTeam));
      message += "'s flag";
      addMessage(capturer, message);
    }
  }

  // play sound -- if my team is same as captured flag then my team lost,
  // but if I'm on the same team as the capturer then my team won.
  if (capturedTeam == int(myTank->getTeam()))
    SOUNDSYSTEM.play(SFX_LOSE);
  else if (capturer && capturer->getTeam() == myTank->getTeam())
    SOUNDSYSTEM.play(SFX_CAPTURE);

  // no need to kill myself, the server will kill everyone involved.

  checkScores = true;
}


void handleNewRabbit(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);
  Player *rabbit = lookupPlayer(id);

  // new mode option,
  unsigned char mode;
  msg = nboUnpackUInt8(msg, mode);

  // mode 0 == swap the current rabbit with this rabbit
  // mode 1 == add this person as a rabbit
  // mode 2 == remove this person from the rabbit list

  // we don't need to mod the hunters if we aren't swaping
  if (mode == 0) {
    for (int i = 0; i < curMaxPlayers; i++) {
      if (remotePlayers[i])
	remotePlayers[i]->setHunted(false);
      if (i != id && remotePlayers[i] &&
          remotePlayers[i]->getTeam() != RogueTeam &&
          remotePlayers[i]->getTeam() != ObserverTeam)
	remotePlayers[i]->changeTeam(HunterTeam);
    }
  }

  if (rabbit != NULL) {
    if (mode != 2) {
      rabbit->changeTeam(RabbitTeam);

      if (rabbit == myTank) {
	wasRabbit = true;
	if (myTank->isPaused()) {
	  serverLink->sendNewRabbit();
	}
	else {
	  if (mode == 0) {
	    hud->setAlert(0, "You are now the rabbit.", 10.0f, false);
	  } else {
	    hud->setAlert(0, "You are now a rabbit.", 10.0f, false);
          }
	  SOUNDSYSTEM.play(SFX_HUNT_SELECT);
	}
	scoreboard->setHuntState(ScoreboardRenderer::HUNT_NONE);
      }
      else if (myTank->getTeam() != ObserverTeam) {
	myTank->changeTeam(HunterTeam);

	if (myTank->isPaused() || myTank->isAlive()) {
	  wasRabbit = false;
        }

	rabbit->setHunted(true);
	scoreboard->setHuntState(ScoreboardRenderer::HUNT_ENABLED);
      }

      if (mode == 0) {
	addMessage(rabbit, "is now the rabbit", ControlPanel::MessageMisc, true);
      } else {
	addMessage(rabbit, "is now a rabbit", ControlPanel::MessageMisc, true);
      }
    }
    else {
      rabbit->changeTeam(HunterTeam);
      if (rabbit == myTank) {
	hud->setAlert(0, "You are no longer a rabbit.", 10.0f, false);
      }
      addMessage(rabbit, "is no longer a rabbit", ControlPanel::MessageMisc, true);
    }
  }

#ifdef ROBOT
  for (int r = 0; r < numRobots; r++) {
    if (robots[r]) {
      if (robots[r]->getId() == id) {
	robots[r]->changeTeam(RabbitTeam);
      } else {
	robots[r]->changeTeam(HunterTeam);
      }
    }
  }
#endif
}


void handleNearFlag(void *msg)
{
  fvec3 pos;
  std::string flagName;
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackStdString(msg, flagName);

  std::string fullMessage = "Closest Flag: " + flagName;
  std::string colorMsg;
  colorMsg += ColorStrings[YellowColor];
  colorMsg += fullMessage;
  colorMsg += ColorStrings[DefaultColor];

  addMessage(NULL, colorMsg, ControlPanel::MessageServer, false, NULL);

  if (myTank) {
    hud->setAlert(0, fullMessage.c_str(), 5.0f, false);
  }
}


// if you are driving with a tank in observer mode
// and do not want local shot effects,
// disable shot effects for that specific tank
static bool showShotEffects(int shooterid)
{
  if (!BZDB.isTrue("enableLocalShotEffect"))
    return false;

  if (ROAM.getMode() == Roaming::roamViewFP)
    return false;

  if (ROAM.getTargetTank() && shooterid != ROAM.getTargetTank()->getId())
    return false;

  return true;
}


static void playShotSound (const FiringInfo &info, bool localSound)
{
  const fvec3& pos = info.shot.pos;
  const bool importance = false;

  switch (info.shotType)
  {
  default:
    SOUNDSYSTEM.play(SFX_FIRE, pos, importance, localSound);
    break;

  case ShockWaveShot:
    SOUNDSYSTEM.play(SFX_SHOCK, pos, importance, localSound);
    break;

  case LaserShot:
    SOUNDSYSTEM.play(SFX_LASER, pos, importance, localSound);
    break;

  case GMShot:
    SOUNDSYSTEM.play(SFX_MISSILE, pos, importance, localSound);
    break;

  case ThiefShot:
    SOUNDSYSTEM.play(SFX_THIEF, pos, importance, localSound);
    break;
  }
}


void handleShotBegin(bool human, void *msg)
{
  PlayerId shooterid;
  uint16_t id;

  msg = nboUnpackUInt8(msg, shooterid);
  msg = nboUnpackUInt16(msg, id);

  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);
  firingInfo.shot.player = shooterid;
  firingInfo.shot.id     = id;

  if (shooterid >= playerSize) {
    return;
  }

  if (shooterid == myTank->getId()) {
    // the shot is ours, find the shot we made, and kill it
    // then rebuild the shot with the info from the server
    myTank->updateShot(firingInfo, id, firingInfo.timeSent);
  }
  else {
    RemotePlayer *shooter = remotePlayers[shooterid];

    if (shooterid != ServerPlayer) {
      if (shooter && remotePlayers[shooterid]->getId() == shooterid) {
	shooter->addShot(firingInfo);

	if (SceneRenderer::instance().useQuality() >= _MEDIUM_QUALITY) {
	  fvec3 shotPos;
	  shooter->getMuzzle(shotPos);

	  if (showShotEffects(shooterid)) {
	    EFFECTS.addShotEffect(shooter->getColor(), shotPos,
	                          shooter->getAngle(), shooter->getVelocity());
          }
	}
      }
      else {
	return;
      }
    }

    if (human) {
      playShotSound(firingInfo, isViewTank(shooter));
    }
  }
}


void handleWShotBegin (void *msg)
{
  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);

  WorldPlayer *worldWeapons = world->getWorldWeapons();

  worldWeapons->addShot(firingInfo);
  playShotSound(firingInfo, false);
}


void handleScore(void *msg)
{
  uint8_t numScores;
  PlayerId id;
  float rank;
  uint16_t wins, losses, tks;
  msg = nboUnpackUInt8(msg, numScores);

  for (uint8_t s = 0; s < numScores; s++) {
    msg = nboUnpackUInt8(msg, id);
    msg = nboUnpackFloat(msg, rank);
    msg = nboUnpackUInt16(msg, wins);
    msg = nboUnpackUInt16(msg, losses);
    msg = nboUnpackUInt16(msg, tks);

    Player *sPlayer = NULL;
    if (id == myTank->getId()) {
      sPlayer = myTank;
    } else {
      int i = lookupPlayerIndex(id);
      if (i >= 0)
	sPlayer = getPlayerByIndex(i);
      else
	logDebugMessage(1, "Received score update for unknown player!\n");
    }
    if (sPlayer) {
      if (sPlayer == myTank) {
	ExportInformation &ei = ExportInformation::instance();
	ei.setInformation("Score",
	  TextUtils::format("%d (%d-%d) [%d]",
	  wins - losses, wins, losses, tks),
	  ExportInformation::eitPlayerStatistics,
	  ExportInformation::eipPrivate);
      }
      sPlayer->changeScore(rank, wins, losses, tks);
    }
  }
}


void handleTeleport(void *msg)
{
  PlayerId id;
  uint16_t srcID, dstID;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, srcID);
  msg = nboUnpackUInt16(msg, dstID);
  Player *tank = lookupPlayer(id);
  if (tank) {
    if (tank != myTank) {
      tank->setTeleport(TimeKeeper::getTick(), short(srcID), short(dstID));
      const MeshFace* linkDst = linkManager.getLinkDstFace(dstID);
      const MeshFace* linkSrc = linkManager.getLinkSrcFace(srcID);
      if (linkDst && linkSrc) {
        const fvec3& pos = linkDst->getPosition();
        if (!linkSrc->linkSrcNoSound()) {
          SOUNDSYSTEM.play(SFX_TELEPORT, pos, false, false);
        }
      }
    }
  }
}


void handleFlagTransferred(Player *fromTank, Player *toTank, int flagIndex, ShotType shotType)
{
  Flag& f = world->getFlag(flagIndex);

  fromTank->setShotType(StandardShot);
  fromTank->setFlag(Flags::Null);
  toTank->setShotType(shotType);
  toTank->setFlag(f.type);

  if (fromTank->getPlayerType() == ComputerPlayer) {
    RobotPlayer *robot = lookupRobotPlayer(fromTank->getId());
    if (!robot)
      return;
    robot->setShotType(StandardShot);
    robot->setFlag(Flags::Null);
  }

  if (toTank->getPlayerType() == ComputerPlayer) {
    RobotPlayer *robot = lookupRobotPlayer(toTank->getId());
    if (!robot)
      return;
    robot->setShotType(shotType);
    robot->setFlag(f.type);
  }

  if ((fromTank == myTank) || (toTank == myTank))
    updateFlag(myTank->getFlag());

  const fvec3& pos = toTank->getPosition();
  if (f.type->flagTeam != ::NoTeam) {
    if ((toTank->getTeam() == myTank->getTeam()) && (f.type->flagTeam != myTank->getTeam())) {
      SOUNDSYSTEM.play(SFX_TEAMGRAB, pos, false, false);
    } else if ((fromTank->getTeam() == myTank->getTeam()) && (f.type->flagTeam == myTank->getTeam())) {
      hud->setAlert(1, "Flag Alert!!!", 3.0f, true);
      SOUNDSYSTEM.play(SFX_ALERT);
    }
  }

  std::string message(toTank->getCallSign());
  message += " stole ";
  message += fromTank->getCallSign();
  message += "'s flag";
  addMessage(toTank, message);
}




void handleMessage(void *msg)
{
  PlayerId src;
  PlayerId dst;
  uint8_t type;
  msg = nboUnpackUInt8(msg, src);
  msg = nboUnpackUInt8(msg, dst);
  msg = nboUnpackUInt8(msg, type);
  Player *srcPlayer = lookupPlayer(src);
  Player *dstPlayer = lookupPlayer(dst);
  TeamColor dstTeam = PlayerIdToTeam(dst);
  bool toAll = (dst == AllPlayers);
  bool fromServer = (src == ServerPlayer);
  bool toAdmin = (dst == AdminPlayers);
  std::string dstName;

  const std::string srcName = fromServer ? "SERVER" : (srcPlayer ? srcPlayer->getCallSign() : "(UNKNOWN)");
  if (dstPlayer) {
    dstName = dstPlayer->getCallSign();
  } else if (toAdmin) {
    dstName = "Admin";
  } else {
    dstName = "(UNKNOWN)";
  }
  std::string fullMsg;

  bool ignore = false;
  unsigned int i;
  for (i = 0; i < silencePlayers.size(); i++) {
    const std::string &silenceCallSign = silencePlayers[i];
    if (srcName == silenceCallSign || "*" == silenceCallSign) {
      ignore = true;
      break;
    }
  }

  if (ignore) {
#ifdef DEBUG
    // to verify working
    std::string msg2 = "Ignored Msg";
    if (silencePlayers[i] != "*")
      msg2 += " from " + silencePlayers[i];

    addMessage(NULL, msg2);
#endif
    return;
  }

  // ensure that we aren't receiving a partial multibyte character
  UTF8StringItr itr = (char*)msg;
  UTF8StringItr prev = itr;
  while (*itr && (itr.getBufferFromHere() - (char*)msg) < MessageLen) {
    prev = itr++;
  }
  if ((itr.getBufferFromHere() - (char*)msg) >= MessageLen) {
    *(const_cast<char*>(prev.getBufferFromHere())) = '\0';
  }

  // if filtering is turned on, filter away the goo
  if (wordFilter != NULL) {
    wordFilter->filter((char *)msg);
  }

  const char *origText = stripAnsiCodes((char*)msg);
  std::string text = BundleMgr::getCurrentBundle()->getLocalString(origText);

  if (toAll || toAdmin || srcPlayer == myTank  || dstPlayer == myTank ||
    dstTeam == myTank->getTeam()) {
    // message is for me
    std::string colorStr;
    if (srcPlayer == NULL) {
      colorStr += ColorStrings[RogueTeam];
    } else {
      const PlayerId pid = srcPlayer->getId();
      if (pid < 200) {
        if (srcPlayer && srcPlayer->getTeam() != NoTeam) {
          colorStr += ColorStrings[srcPlayer->getTeam()];
        } else {
          colorStr += ColorStrings[RogueTeam];
        }
      }
      else if (pid == ServerPlayer) {
        colorStr += ColorStrings[YellowColor];
      } else {
        colorStr += ColorStrings[CyanColor]; // replay observers
      }
    }

    fullMsg += colorStr;

    // play a sound on a message not from self or server
    bool playSound = true;
    if (fromServer) {
      // play no sounds from server if beeping is turned off
      if (!BZDB.isTrue("beepOnServerMsg")) {
        playSound = false;
      }
    }
    else if (srcPlayer == myTank) {
      // don't play sounds for messages from self
      playSound = false;
    }

    // direct message to or from me
    if (dstPlayer) {
      if (dstPlayer == myTank && srcPlayer == myTank) {
        fullMsg = text; // talking to myself? that's strange
      }
      else {
        if (BZDB.get("killerhighlight") == "1") {
          fullMsg += ColorStrings[PulsatingColor];
        } else if (BZDB.get("killerhighlight") == "2") {
          fullMsg += ColorStrings[UnderlineColor];
        }

        if (srcPlayer == myTank) {
          if (type == ActionMessage) {
            fullMsg += "[->" + dstName + " " + text + "]";
          } else {
            fullMsg += "[->" + dstName + "]";
            fullMsg += std::string(ColorStrings[ResetColor]) + " ";
            fullMsg += std::string(ColorStrings[CyanColor]) + text;
          }
        } else {
          if (type == ActionMessage) {
            fullMsg += "[" + srcName + " " + text + "->]";
          } else {
            fullMsg += "[" + srcName + "->]";
            fullMsg += std::string(ColorStrings[ResetColor]) + " ";
            fullMsg += std::string(ColorStrings[CyanColor]) + text;
          }

          if (srcPlayer) {
            myTank->setRecipient(srcPlayer);
          }

          // play a sound on a private message not from self or server
          if (playSound) {
            static TimeKeeper lastMsg = TimeKeeper::getSunGenesisTime();
            if (TimeKeeper::getTick() - lastMsg > 2.0f) {
              SOUNDSYSTEM.play(SFX_MESSAGE_PRIVATE);
            }
            lastMsg = TimeKeeper::getTick();
          }
        }
      }
    }
    else { // !dstPlayer, team or admin message
      if (toAdmin) {
        // play a sound on a private message
        if (playSound) {
          static TimeKeeper lastMsg = TimeKeeper::getSunGenesisTime();
          if (TimeKeeper::getTick() - lastMsg > 2.0f) {
            SOUNDSYSTEM.play(SFX_MESSAGE_ADMIN);
          }
          lastMsg = TimeKeeper::getTick();
        }
        fullMsg += "[Admin] ";
      }
      else if (dstTeam != NoTeam) {
#ifdef BWSUPPORT
        fullMsg = "[to ";
        fullMsg += Team::getName(TeamColor(dstTeam));
        fullMsg += "] ";
#else
        fullMsg += "[Team] ";
#endif
        // play a sound on a team message
        if (playSound) {
          static TimeKeeper lastMsg = TimeKeeper::getSunGenesisTime();
          if (TimeKeeper::getTick() - lastMsg > 2.0f)
            SOUNDSYSTEM.play(SFX_MESSAGE_TEAM);
          lastMsg = TimeKeeper::getTick();
        }
      }

      // display action messages differently
      if (type == ActionMessage) {
        fullMsg += srcName + " " + text;
      } else {
        fullMsg += srcName + colorStr + ": " +
                   std::string(ColorStrings[CyanColor]) + text;
      }
    }

    std::string oldcolor = "";

    if (!srcPlayer || (srcPlayer->getTeam() == NoTeam)) {
      oldcolor = ColorStrings[RogueTeam];
    }
    else if (srcPlayer->getTeam() == ObserverTeam) {
      oldcolor = ColorStrings[CyanColor];
    }
    else {
      oldcolor = ColorStrings[srcPlayer->getTeam()];
    }

    if (fromServer) {
      addMessage(NULL, fullMsg, ControlPanel::MessageServer, false, oldcolor.c_str());
    } else {
      addMessage(NULL, fullMsg, ControlPanel::MessageChat, false, oldcolor.c_str());
    }

    if (!srcPlayer || (srcPlayer != myTank)) {
      hud->setAlert(0, fullMsg.c_str(), 3.0f, false);
    }
  }
}


static std::string getQueryGLResult(const std::string& query)
{
  GLenum queryID = 0;

#define GL_QUERY_COMPARE(x) if (query == #x) { queryID = GL_ ## x; }

  // string queries
  GL_QUERY_COMPARE(VENDOR);
  GL_QUERY_COMPARE(VERSION);
  GL_QUERY_COMPARE(RENDERER);
  GL_QUERY_COMPARE(EXTENSIONS);
  if (queryID != 0) {
    const char* resultPtr = (const char*)glGetString(queryID);
    if (resultPtr == NULL) {
      return "";
    }
    return resultPtr;
  }

  // integer queries
  GL_QUERY_COMPARE(MAX_LIGHTS);
  GL_QUERY_COMPARE(MAX_ATTRIB_STACK_DEPTH);
  GL_QUERY_COMPARE(MAX_CLIENT_ATTRIB_STACK_DEPTH);
  GL_QUERY_COMPARE(MAX_MODELVIEW_STACK_DEPTH);
  GL_QUERY_COMPARE(MAX_PROJECTION_STACK_DEPTH);
  GL_QUERY_COMPARE(MAX_TEXTURE_STACK_DEPTH);
  GL_QUERY_COMPARE(MAX_TEXTURE_SIZE);
  GL_QUERY_COMPARE(MAX_TEXTURE_UNITS);
  GL_QUERY_COMPARE(MAX_TEXTURE_COORDS);
  GL_QUERY_COMPARE(MAX_TEXTURE_IMAGE_UNITS);
  GL_QUERY_COMPARE(MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  GL_QUERY_COMPARE(MAX_COMBINED_TEXTURE_IMAGE_UNITS);
  GL_QUERY_COMPARE(MAX_ELEMENTS_VERTICES);
  GL_QUERY_COMPARE(MAX_ELEMENTS_INDICES);
  GL_QUERY_COMPARE(MAX_DRAW_BUFFERS);
  GL_QUERY_COMPARE(MAX_SAMPLES);
  GL_QUERY_COMPARE(RED_BITS);
  GL_QUERY_COMPARE(BLUE_BITS);
  GL_QUERY_COMPARE(GREEN_BITS);
  GL_QUERY_COMPARE(ALPHA_BITS);
  GL_QUERY_COMPARE(DEPTH_BITS);
  GL_QUERY_COMPARE(STENCIL_BITS);
  if (queryID != 0) {
    GLint value;
    glGetIntegerv(queryID, &value);
    return TextUtils::itoa(value);
  }

#undef GL_QUERY_COMPARE

  return "";  
}


void handleQueryGL(void *msg)
{
  std::string queryStr;
  msg = nboUnpackStdString(msg, queryStr);

  std::string result = getQueryGLResult(queryStr);

  std::vector<std::string> results;
  const std::string prefix = "QueryGL/" + queryStr + ": ";
  const size_t maxLen = MessageLen - prefix.size() - 15;
  while (!result.empty()) {
    if (result.size() > maxLen) {
      results.push_back(prefix + result.substr(0, maxLen));
      result = result.substr(maxLen);
    } else {
      results.push_back(prefix + result);
      result.clear();
    }
  }

  for (size_t r = 0; r < results.size(); r++) {
    char buf[MessageLen];
    strncpy(buf, results[r].c_str(), MessageLen);
    serverLink->sendMessage(ServerPlayer, buf);
  }
}


void handleGMUpdate(void *msg)
{
  ShotUpdate shot;
  msg = shot.unpack(msg);
  Player *tank = lookupPlayer(shot.player);
  if (!tank || tank == myTank)
    return;

  RemotePlayer *remoteTank = (RemotePlayer*)tank;
  RemoteShotPath *shotPath = (RemoteShotPath*)remoteTank->getShot(shot.id);
  if (shotPath)
    shotPath->update(shot, msg);
  PlayerId targetId;
  msg = nboUnpackUInt8(msg, targetId);
  Player *targetTank = getPlayerByIndex(targetId);

  if (targetTank && (targetTank == myTank) && (myTank->isAlive()))
  {
    static TimeKeeper lastLockMsg;
    if (TimeKeeper::getTick() - lastLockMsg > 0.75)
    {
      SOUNDSYSTEM.play(SFX_LOCK, shot.pos, false, false);
      lastLockMsg = TimeKeeper::getTick();
      addMessage(tank, "locked on me");
    }
  }
}



static void updateFlags(float dt)
{
  for (int i = 0; i < numFlags; i++) {
    Flag &flag = world->getFlag(i);
    if (flag.status == FlagOnTank) {
      // position flag on top of tank
      Player *tank = lookupPlayer(flag.owner);
      if (tank) {
	const fvec3& pos = tank->getPosition();
	flag.position = fvec3(pos.x, pos.y, pos.z + tank->getDimensions().z);
      }
    }
    world->updateFlag(i, dt);
  }
  FlagSceneNode::setTimeStep(dt);
}


bool addExplosion(const fvec3& _pos,
		  float size, float duration, bool groundLight)
{
  // ignore if no prototypes available;
  if (prototypeExplosions.size() == 0) return false;

  // don't show explosions if quality isn't at least medium
  if (RENDERER.useQuality() < _MEDIUM_QUALITY) return false;

  // don't add explosion if blending or texture mapping are off
  if (!BZDBCache::blend || !BZDBCache::texture)
    return false;

  // pick a random prototype explosion
  const int index = (int)(bzfrand() * (float)prototypeExplosions.size());

  // make a copy and initialize it
  BillboardSceneNode *newExplosion = prototypeExplosions[index]->copy();
  fvec3 pos = _pos;
  newExplosion->move(pos);
  newExplosion->setSize(size);
  newExplosion->setDuration(duration);
  newExplosion->setAngle((float)(2.0 * M_PI * bzfrand()));
  newExplosion->setLight();
  newExplosion->setLightColor(1.0f, 0.8f, 0.5f);
  newExplosion->setLightAttenuation(0.05f, 0.0f, 0.03f);
  newExplosion->setLightScaling(size / BZDBCache::tankLength);
  newExplosion->setLightFadeStartTime(0.7f * duration);
  if (groundLight) {
    newExplosion->setGroundLight(true);
  }

  // add copy to list of current explosions
  explosions.push_back(newExplosion);

  // the rest of the stuff is for tank explosions
  if (size < (3.0f * BZDBCache::tankLength))
    return true;

  // bring on the noise, a tank blew up
  int boom = (int) (bzfrand() * 8.0) + 3;
  const float lightGain = (float)boom + 1.0f;

  // turn up the volume
  newExplosion->setLightColor(1.0f * lightGain,
    0.8f * lightGain,
    0.5f * lightGain);
  while (boom--) {
    // pick a random prototype explosion
    const int idx = (int)(bzfrand() * (float)prototypeExplosions.size());

    // make a copy and initialize it
    BillboardSceneNode *newExpl = prototypeExplosions[idx]->copy();
    fvec3 explPos = _pos;;
    explPos.x += (float)(bzfrand() * 12.0 - 6.0);
    explPos.y += (float)(bzfrand() * 12.0 - 6.0);
    explPos.z += (float)(bzfrand() * 10.0);
    newExpl->move(explPos);
    newExpl->setSize(size);
    newExpl->setDuration(duration);
    newExpl->setAngle((float)(2.0 * M_PI * bzfrand()));

    // add copy to list of current explosions
    explosions.push_back(newExpl);
  }

  return true;
}


void addTankExplosion(const fvec3& pos)
{
  addExplosion(pos, BZDB.eval(BZDBNAMES.TANKEXPLOSIONSIZE), 1.2f, false);
}


void addShotExplosion(const fvec3& pos)
{
  // only play explosion sound if you see an explosion
  if (addExplosion(pos, 1.2f * BZDBCache::tankLength, 0.8f, false))
    SOUNDSYSTEM.play(SFX_SHOT_BOOM, pos, false, false);
}


void addShotPuff(const fvec3& pos, const fvec3& vel)
{
  if (BZDB.evalInt("gmPuffEffect") == 1) {
    addExplosion(pos, 0.3f * BZDBCache::tankLength, 0.8f, true);
    return;
  }
  const float azimuth   = atan2f(vel.y, vel.x);
  const float elevation = atan2f(vel.z, vel.xy().length());
  EFFECTS.addGMPuffEffect(pos, fvec2(azimuth, elevation));
}


// process pending input events
void processInputEvents(float maxProcessingTime)
{
  if (mainWindow && display) {
    TimeKeeper start = TimeKeeper::getCurrent();
    while (display->isEventPending() &&
      !CommandsStandard::isQuit() &&
      (TimeKeeper::getCurrent() - start < maxProcessingTime)) {
	// process one event
	doEvent(display);
    }
  }
}


static void updateExplosions(float dt)
{
  // update time of all explosions
  size_t i;
  const size_t count = explosions.size();

  if (count > 0) {
    for (i = 0; i < count; i++) {
      explosions[i]->updateTime(dt);
    }

    // reap expired explosions
    for (i = count - 1; i > 0; i--) {
      if (explosions[i]->isAtEnd()) {
	delete explosions[i];
	std::vector<BillboardSceneNode*>::iterator it = explosions.begin();
	for (size_t j = 0; j < i; j++) {
	  it++;
        }
	explosions.erase(it);
      }
    }
  }
}


static void addExplosions(SceneDatabase *scene)
{
  const size_t count = explosions.size();
  for (size_t i = 0; i < count; i++) {
    scene->addDynamicNode(explosions[i]);
  }
}


#ifdef ROBOT
void handleMyTankKilled(int reason)
{
  // blow me up
  myTank->explodeTank();
  if (reason == GotRunOver)
    SOUNDSYSTEM.play(SFX_RUNOVER);
  else
    SOUNDSYSTEM.play(SFX_DIE);
}
#endif



void handleLimboMessage(void *msg)
{
  std::string limboMessage;
  nboUnpackStdString(msg, limboMessage);
  hud->setLimboMessage(limboMessage);
}


void handleFlagDropped(Player *tank)
{

  if (tank->getPlayerType() == ComputerPlayer) {
    RobotPlayer *robot = lookupRobotPlayer(tank->getId());
    if (!robot)
      return;
    robot->setShotType(StandardShot);
    robot->setFlag(Flags::Null);
  } else {
    tank->setShotType(StandardShot);

    // skip it if player doesn't actually have a flag
    if (tank->getFlag() == Flags::Null) return;

    if (tank == myTank) {
      // make sure the player must reload after theft
      if (tank->getFlag() == Flags::Thief)
	myTank->forceReload(BZDB.eval(BZDBNAMES.THIEFDROPTIME));

      // update display and play sound effects
      SOUNDSYSTEM.play(SFX_DROP_FLAG);
      updateFlag(Flags::Null);
    } else if (isViewTank(tank)) {
      SOUNDSYSTEM.play(SFX_DROP_FLAG);
    }

    // add message
    std::string message("dropped ");
    message += tank->getFlag()->flagName;
    message += " flag";
    addMessage(tank, message);

    // player no longer has flag
    tank->setFlag(Flags::Null);
  }
}


bool gotBlowedUp(BaseLocalPlayer *tank, BlowedUpReason reason, PlayerId killer,
			const ShotPath *hit, int phydrv)
{
  if (!tank || (tank->getTeam() == ObserverTeam || !tank->isAlive()))
    return false;

  int shotId = -1;
  FlagType *flagType = Flags::Null;
  if (hit) {
    shotId = hit->getShotId();
    flagType = hit->getFlag();
  }

  // you can't take it with you
  const FlagType *flag = tank->getFlag();
  if (flag != Flags::Null) {
    if (myTank->isAutoPilot())
      teachAutoPilot(myTank->getFlag(), -1);

    // tell other players I've dropped my flag
    serverLink->sendDropFlag(tank->getPosition());
    tank->setShotType(StandardShot);

    // drop it
    handleFlagDropped(tank);
  }

  // restore the sound, this happens when paused tank dies
  // (genocide or team flag captured)
  if (savedVolume != -1) {
    SOUNDSYSTEM.setVolume(savedVolume*0.1f);
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
    EFFECTS.addDeathEffect(tank->getColor(), tank->getPosition(), tank->getAngle());

    if (isViewTank(tank)) {
      if (reason == GotRunOver)
	SOUNDSYSTEM.play(SFX_RUNOVER);
      else
	SOUNDSYSTEM.play(SFX_DIE);

      ForceFeedback::death();
    } else {
      const fvec3& pos = tank->getPosition();
      if (reason == GotRunOver)
	SOUNDSYSTEM.play(SFX_RUNOVER, pos, false, getLocalPlayer(killer) == myTank);
      else
	SOUNDSYSTEM.play(SFX_EXPLOSION, pos, false, getLocalPlayer(killer) == myTank);
    }

    if ((tank != myTank) ||
        (SceneRenderer::instance().useQuality() >= _EXPERIMENTAL_QUALITY)) {
      const fvec3& pos = tank->getPosition();
      const fvec3 explodePos(pos.x, pos.y, pos.z + tank->getMuzzleHeight());
      addTankExplosion(explodePos);
    }

    // tell server I'm dead in case it doesn't already know
    if (reason == GotShot || reason == GotRunOver ||
      reason == GenocideEffect || reason == SelfDestruct ||
      reason == WaterDeath || reason == PhysicsDriverDeath)
      serverLink->sendKilled(tank->getId(), killer, reason, shotId, flagType,
      phydrv);
  }

  // print reason if it's my tank
  if ((tank == myTank) && (reason < LastReason)) {
    std::string blowedUpNotice;
    if (reason < PhysicsDriverDeath) {
      if (blowedUpMessage[reason])
	blowedUpNotice = blowedUpMessage[reason];
      else
	blowedUpNotice = "Invalid reason";
    } else {
      const PhysicsDriver *driver = PHYDRVMGR.getDriver(phydrv);
      if (driver)
	blowedUpNotice = driver->getDeathMsg();
      else
	blowedUpNotice = "Killed by unknown obstacle";
    }

    // first, check if i'm the culprit
    if (reason == GotShot && getLocalPlayer(killer) == myTank) {
      blowedUpNotice = "Shot myself";
    } else {
      // 1-4 are messages sent when the player dies because of someone else
      if (reason >= GotShot && reason <= GenocideEffect) {
	Player *killerPlayer = lookupPlayer(killer);
	if (!killerPlayer) {
	  blowedUpNotice = "Killed by the server";
	} else {

	  // matching the team-display style of other kill messages
	  TeamColor team = killerPlayer->getTeam();
	  if (hit)
	    team = hit->getTeam();
	  if (World::getWorld()->allowTeams() &&
	      (myTank->getTeam() == team) &&
	      (team != RogueTeam) && (team != ObserverTeam)) {
	    blowedUpNotice += "teammate " ;
	    blowedUpNotice += killerPlayer->getCallSign();
	  } else {
	    blowedUpNotice += killerPlayer->getCallSign();
	    blowedUpNotice += " (";
	    blowedUpNotice += Team::getName(killerPlayer->getTeam());
	    blowedUpNotice += ")";
	    if (flagType != Flags::Null) {
	      blowedUpNotice += " with ";
	      blowedUpNotice += flagType->flagAbbv;
	    }
	  }
	}
      }
    }
    hud->setAlert(0, blowedUpNotice.c_str(), 4.0f, true);
    showMessage(blowedUpNotice);
  }

  // make sure shot is terminated locally (if not globally) so it can't
  // hit me again if I had the shield flag.  this is important for the
  // shots that aren't stopped by a hit and so may stick around to hit
  // me on the next update, making the shield useless.
  return (reason == GotShot && flag == Flags::Shield && shotId != -1);
}







static void checkEnvironment()
{
  if (!myTank || (myTank->getTeam() == ObserverTeam)) {
    return;
  }

  // skip this if i'm dead or paused
  if (!myTank->isAlive() || myTank->isPaused()) {
    return;
  }

  if (myTank->onSolidSurface()) {

    FlagType* flagd = myTank->getFlag();

    if (flagd->flagTeam != NoTeam) {
      // have I captured a flag?
      const TeamColor base = world->whoseBase(myTank->getPosition());
      const TeamColor team = myTank->getTeam();
      if (((base != NoTeam) && ((flagd->flagTeam == team) && (base != team))) ||
          ((flagd->flagTeam != team) && (base == team))) {
        serverLink->sendCaptureFlag(base);
      }
    }
    else if (flagd == Flags::Null) {
      // Don't grab too fast
      static TimeKeeper lastGrabSent;
      if (TimeKeeper::getTick()-lastGrabSent > 0.2) {
        // grab any and all flags i'm driving over
        const fvec3& tpos = myTank->getPosition();
        const float radius = myTank->getRadius() + BZDBCache::flagRadius;
        const float radius2 = radius * radius;
        for (int i = 0; i < numFlags; i++) {
          if ((world->getFlag(i).type == Flags::Null) ||
              (world->getFlag(i).status != FlagOnGround)) {
            continue;
          }
          const fvec3& fpos = world->getFlag(i).position;
          if ((fabs(tpos.z - fpos.z) < 0.1f) &&
              ((tpos.xy() - fpos.xy()).lengthSq() < radius2)) {
            serverLink->sendPlayerUpdate(myTank);
            lastGrabSent = TimeKeeper::getTick();
          }
        }
      }
    }
  }

  // see if i've been shot
  const ShotPath *hit = NULL;
  float minTime = Infinity;

  myTank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      myTank->checkHit(remotePlayers[i], hit, minTime);
    }
  }

  // Check Server Shots
  myTank->checkHit(World::getWorld()->getWorldWeapons(), hit, minTime);

  // used later
  //	float waterLevel = World::getWorld()->getWaterLevel();

  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendHit(myTank->getId(), hit->getPlayer(), hit->getShotId());

    FlagType *killerFlag = hit->getFlag();
    bool stopShot;

    if (killerFlag == Flags::Thief) {
      if (myTank->getFlag() != Flags::Null)
	serverLink->sendTransferFlag(myTank->getId(), hit->getPlayer());
      stopShot = true;
    } else {
      stopShot = gotBlowedUp(myTank, GotShot, hit->getPlayer(), hit);
    }

    if (stopShot || hit->isStoppedByHit()) {
      Player *hitter = lookupPlayer(hit->getPlayer());
      if (hitter)
	hitter->endShot(hit->getShotId());
    }
  } else if (myTank->getDeathPhysicsDriver() >= 0) { // if not dead yet, see if i'm sitting on death
    gotBlowedUp(myTank, PhysicsDriverDeath, ServerPlayer, NULL, myTank->getDeathPhysicsDriver());
    // this is done on the server now, we should remove this when we are sure its ok.
    /*	else if ((waterLevel > 0.0f) && (myTank->getPosition().z <= waterLevel))  // if not dead yet, see if i've dropped below the death level
    gotBlowedUp(myTank, WaterDeath, ServerPlayer); */
  } else {
    // if not dead yet, see if i got squished
    for (i = 0; i < curMaxPlayers; i++) {
      if (checkSquishKill(myTank, remotePlayers[i])) {
        gotBlowedUp(myTank, GotRunOver, remotePlayers[i]->getId());
      }
    }
  }
}



bool inLockRange(float angle, float distance, float bestDistance, RemotePlayer *player)
{
  if (player->isPaused() || player->isNotResponding() || player->getFlag() == Flags::Stealth)
    return false; // can't lock to paused, NR, or stealth

  if (angle >=  BZDB.eval(BZDBNAMES.LOCKONANGLE))
    return false;

  if (distance >= bestDistance)
    return false;

  return true;
}


bool inLookRange(float angle, float distance, float bestDistance, RemotePlayer *player)
{
  // usually about 17 degrees
  if (angle >= BZDB.eval(BZDBNAMES.TARGETINGANGLE))
    return false;

  if (distance > bestDistance)
    return false;

  if (player->getFlag() == Flags::Stealth || player->getFlag() == Flags::Cloaking)
    return myTank->getFlag() == Flags::Seer;

  return true;
}


static bool isKillable(const Player *target)
{
  if (target == myTank)
    return false;
  if (target->getTeam() == RogueTeam)
    return true;
  if (World::getWorld()->allowTeamKills() && target->getTeam() != myTank->getTeam())
    return true;

  return false;
}


void setLookAtMarker(void)
{
  // get info about my tank
  const float c = cosf(- myTank->getAngle());
  const float s = sinf(- myTank->getAngle());
  const fvec3& myPos = myTank->getPosition();

  // initialize best target
  Player *bestTarget = NULL;
  float bestDistance = Infinity;

  // figure out which tank is centered in my sights
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!remotePlayers[i] || !remotePlayers[i]->isAlive())
      continue;

    // compute position in my local coordinate system
    const fvec3& rPos = remotePlayers[i]->getPosition();
    const float x = (c * (rPos.x - myPos.x)) - (s * (rPos.y - myPos.y));
    const float y = (s * (rPos.x - myPos.x)) + (c * (rPos.y - myPos.y));

    // ignore things behind me
    if (x < 0.0f)
      continue;

    // get distance and sin(angle) from directly forward
    const float d = hypotf(x, y);
    const float a = fabsf(y / d);


    if (inLookRange(a, d, bestDistance, remotePlayers[i])) {
      // check and see if we can cast a ray from our point to the object
      fvec3 vec = rPos - myPos;

      Ray ray = Ray(myPos, vec);

      // get the list of objects that fall in this ray
      const ObsList *olist = COLLISIONMGR.rayTest (&ray, d);

      bool blocked = false;
      if (olist && olist->count > 0) {
	for (int o = 0; o < olist->count; o++) {
	  const Obstacle *obs = olist->list[o];
          const float timet = obs->intersect(ray);
          if (timet > 1.0f) {
            blocked = true;
            break;
          }
	}
      }

      // if there is nothing between us then go and add it to the list
      if (!blocked) {
	// is it better?
	bestTarget = remotePlayers[i];
	bestDistance = d;
      }
    }
  }

  if (!bestTarget)
    return;

  std::string label = bestTarget->getCallSign();
  if (bestTarget->getFlag() != Flags::Null) {
    std::string flagName = bestTarget->getFlag()->flagAbbv;
    label += std::string("(") + flagName + std::string(")");
  }
  hud->AddEnhancedNamedMarker(bestTarget->getPosition(), Team::getRadarColor(bestTarget->getTeam()), label, !isKillable(bestTarget), 2.0f);
}


static inline bool tankHasShotType(const Player *tank, const FlagType *ft)
{
  const int maxShots = tank->getMaxShots();
  for (int i = 0; i < maxShots; i++) {
    const ShotPath *sp = tank->getShot(i);
    if ((sp != NULL) && (sp->getFlag() == ft)) {
      return true;
    }
  }
  return false;
}


void setTarget()
{
  // get info about my tank
  const float c = cosf(-myTank->getAngle());
  const float s = sinf(-myTank->getAngle());
  const float x0 = myTank->getPosition().x;
  const float y0 = myTank->getPosition().y;

  // initialize best target
  Player *bestTarget = NULL;
  float bestDistance = Infinity;
  bool lockedOn = false;

  // figure out which tank is centered in my sights
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!remotePlayers[i] || !remotePlayers[i]->isAlive()) continue;

    // compute position in my local coordinate system
    const fvec3& pos = remotePlayers[i]->getPosition();
    const float x = c * (pos.x - x0) - s * (pos.y - y0);
    const float y = s * (pos.x - x0) + c * (pos.y - y0);

    // ignore things behind me
    if (x < 0.0f) continue;

    // get distance and sin(angle) from directly forward
    const float d = hypotf(x, y);
    const float a = fabsf(y / d);

    // see if it's inside lock-on angle (if we're trying to lock-on)
    if (a < BZDB.eval(BZDBNAMES.LOCKONANGLE) &&	// about 8.5 degrees
      ((myTank->getFlag() == Flags::GuidedMissile) ||		// am i locking on?
      tankHasShotType(myTank, Flags::GuidedMissile)) &&
      remotePlayers[i]->getFlag() != Flags::Stealth &&		// can't lock on stealth
      !remotePlayers[i]->isPaused() &&				// can't lock on paused
      !remotePlayers[i]->isNotResponding() &&			// can't lock on not responding
      d < bestDistance) {					// is it better?
	bestTarget = remotePlayers[i];
	bestDistance = d;
	lockedOn = true;
    } else if (a < BZDB.eval(BZDBNAMES.TARGETINGANGLE) && // about 17 degrees
      ((remotePlayers[i]->getFlag() != Flags::Stealth) || (myTank->getFlag() == Flags::Seer)) && // can't "see" stealth unless have seer
      d < bestDistance && !lockedOn) { // is it better?
	bestTarget = remotePlayers[i];
	bestDistance = d;
    }
  }
  if (!lockedOn)
    myTank->setTarget(NULL);

  if (!bestTarget) {
    return;
  }

  const bool forbidIdentify = BZDB.isTrue("_forbidIdentify");

  if (lockedOn) {
    myTank->setTarget(bestTarget);
    myTank->setNemesis(bestTarget);

    std::string msg("Locked on");
    if (!forbidIdentify) {
      msg += " ";
      msg += bestTarget->getCallSign();
      msg += " (";
      msg += Team::getName(bestTarget->getTeam());
      if (bestTarget->getFlag() != Flags::Null) {
        msg += ") with ";
        msg += bestTarget->getFlag()->flagName;
      } else {
        msg += ")";
      }
    }
    hud->setAlert(1, msg.c_str(), 2.0f, 1);
    msg = ColorStrings[DefaultColor] + msg;
    addMessage(NULL, msg);
  }
  else if (forbidIdentify) {
    // do nothing
  }
  else if (myTank->getFlag() == Flags::Colorblindness) {
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
    msg += ")";
    if (bestTarget->getFlag() != Flags::Null) {
      msg += " with ";
      msg += bestTarget->getFlag()->flagName;
    }
    hud->setAlert(1, msg.c_str(), 2.0f, 0);
    msg = ColorStrings[DefaultColor] + msg;
    addMessage(NULL, msg);
    myTank->setNemesis(bestTarget);
  }
}


static void setHuntTarget()
{
  if (BZDB.isTrue("_forbidHunting")) {
    return;
  }
  // get info about my tank
  const float  degrees = myTank->getAngle();
  const fvec3& myPos = myTank->getPosition();
  const float c = cosf(-degrees);
  const float s = sinf(-degrees);
  const float x0 = myPos.x;
  const float y0 = myPos.y;

  // initialize best target
  Player *bestTarget = NULL;
  float bestDistance = Infinity;
  bool lockedOn = false;

  // figure out which tank is centered in my sights
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!remotePlayers[i] || !remotePlayers[i]->isAlive()) continue;

    // compute position in my local coordinate system
    const fvec3& pos = remotePlayers[i]->getPosition();
    const float x = c * (pos.x - x0) - s * (pos.y - y0);
    const float y = s * (pos.x - x0) + c * (pos.y - y0);

    // ignore things behind me
    if (x < 0.0f) continue;

    // get distance and sin(angle) from directly forward
    const float d = hypotf(x, y);
    const float a = fabsf(y / d);

    // see if it's inside lock-on angle (if we're trying to lock-on)
    if (a < BZDB.eval(BZDBNAMES.LOCKONANGLE) && // about 8.5 degrees
      myTank->getFlag() == Flags::GuidedMissile && // am i locking on?
      remotePlayers[i]->getFlag() != Flags::Stealth && // can't lock on stealth
      !remotePlayers[i]->isPaused() && // can't lock on paused
      !remotePlayers[i]->isNotResponding() && // can't lock on not responding
      d < bestDistance) { // is it better?
	bestTarget = remotePlayers[i];
	bestDistance = d;
	lockedOn = true;
    } else if (a < BZDB.eval(BZDBNAMES.TARGETINGANGLE) && // about 17 degrees
      ((remotePlayers[i]->getFlag() != Flags::Stealth) ||
      (myTank->getFlag() == Flags::Seer)) && // can't "see" stealth unless have seer
      d < bestDistance && !lockedOn) { // is it better?
	bestTarget = remotePlayers[i];
	bestDistance = d;
    }
  }
  if (!bestTarget) return;

  if (bestTarget->isHunted() &&
      (myTank->getFlag() != Flags::Blindness) &&
      (myTank->getFlag() != Flags::Colorblindness) &&
      (bestTarget->getFlag() != Flags::Stealth)) {
    if (myTank->getTarget() == NULL) { // Don't interfere with GM lock display
      std::string msg("SPOTTED: ");
      msg += bestTarget->getCallSign();
      msg += " (";
      msg += Team::getName(bestTarget->getTeam());
      if (bestTarget->getFlag() != Flags::Null) {
        msg += ") with ";
        msg += bestTarget->getFlag()->flagName;
      } else {
        msg += ")";
      }
      hud->setAlert(1, msg.c_str(), 2.0f, 0);
    }
    if (!pulse.isOn()) {
      const fvec3& bestTargetPosition = bestTarget->getPosition();
      SOUNDSYSTEM.play(SFX_HUNT, bestTargetPosition, false, false);
      pulse.setClock(1.0f);
    }
  }
}


static void updateDaylight(double offset)
{
  static const double SecondsInDay = 86400.0;

  // update sun, moon & sky
  RENDERER.setTimeOfDay(Daylight::unixEpoch + (offset / SecondsInDay));
}


#ifdef ROBOT

//
// some robot stuff
//

static void checkEnvironment(RobotPlayer *tank)
{
  // skip this if i'm dead or paused
  if (!tank->isAlive() || tank->isPaused()) return;

  // see if i've been shot
  const ShotPath *hit = NULL;
  float minTime = Infinity;
  tank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i] && remotePlayers[i]->getId() != tank->getId()) {
      tank->checkHit(remotePlayers[i], hit, minTime);
    }
  }

  // Check Server Shots
  tank->checkHit(World::getWorld()->getWorldWeapons(), hit, minTime);

  float waterLevel = World::getWorld()->getWaterLevel();

  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendHit(tank->getId(), hit->getPlayer(), hit->getShotId());

    // play the I got shot sound
    SOUNDSYSTEM.play(SFX_HIT);

    FlagType *killerFlag = hit->getFlag();
    bool stopShot;

    if (killerFlag == Flags::Thief) {
      if (tank->getFlag() != Flags::Null)
	serverLink->sendTransferFlag(tank->getId(), hit->getPlayer());
      stopShot = true;
    } else {
      stopShot = gotBlowedUp(tank, GotShot, hit->getPlayer(), hit);
    }

    if (stopShot || hit->isStoppedByHit()) {
      Player *hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }
  else if (tank->getDeathPhysicsDriver() >= 0) {
    // if not dead yet, see if i'm sitting on death
    gotBlowedUp(tank, PhysicsDriverDeath, ServerPlayer, NULL,
      tank->getDeathPhysicsDriver());
  }
  else if ((waterLevel > 0.0f) && (tank->getPosition().z <= waterLevel)) {
    // if not dead yet, see if the robot dropped below the death level
    gotBlowedUp(tank, WaterDeath, ServerPlayer);
  }
  else { // if not dead yet, see if i got run over by the steamroller
    // robot vs. myTank
    if (checkSquishKill(tank, myTank, true)) {
      gotBlowedUp(tank, GotRunOver, myTank->getId());
    }
    else {
      // robot vs. remote tanks
      for (i = 0; i < curMaxPlayers; i++) {
        const Player* remote = remotePlayers[i];
        if (checkSquishKill(tank, remote)) {
          gotBlowedUp(tank, GotRunOver, remotePlayers[i]->getId());
          break;
        }
      }
    }
  }
}


static void checkEnvironmentForRobots()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i])
      checkEnvironment(robots[i]);
}

#endif


static void changePlayerTeam(Player* player, TeamColor oldTeam,
                                             TeamColor newTeam)
{
  if (player != myTank) {
    return;
  }

  // FIXME -- player location 

  // change the roaming state
  if (newTeam != ObserverTeam) {
    ROAM.setMode(Roaming::roamViewDisabled);
  }
  else if (!ROAM.isRoaming()) {
    const std::string roamStr = BZDB.get("roamView");
    Roaming::RoamingView roamView = ROAM.parseView(BZDB.get("roamView"));
    if (roamView <= Roaming::roamViewDisabled) {
      roamView = Roaming::roamViewFP;
    }
    ROAM.setMode(roamView);
  }

  if ((oldTeam == ObserverTeam) &&
      (newTeam != ObserverTeam)) {
    BZDB.setBool("slowMotion", false);
  }

  // observer colors are actually cyan, make them black
  const bool observer = (newTeam == ObserverTeam);
  const fvec4* borderColor;
  if (observer) {
    static const fvec4 black(0.0f, 0.0f, 0.0f, 1.0f);
    borderColor = &black;
  } else {
    borderColor = &Team::getRadarColor(myTank->getTeam());
  }
  if (controlPanel) {
    controlPanel->setControlColor(borderColor);
  }
  if (radar) {
    radar->setControlColor(borderColor);
  }
}


void enteringServer(void* buf)
{
#if defined(ROBOT)
  int maxBots = world->getBotsPerIP();
  if (numRobotTanks > maxBots) {
    showMessage(
	TextUtils::format("The server only allows %d robot %s.",
	  maxBots, (maxBots == 1 ? "player" : "players")));
    showMessage("Additional robot players have been removed.");
    numRobotTanks = maxBots;
  }
  int i;
  for (i = 0; i < numRobotTanks; i++)
    serverLink->sendNewPlayer(i,AutomaticTeam);
  numRobots = 0;
#endif
  // the server sends back the team the player was joined to
  void *tmpbuf = buf;
  int16_t team;
  uint16_t type, wins, losses, tks;
  float rank;
  tmpbuf = nboUnpackUInt16(tmpbuf, type);
  tmpbuf = nboUnpackInt16(tmpbuf, team);
  tmpbuf = nboUnpackFloat(tmpbuf, rank);
  tmpbuf = nboUnpackUInt16(tmpbuf, wins);
  tmpbuf = nboUnpackUInt16(tmpbuf, losses);
  tmpbuf = nboUnpackUInt16(tmpbuf, tks);

  myTank->changeScore(rank, wins, losses, tks);

  const TeamColor teamColor = (TeamColor)team;
  const char* teamName = Team::getName(teamColor);

  // if server assigns us a different team, display a message
  std::string teamMsg;
  if (myTank->getTeam() != AutomaticTeam) {
    teamMsg = TextUtils::format("%s team was unavailable, you were joined ",
                                Team::getName(myTank->getTeam()));
    if (teamColor == ObserverTeam) {
      teamMsg += "as an Observer";
    } else {
      teamMsg += TextUtils::format("to the %s", teamName);
    }
  }
  else {
    if (teamColor == ObserverTeam) {
      teamMsg = "You were joined as an observer";
    }
    else {
      if (team != RogueTeam) {
	teamMsg = TextUtils::format("You joined the %s", teamName);
      } else {
	teamMsg = TextUtils::format("You joined as a %s", teamName);
      }
    }
  }

  if (myTank->getTeam() != teamColor) {
    myTank->setTeam(teamColor);
    hud->setAlert(1, teamMsg.c_str(), 8.0f, teamColor == ObserverTeam);
    addMessage(NULL, teamMsg.c_str(), ControlPanel::MessageMisc, true);
  }

  // observer colors are actually cyan, make them black
  const bool observer = (myTank->getTeam() == ObserverTeam);
  const fvec4* borderColor;
  if (observer) {
    static const fvec4 black(0.0f, 0.0f, 0.0f, 1.0f);
    borderColor = &black;
  } else {
    borderColor = &Team::getRadarColor(myTank->getTeam());
  }
  controlPanel->setControlColor(borderColor);
  radar->setControlColor(borderColor);

  if (myTank->getTeam() != ObserverTeam) {
    ROAM.setMode(Roaming::roamViewDisabled);
  }
  else {
    if (!ROAM.isRoaming()) {
      const std::string roamStr = BZDB.get("roamView");

      Roaming::RoamingView roamView = ROAM.parseView(BZDB.get("roamView"));
      if (roamView <= Roaming::roamViewDisabled) {
        roamView = Roaming::roamViewFP;
      }
      ROAM.setMode(roamView);
    }
  }

  setTankFlags();

  // clear now invalid token
  startupInfo.token[0] = '\0';

  // add robot tanks
#if defined(ROBOT)
  addRobots();
#endif

  // resize background and adjust time (this is needed even if we
  // don't sync with the server)
  RENDERER.getBackground()->resize();
  if (bzdbSyncTime < 0.0f) {
    updateDaylight(epochOffset);
  } else {
    epochOffset = (double)bzdbSyncTime;
    updateDaylight(epochOffset);
  }
  lastEpochOffset = epochOffset;

  // restore the sound
  if (savedVolume != -1) {
    SOUNDSYSTEM.setVolume(savedVolume*0.1f);
    savedVolume = -1;
  }

  // initialize some other stuff
  updateNumPlayers();
  updateFlag(Flags::Null);
  updateHighScores();
  hud->setHeading(myTank->getAngle());
  hud->setAltitude(myTank->getPosition().z);
  hud->setTimeLeft((uint32_t)~0);
  fireButton = false;
  firstLife = true;

  BZDB.setBool("displayMainFlags",  true);
  BZDB.setBool("displayRadarFlags", true);
  BZDB.setBool("displayRadar",      true);
  BZDB.setBool("displayConsole",    true);
  if (myTank->getTeam() != ObserverTeam) {
    BZDB.setBool("slowMotion", false);
  }

  entered = true;
}


static void resetServerVar(const std::string &name, void*)
{
  // reset server-side variables
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    const std::string defval = BZDB.getDefault(name);
    BZDB.set(name, defval);
  }
}


void leaveGame()
{

  entered = false;
  joiningGame = false;

  thirdPersonVars.clear();
  MsgStrings::reset();

  // -- don't know enough to remove this code, but
  // -- the 'radar' pointer points to '_radar' in startPlaying()
  //
  // no more radar
  //  radar->setWorld(NULL);
  //  controlPanel->setRadarRenderer(NULL);
  /*
  delete radar;
  radar = NULL;
  */

#if defined(ROBOT)
  // shut down robot connections
  int i;
  for (i = 0; i < numRobots; i++) {
    if (robots[i])
      serverLink->sendExit();
    delete robots[i];
    robots[i] = NULL;
  }
  numRobots = 0;

  std::vector<BzfRegion*>::iterator itr;
  for (itr = obstacleList.begin(); itr != obstacleList.end(); ++itr) {
    delete (*itr);
  }
  obstacleList.clear();
#endif

  // my tank goes away
  const bool sayGoodbye = (myTank != NULL);
  LocalPlayer::setMyTank(NULL);
  delete myTank;
  myTank = NULL;

  // reset the daylight time
  const bool fixedTime = BZDB.isSet("fixedTime");
  if (bzdbSyncTime >= 0.0f) {
    // return to the desired user time
    epochOffset = userTimeEpochOffset;
  }
  else if (fixedTime) {
    // save the current user time
    userTimeEpochOffset = epochOffset;
  }
  else {
    // revert back to when the client was started?
    epochOffset = userTimeEpochOffset;
  }

  updateDaylight(epochOffset);
  lastEpochOffset = epochOffset;
  BZDB.set(BZDBNAMES.SYNCTIME,
           BZDB.getDefault(BZDBNAMES.SYNCTIME));

  // flush downloaded textures (before the BzMaterials are nuked)
  Downloads::instance().removeTextures();

  // delete world
  World::setWorld(NULL);
  delete world;
  world = NULL;
  teams = NULL;
  curMaxPlayers = 0;
  numFlags = 0;
  remotePlayers = NULL;

  // update UI
  hud->setPlaying(false);
  hud->setCracks(false);
  hud->setPlayerHasHighScore(false);
  hud->setTeamHasHighScore(false);
  hud->setHeading(0.0f);
  hud->setAltitude(0.0f);
  hud->setAltitudeTape(false);

  // shut down server connection
  if (sayGoodbye) serverLink->sendExit();
  ServerLink::setServer(NULL);
  delete serverLink;
  serverLink = NULL;

  // reset viewpoint
  fvec3 eyePoint, targetPoint;
  eyePoint.x = 0.0f;
  eyePoint.y = 0.0f;
  eyePoint.z = 0.0f + BZDBCache::muzzleHeight;
  targetPoint.x = eyePoint.x - 1.0f;
  targetPoint.y = eyePoint.y + 0.0f;
  targetPoint.z = eyePoint.z + 0.0f;
  RENDERER.getViewFrustum().setProjection((float)(60.0 * DEG2RAD),
    NearPlaneNormal,
    FarPlaneDefault,
    FarDeepPlaneDefault,
    mainWindow->getWidth(),
    mainWindow->getHeight(),
    mainWindow->getViewHeight());
  RENDERER.getViewFrustum().setView(eyePoint, targetPoint);

  // reset some flags
  gameOver = false;
  serverError = false;
  serverDied = false;

  // delete scene database (after the world has been destroyed)
  RENDERER.setSceneDatabase(NULL);

  // reset the BZDB variables
  BZDB.iterate(resetServerVar, NULL);

  // purge any custom flags we may have accumulated
  Flags::clearCustomFlags();

  return;
}


static void addVarToAutoComplete(const std::string &name, void* /*userData*/)
{
  if ((name.size() <= 0) || ((name[0] != '_') && (name[0] != '$'))) {
    return; // skip private variables
  }

  if (BZDB.getPermission(name) == StateDatabase::Server) {
    completer.registerWord(name);
  }

  return;
}


void joinInternetGame2()
{
  justJoined = true;

  showError("Entering game...");

  ServerLink::setServer(serverLink);
  World::setWorld(world);

  canSpawn = true;

  // prep teams
  teams = world->getTeams();

  // prep players
  curMaxPlayers = 0;
  remotePlayers = world->getPlayers();
  playerSize = world->getPlayersSize();

  // reset the autocompleter
  completer.reset();
  BZDB.iterate(addVarToAutoComplete, NULL);

  // prep flags
  numFlags = world->getMaxFlags();

  // make scene database
  setSceneDatabase();
  mainWindow->getWindow()->yieldCurrent();

  // make radar
  //  radar = new RadarRenderer(*sceneRenderer, *world);
  //  mainWindow->getWindow()->yieldCurrent();
  radar->setWorld(world);
  controlPanel->setRadarRenderer(radar);
  controlPanel->resize();

  // make local player
  myTank = new LocalPlayer(serverLink->getId(), startupInfo.callsign);
  myTank->setTeam(startupInfo.team);
  LocalPlayer::setMyTank(myTank);

  if (world->allowRabbit() && myTank->getTeam() != ObserverTeam)
    myTank->setTeam(HunterTeam);

  // tell server we want to join
  bool noSounds = BZDB.isSet ("_noRemoteSounds") && BZDB.isTrue ("_noRemoteSounds");
  serverLink->sendEnter(myTank->getId(), TankPlayer, AllUpdates, myTank->getTeam(),
                        myTank->getCallSign(),
                        startupInfo.token,
                        startupInfo.referrer);
  startupInfo.token[0] = '\0';
  startupInfo.referrer[0] = '\0';

  // Get the server's server key
  std::string serverKey = startupInfo.serverName;
  const unsigned int port = (int)ntohs((unsigned short)startupInfo.serverPort);
  if (port != ServerPort) {
    char portBuf[20];
    sprintf(portBuf, "%d", port);
    serverKey += ":";
    serverKey += portBuf;
  }

  //ServerItem server = new ServerItem();
  //Address::getHostByAddress(info.ping.serverId.serverHost);

  ServerList &serverList = ServerList::instance();

  if (!(serverList.lookupServer(serverKey) == NULL))
    serverList.markAsRecent(serverList.lookupServer(serverKey));

  serverLink->sendCaps(myTank->getId(), true, !noSounds);

  // give them our motto
  if (myTank && startupInfo.motto.size())
    myTank->customData["motto"] = startupInfo.motto;

  // send all our custom data pairs
  std::map<std::string, std::string>::iterator itr = myTank->customData.begin();
  while (itr != myTank->customData.end()) {
    serverLink->sendCustomData(itr->first, itr->second);
    itr++;
  }

  ExportInformation &ei = ExportInformation::instance();
  ei.setInformation("Callsign", myTank->getCallSign(), ExportInformation::eitPlayerInfo, ExportInformation::eipPrivate);
  ei.setInformation("Team", Team::getName(myTank->getTeam()), ExportInformation::eitPlayerInfo, ExportInformation::eipPrivate);
  ei.setInformation("Server", TextUtils::format("%s:%d", startupInfo.serverName, startupInfo.serverPort),
    ExportInformation::eitServerInfo, ExportInformation::eipStandard);

  // hopefully it worked!  pop all the menus.
  HUDDialogStack *stack = HUDDialogStack::get();
  while (stack->isActive())
    stack->pop();
  joiningGame = false;
}


static void renderDialog()
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


static void checkDirtyControlPanel(ControlPanel *cp)
{
  if (cp) {
    if (HUDDialogStack::get()->isActive()) {
      cp->invalidate();
    }
  }
  return;
}


static void drawUI()
{
  // setup the triangle counts  (FIXME: hackish)
  if (showDrawFPS && showDrawTime) {
    hud->setFrameTriangleCount(RENDERER.getFrameTriangleCount());
    // NOTE:  the radar triangle count is actually from the previous frame
    if (radar)
      hud->setFrameRadarTriangleCount(radar->getFrameTriangleCount());
    else
      hud->setFrameRadarTriangleCount(0);
  } else {
    hud->setFrameTriangleCount(0);
    hud->setFrameRadarTriangleCount(0);
  }

  // update the HUD (player list, alerts)
  if (hud) {
    hud->render();
  }

  // draw the control panel
  if (controlPanel) {
    controlPanel->render(RENDERER);
  }

  // draw the radar
  if (radar) {
    const bool showBlankRadar = !myTank || (myTank && myTank->isPaused());
    const bool observer = myTank && (myTank->getTeam() == ObserverTeam);
    radar->render(RENDERER, showBlankRadar, observer);
  }

  // update the HUD (menus)
  renderDialog();

  return;
}


//
// stuff to draw a frame
//

static bool trackPlayerShot(Player *target,
			    fvec3& eyePoint, fvec3& targetPoint)
{
  // follow the first shot
  if (BZDB.isTrue("trackShots")) {
    const int maxShots = target->getMaxShots();
    const ShotPath *sp = NULL;
    // look for the oldest active shot
    float remaining = +MAXFLOAT;
    for (int s = 0; s < maxShots; s++) {
      const ShotPath *spTmp = target->getShot(s);
      if (spTmp != NULL) {
	const float t = float(spTmp->getReloadTime() -
	  (spTmp->getCurrentTime() - spTmp->getStartTime()));
	if ((t > 0.0f) && (t < remaining)) {
	  sp = spTmp;
	  remaining = t;
	}
      }
    }
    if (sp != NULL) {
      const fvec3& pos = sp->getPosition();
      const fvec3& vel = sp->getVelocity();
      const float speed = vel.length();
      if (speed > 0.0f) {
	const float ilen = 1.0f / speed;
	const fvec3 dir = vel * ilen;
	fvec3 topDir(1.0f, 0.0f, 0.0f);
	const float hlen = dir.xy().length();
	if (hlen > 0.0f) {
	  topDir.z = hlen;
	  const float hfactor = -fabsf(dir.z / hlen);
	  topDir.x = hfactor * dir.x;
	  topDir.y = hfactor * dir.y;
	}
	const float offset = -10.0f;
	const float tOffset = +2.0f;
	eyePoint = pos + (offset * dir) + (tOffset * topDir);
	targetPoint = eyePoint + dir;
	return true;
      }
    }
  }
  return false;
}


static void setupNearPlane()
{
  NearPlane = NearPlaneNormal;

  const bool showTreads = BZDB.isTrue("showTreads") ||
                          thirdPersonVars.b3rdPerson;
  if (!showTreads || !myTank)
    return;

  const Player *tank = myTank;
  if (ROAM.isRoaming()) {
    if (ROAM.getMode() != Roaming::roamViewFP) {
      return;
    }
    tank = ROAM.getTargetTank();
  }
  if (tank == NULL)
    return;

  const float halfLength = 0.5f * BZDBCache::tankLength;
  const float length = tank->getDimensions().y;
  if (fabsf(length - halfLength) > 0.1f)
    NearPlane = NearPlaneClose;

  return;
}


static void setupFarPlane()
{
  FarPlane = FarPlaneScale * BZDBCache::worldSize;
  FarPlaneCull = false;
  FarDeepPlane = FarPlane * FarDeepPlaneScale;

  const bool mapFog = BZDB.get(BZDBNAMES.FOGMODE) != "none";

  float farDist = FarPlane;

  if (BZDB.get("_cullDist") == "fog") {
    if (mapFog && !BZDB.isTrue("_fogNoSky")) {
      const float fogMargin = 1.01f;
      const std::string &fogMode = BZDB.get("_fogMode");
      if (fogMode == "linear") {
	farDist = fogMargin * BZDB.eval("_fogEnd");
      }
      else {
	const float density = BZDB.eval("_fogDensity");
	if (density > 0.0f) {
	  const float fogFactor = 0.01f;
	  if (fogMode == "exp2") {
	    farDist = fogMargin * sqrtf(-logf(fogFactor)) / density;
	  } else { // default to 'exp'
	    farDist = fogMargin * -logf(fogFactor) / density;
          }
	} else {
	  // default far plane
	}
      }
    }
    else {
      // default far plane
    }
  }
  else {
    const float dist = BZDB.eval("_cullDist");
    if (dist > 0.0f) {
      farDist = dist;
    }
    /* else */
    // default far plane
  }

  if (farDist < FarPlane) {
    FarPlane = farDist;
    FarPlaneCull = true;
  }

  return;
}


//============================================================================//

static void drawThreeChannel(float fov, const fvec3& myTankDir,
                             fvec3& eyePoint, fvec3& targetPoint)
{
  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  // draw center channel
  RENDERER.render(false);
  drawUI();

  // set up for drawing left channel
  mainWindow->setQuadrant(MainWindow::LowerLeft);
  // FIXME -- this assumes up is along +z
  const float cFOV = cosf(fov);
  const float sFOV = sinf(fov);
  targetPoint.x = eyePoint.x + (cFOV * myTankDir.x) - (sFOV * myTankDir.y);
  targetPoint.y = eyePoint.y + (cFOV * myTankDir.y) + (sFOV * myTankDir.x);
  targetPoint.z = eyePoint.z + myTankDir.z;
  viewFrustum.setView(eyePoint, targetPoint);

  // draw left channel
  RENDERER.render(false, true, true);

  // set up for drawing right channel
  mainWindow->setQuadrant(MainWindow::LowerRight);
  // FIXME -- this assumes up is along +z
  targetPoint.x = eyePoint.x + (cFOV * myTankDir.x) + (sFOV * myTankDir.y);
  targetPoint.y = eyePoint.y + (cFOV * myTankDir.y) - (sFOV * myTankDir.x);
  targetPoint.z = eyePoint.z + myTankDir.z;
  viewFrustum.setView(eyePoint, targetPoint);

  // draw right channel
  RENDERER.render(true, true, true);

#if defined(DEBUG_RENDERING)
  // set up for drawing rear channel
  mainWindow->setQuadrant(MainWindow::UpperLeft);
  // FIXME -- this assumes up is along +z
  targetPoint.x = eyePoint.x - myTankDir.x;
  targetPoint.y = eyePoint.y - myTankDir.y;
  targetPoint.z = eyePoint.z + myTankDir.z;
  viewFrustum.setView(eyePoint, targetPoint);

  // draw rear channel
  RENDERER.render(true, true, true);
#endif
  // back to center channel
  mainWindow->setQuadrant(MainWindow::UpperRight);
}


//============================================================================//

static void drawStacked()
{
  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  float EyeDisplacement = 0.25f * BZDBCache::tankWidth;
  float FocalPlane = BZDB.eval(BZDBNAMES.BOXBASE);
  if (BZDB.isSet("eyesep")) {
    EyeDisplacement = BZDB.eval("eyesep");
  }
  if (BZDB.isSet("focal")) {
    FocalPlane = BZDB.eval("focal");
  }

  // setup view for left eye
  viewFrustum.setOffset(EyeDisplacement, FocalPlane);

  // draw left eye's view
  RENDERER.render(false);
  drawUI();

  // set up view for right eye
  mainWindow->setQuadrant(MainWindow::UpperHalf);
  viewFrustum.setOffset(-EyeDisplacement, FocalPlane);

  // draw right eye's view
  RENDERER.render(true, true);
  drawUI();

  // draw common stuff

  // back to left channel
  mainWindow->setQuadrant(MainWindow::LowerHalf);
}


//============================================================================//

static void drawStereo()
{
  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  float EyeDisplacement = 0.25f * BZDBCache::tankWidth;
  float FocalPlane = BZDB.eval(BZDBNAMES.BOXBASE);
  if (BZDB.isSet("eyesep"))
    EyeDisplacement = BZDB.eval("eyesep");
  if (BZDB.isSet("focal"))
    FocalPlane = BZDB.eval("focal");

  // setup view for left eye
#ifdef USE_GL_STEREO
  glDrawBuffer(GL_BACK_LEFT);
#endif
  viewFrustum.setOffset(EyeDisplacement, FocalPlane);

  // draw left eye's view
  RENDERER.render(false);
#ifndef USE_GL_STEREO
  drawUI();
#endif

  // set up view for right eye
#ifdef USE_GL_STEREO
  glDrawBuffer(GL_BACK_RIGHT);
#else
  mainWindow->setQuadrant(MainWindow::UpperLeft);
#endif
  viewFrustum.setOffset(-EyeDisplacement, FocalPlane);

  // draw right eye's view
  RENDERER.render(true, true);
#ifndef USE_GL_STEREO
  drawUI();
#endif

  // draw common stuff
#ifdef USE_GL_STEREO
  glDrawBuffer(GL_BACK);
  drawUI();
#endif

#ifndef USE_GL_STEREO
  // back to left channel
  mainWindow->setQuadrant(MainWindow::UpperRight);
#endif
}


//============================================================================//

static void drawAnaglyph()
{
  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  float EyeDisplacement = 0.25f * BZDBCache::tankWidth;
  float FocalPlane = BZDB.eval(BZDBNAMES.BOXBASE);
  if (BZDB.isSet("eyesep"))
    EyeDisplacement = BZDB.eval("eyesep");
  if (BZDB.isSet("focal"))
    FocalPlane = BZDB.eval("focal");

  // setup view for left eye
  glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
  viewFrustum.setOffset(EyeDisplacement, FocalPlane);

  // draw left eye's view
  RENDERER.render(false);
  drawUI();

  // set up view for right eye
  glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
  // for red/blue to somewhat work ...
  //glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
  viewFrustum.setOffset(-EyeDisplacement, FocalPlane);

  // draw right eye's view
  RENDERER.render(true, true);
  drawUI();
}


//============================================================================//

static void drawInterlaced()
{
  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  float EyeDisplacement = 0.25f * BZDBCache::tankWidth;
  float FocalPlane = BZDB.eval(BZDBNAMES.BOXBASE);
  const int width = mainWindow->getWidth();
  const int height = mainWindow->getHeight();
  if (BZDB.isSet("eyesep"))
    EyeDisplacement = BZDB.eval("eyesep");
  if (BZDB.isSet("focal"))
    FocalPlane = BZDB.eval("focal");

  if (BZDBCache::shadowMode == SceneRenderer::StencilShadows) {
    BZDB.setInt("shadowMode", SceneRenderer::StippleShadows);
    addMessage(NULL, "Disabled stencilShadows for interlaced mode");
  }

  OpenGLGState::resetState();
  // enable stencil test
  glEnable(GL_STENCIL_TEST);

  // clear stencil
  glClearStencil(0);
  // Clear color and stencil buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  // All drawing commands fail the stencil test, and are not
  // drawn, but increment the value in the stencil buffer.
  glStencilFunc(GL_NEVER, 0x0, 0x0);
  glStencilOp(GL_INCR, GL_INCR, GL_INCR);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int y = 0; y <= height; y += 2) {
    glBegin(GL_LINES);
    glVertex2i(0, y);
    glVertex2i(width, y);
    glEnd();
  }

  // draw except where the stencil pattern is 0x1
  // do not change the stencil buffer
  glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  // setup view for left eye
  viewFrustum.setOffset(EyeDisplacement, FocalPlane);
  // draw left eye's view
  RENDERER.render(false);

  // draw where the stencil pattern is 0x1
  // do not change the stencil buffer
  glStencilFunc(GL_EQUAL, 0x1, 0x1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  // set up view for right eye
  viewFrustum.setOffset(-EyeDisplacement, FocalPlane);
  // draw right eye's view
  RENDERER.render(true, true);

  glStencilFunc(GL_ALWAYS, 0x1, 0x1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  drawUI();
}


//============================================================================//

static void setupRoamingCamera(float muzzleHeight,
                               fvec3& eyePoint, fvec3& targetPoint, float& fov)
{
  hud->setAltitude(-1.0f);

  float roamViewAngle;
  const Roaming::RoamingCamera* roam = ROAM.getCamera();

  if ((ROAM.getMode() != Roaming::roamViewFree) && ROAM.getTargetTank()) {
    Player* target = ROAM.getTargetTank();
    const fvec3& targetTankDir = target->getForward();

    if (ROAM.getMode() == Roaming::roamViewTrack) {
      // fixed camera tracking target
      eyePoint = roam->pos;
      targetPoint = target->getPosition();
      targetPoint.z += target->getMuzzleHeight();
    }
    else if (ROAM.getMode() == Roaming::roamViewFollow) {
      // camera following target
      if (!trackPlayerShot(target, eyePoint, targetPoint)) {
        targetPoint = target->getPosition();
        eyePoint    = target->getPosition();
        eyePoint.x -= targetTankDir.x * 40.0f;
        eyePoint.y -= targetTankDir.y * 40.0f;
        eyePoint.z += muzzleHeight * 6.0f;
      }
    }
    else if (ROAM.getMode() == Roaming::roamViewFP) {
      // target's view
      if (!trackPlayerShot(target, eyePoint, targetPoint)) {
        eyePoint = target->getPosition();
        eyePoint.z += target->getMuzzleHeight();
        targetPoint = eyePoint + targetTankDir;
        hud->setAltitude(target->getPosition().z);
      }
    }
    else if (ROAM.getMode() == Roaming::roamViewFlag) {
      // track team flag
      Flag *targetFlag = ROAM.getTargetFlag();
      eyePoint = roam->pos;
      targetPoint = targetFlag->position;
      if (targetFlag->status != FlagOnTank) {
        targetPoint.z += muzzleHeight;
      } else {
        targetPoint.z -= (BZDBCache::tankHeight - BZDBCache::muzzleHeight);
      }
    }
    const fvec2 delta2d = targetPoint.xy() - eyePoint.xy();
    roamViewAngle = (float)(RAD2DEG * atan2(delta2d.y, delta2d.x));
  }
  else {
    // free Roaming
    const float phiRadians   = (float)(roam->phi   * DEG2RAD);
    const float thetaRadians = (float)(roam->theta * DEG2RAD);
    const fvec3 dir(cosf(phiRadians) * cosf(thetaRadians),
                    cosf(phiRadians) * sinf(thetaRadians),
                    sinf(phiRadians));
    eyePoint = roam->pos;
    targetPoint = eyePoint + dir;
    roamViewAngle = roam->theta;
  }

  const fvec3 virtPos(eyePoint.x, eyePoint.y, 0.0f);
  if (myTank) {
    myTank->move(virtPos, (float)(roamViewAngle * DEG2RAD));
  }

  fov = (float)(roam->zoom * DEG2RAD);
}


static void setupCamera(const fvec3& myTankPos, const fvec3& myTankDir,
                        float muzzleHeight,
                        fvec3& eyePoint, fvec3& targetPoint, float& fov)
{
  if (ROAM.isRoaming()) {
    setupRoamingCamera(muzzleHeight, eyePoint, targetPoint, fov);
  }
  else {
    // default setup
    eyePoint = myTankPos;
    eyePoint.z += muzzleHeight;
    targetPoint = eyePoint + myTankDir;

    if (myTank && thirdPersonVars.b3rdPerson) {
      // 3rd person camera
      const float distScale = thirdPersonVars.targetMultiplier;
      targetPoint = eyePoint + (distScale * myTankDir);
      const float offsetXY = thirdPersonVars.cameraOffsetXY;
      const float offsetZ  = thirdPersonVars.cameraOffsetZ;
      eyePoint.x -= (myTankDir.x * offsetXY);
      eyePoint.y -= (myTankDir.y * offsetXY);
      eyePoint.z += (muzzleHeight * offsetZ);
    }
  }

  // also set the position and rotation for the sound system
  const fvec2 dir = (targetPoint.xy() - eyePoint.xy());
  const float theta = atan2f(dir.y, dir.x);
  SOUNDSYSTEM.setReceiver(eyePoint.x, eyePoint.y, eyePoint.z, theta, false);
}


//============================================================================//


static void addDynamicSceneNodes()
{
  SceneDatabase* scene = RENDERER.getSceneDatabase();
  if (!scene || !myTank) {
    return;
  }

  const bool seerView = (myTank->getFlag() == Flags::Seer);
  const bool showTreads = BZDB.isTrue("showTreads") ||
                          thirdPersonVars.b3rdPerson;

  // add my tank if required
  const bool inMyCockpit = true;
  const bool showMyTreads = showTreads;

  myTank->addToScene(scene, myTank->getTeam(),
                     inMyCockpit, seerView,
                     showMyTreads, showMyTreads /*showIDL*/);

  // add my shells
  myTank->addShots(scene, false);

  // add server shells
  if (world) {
    world->getWorldWeapons()->addShots(scene, false);
  }

  // add antidote flag
  myTank->addAntidote(scene);

  // add flags
  world->addFlags(scene, seerView);

  // add other tanks and shells
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      const bool colorblind = (myTank->getFlag() == Flags::Colorblindness);
      remotePlayers[i]->addShots(scene, colorblind);

      TeamColor effectiveTeam = RogueTeam;
      if (!colorblind) {
        if ((remotePlayers[i]->getFlag() == Flags::Masquerade)
            && (myTank->getFlag() != Flags::Seer)
            && (myTank->getTeam() != ObserverTeam)) {
          effectiveTeam = myTank->getTeam();
        } else {
          effectiveTeam = remotePlayers[i]->getTeam();
        }
      }

      const bool inCockpit =
        ROAM.isRoaming() && (ROAM.getMode() == Roaming::roamViewFP) &&
        ROAM.getTargetTank() && (ROAM.getTargetTank()->getId() == i);
      const bool showPlayer = !inCockpit || showTreads;

      // add player tank if required
      if ((myTank->getFlag() == Flags::Seer) || showPlayer) {
        remotePlayers[i]->addToScene(scene, effectiveTeam, inCockpit,
                                     seerView, showPlayer, showPlayer,
                                     thirdPersonVars.b3rdPerson);
      }
    }
  }

  // add explosions
  addExplosions(scene);

  // if inside a building, add some eighth dimension scene nodes.
  const std::vector<const Obstacle*> &list = myTank->getInsideBuildings();
  for (unsigned int n = 0; n < list.size(); n++) {
    const Obstacle *obs = list[n];
    const int nodeCount = obs->getInsideSceneNodeCount();
    SceneNode **nodeList = obs->getInsideSceneNodeList();
    for (int o = 0; o < nodeCount; o++) {
      scene->addDynamicNode(nodeList[o]);
    }
  }
}


//============================================================================//

static void drawFakeCursor(int type)
{
  int mx, my;
  const int width = mainWindow->getWidth();
  const int height = mainWindow->getHeight();
  const int ox = mainWindow->getOriginX();
  const int oy = mainWindow->getOriginY();
  mainWindow->getWindow()->getMouse(mx, my);
  my = height - my - 1; // flip the y axis

  glScissor(ox, oy, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if (type <= 1) {
    // old style
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
  }
  else {
    // new style
    const int xc = ox + (width / 2);
    const int y2 = oy + (mainWindow->getViewHeight() / 2);
    const int yc = (height - y2 - 1); // flip the y axis

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);

    const TeamColor myTeam = (myTank != NULL) ? myTank->getTeam()
                                              : ObserverTeam;
    if (myTeam != ObserverTeam) {
      // draw a drag bar
      const fvec4 fadeWhite(1.0f, 1.0f, 1.0f, 0.2f);
      const fvec4& teamColor = Team::getRadarColor(myTeam);
      glLineWidth(1.49f);
      glBegin(GL_LINES);
      glColor4fv(fadeWhite); glVertex2i(xc, yc);
      glColor4fv(teamColor); glVertex2i(mx, my);
      glEnd();
      glLineWidth(1.0f);
    }
    else {
      // draw a cross
      const fvec4 fadeWhite(1.0f, 1.0f, 1.0f, 0.0f);
      const fvec4 fullWhite(1.0f, 1.0f, 1.0f, 1.0f);
      const float dist = 16.0f;
      glLineWidth(1.49f);
      glBegin(GL_LINES);
      glColor4fv(fullWhite); glVertex2i(mx, my);
      glColor4fv(fadeWhite); glVertex2i((int)(mx - dist), my);
      glColor4fv(fullWhite); glVertex2i(mx, my);
      glColor4fv(fadeWhite); glVertex2i((int)(mx + dist), my);
      glColor4fv(fullWhite); glVertex2i(mx, my);
      glColor4fv(fadeWhite); glVertex2i(mx, (int)(my - dist));
      glColor4fv(fullWhite); glVertex2i(mx, my);
      glColor4fv(fadeWhite); glVertex2i(mx, (int)(my + dist));
      glEnd();
      glLineWidth(1.0f);
    }
 
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f); glVertex2i(mx, my);
    glEnd();

    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 0.0f); glVertex2i(mx, my);
    glEnd();
    glPointSize(1.0f);

    glShadeModel(GL_FLAT);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
  }

  glPopMatrix();
}


void handleMouseDrawing()
{
  static bool cursorIsHidden = false;
  static BZDB_int fakeCursor("fakecursor");

  if (fakeCursor) {
    if (!cursorIsHidden) {
      cursorIsHidden = true;
      mainWindow->getWindow()->hideMouse();
    }
    if (fakeCursor) {
      drawFakeCursor(fakeCursor);
    }
  }
  else {
    if (cursorIsHidden) {
      cursorIsHidden = false;
      mainWindow->getWindow()->showMouse();
    }
  }
}


//============================================================================//

void drawFrame(const float dt)
{
  static SceneRenderer::ViewType viewType = RENDERER.getViewType();
  static BzfMedia* media = PlatformFactory::getMedia();

  static const fvec3 defaultPos(0.0f, 0.0f, 0.0f);
  static const fvec3 defaultDir(1.0f, 0.0f, 0.0f);
  static int   frameCount = 0;
  static float cumTime = 0.0f;

  const fvec3* myTankPos;
  const fvec3* myTankDir;
  float fov;
  fvec3 eyePoint;
  fvec3 targetPoint;

  checkDirtyControlPanel(controlPanel);

  // compute fps
  frameCount++;
  cumTime += float(dt);
  if (cumTime >= 2.0) {
    if (showDrawFPS) {
      hud->setFPS(float(frameCount) / cumTime);
    } else {
      hud->setFPS(-1.0f);
    }
    cumTime = 0.00000001f;
    frameCount = 0;
  }

  // drift clouds
  RENDERER.getBackground()->addCloudDrift(1.0f * dt, 0.731f * dt);

  // get tank camera info
  float muzzleHeight;
  if (!myTank) {
    myTankPos = &defaultPos;
    myTankDir = &defaultDir;
    muzzleHeight = BZDBCache::muzzleHeight;
    fov = BZDB.eval("defaultFOV");
  }
  else {
    myTankPos = &myTank->getPosition();
    myTankDir = &myTank->getForward();
    muzzleHeight = myTank->getMuzzleHeight();

    fov = BZDB.eval("displayFOV");
    if (myTank->getFlag() == Flags::WideAngle) {
      fov *= 2.0f;
    }
    if (viewType == SceneRenderer::ThreeChannel) {
      fov *= 0.75f;
    }
  }
  fov *= (float)DEG2RAD;

  setupCamera(*myTankPos, *myTankDir, muzzleHeight, // const
              eyePoint, targetPoint, fov);        // modified

  // only use a close plane for drawing in the
  // cockpit, and even then only for odd sized tanks
  setupNearPlane();

  // based on fog and _cullDist
  setupFarPlane();

  ViewFrustum& viewFrustum = RENDERER.getViewFrustum();

  viewFrustum.setFarPlaneCull(FarPlaneCull);
  viewFrustum.setProjection(fov, NearPlane, FarPlane, FarDeepPlane,
			    mainWindow->getWidth(),
			    mainWindow->getHeight(),
			    mainWindow->getViewHeight());
  viewFrustum.setView(eyePoint, targetPoint);

  // add dynamic nodes
  addDynamicSceneNodes();

  // turn blanking and inversion on/off as appropriate
  RENDERER.setBlank(myTank && (myTank->isPaused() ||
			       (myTank->getFlag() == Flags::Blindness)));
  RENDERER.setInvert(myTank && myTank->isPhantomZoned());

  // turn on scene dimming when showing menu, when we're dead
  // and no longer exploding, or when we are in a building.
  bool insideDim = false;
  if (myTank) {
    const float teleProx = myTank->getTeleporterProximity();
    // teleporter glow has priority
    if (teleProx < 0.25f) {
      const float hnp = 0.5f * NearPlane; // half near plane distance
      const fvec3& eye = viewFrustum.getEye();
      const fvec3& dir = viewFrustum.getDirection();
      fvec3 clipPos;
      clipPos.x = eye.x + (dir.x * hnp);
      clipPos.y = eye.y + (dir.y * hnp);
      clipPos.z = eye.z;
      const Obstacle *obs;
      obs = world->inBuilding(clipPos, myTank->getAngle(), hnp, 0.0f, 0.0f);
      if ((obs != NULL) && !obs->isPassable()) {
        insideDim = true;
      }
    }
  }
  RENDERER.setDim(HUDDialogStack::get()->isActive() || insideDim ||
		  ((myTank && !ROAM.isRoaming()) &&
		   !myTank->isAlive() && !myTank->isExploding()));

  // turn on panel dimming when showing the menu (both radar and chat)
  const float dimFactor = HUDDialogStack::get()->isActive() ? 0.8f : 0.0f;
  if (radar) {
    radar->setDimming(dimFactor);
  }
  if (controlPanel) {
    controlPanel->setDimming(dimFactor);
  }

  // set hud state
  hud->setDim(HUDDialogStack::get()->isActive());
  hud->setPlaying(myTank && (myTank->isAlive() && !myTank->isPaused()));
  hud->setRoaming(ROAM.isRoaming());
  if (!ROAM.isRoaming()) {
    hud->setCracks(myTank && !myTank->isAlive() && !firstLife && !justJoined);
  }
  else {
    if (ROAM.getMode() != Roaming::roamViewFP) {
      hud->setCracks(false);
    } else {
      const Player* target = ROAM.getTargetTank();
      hud->setCracks(target && !target->isAlive());
    }
  }

  // get frame start time
  if (showDrawTime) {
#if defined(DEBUG_RENDERING)
    // get an accurate measure of frame time (at expense of frame rate)
    if (BZDB.isTrue("glFinish")) {
      glFinish();
    }
#endif
    media->stopwatch(true);
  }

  // so observers can have enhanced radar
  if (ROAM.isRoaming() && myTank) {
    if (ROAM.getMode() == Roaming::roamViewFP && ROAM.getTargetTank()) {
      myTank->setZpos(ROAM.getTargetTank()->getPosition().z);
    } else {
      myTank->setZpos(ROAM.getCamera()->pos.z);
    }
  }

  // let the hud save off the view matrix so it can do view projections
  if (hud) {
    hud->saveMatrixes(viewFrustum.getViewMatrix(),
                      viewFrustum.getProjectionMatrix());
  }

  // draw frame
  switch (viewType) {
    case SceneRenderer::ThreeChannel: {
      drawThreeChannel(fov, *myTankDir, eyePoint, targetPoint);
      break;
    }
    case SceneRenderer::Stacked:    { drawStacked();    break; }
    case SceneRenderer::Stereo:     { drawStereo();     break; }
    case SceneRenderer::Anaglyph:   { drawAnaglyph();   break; }
    case SceneRenderer::Interlaced: { drawInterlaced(); break; }
    default: { // normal rendering
      RENDERER.render();
      drawUI();
      break;
    }
  }

  // get frame end time
  if (showDrawTime) {
#if defined(DEBUG_RENDERING)
    // get an accurate measure of frame time (at expense of frame rate)
    if (BZDB.isTrue("glFinish")) {
      glFinish();
    }
#endif
    hud->setDrawTime((float)media->stopwatch(false));
  } else {
    hud->setDrawTime(-1.0f);
  }

  handleMouseDrawing();

  verticalSync();

  mainWindow->getWindow()->swapBuffers();

  // remove dynamic nodes from this frame
  SceneDatabase* scene = RENDERER.getSceneDatabase();
  if (scene) {
    scene->removeDynamicNodes();
  }
}


static void updateRoamingCamera(float dt)
{
  static Roaming::RoamingCamera prevDeltaCamera;
  static bool inited = false;

  if (!inited) {
    memset(&prevDeltaCamera, 0, sizeof(Roaming::RoamingCamera));
    inited = true;
  }

  Roaming::RoamingCamera deltaCamera;
  memset(&deltaCamera, 0, sizeof(Roaming::RoamingCamera));

  // move roaming camera
  if (myTank) {
    bool control = ((shiftKeyStatus & BzfKeyEvent::ControlKey) != 0);
    bool alt     = ((shiftKeyStatus & BzfKeyEvent::AltKey)     != 0);
    bool shift   = ((shiftKeyStatus & BzfKeyEvent::ShiftKey)   != 0);

    if (display->hasGetKeyMode()) {
      display->getModState (shift, control, alt);
    }

    if (!control && !shift) {
      deltaCamera.pos.x = (float)(4 * myTank->getSpeed()) * BZDBCache::tankSpeed;
    }

    if (alt) {
      deltaCamera.pos.y = (float)(4 * myTank->getRotation()) * BZDBCache::tankSpeed;
    } else {
      deltaCamera.theta  = ROAM.getZoom() * (float)myTank->getRotation();
    }

    if (control) {
      deltaCamera.phi = -2.0f * ROAM.getZoom() / 3.0f * (float)myTank->getSpeed();
    }

    if (shift) {
      deltaCamera.pos.z = (float)(-4 * myTank->getSpeed()) * BZDBCache::tankSpeed;
    }
  }

  // adjust for slow keyboard
  if (BZDB.isTrue("slowMotion")) {
    float st = BZDB.eval("roamSmoothTime");
    if (st < 0.1f)
      st = 0.1f;

    const float at = (dt / st);
    const float bt = (1.0f - at);
    deltaCamera.pos   = (at * deltaCamera.pos)   + (bt * prevDeltaCamera.pos);
    deltaCamera.theta = (at * deltaCamera.theta) + (bt * prevDeltaCamera.theta);
    deltaCamera.phi   = (at * deltaCamera.phi)   + (bt * prevDeltaCamera.phi);
  }

  deltaCamera.zoom = roamDZoom;

  ROAM.updatePosition(&deltaCamera, dt);

  // copy the old delta values
  memcpy(&prevDeltaCamera, &deltaCamera, sizeof(Roaming::RoamingCamera));

  return;
}


static void prepareTheHUD()
{
  // prep the HUD
  if (myTank == NULL) {
    return;
  }

  const fvec3& myPos = myTank->getPosition();
  hud->setHeading(myTank->getAngle());
  hud->setAltitude(myPos.z);
  if (world->allowTeamFlags()) {
    const fvec4& myTeamColor = Team::getTankColor(myTank->getTeam());
    // markers for my team flag
    for (int i = 0; i < numFlags; i++) {
      const Flag &flag = world->getFlag(i);
      if ((flag.type->flagTeam == myTank->getTeam())
          && ((flag.status != FlagOnTank) ||
              (flag.owner != myTank->getId()))) {
        const fvec3& flagPos = flag.position;
        const float heading = atan2f(flagPos.y - myPos.y, flagPos.x - myPos.x);
        hud->addMarker(heading, myTeamColor);
        hud->AddEnhancedMarker(flagPos, myTeamColor,
                               false, BZDBCache::flagPoleSize * 2.0f);
      }
    }
  }

  const fvec3* antiPos = myTank->getAntidoteLocation();
  if (antiPos != NULL) {
    // marker for my antidote flag
    const float heading = atan2f(antiPos->y - myPos.y, antiPos->x - myPos.x);
    const fvec4 antiColor(1.0f, 1.0f, 0.0f, 1.0f); // yellow
    hud->addMarker(heading, antiColor);
    hud->AddEnhancedMarker(*antiPos, antiColor, false,
                           BZDBCache::flagPoleSize * 2.0f);
  }

  return;
}


static void updatePauseCountdown(float dt)
{
  if (!myTank) {
    pauseCountdown = 0.0f;
  }

  if (pauseCountdown > 0.0f && !myTank->isAlive()) {
    pauseCountdown = 0.0f;
    hud->setAlert(1, NULL, 0.0f, true);
  }
  if (pauseCountdown > 0.0f) {
    const int oldPauseCountdown = (int)(pauseCountdown + 0.99f);
    pauseCountdown -= dt;
    if (pauseCountdown <= 0.0f) {
      // make sure it is really safe to pause..  since the server
      // might make us drop our flag, make sure the player is on the
      // ground and not in a building.  prevents getting kicked
      // later for being in places we shouldn't without holding the
      // right flags.
      ServerLink *server = ServerLink::getServer();
      if (myTank->getLocation() == LocalPlayer::InBuilding) {
	// custom message when trying to pause while in a building
	// (could get stuck on un-pause if flag is taken/lost)
	server->sendPaused(false);
	hud->setAlert(1, "Can't pause while inside a building", 1.0f, false);

      } else if (myTank->getLocation() == LocalPlayer::InAir || myTank->isFalling()) {
	// custom message when trying to pause when jumping/falling
	server->sendPaused(false);
	hud->setAlert(1, "Can't pause when you are in the air", 1.0f, false);

      } else if (myTank->getLocation() != LocalPlayer::OnGround &&
	myTank->getLocation() != LocalPlayer::OnBuilding) {
	  // catch-all message when trying to pause when you should not
	  server->sendPaused(false);
	  hud->setAlert(1, "Unable to pause right now", 1.0f, false);

      } else if (myTank->isPhantomZoned()) {
	// custom message when trying to pause while zoned
	server->sendPaused(false);
	hud->setAlert(1, "Can't pause when you are in the phantom zone", 1.0f, false);

      } else {
	// okay, now we pause.  first drop any team flag we may have.
	const FlagType *flagd = myTank->getFlag();
	if (flagd->flagTeam != NoTeam) {
	  serverLink->sendDropFlag(myTank->getPosition());
	  myTank->setShotType(StandardShot);
	}

	if (World::getWorld() && World::getWorld()->allowRabbit() &&
	  (myTank->getTeam() == RabbitTeam))
	  serverLink->sendNewRabbit();

	// now actually pause
//FIXME	myTank->setPause(true);
//FIXME	hud->setAlert(1, NULL, 0.0f, true);
//FIXME	showMessage("Paused");

	// turn off the sound
	if (savedVolume == -1) {
	  savedVolume = (int)(SOUNDSYSTEM.getVolume() * 10);
	  SOUNDSYSTEM.setVolume(0);
	}

	// ungrab mouse
	mainWindow->ungrabMouse();
      }
    } else if ((int)(pauseCountdown + 0.99f) != oldPauseCountdown &&
      !pausedByUnmap) {
	// update countdown alert
	char msgBuf[40];
	sprintf(msgBuf, "Pausing in %d", (int)(pauseCountdown + 0.99f));
	hud->setAlert(1, msgBuf, 1.0f, false);
    }
  }
  return;
}


static void updateDestructCountdown(float dt)
{
  if (!myTank)
    destructCountdown = 0.0f;

  if (destructCountdown > 0.0f && !myTank->isAlive()) {
    destructCountdown = 0.0f;
    hud->setAlert(1, NULL, 0.0f, true);
  }
  if (destructCountdown > 0.0f) {
    static BZDB_float maxVelocity(BZDBNAMES.MAXSELFDESTRUCTVEL);
    if (myTank->getVelocity().length() > maxVelocity) {
      destructCountdown = 0.0f;
      hud->setAlert(1, "No Self Destruct while moving", 1.0f, false);
    } else {
      const int oldDestructCountdown = (int)(destructCountdown + 0.99f + 1);
      destructCountdown -= dt;
      if (destructCountdown <= 0.0f) {
	// now actually destruct
	gotBlowedUp(myTank, SelfDestruct, myTank->getId());
	hud->setAlert(1, NULL, 0.0f, true);
      } else if ((int)(destructCountdown + 0.99f) != oldDestructCountdown) {
	// update countdown alert
	char msgBuf[40];
	sprintf(msgBuf, "Self Destructing in %d", (int)(destructCountdown + 0.99f));
	hud->setAlert(1, msgBuf, 1.0f, false);
      }
    }
  }
  return;
}


void handleJoyStick(void)
{
  if (!mainWindow->haveJoystick())
    return;

  static const BzfKeyEvent::Button button_map[] = {
    BzfKeyEvent::BZ_Button_1,
    BzfKeyEvent::BZ_Button_2,
    BzfKeyEvent::BZ_Button_3,
    BzfKeyEvent::BZ_Button_4,
    BzfKeyEvent::BZ_Button_5,
    BzfKeyEvent::BZ_Button_6,
    BzfKeyEvent::BZ_Button_7,
    BzfKeyEvent::BZ_Button_8,
    BzfKeyEvent::BZ_Button_9,
    BzfKeyEvent::BZ_Button_10,
    BzfKeyEvent::BZ_Button_11,
    BzfKeyEvent::BZ_Button_12,
    BzfKeyEvent::BZ_Button_13,
    BzfKeyEvent::BZ_Button_14,
    BzfKeyEvent::BZ_Button_15,
    BzfKeyEvent::BZ_Button_16,
    BzfKeyEvent::BZ_Button_16,
    BzfKeyEvent::BZ_Button_17,
    BzfKeyEvent::BZ_Button_18,
    BzfKeyEvent::BZ_Button_19,
    BzfKeyEvent::BZ_Button_20,
    BzfKeyEvent::BZ_Button_21,
    BzfKeyEvent::BZ_Button_22,
    BzfKeyEvent::BZ_Button_23,
    BzfKeyEvent::BZ_Button_24,
    BzfKeyEvent::BZ_Button_25,
    BzfKeyEvent::BZ_Button_26,
    BzfKeyEvent::BZ_Button_27,
    BzfKeyEvent::BZ_Button_28,
    BzfKeyEvent::BZ_Button_29,
    BzfKeyEvent::BZ_Button_30,
    BzfKeyEvent::BZ_Button_31,
    BzfKeyEvent::BZ_Button_32
  };

  static unsigned long old_buttons = 0;
  const int button_count = countof(button_map);
  unsigned long new_buttons = mainWindow->getJoyButtonSet();
  if (old_buttons != new_buttons) {
    for (int j = 0; j < button_count; j++) {
      if ((old_buttons & (1<<j)) != (new_buttons & (1<<j))) {
	BzfKeyEvent ev;
	ev.button = button_map[j];
	ev.chr = 0;
	ev.shift = 0;
	doKey(ev, (new_buttons & (1<<j)) != 0);
      }
    }
  }

  old_buttons = new_buttons;

  static const BzfKeyEvent::Button hatswitch_map[] = {
    BzfKeyEvent::BZ_Hatswitch_1_up,
    BzfKeyEvent::BZ_Hatswitch_1_right,
    BzfKeyEvent::BZ_Hatswitch_1_down,
    BzfKeyEvent::BZ_Hatswitch_1_left,
    BzfKeyEvent::BZ_Hatswitch_2_up,
    BzfKeyEvent::BZ_Hatswitch_2_right,
    BzfKeyEvent::BZ_Hatswitch_2_down,
    BzfKeyEvent::BZ_Hatswitch_2_left};

    static unsigned int old_direction[] = { 0, 0}; // BZ_Hatswitch_1 and BZ_Hatswitch_2

    // How many are there really
    int hatswitch_count = std::min(mainWindow->getJoyDeviceNumHats(),
      (unsigned int)countof(old_direction));

    for (int j = 0; j < hatswitch_count; j++) {
      unsigned int hat_direction = mainWindow->getJoyHatswitch(j);
      if (hat_direction != old_direction[j]) {
	int mask = 1;
	for (int k = j; k < 4; ++k, mask <<= 1) {
	  if (((old_direction[j] ^ hat_direction) & mask) != 0) {
	    BzfKeyEvent ev;
	    ev.button = hatswitch_map[j * 4 + k];
	    ev.chr = 0;
	    ev.shift = 0;
	    doKey(ev, (hat_direction & mask) != 0);
	  }
	}
	old_direction[j] = hat_direction;
      }
    }
}


void updateTimeOfDay(const float dt)
{
  // update time of day -- update sun and sky every few seconds
  if (bzdbSyncTime < 0.0f) {
    if (!BZDB.isSet("fixedTime"))
      epochOffset += (double)dt;

    epochOffset += (double)(50.0f * dt * clockAdjust);
  } else {
    epochOffset = (double)bzdbSyncTime;
    lastEpochOffset += (double)dt;
  }
  if (fabs(epochOffset - lastEpochOffset) >= 4.0) {
    updateDaylight(epochOffset);
    lastEpochOffset = epochOffset;
  }
}


void checkForServerBail(void)
{
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
}


void updateVideoFormatTimer(const float dt)
{
  // update test video format timer
  if (testVideoFormatTimer > 0.0f) {
    testVideoFormatTimer -= dt;
    if (testVideoFormatTimer <= 0.0f) {
      testVideoFormatTimer = 0.0f;
      setVideoFormat(testVideoPrevFormat);
    }
  }
}





void moveRoamingCamera(const float dt)
{
  if (!ROAM.isRoaming()) {
    return;
  }
  updateRoamingCamera(dt);
  ROAM.buildRoamingLabel();
}


void doTankMotions(const float /*dt*/)
{
  // do dead reckoning on remote players
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      const bool wasNotResponding = remotePlayers[i]->isNotResponding();
      remotePlayers[i]->doDeadReckoning();
      const bool isNotResponding = remotePlayers[i]->isNotResponding();

      if (!wasNotResponding && isNotResponding)
	addMessage(remotePlayers[i], "not responding");
      else if (wasNotResponding && !isNotResponding)
	addMessage(remotePlayers[i], "okay");
    }
  }

  // do motion
  if (myTank) {
    if (myTank->isAlive() && !myTank->isPaused()) {
      doMotion();
      if (scoreboard->getHuntState()==ScoreboardRenderer::HUNT_ENABLED)
	setHuntTarget(); //spot hunt target

      if (myTank->getTeam() != ObserverTeam &&
	((fireButton && myTank->getFlag() == Flags::MachineGun) ||
	(myTank->getFlag() == Flags::TriggerHappy)))
	myTank->fireShot();

      setLookAtMarker();

      // see if we have a target, if so lock on to the bastage
      if (myTank->getTarget())
	hud->AddLockOnMarker(myTank->getTarget()->getPosition(),
	myTank->getTarget()->getCallSign(),
	!isKillable(myTank->getTarget()));

    } else {
      int mx, my;
      mainWindow->getMousePosition(mx, my);
    }
    myTank->update();
  }
}


void updateTimes(const float dt)
{
  updateTimeOfDay(dt);
  updateVideoFormatTimer(dt);

  // update the countdowns
  updatePauseCountdown(dt);
  updateDestructCountdown(dt);
}


void updatePositions(const float dt)
{
  // notify if input changed
  if ((myTank != NULL) && (myTank->queryInputChange() == true))
    showMessage(LocalPlayer::getInputMethodName(myTank->getInputMethod()) + " movement");

  moveRoamingCamera(dt);

  updateShots(dt);

  // update track marks  (before any tanks are moved)
  TrackMarks::update(dt);

  doTankMotions(dt);

#ifdef ROBOT
  if (entered)
    updateRobots(dt);
#endif
}


void checkEnvironment(const float dt)
{
  // update the wind
  if (world)
    world->updateWind(dt);

  // check for flags and hits
  checkEnvironment();

#ifdef ROBOT
  if (entered)
    checkEnvironmentForRobots();
#endif

}


void updateTanks(const float dt)
{
  // adjust properties based on flags (dimensions, cloaking, etc...)
  if (myTank)
    myTank->updateTank(dt, true);

  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i])
      remotePlayers[i]->updateTank(dt, false);
  }
}


void updateWorldEffects(const float dt)
{
  // reposition flags
  updateFlags(dt);

  // update explosion animations
  updateExplosions(dt);

  // update mesh animations
  if (world)
    world->updateAnimations(dt);
}


void doUpdates(const float dt)
{
  float doneDT = dt;
  float dtLimit = MAX_DT_LIMIT;
  float realDT = dt;

  if (doneDT > dtLimit) {
    realDT = dtLimit;
    doneDT -= dtLimit;
  }

  while (doneDT > 0) {
    updateTimes(realDT);
    updatePositions(realDT);
    checkEnvironment(realDT);
    updateTanks(realDT);
    updateWorldEffects(realDT);

    doneDT -= dtLimit;

    if (doneDT < dtLimit) // if we only have a nubby left, don't do a full dt.
      realDT = doneDT;
  }

  ExportInformation::instance().sendPulse();

  // update AutoHunt
  AutoHunt::update();

  forcedControls = false;
}




bool checkForCompleteDownloads(void)
{
  // check if we are waiting for initial texture downloading
  if (!Downloads::instance().requestFinalized())
    return false;

  // downloading is terminated. go!
  Downloads::instance().finalizeDownloads();
  if (downloadingData) {
    downloadingData = false;
    return true;
  } else {
    setSceneDatabase();
  }

  return false;
}


void doFPSLimit(void)
{
  // always cap out at 200 fps unless a limit is set.
  static TimeKeeper lastTime = TimeKeeper::getCurrent();
  float fpsLimit = 200;
  if (debugLevel > 0) {
    fpsLimit = BZDB.eval("fpsLimit");
  }

  if (BZDB.isTrue("saveEnergy") && (BZDB.eval("fpsLimit") < fpsLimit)) {
    // limit the fps to save battery life by minimizing cpu usage
    fpsLimit = BZDB.eval("fpsLimit");
  }

  static bool notify = false;

#ifndef DEBUG
  if (fpsLimit < 20.0 || isnan(fpsLimit)) {
    fpsLimit = 20.0f;
    BZDB.setFloat("fpsLimit", fpsLimit);
    notify = true;
  }
#else
  if (fpsLimit < 1.0) {
    fpsLimit = 1.0;
    BZDB.setFloat("fpsLimit", fpsLimit);
    notify = true;
  }
#endif

  if (notify) {
    char clamped[80] = {0};
    snprintf(clamped, sizeof(clamped), "WARNING: fpsLimit is set too low, clamped to %f", fpsLimit);
    showMessage(clamped);
    notify = false;
  }

  const float elapsed = float(TimeKeeper::getCurrent() - lastTime);
  if (elapsed > 0.0f) {
    const float period = (1.0f / fpsLimit);
    const float remaining = (period - elapsed);
    if (remaining > 0.0f) {
      TimeKeeper::sleep(remaining);
    }
  }

  lastTime = TimeKeeper::getCurrent();
}


//
// main playing loop
//

static void playingLoop()
{
  // start timing
  TimeKeeper::setTick();
  updateDaylight(epochOffset);

  worldDownLoader = new WorldDownLoader;

  // main loop
  while (!CommandsStandard::isQuit()) {
    BZDBCache::update();

    canSpawn = true;

    // set this step game time
    GameTime::setStepTime();

    // get delta time
    TimeKeeper prevTime = TimeKeeper::getTick();
    TimeKeeper::setTick();
    const float dt = float(TimeKeeper::getTick() - prevTime);

    mainWindow->getWindow()->yieldCurrent();

    doMessages();    // handle incoming packets

    if (world) { // udpate the world collision grid if required
      world->checkCollisionManager();
    }

    mainWindow->getWindow()->yieldCurrent();

    handlePendingJoins();

    struct in_addr inAddress;
    if (dnsLookupDone(inAddress)) {
      joinInternetGame(&inAddress);
      scoreboard->huntReset();
    }

    mainWindow->getWindow()->yieldCurrent();

    // handle pending events for some small fraction of time
    clockAdjust = 0.0f;
    processInputEvents(0.1f);

    handleJoyStick();

    mainWindow->getWindow()->yieldCurrent();

    callPlayingCallbacks();    // invoke callbacks

    mainWindow->getWindow()->yieldCurrent();

    if (CommandsStandard::isQuit()) {
      break; // quick out
    }

    checkForServerBail();

    doUpdates(dt);

    // prep the HUD
    prepareTheHUD();

    // draw the frame
    if (!unmapped) {
      drawFrame(dt);
    }
    else { // wait around a little to avoid spinning the CPU when iconified
      TimeKeeper::sleep(0.05f);
    }

    // play the sounds
    SOUNDSYSTEM.update(TimeKeeper::getCurrent().getSeconds());

    doNetworkStuff();

    if (checkForCompleteDownloads()) {
      joinInternetGame2(); // we did the inital downloads, so we should join
    }

    doFPSLimit();

    if (serverLink) {
      serverLink->flush();
    }
  } // end main client loop


  delete worldDownLoader;
  // restore the sound.  if we don't do this then we'll save the
  // wrong volume when we dump out the configuration file if the
  // app exits when the game is paused.
  if (savedVolume != -1) {
    SOUNDSYSTEM.setVolume(savedVolume*0.1f);
    savedVolume = -1;
  }

  // hide window
  mainWindow->showWindow(false);
}


//
// game initialization
//

static float timeConfiguration(bool useZBuffer)
{
  // prepare depth buffer if requested
  BZDB.set("zbuffer", "1");
  if (useZBuffer) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  // use glFinish() to get accurate timings
  //glFinish();
  TimeKeeper startTime = TimeKeeper::getCurrent();
  RENDERER.setExposed();
  RENDERER.render();
  // glFinish();
  TimeKeeper endTime = TimeKeeper::getCurrent();

  // turn off depth buffer
  if (useZBuffer) glDisable(GL_DEPTH_TEST);

  return float(endTime - startTime);
}


static void timeConfigurations()
{
  static const float MaxFrameTime = 0.050f; // seconds
  TextureManager &tm = TextureManager::instance();

  // ignore results of first test.  OpenGL could be doing lazy setup.
  BZDB.set("blend", "0");
  BZDB.set("smooth", "0");
  BZDB.set("lighting", "0");
  BZDB.set("texture", "0");
  RENDERER.setQuality(0);
  BZDB.set("dither", "1");
  BZDB.set("shadows", "0");
  BZDB.set("radarStyle", "0");
  tm.setMaxFilter(OpenGLTexture::Off);
  timeConfiguration(true);

  // time lowest quality with and without blending.  some systems
  // stipple very slowly even though everything else is fast.  we
  // don't want to conclude the system is slow because of stippling.
  printError("  lowest quality");
  const float timeNoBlendNoZ = timeConfiguration(false);
  const float timeNoBlendZ   = timeConfiguration(true);
  BZDB.set("blend", "1");
  const float timeBlendNoZ   = timeConfiguration(false);
  const float timeBlendZ     = timeConfiguration(true);
  if ((timeNoBlendNoZ > MaxFrameTime) &&
      (timeNoBlendZ   > MaxFrameTime) &&
      (timeBlendNoZ   > MaxFrameTime) &&
      (timeBlendZ     > MaxFrameTime)) {
    if ((timeNoBlendNoZ < timeNoBlendZ) &&
        (timeNoBlendNoZ < timeBlendNoZ) &&
        (timeNoBlendNoZ < timeBlendZ)) {
          // no depth, no blending definitely fastest
      BZDB.set("zbuffer", "1");
      BZDB.set("blend", "1");
    }
    if ((timeNoBlendZ < timeBlendNoZ) &&
        (timeNoBlendZ < timeBlendZ)) {
        // no blending faster than blending
      BZDB.set("zbuffer", "1");
      BZDB.set("blend", "1");
    }
    if (timeBlendNoZ < timeBlendZ) {
      // blending faster than depth
      BZDB.set("zbuffer", "1");
      BZDB.set("blend", "1");
    }
    // blending and depth faster than without either
    BZDB.set("zbuffer", "1");
    BZDB.set("blend", "1");
    return;
  }

  // leave blending on if blending clearly faster than stippling
  if (timeBlendNoZ > timeNoBlendNoZ ||
    (timeBlendNoZ > timeNoBlendZ && timeBlendZ > timeNoBlendNoZ) ||
    timeBlendZ > timeNoBlendZ) {
      BZDB.set("blend", "0");
  }

  // try texturing.  if it's too slow then fall back to
  // lowest quality and return.
  tm.setMaxFilter(OpenGLTexture::Nearest);
  BZDB.set("texture", tm.getMaxFilterName());
  RENDERER.setQuality(1);
  printError("  lowest quality with texture");
  if ((timeConfiguration(false) > MaxFrameTime) ||
      (timeConfiguration(true) > MaxFrameTime)) {
    BZDB.set("texture", "0");
    tm.setMaxFilter(OpenGLTexture::Off);
    RENDERER.setQuality(0);
    return;
  }

  // everything
  printError("  full quality");
  BZDB.set("blend", "1");
  BZDB.set("smooth", "1");
  BZDB.set("lighting", "1");
  tm.setMaxFilter(OpenGLTexture::LinearMipmapLinear);
  BZDB.set("texture", tm.getMaxFilterName());
  RENDERER.setQuality(2);
  BZDB.set("dither", "1");
  BZDB.set("shadows", "1");
  BZDB.set("radarStyle", "3");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // try it without shadows -- some platforms stipple very slowly
  BZDB.set("shadows", "0");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no high quality
  printError("  medium quality");
  RENDERER.setQuality(1);
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }
  printError("  low quality");
  RENDERER.setQuality(0);
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // lower quality texturing
  printError("  nearest texturing");
  tm.setMaxFilter(OpenGLTexture::Nearest);
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no texturing
  printError("  no texturing");
  BZDB.set("texture", "0");
  tm.setMaxFilter(OpenGLTexture::Off);
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no blending
  printError("  no blending");
  BZDB.set("blend", "0");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no smoothing.  shouldn't really affect fill rate too much.
  printError("  no smoothing");
  BZDB.set("smooth", "0");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no lighting.  shouldn't really affect fill rate, either.
  printError("  no lighting");
  BZDB.set("lighting", "0");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }

  // no dithering
  printError("  no dithering");
  BZDB.set("dither", "0");
  if (timeConfiguration(true)  < MaxFrameTime) { return; }
  if (timeConfiguration(false) < MaxFrameTime) { return; }
}


static void findFastConfiguration()
{
  // time the rendering of the background with various rendering styles
  // until we find one fast enough.  these tests assume that we're
  // going to be fill limited.  each test comes in a pair:  with and
  // without the zbuffer.
  //
  // this, of course, is only a rough estimate since we're not drawing
  // a normal frame (no radar, no HUD, no buildings, etc.).  the user
  // can always turn stuff on later and the settings are remembered
  // across invocations.

  // setup projection
  float muzzleHeight = BZDBCache::muzzleHeight;
  static const fvec3 eyePoint(0.0f, 0.0f, muzzleHeight);
  static const fvec3 targetPoint(0.0f, 10.0f, muzzleHeight);
  RENDERER.getViewFrustum().setProjection((float)(45.0 * DEG2RAD),
    NearPlaneNormal,
    FarPlaneDefault,
    FarDeepPlaneDefault,
    mainWindow->getWidth(),
    mainWindow->getHeight(),
    mainWindow->getViewHeight());
  RENDERER.getViewFrustum().setView(eyePoint, targetPoint);

  // add a big wall in front of where we're looking.  this is important
  // because once textures are off, the background won't draw much of
  // anything.  this will ensure that we continue to test polygon fill
  // rate.  with one polygon it doesn't matter if we use a z or bsp
  // database.
  static const fvec3 base (-10.0f, 10.0f,  0.0f);
  static const fvec3 sEdge( 20.0f,  0.0f,  0.0f);
  static const fvec3 tEdge(  0.0f,  0.0f, 10.0f);
  static const fvec4 color(1.0f, 1.0f, 1.0f, 0.5f);
  SceneDatabase *timingScene = new ZSceneDatabase;
  WallSceneNode *node = new QuadWallSceneNode(base,
    sEdge, tEdge, 1.0f, 1.0f, true);
  node->setColor(color);
  node->setModulateColor(color);
  node->setLightedColor(color);
  node->setLightedModulateColor(color);
  node->setTexture(HUDuiControl::getArrow());
  node->setMaterial(OpenGLMaterial(color, color));
  timingScene->addStaticNode(node, false);
  timingScene->finalizeStatics();
  RENDERER.setSceneDatabase(timingScene);
  RENDERER.setDim(false);

  timeConfigurations();

  RENDERER.setSceneDatabase(NULL);
}


static void defaultErrorCallback(const char *msg)
{
  std::string message = ColorStrings[RedColor];
  message += msg;
  showMessage(message);
}


static void startupErrorCallback(const char *msg)
{
  showMessage(msg);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  controlPanel->render(RENDERER);
  mainWindow->getWindow()->swapBuffers();
}


void startPlaying()
{
  // initalization
  mainWindow = &RENDERER.getWindow();

  // register some commands
  const std::vector<CommandListItem>& commandList = getCommandList();
  for (size_t c = 0; c < commandList.size(); ++c) {
    CMDMGR.add(commandList[c].name, commandList[c].func, commandList[c].help);
  }

  setChangePlayerTeamCallback(changePlayerTeam);

  // initialize the tank display lists
  // (do this before calling SceneRenderer::render())
  TankGeometryMgr::init();
  SphereLodSceneNode::init();

  // make control panel
  ControlPanel _controlPanel(*mainWindow, RENDERER);
  controlPanel = &_controlPanel;

  // make the radar
  RadarRenderer _radar(RENDERER, world);
  radar = &_radar;

  // tie the radar to the control panel
  controlPanel->setRadarRenderer(radar);

  // tell the control panel how many frame buffers there are.  we
  // cheat when drawing the control panel, not drawing it if it
  // hasn't changed.  that only works if we've filled all the
  // frame buffers (e.g. front and back buffers) with the correct
  // data.
  // FIXME -- assuming the contents of any frame buffer except the
  // front buffer are anything but garbage violates the OpenGL
  // spec.  we really should redraw the control panel every frame
  // but this works on every system so far.
  int n = 3; // assume triple buffering
  switch (RENDERER.getViewType()) {
    case SceneRenderer::Stacked:
    case SceneRenderer::Stereo: {
    #ifndef USE_GL_STEREO
      // control panel drawn twice per frame
      n *= 2;
    #endif
      break;
    }
    case SceneRenderer::ThreeChannel:
    default: {
      // only one copy of control panel visible
      break;
    }
  }
  controlPanel->setNumberOfFrameBuffers(n);

  // if no configuration, turn off fancy rendering so startup is fast,
  // even on a slow machine.
  if (!startupInfo.hasConfiguration) {
    BZDB.set("blend", "0");
    BZDB.set("smooth", "0");
    BZDB.set("lighting", "0");
    BZDB.set("tesselation", "1");  // lighting set to 0 overrides
    BZDB.set("texture", "0");
    RENDERER.setQuality(0);
    BZDB.set("dither", "0");
    BZDB.set("shadows", "0");
    BZDB.set("radarStyle", "0");
    TextureManager::instance().setMaxFilter(OpenGLTexture::Off);
  }

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
  RENDERER.render();
  controlPanel->render(RENDERER);
  mainWindow->getWindow()->swapBuffers();

  // startup error callback adds message to control panel and
  // forces an immediate redraw.
  setErrorCallback(startupErrorCallback);

  // initialize epoch offset (time)
  userTimeEpochOffset = (double)mktime(&userTime);
  epochOffset = userTimeEpochOffset;
  updateDaylight(epochOffset);
  lastEpochOffset = epochOffset;

  // catch kill signals before changing video mode so we can
  // put it back even if we die.  ignore a few signals.
  bzSignal(SIGILL,  SIG_PF(dying));
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

  std::string videoFormat;
  int format = -1;
  if (BZDB.isSet("resolution")) {
    videoFormat = BZDB.get("resolution");
    if (videoFormat.length() != 0) {
      format = display->findResolution(videoFormat.c_str());
      if (format >= 0)
	mainWindow->getWindow()->callResizeCallbacks();
    }
  };
  // set the resolution (only if in full screen mode)
  if (!BZDB.isSet("_window") && BZDB.isSet("resolution")) {
    if (videoFormat.length() != 0) {
      if (display->isValidResolution(format) &&
	display->getResolution() != format &&
	display->setResolution(format)) {

	  // handle resize
	  if (BZDB.isSet("geometry")) {
	    int w, h, x, y, count;
	    char xs, ys;
	    count = sscanf(BZDB.get("geometry").c_str(),
	      "%dx%d%c%d%c%d", &w, &h, &xs, &x, &ys, &y);
	    if (w < 256) w = 256;
	    if (h < 192) h = 192;
	    if (count == 6) {
	      if (xs == '-') x = display->getWidth() - x - w;
	      if (ys == '-') y = display->getHeight() - y - h;
	      mainWindow->setPosition(x, y);
	    }
	    mainWindow->setSize(w, h);
	  } else {
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
  RENDERER.render();
  controlPanel->render(RENDERER);
  mainWindow->getWindow()->swapBuffers();
  mainWindow->getWindow()->yieldCurrent();

  // make heads up display
  HUDRenderer _hud(display, RENDERER);
  hud = &_hud;
  scoreboard = hud->getScoreboard();

  // initialize control panel and hud
  updateNumPlayers();
  updateFlag(Flags::Null);
  updateHighScores();
  notifyBzfKeyMapChanged();

  // make background renderer
  BackgroundRenderer background(RENDERER);
  RENDERER.setBackground(&background);

  // if no configuration file try to determine rendering settings
  // that yield reasonable performance.
  if (!startupInfo.hasConfiguration) {
    printError("testing performance;  please wait...");
    findFastConfiguration();
    dumpResources();
  }

  static const fvec3 zero(0.0f, 0.0f, 0.0f);

  TextureManager &tm = TextureManager::instance();

  bool done = false;
  int explostion = 1;
  while (!done) {
    char text[256];
    sprintf(text, "explode%d", explostion);

    int tex = tm.getTextureID(text, false);

    if (tex < 0) {
      done = true;
    } else {
      // make explosion scene node
      BillboardSceneNode *explosion = new BillboardSceneNode(zero);
      explosion->setTexture(tex);
      explosion->setTextureAnimation(8, 8);

      // add it to list of prototype explosions
      prototypeExplosions.push_back(explosion);
      explostion++;
    }
  }

  // let other stuff do initialization
  sceneBuilder = new SceneDatabaseBuilder();
  World::init();

  // prepare dialogs
  mainMenu = new MainMenu;

  // normal error callback (doesn't force a redraw)
  setErrorCallback(defaultErrorCallback);

  // print debugging info
  {
    // Application version
    logDebugMessage(1, "BZFlag version:   %s\n", getAppVersion());

    // Protocol version
    logDebugMessage(1, "BZFlag protocol:  %s\n", getProtocolVersion());

    // OpenGL Driver Information
    logDebugMessage(1, "OpenGL vendor:    %s\n", (const char*)glGetString(GL_VENDOR));
    logDebugMessage(1, "OpenGL version:   %s\n", (const char*)glGetString(GL_VERSION));
    logDebugMessage(1, "OpenGL renderer:  %s\n", (const char*)glGetString(GL_RENDERER));

    // Depth Buffer bitplanes
    GLint depthBits;
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    logDebugMessage(1, "Depth Buffer:     %i bitplanes\n", depthBits);

    // Stencil Buffer bitplanes
    GLint stencilBits;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    logDebugMessage(1, "Stencil Buffer:   %i bitplanes\n", stencilBits);
  }

  // windows version can be very helpful in debug logs
#ifdef _WIN32
  if (debugLevel >= 1) {
    OSVERSIONINFO info;
    ZeroMemory(&info, sizeof(OSVERSIONINFO));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    logDebugMessage(1, "Running on Windows %s%d.%d %s\n",
      (info.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "NT " : "",
      info.dwMajorVersion, info.dwMinorVersion,
      info.szCSDVersion);
  }
#endif

  // print expiration
  if (timeBombString()) {
    // add message about date of expiration
    char bombMessage[80];
    snprintf(bombMessage, 80, "This release will expire on %s", timeBombString());
    showMessage(bombMessage);
  }

  // send informative header to the console
  std::string tmpString;

  showMessage("");
  // print app version
  tmpString = ColorStrings[RedColor];
  tmpString += "BZFlag version: ";
  tmpString += getAppVersion();
  tmpString += " (";
  tmpString += getProtocolVersion();
  tmpString += ")";
  showMessage(tmpString);
  // print copyright
  tmpString = ColorStrings[YellowColor];
  tmpString += bzfcopyright;
  showMessage(tmpString);
  // print license
  tmpString = ColorStrings[CyanColor];
  tmpString += "Distributed under the terms of the LGPL";
  showMessage(tmpString);
  // print author
  tmpString = ColorStrings[GreenColor];
  tmpString += "Author: Chris Schoeneman <crs23@bigfoot.com>";
  showMessage(tmpString);
  // print maintainer
  tmpString = ColorStrings[CyanColor];
  tmpString += "Maintainer: Tim Riker <Tim@Rikers.org>";
  showMessage(tmpString);
  // print audio driver
  std::string audioStr;
  PlatformFactory::getMedia()->audioDriver(audioStr);
  if (tmpString != "" && audioStr != "") {
    tmpString = ColorStrings[BlueColor];
    tmpString += "Audio Driver: " + audioStr;
    showMessage(tmpString);
  }
  // print GL renderer
  tmpString = ColorStrings[PurpleColor];
  tmpString += "OpenGL Driver: ";
  tmpString += (const char*)glGetString(GL_RENDERER);
  tmpString += "  (GLEW ";
  tmpString += (const char*)glewGetString(GLEW_VERSION);
  tmpString += ")";
  showMessage(tmpString);

  // get current MOTD
  if (!BZDB.isTrue("disableMOTD")) {
    motd = new MessageOfTheDay;
    motd->getURL(BZDB.get("motdServer"));
  }

  // inform user of silencePlayers on startup
  for (unsigned int j = 0; j < silencePlayers.size(); j ++) {
    std::string aString = silencePlayers[j];
    aString += " Silenced";
    if (silencePlayers[j] == "*") {
      aString = "Silenced All Msgs";
    }
    showMessage(aString);
  }

  // enter game if we have all the info we need, otherwise
  // pop up main menu
  if (startupInfo.autoConnect &&
      startupInfo.callsign[0] &&
      startupInfo.serverName[0]) {
    joinRequested = true;
    showMessage("Trying...");
  } else {
    mainMenu->createControls();
    HUDDialogStack::get()->push(mainMenu);
  }

  // set up the cache singleton to work in the data dir
  std::string cacheDir = getCacheDirName();
  CACHEMGR.setCacheDirectory(cacheDir);

    //////////
   ////////////
  //////////////
  playingLoop();
  //////////////
   ////////////
    //////////

  // clean up
  TankGeometryMgr::kill();
  SphereLodSceneNode::kill();
  if (resourceDownloader) {
    delete resourceDownloader;
  }
  delete motd;
  for (unsigned int ext = 0; ext < prototypeExplosions.size(); ext++) {
    delete prototypeExplosions[ext];
  }
  prototypeExplosions.clear();
  leaveGame();
  setErrorCallback(NULL);
  while (HUDDialogStack::get()->isActive()) {
    HUDDialogStack::get()->pop();
  }
  delete mainMenu;
  delete sceneBuilder;
  RENDERER.setBackground(NULL);
  RENDERER.setSceneDatabase(NULL);
  World::done();
  mainWindow = NULL;
  controlPanel = NULL;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
