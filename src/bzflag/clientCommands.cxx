/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "commands.h"

/* system implementation headers */
#ifndef _WIN32
#  include <sys/types.h>
#  include <dirent.h>
#endif
#include <string>
#include <vector>
#include <cstdlib>

/* common implementation headers */
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FileManager.h"
#include "DirectoryNames.h"
#include "version.h"
#include "SceneRenderer.h"
#include "bzglob.h"
#include "BzPNG.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "sound.h"
#include "ComposeDefaultKey.h"
#include "SilenceDefaultKey.h"
#include "ServerCommandKey.h"
#include "Roaming.h"
#include "playing.h"
#include "HUDRenderer.h"
#include "HUDui.h"

/** jump
 */
static std::string cmdJump(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** fire weapon
 */
static std::string cmdFire(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** drop a flag
 */
static std::string cmdDrop(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** toggle radar visibility
 */
static std::string cmdToggleRadar(const std::string&,
				  const CommandManager::ArgList& args, bool*);

/** toggle console visibility
 */
static std::string cmdToggleConsole(const std::string&,
				    const CommandManager::ArgList& args, bool*);

/** toggle flags
 */
static std::string cmdToggleFlags(const std::string&,
				  const CommandManager::ArgList& args, bool*);

/** identify to a server
 */
static std::string cmdIdentify(const std::string&,
			       const CommandManager::ArgList& args, bool*);

/** restart/respawn
 */
static std::string cmdRestart(const std::string&,
			      const CommandManager::ArgList& args, bool*);

/** self-destruct
 */
static std::string cmdDestruct(const std::string&,
			       const CommandManager::ArgList& args, bool*);

/** pause
 */
static std::string cmdPause(const std::string&,
			    const CommandManager::ArgList& args, bool*);

/** select tabbed message
 */
static std::string cmdMessagePanel(const std::string&,
				   const CommandManager::ArgList& args, bool*);

/** toggle auto-pilot
 */
static std::string cmdAutoPilot(const std::string&,
				const CommandManager::ArgList& args, bool*);

/** change radar scale
 */
static std::string cmdRadarZoom(const std::string&,
				const CommandManager::ArgList& args, bool*);

/** change view angle
 */
static std::string cmdViewZoom(const std::string&,
			       const CommandManager::ArgList& args, bool*);

/** send
 */
static std::string cmdSend(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** capture a screenshot
 */
static std::string cmdScreenshot(const std::string&,
				 const CommandManager::ArgList& args, bool*);

/** time
 */
static std::string cmdTime(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** roam
 */
static std::string cmdRoam(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** silence another player
 */
static std::string cmdSilence(const std::string&,
			      const CommandManager::ArgList& args, bool*);

/** perform a server command
 */
static std::string cmdServerCommand(const std::string&,
				    const CommandManager::ArgList& args,
				    bool*);

/** scroll the chat panel
 */
static std::string cmdScrollPanel(const std::string&,
				  const CommandManager::ArgList& args, bool*);

/** hunt a player
 */
static std::string cmdHunt(const std::string&,
			   const CommandManager::ArgList& args, bool*);

/** hunt another player
 */
static std::string cmdAddHunt(const std::string&,
			      const CommandManager::ArgList& args, bool*);

/** iconify window
 */
static std::string cmdIconify(const std::string&,
			      const CommandManager::ArgList& args, bool*);

/** mouse box size flags
 */
static std::string cmdMouseBox(const std::string&,
			       const CommandManager::ArgList& args, bool*);

/** toggle mouse capture
 */
static std::string cmdMouseGrab(const std::string&,
				const CommandManager::ArgList& args, bool*);

/** toggle Full Screen
 */
static std::string cmdToggleFS(const std::string&,
			       const CommandManager::ArgList& args, bool*);

/** cycle to the next radar zoom level
 */
static std::string cmdCycleRadar(const std::string&,
				  const CommandManager::ArgList& args, bool*);

/** cycle to the next panel tab
 */
static std::string cmdCyclePanel(const std::string&,
				  const CommandManager::ArgList& args, bool*);


const struct CommandListItem commandList[] = {
  { "fire",		&cmdFire,		"fire:  fire a shot" },
  { "jump",		&cmdJump,		"jump:  make player jump" },
  { "drop",		&cmdDrop,		"drop:  drop the current flag" },
  { "identify",		&cmdIdentify,		"identify:  identify/lock-on-to player in view" },
  { "restart",		&cmdRestart,		"restart:  restart playing" },
  { "destruct",		&cmdDestruct,		"destruct:  self destruct" },
  { "pause",		&cmdPause,		"pause:  pause/resume" },
  { "send",		&cmdSend,		"send {all|team|nemesis|recipient|admin}:  start composing a message" },
  { "screenshot",	&cmdScreenshot,		"screenshot:  take a screenshot" },
  { "time",		&cmdTime,		"time {forward|backward}:  adjust the current time" },
  { "roam",		&cmdRoam,		"roam {zoom|cycle} <args>:  roam around" },
  { "silence",		&cmdSilence,		"silence:  silence/unsilence a player" },
  { "servercommand",	&cmdServerCommand,	"servercommand:  quick admin" },
  { "scrollpanel",	&cmdScrollPanel,	"scrollpanel {up|down}:  scroll message panel" },
  { "hunt",		&cmdHunt,		"hunt:  hunt a specific player" },
  { "addhunt",		&cmdAddHunt,		"addhunt:  add/modify hunted player(s)" },
  { "iconify",		&cmdIconify,		"iconify: iconify & pause bzflag" },
  { "mousebox",		&cmdMouseBox,		"mousebox <size>:  change the mousebox size"},
  { "mousegrab",	&cmdMouseGrab,		"mousegrab: toggle exclusive mouse mode" },
  { "fullscreen",	&cmdToggleFS,		"fullscreen: toggle fullscreen mode" },
  { "autopilot",	&cmdAutoPilot,		"autopilot:  set/unset autopilot bot code" },
  { "radarZoom",	&cmdRadarZoom,		"radarZoom {in/out}: change maxRadar range"},
  { "viewZoom",		&cmdViewZoom,		"viewZoom {in/out/toggle}: change view angle" },
  { "messagepanel",	&cmdMessagePanel,	"messagepanel {all|chat|server|misc}:  set message tab" },
  { "toggleRadar",	&cmdToggleRadar,	"toggleRadar:  toggle radar visibility" },
  { "toggleConsole",	&cmdToggleConsole,	"toggleConsole:  toggle console visibility" },
  { "toggleFlags",	&cmdToggleFlags,	"toggleFlags {main|radar}:  turn off/on field radar flags" },
  { "cycleRadar",	&cmdCycleRadar,		"cycleRadar {level1 [level2 ...] [off]}:  cycle to the next radar zoom level" },
  { "cyclePanel",	&cmdCyclePanel,		"cyclePanel {left[_off]|right[_off]}:  cycle to the previous or next message panel tab" }
};


static std::string cmdToggleFS(const std::string&,
			       const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: fullscreen";
  mainWindow->toggleFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  return std::string();
}

static std::string cmdMouseBox(const std::string&,
			       const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1) {
    return "usage: mousebox <size>";
  }
  const char* start = args[0].c_str();
  char* end;
  const int value = (int) strtol(args[0].c_str(), &end, 10);
  if (start == end) {
    return "bad number";
  }
  RENDERER.setMaxMotionFactor(value);
  return std::string();
}

static std::string cmdMouseGrab(const std::string&,
				const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: mousegrab";
  const bool grabbing = !(BZDB.isTrue("mousegrab"));
  BZDB.set("mousegrab", grabbing ? "true" : "false");
  mainWindow->enableGrabMouse(grabbing);
  return std::string();
}

static std::string cmdIconify(const std::string&,
			      const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: iconify";

  if (!BZDB.isTrue("Win32NoMin"))
    mainWindow->iconify();
  return std::string();
}

static std::string cmdJump(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: jump";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL)
    myTank->setJump();
  return std::string();
}

static std::string cmdToggleFlags(const std::string&, const
				  CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: main|radar";
  if (args[0] == "main") {
    CMDMGR.run("toggle displayMainFlags");
    warnAboutMainFlags();
  } else if (args[0] == "radar") {
    CMDMGR.run("toggle displayRadarFlags");
    warnAboutRadarFlags();
  } else {
    return "usage: main|radar";
  }

  return std::string();
}

static std::string cmdToggleRadar(const std::string&,
				  const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: toggleRadar";
  }

  CMDMGR.run("toggle displayRadar");

  warnAboutRadar();

  return std::string();
}

static std::string cmdToggleConsole(const std::string&,
				    const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: toggleConsole";
  }

  CMDMGR.run("toggle displayConsole");

  warnAboutConsole();

  return std::string();
}


static std::string cmdFire(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: fire";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (fireButton && myTank != NULL && myTank->isAlive()
      && myTank->getTeam() != ObserverTeam)
    myTank->fireShot();
  return std::string();
}

static std::string cmdDrop(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: drop";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    FlagType* flag = myTank->getFlag();
    if ((flag != Flags::Null) && !myTank->isPaused() &&
	(flag->endurance != FlagSticky) && !myTank->isPhantomZoned() &&
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

static std::string cmdIdentify(const std::string&,
			       const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: identify";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL)
    if (myTank->isAlive() && !myTank->isPaused())
      setTarget();
  return std::string();
}

static std::string cmdRestart(const std::string&,
			      const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: restart";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL)
    if (!gameOver && !myTank->isSpawning() && (myTank->getTeam() != ObserverTeam) && !myTank->isAlive() && !myTank->isExploding()) {
      serverLink->sendAlive();
      myTank->setSpawning(true);
      CommandManager::ArgList zoomArgs;
      std::string resetArg = "reset";
      zoomArgs.push_back(resetArg);
      cmdViewZoom("", zoomArgs,NULL);
    }

  return std::string();
}

static std::string cmdDestruct(const std::string&,
			       const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: destruct";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
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

static std::string cmdPause(const std::string&,
			    const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: pause";

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!pausedByUnmap && myTank && myTank->isAlive() && !myTank->isAutoPilot()) {
    if (myTank->isPaused()) {
      // already paused, so unpause
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
      // player aborted pause
      pauseCountdown = 0.0f;
      hud->setAlert(1, "Pause cancelled", 1.5f, true);

    } else if (myTank->getLocation() == LocalPlayer::InBuilding) {
      // custom message when trying to pause while in a building
      // (could get stuck on un-pause if flag is taken)
      hud->setAlert(1, "Can't pause while inside a building", 1.0f, false);

    } else if (myTank->getLocation() == LocalPlayer::InAir) {
      // custom message when trying to pause when jumping/falling
      hud->setAlert(1, "Can't pause when you are in the air", 1.0f, false);

    } else if (myTank->getLocation() != LocalPlayer::OnGround &&
	       myTank->getLocation() != LocalPlayer::OnBuilding) {
      // catch-all message when trying to pause when you should not
      hud->setAlert(1, "Unable to pause right now", 1.0f, false);

    } else {
      // update the pause alert message
      pauseCountdown = 5.0f;
      char msgBuf[40];
      sprintf(msgBuf, "Pausing in %d", (int) (pauseCountdown + 0.99f));
      hud->setAlert(1, msgBuf, 1.0f, false);
    }
  }
  return std::string();
}

static std::string cmdAutoPilot(const std::string&,
				const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: autopilot";

  // don't enable autopilot until we've fully joined and checked the value
  // of the server-side _disableBots
  if (! BZDB.isSet(StateDatabase::BZDB_DISABLEBOTS))
    return std::string();

  LocalPlayer *myTank = LocalPlayer::getMyTank();

  if (!BZDB.isTrue(StateDatabase::BZDB_TANKWIDTH))
    return std::string();

  if ((myTank == NULL) || (myTank->getTeam() == ObserverTeam))
    return std::string();

  if (myTank->isAutoPilot()) {

      myTank->activateAutoPilot(false);
      hud->setAlert(0, "autopilot disabled", 1.0f, true);

      // grab mouse
      if (shouldGrabMouse()) mainWindow->grabMouse();

  } else if (BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    hud->setAlert(0, "autopilot not allowed on this server", 1.0f, true);
  } else {

      // don't enable the AutoPilot if you have within the last 5 secs
      static TimeKeeper LastAutoPilotEnable = TimeKeeper::getSunGenesisTime();
      if ((TimeKeeper::getCurrent() - LastAutoPilotEnable) > 5) {
	// reset timer
	LastAutoPilotEnable = TimeKeeper::getCurrent();

	// enable autopilot
	myTank->activateAutoPilot();
	hud->setAlert(0, "autopilot enabled", 1.0f, true);

	// ungrab mouse
	mainWindow->ungrabMouse();
      } else {
	controlPanel->addMessage("You may not enable the Autopilot more than once every five seconds.");
      }

  }

  return std::string();
}

static std::string cmdRadarZoom(const std::string&,
				const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: radarZoom {in|out}";

  float range = BZDB.eval("displayRadarRange");

  if (args[0] == "out") {
    range *= 1.05f;
    if (range > 2.0f)
      range = 2.0f;
    BZDB.setFloat("displayRadarRange", range);
  } else if (args[0] == "in") {
    range /= 1.05f;
    if (range < 0.005f)
      range = 0.005f;
    BZDB.setFloat("displayRadarRange", range);
  } else {
    return "usage: radarZoom {in|out}";
  }

  return std::string();
}

static std::string cmdViewZoom(const std::string&,
			       const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: viewZoom {in|out|toggle}";

  float fov = BZDB.eval("displayFOV");
  float defFov = BZDB.eval("defaultFOV");

  if (args[0] == "out") {
    fov += 1.0f;
    if (fov > defFov)
      fov = defFov;
    BZDB.setFloat("displayFOV", fov);
  } else if (args[0] == "in") {
    fov -= 1.0f;
    if (fov < 15.0f)
      fov = 15.0f;
    BZDB.setFloat("displayFOV", fov);
  } else if (args[0] == "toggle") {
    if (fov < 15.5f) {
      fov = defFov;
    } else {
      fov = 15.0f;
    }
    BZDB.setFloat("displayFOV", fov);
    // also toggle the observer fov
    if (ROAM.getZoom() != defFov) {
      ROAM.setZoom(defFov);
    } else {
      ROAM.setZoom(15.0f);
    }
  } else if (args[0] == "reset") {
    fov = defFov;
    ROAM.setZoom(defFov);
    BZDB.setFloat("displayFOV", fov);
  } else {
    return "usage: viewZoom {in|out|toggle|reset}";
  }

  return std::string();
}

static std::string cmdMessagePanel(const std::string&,
				   const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: messagepanel {all|chat|server|misc}";

  int oldMode = controlPanel->getMessagesMode();
  int newMode;
  if (args[0] == "all")
    newMode = 0;
  else if (args[0] == "chat")
    newMode = 1;
  else if (args[0] == "server")
    newMode = 2;
  else if (args[0] == "misc")
    newMode = 3;
  else
    return "usage: messagepanel {all|chat|server|misc}";

  if (newMode == oldMode)
    newMode = -1;
  controlPanel->setMessagesMode(newMode);

  return std::string();
}

static std::string cmdSend(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  static ComposeDefaultKey composeKeyHandler;
  if (args.size() != 1)
    return "usage: send {all|team|nemesis|recipient|admin}";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank)
    return "use send only when connected";
  std::string composePrompt;
  if (args[0] == "all") {
    void* buf = messageMessage;
    buf = nboPackUByte(buf, AllPlayers);
    composePrompt = "Send to all: ";
  } else if (args[0] == "team") {
    if (World::getWorld()->allowTeams() || myTank->getTeam() == ObserverTeam)
    {
      void* buf = messageMessage;
      buf = nboPackUByte(buf, TeamToPlayerId(myTank->getTeam()));
      composePrompt = "Send to teammates: ";
    }
    else {
      void* buf = messageMessage;
      buf = nboPackUByte(buf, AllPlayers);
      composePrompt = "Send to all: ";
    }
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
	if (remotePlayers[i]) {
	  myTank->setRecipient(remotePlayers[i]);
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
  } else if (args[0] == "admin") {
    void* buf = messageMessage;
    buf = nboPackUByte(buf, AdminPlayers);
    composePrompt = "Send to Admin: ";

  } else {
    return "usage: send {all|team|nemesis|recipient|admin}";
  }
  messageHistoryIndex = 0;
  hud->setComposing(composePrompt);
  HUDui::setDefaultKey(&composeKeyHandler);
  return std::string();
}


struct ScreenshotData
{
  std::string renderer;
  unsigned char* pixels;
  int xsize;
  int ysize;
  int channels;
};

#ifdef _WIN32
static DWORD WINAPI writeScreenshot(void* data)
#else
static void* writeScreenshot(void* data)
#endif
{
  ScreenshotData* ssdata = (ScreenshotData*)data;

  const std::string dirname = getScreenShotDirName();
  const std::string prefix  = "bzfi";
  const std::string ext     = ".png";

  // scan the directory and start numbering with the filename
  // that follows the existing filename with the highest snap number
  int snap = 0;

#ifdef _WIN32
  const std::string pattern = dirname + prefix + "*" + ext;
  WIN32_FIND_DATA findData;
  HANDLE h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    std::string file = findData.cFileName;
    snap = atoi((file.substr(file.length() - 8, 4)).c_str());
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      const int number = atoi((file.substr(file.length() - 8, 4)).c_str());
      if (snap < number) {
	snap = number;
      }
    }
  }
#else
  const std::string pattern = prefix + "*" + ext;
  DIR* directory = opendir(dirname.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (glob_match(pattern, file)) {
	const int number = atoi((file.substr(file.length() - 8, 4)).c_str());
	if (snap < number) {
	  snap = number;
	}
      }
    }
    closedir(directory);
  }
#endif // _WIN32

  snap++;
  std::string filename = dirname + prefix + TextUtils::format("%04d", snap) + ext;

  std::ostream* f = FILEMGR.createDataOutStream(filename.c_str(), true, true);

  if (f != NULL) {
    delete(f);

    const std::string& renderer = ssdata->renderer;
    unsigned char* pixels       = ssdata->pixels;
    const int xsize	     = ssdata->xsize;
    const int ysize	     = ssdata->ysize;
    const int channels	  = ssdata->channels;

    // Gamma-correction is preapplied by BZFlag's gamma table
    // This ignores the PNG gAMA chunk, but so do many viewers (including Mozilla)
    if (BZDB.isSet("gamma")) {
      const float gamma = BZDB.eval("gamma");
      if (gamma != 1.0f) {
	unsigned char gammaTable[256];
	for (int i = 0; i < 256; i++) {
	  const float lum    = float(i) / 256.0f;
	  const float lumadj = pow(lum, 1.0f / gamma);
	  gammaTable[i] = (unsigned char) (lumadj * 256);
	}
	const int pixelCount = (xsize * ysize * channels);
	for (int i = 0; i < pixelCount; i++) {
	  pixels[i] = gammaTable[pixels[i]];
	}
      }
    }

    const std::string versionStr = std::string("BZFlag") + getAppVersion();
    std::vector<BzPNG::Chunk> chunks;
    chunks.push_back(BzPNG::Chunk("tEXt", "Software", versionStr));
    chunks.push_back(BzPNG::Chunk("tEXt", "GL Renderer", renderer));

    char buf[128];
    if (BzPNG::save(filename, chunks, xsize, ysize, channels, pixels)) {
      snprintf(buf, sizeof(buf), "%s: %dx%d", filename.c_str(), xsize, ysize);
    } else {
      snprintf(buf, sizeof(buf), "%s: failed to save", filename.c_str());
    }
    ControlPanel::addMutexMessage(buf);
  }

  delete[] ssdata->pixels;
  delete ssdata;

  return NULL;
}

static std::string cmdScreenshot(const std::string&, const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: screenshot";
  }

  ScreenshotData* ssdata = new ScreenshotData;
  ssdata->renderer += (const char*)(glGetString(GL_VENDOR));
  ssdata->renderer += ": ";
  ssdata->renderer += (const char*)(glGetString(GL_RENDERER));
  ssdata->renderer += " (OpenGL ";
  ssdata->renderer += (const char*)(glGetString(GL_VERSION));
  ssdata->renderer += ")";
  int w = mainWindow->getWidth();
  int h = mainWindow->getHeight();
  ssdata->xsize = w;
  ssdata->ysize = h;
  ssdata->channels = 3; // GL_RGB
  ssdata->pixels = new unsigned char[h * w * 3];
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, ssdata->pixels);
  glPopClientAttrib();

#if defined(HAVE_PTHREADS)
  pthread_t thread;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, writeScreenshot, (void *) ssdata);
  pthread_attr_destroy(&attr);
#elif defined(_WIN32)
  CreateThread(
	    NULL, // Security attributes
	    0, // Stack size (0 -> default)
	    writeScreenshot,
	    ssdata,
	    0, // creation flags (0 -> run immediately)
	    NULL); // thread id return value (NULL -> don't care)
#else
  // no threads?  sucks to be you, but we'll still write the screenshot
  writeScreenshot(ssdata);
#endif

  return std::string();
}

static std::string cmdTime(const std::string&,
			   const CommandManager::ArgList& args, bool*)
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

static std::string cmdRoam(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  if (args.size() == 0)
    return "usage: roam {zoom|cycle} <args>";
  if (!ROAM.isRoaming())
    return std::string();
  if (args[0] == "zoom") {
    if (args.size() != 2)
      return "usage: roam zoom {in|out|normal|stop}";
    if (!roamButton || args[1] == "stop") {
      roamDZoom = 0.0f;
    } else if (args[1] == "out") {
      roamDZoom = +2.0f * BZDBCache::tankSpeed;
    } else if (args[1] == "in") {
      roamDZoom = -2.0f * BZDBCache::tankSpeed;
    } else if (args[1] == "normal") {
      ROAM.setZoom(60.0f);
    } else {
      return "usage: roam zoom {in|out|normal|stop}";
    }
  } else if (args[0] == "cycle") {
    if (args.size() != 3)
      return "usage: roam cycle {type|subject} {forward|backward}";
    if (args[1] == "type") {
      if (args[2] == "forward") {
	ROAM.setMode(Roaming::RoamingView((ROAM.getMode() + 1) % Roaming::roamViewCount));
      } else if (args[2] == "backward") {
	int setto = (ROAM.getMode() - 1) % Roaming::roamViewCount;
	if (setto < 0) setto += Roaming::roamViewCount;
	ROAM.setMode(Roaming::RoamingView(setto));
      } else {
	return "usage: roam cycle {type|subject} {forward|backward}";
      }
    } else if (args[1] == "subject") {
      if (args[2] == "forward") {
	ROAM.changeTarget(Roaming::next);
      } else if (args[2] == "backward") {
	ROAM.changeTarget(Roaming::previous);
      } else {
	return "usage: roam cycle {type|subject} {forward|backward}";
      }
    } else {
      return "usage: roam cycle {type|subject} {forward|backward}";
    }
  } else {
    return "usage: roam {zoom|cycle} <args>";
  }
  return std::string();
}

static std::string cmdSilence(const std::string&,
			      const CommandManager::ArgList& args, bool*)
{
  static SilenceDefaultKey silenceKeyHandler;
  if (args.size() != 0)
    return "usage: silence";
  messageHistoryIndex = 0;
  hud->setComposing("[Un]Silence: ");
  HUDui::setDefaultKey(&silenceKeyHandler);
  return std::string();
}

static std::string cmdServerCommand(const std::string&,
				    const CommandManager::ArgList& args, bool*)
{
  static ServerCommandKey serverCommandKeyHandler;
  if (args.size() != 0)
    return "usage: servercommand";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank)
    return "use only when connected";
  static bool prevAdmin = myTank->isAdmin();
  if (!prevAdmin && myTank->isAdmin()) serverCommandKeyHandler.adminInit();
  if (prevAdmin && !myTank->isAdmin()) serverCommandKeyHandler.nonAdminInit();
  prevAdmin = myTank->isAdmin();

  messageHistoryIndex = 0;
  serverCommandKeyHandler.init();
  HUDui::setDefaultKey(&serverCommandKeyHandler);
  return std::string();
}

static std::string cmdScrollPanel(const std::string&,
				  const CommandManager::ArgList& args, bool*)
{
  if ((args.size() < 1) || (args.size() > 2)) {
    return "usage: scrollpanel {up|up_page|down|down_page|top|bottom} [count]\n";
  }
  int count = 1;
  int linecount = 2;
  if (args.size() == 2) {
    count = atoi(args[1].c_str());
    linecount = count;
  }
  // whence - (0 = set, 1 = cur, 2 = end)
  if (args[0] == "up") {
    controlPanel->setMessagesOffset(+linecount, 1 /* current */, false);
  } else if (args[0] == "down") {
    controlPanel->setMessagesOffset(-linecount, 1 /* current */, false);
  } else if (args[0] == "up_page") {
    controlPanel->setMessagesOffset(+count, 1 /* current */, true);
  } else if (args[0] == "down_page") {
    controlPanel->setMessagesOffset(-count, 1 /* current */, true);
  } else if (args[0] == "top") {
    controlPanel->setMessagesOffset(123456789, 0 /* set */, false);
  } else if (args[0] == "bottom") {
    controlPanel->setMessagesOffset(0, 0 /* set */, false);
  } else if (args[0] == "pause") {
    controlPanel->togglePaused();
  }
  return std::string();
}


static std::string cmdHunt(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: hunt";
  hud->getScoreboard()->huntKeyEvent (false);
  return std::string();
}

static std::string cmdAddHunt(const std::string&,
			      const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: addhunt";
  hud->getScoreboard()->huntKeyEvent (true);
  return std::string();
}

static std::string cmdCycleRadar(const std::string&,
				  const CommandManager::ArgList& args, bool*)
{
  const std::string usageText = "usage: cycleRadar {level1 [level2 ...] [off]}:  cycle to the next radar zoom level";

  if (args.size() == 0)
    return usageText;

  std::vector<float> radarLevels;

  for (size_t i = 0; i < args.size(); ++i)
    if (args[i] == "off")
      radarLevels.push_back(0.0f);
    else if (atof(args[i].c_str()) > 0.0f)
      radarLevels.push_back((float)atof(args[i].c_str()));
    else
      return usageText;

  if (radarLevels.size() == 0)
    return usageText;

  if (radarLevels.size() == 1) {
    // only one specified... just set it
    BZDB.set("displayRadar", radarLevels[0] > 0.0f ? "1" : "0");

    if (radarLevels[0] > 0.0f)
      BZDB.setFloat("displayRadarRange", radarLevels[0]);

    return std::string();
  }

  static size_t radarLevelIndex = radarLevels.size() - 1;

  // if it's off and the current level is some form of on, turn it back on and set it
  if (BZDB.get("displayRadar") == "0" && radarLevels[radarLevelIndex] > 0.0f) {
    BZDB.set("displayRadar", "1");
    BZDB.setFloat("displayRadarRange", radarLevels[radarLevelIndex]);

    return std::string();
  }

  ++radarLevelIndex;
  if (radarLevelIndex >= radarLevels.size())
    radarLevelIndex = 0;

  if (radarLevels[radarLevelIndex] == 0.0f) {
    BZDB.set("displayRadar", "0");
  } else {
    BZDB.setFloat("displayRadarRange", radarLevels[radarLevelIndex]);
    BZDB.set("displayRadar", "1");
  }

  return std::string();
}

static std::string cmdCyclePanel(const std::string&,
				  const CommandManager::ArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: cyclePanel {left[_off]|right[_off]}\n";

  bool forward = args[0] == "right" || args[0] == "right_off";
  bool includeOff = args[0] == "left_off" || args[0] == "right_off";

  if (! BZDB.isTrue("displayConsole")) {
    if (forward && includeOff && controlPanel->getMessagesMode() == 3)
      // reversed directions... put it back at beginning
      controlPanel->setMessagesMode(0);
    else if (! forward && includeOff && controlPanel->getMessagesMode() == 0)
      // reversed directions... put it back at end
      controlPanel->setMessagesMode(3);

    BZDB.setBool("displayConsole", true);
  } else {
    if (controlPanel->getMessagesMode() == 0) {
      controlPanel->setMessagesMode(forward ? 1 : 3);

      if (! forward && includeOff)
	BZDB.setBool("displayConsole", false);
    } else if (controlPanel->getMessagesMode() == 1) {
      controlPanel->setMessagesMode(forward ? 2 : 0);
    } else if (controlPanel->getMessagesMode() == 2) {
      controlPanel->setMessagesMode(forward ? 3 : 1);
    } else {
      controlPanel->setMessagesMode(forward ? 0 : 2);

      if (forward && includeOff)
	BZDB.setBool("displayConsole", false);
    }
  }

  return std::string();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
