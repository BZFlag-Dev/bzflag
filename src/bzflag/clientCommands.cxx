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

/* interface header */
#include "commands.h"

/* system implementation headers */
#include "../zlib/zconf.h"
#include "../zlib/zlib.h"
#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#endif

/* common implementation headers */
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FileManager.h"
#include "DirectoryNames.h"
#include "version.h"
#include "bzglob.h"

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

#ifdef SNAPPING
/** capture a screenshot
 */
static std::string cmdScreenshot(const std::string&,
				 const CommandManager::ArgList& args, bool*);
#endif

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

/** toggle mouse capture
 */
static std::string cmdMouseGrab(const std::string&,
				const CommandManager::ArgList& args, bool*);

/** toggle Full Screen
 */
static std::string cmdToggleFS(const std::string&,
			       const CommandManager::ArgList& args, bool*);

const struct CommandListItem commandList[] = {
  { "fire",	&cmdFire,	"fire:  fire a shot" },
  { "jump",	&cmdJump,	"jump:  make player jump" },
  { "drop",	&cmdDrop,	"drop:  drop the current flag" },
  { "identify",	&cmdIdentify,	"identify:  identify/lock-on-to player in view" },
  { "restart",	&cmdRestart,	"restart:  restart playing" },
  { "destruct", &cmdDestruct,	"destruct:  self destruct" },
  { "pause",	&cmdPause,	"pause:  pause/resume" },
  { "send",	&cmdSend,	"send {all|team|nemesis|recipient|admin}:  start composing a message" },
#ifdef SNAPPING
  { "screenshot", &cmdScreenshot, "screenshot:  take a screenshot" },
#endif
  { "time",	&cmdTime,	"time {forward|backward|<seconds>}:  adjust the current time" },
  { "roam",	&cmdRoam,	"roam {zoom|cycle} <args>:  roam around" },
  { "silence",	&cmdSilence,	"silence:  silence/unsilence a player" },
  { "servercommand",	&cmdServerCommand,	"servercommand:  quick admin" },
  { "scrollpanel",	&cmdScrollPanel,	"scrollpanel {up|down}:  scroll message panel" },
  { "hunt",	&cmdHunt,	"hunt:  hunt a specific player" },
  { "addhunt",	&cmdAddHunt,	"addhunt:  add/modify hunted player(s)" },
  { "iconify",  &cmdIconify,	"iconify: iconify & pause bzflag" },
  { "mousegrab", &cmdMouseGrab, "mousegrab: toggle exclusive mouse mode" },
  { "fullscreen", &cmdToggleFS, "fullscreen: toggle fullscreen mode" },
  { "autopilot",&cmdAutoPilot,	"autopilot:  set/unset autopilot bot code" },
  { "radarZoom", &cmdRadarZoom, "radarZoom {in/out}: change maxRadar range"},
  { "viewZoom",  &cmdViewZoom,  "viewZoom {in/out/toggle}: change view angle"},
  { "messagepanel", &cmdMessagePanel,
    "messagepanel {all|chat|server|misc}:  set message tab" },
  { "toggleRadar", &cmdToggleRadar, "toggleRadar:  toggle radar visibility"},
  { "toggleConsole", &cmdToggleConsole, "toggleConsole:  toggle console visibility"},
  { "toggleFlags", &cmdToggleFlags, "toggleFlags {main|radar}:  turn off/on field radar flags"}
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
      serverLink->sendAlive(myTank->getId());
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
  ServerLink	*server = ServerLink::getServer();
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
      server->sendPaused(false);

    } else if (myTank->getLocation() == LocalPlayer::InBuilding) {
      // custom message when trying to pause while in a building
      // (could get stuck on un-pause if flag is taken)
      hud->setAlert(1, "Can't pause while inside a building", 1.0f, false);

    } else if (myTank->getLocation() == LocalPlayer::InAir || myTank->isFalling()) {
      // custom message when trying to pause when jumping/falling
      hud->setAlert(1, "Can't pause when you are in the air", 1.0f, false);

    } else if (myTank->getLocation() != LocalPlayer::OnGround &&
	       myTank->getLocation() != LocalPlayer::OnBuilding) {
      // catch-all message when trying to pause when you should not
      hud->setAlert(1, "Unable to pause right now", 1.0f, false);

    } else {
      // update the pause alert message
      pauseCountdown = 5.0f;
      server->sendPaused(true);

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

  if (BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    hud->setAlert(0, "autopilot not allowed on this server", 1.0f, true);
    return std::string();
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL && myTank->getTeam() != ObserverTeam) {
    if (myTank->isAutoPilot()) {

      myTank->activateAutoPilot(false);
      hud->setAlert(0, "autopilot disabled", 1.0f, true);

      // grab mouse
      if (shouldGrabMouse()) mainWindow->grabMouse();

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
	return std::string();
      }

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

  if (args[0] == "out") {
    fov += 1.0f;
    if (fov > 60.0f)
      fov = 60.0f;
    BZDB.setFloat("displayFOV", fov);
  } else if (args[0] == "in") {
    fov -= 1.0f;
    if (fov < 15.0f)
      fov = 15.0f;
    BZDB.setFloat("displayFOV", fov);
  } else if (args[0] == "toggle") {
    if (fov < 37.5) { // 60+15/2
      fov = 60.0f;
    } else {
      fov = 15.0f;
    }
    BZDB.setFloat("displayFOV", fov);
    // also toggle the observer fov
    if (ROAM.getZoom() != 60.0f) {
      ROAM.setZoom(60.0f);
    } else {
      ROAM.setZoom(15.0f);
    }
  } else if (args[0] == "reset") {
    fov = 60.0f;
    ROAM.setZoom(60.0f);
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

  int mode = 0;
  if (args[0] == "all")
    mode = 0;
  else if (args[0] == "chat")
    mode = 1;
  else if (args[0] == "server")
    mode = 2;
  else if (args[0] == "misc")
    mode = 3;
  else
    return "usage: messagepanel {all|chat|server|misc}";
    
  controlPanel->setMessagesMode(mode);

  if (!BZDB.isTrue("displayConsole")) {
    BZDB.setBool("displayConsole", true);
  }

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
    msgDestination = AllPlayers;
    composePrompt = "Send to all: ";
  } else if (args[0] == "team") {
    if (World::getWorld() && World::getWorld()->allowTeams()) {
      msgDestination = TeamToPlayerId(myTank->getTeam());
      composePrompt = "Send to teammates: ";
    } else {
      msgDestination = AllPlayers;
      composePrompt = "Send to all: ";
    }
  } else if (args[0] == "nemesis") {
    const Player* nemesis = myTank->getNemesis();
    if (!nemesis)
      return std::string();
    msgDestination = nemesis->getId();
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
      msgDestination = recipient->getId();
      composePrompt = "Send to ";
      composePrompt += recipient->getCallSign();
      composePrompt += ": ";
    }
  } else if (args[0] == "admin") {
    msgDestination = AdminPlayers;
    composePrompt = "Send to Admin : ";
  } else {
    return "usage: send {all|team|nemesis|recipient|admin}";
  }
  messageHistoryIndex = 0;
  hud->setComposing(composePrompt);
  HUDui::setDefaultKey(&composeKeyHandler);
  return std::string();
}

#ifdef SNAPPING
static std::string cmdScreenshot(const std::string&,
				 const CommandManager::ArgList& args, bool*)
{
  int snap = atoi(BZDB.get(std::string("lastScreenshot")).c_str());
  snap++;

  if (args.size() != 0)
    return "usage: screenshot";

  std::string filename = getScreenShotDirName();
  const std::string prefix = "bzfi";
  const std::string ext = ".png";
  // if this is the first shot we've taken, scan the directory 
  // and start numbering with the first available
  if (snap == 0) {
    std::string pattern = filename + prefix + "*" + ext;
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(pattern.c_str(), &findData);
    if (h != INVALID_HANDLE_VALUE) {
      std::string file;
      while (FindNextFile(h, &findData)) {
	file = findData.cFileName;
	int number = atoi((file.substr(file.length()-8, 4)).c_str());
	if (number > snap)
	  snap = number;
      }
    }
#else
    DIR *directory = opendir(filename.c_str());
    if (directory) {
      struct dirent* contents;
      std::string file, suffix;
      while ((contents = readdir(directory))) {
	file = contents->d_name;
	if (glob_match(pattern, file)) {
	  int number = atoi((file.substr(file.length()-8, 4)).c_str());
	  if (number > snap)
	    number = snap;
	}
      }
      closedir(directory);
    }
#endif // _WIN32
  }

  filename += prefix + TextUtils::format("%04d", ++snap) + ext;

  BZDB.setInt(std::string("lastScreenshot"),snap);

  std::ostream* f = FILEMGR.createDataOutStream (filename.c_str(), true, true);

  if (f != NULL) {
    int w = mainWindow->getWidth(), h = mainWindow->getHeight();
    const unsigned long blength = h * w * 3 + h;    //size of b[] and br[]
    unsigned char* b = new unsigned char[blength];  //screen of pixels + column for filter type required by PNG - filtered data

    // pause to prevent dt from accumulating until done screenshotting
    LocalPlayer *myTank = LocalPlayer::getMyTank();
    bool temp_paused = false;
    if (myTank && !myTank->isPaused())
	{
      myTank->setDeadStop();
	  serverLink->sendPlayerUpdate(myTank);
      temp_paused = true;
    }

    //Prepare gamma table
    const bool gammaAdjust = BZDB.isSet("gamma");
    unsigned char gammaTable[256];
    if (gammaAdjust) {
      float gamma = (float) atof(BZDB.get("gamma").c_str());
      for (int i = 0; i < 256; i++) {
	float lum = ((float) i) / 256.0f;
	float lumadj = pow(lum, 1.0f / gamma);
	gammaTable[i] = (unsigned char) (lumadj * 256);
      }
    }

    /* Write a PNG stream.
       FIXME: Note that there are several issues with this code, altho it produces perfectly fine PNGs.
       1. We do no filtering.  Sub filters increase screenshot size, other filters would require a lot of effort to implement.
       2. Gamma-correction is preapplied by BZFlag's gamma table.  This ignores the PNG gAMA chunk, but so do many viewers (including Mozilla)
       3. Timestamp (tIME) chunk is not added to the file, but would be a good idea.
    */
    int temp = 0; //temporary values for binary file writing
    char tempByte = 0;
    int crc = 0;  //used for running CRC values

    // Write PNG headers
    (*f) << "\211PNG\r\n\032\n";
#define PNGTAG(t_) ((((int)t_[0]) << 24) | \
		   (((int)t_[1]) << 16) | \
		   (((int)t_[2]) <<  8) | \
		   (int)t_[3])

    // IHDR chunk
    temp = htonl((int) 13);       //(length) IHDR is always 13 bytes long
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("IHDR")); //(tag) IHDR
    f->write((char*) &temp, 4);
    crc = crc32(crc, (unsigned char*) &temp, 4);
    temp = htonl(w);	      //(data) Image width
    f->write((char*) &temp, 4);
    crc = crc32(crc, (unsigned char*) &temp, 4);
    temp = htonl(h);	      //(data) Image height
    f->write((char*) &temp, 4);
    crc = crc32(crc, (unsigned char*) &temp, 4);
    tempByte = 8;		 //(data) Image bitdepth (8 bits/sample = 24 bits/pixel)
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    tempByte = 2;		 //(data) Color type: RGB = 2
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    tempByte = 0;
    int i;
    for (i = 0; i < 3; i++) { //(data) Last three tags are compression (only 0 allowed), filtering (only 0 allowed), and interlacing (we don't use it, so it's 0)
      f->write(&tempByte, 1);
      crc = crc32(crc, (unsigned char*) &tempByte, 1);
    }
    crc = htonl(crc);
    f->write((char*) &crc, 4);    //(crc) write crc

    // IDAT chunk
    for (i = h - 1; i >= 0; i--) {
      const unsigned long line = (h - (i + 1)) * (w * 3 + 1); //beginning of this line
      b[line] = 0;  //filter type byte at the beginning of each scanline (0 = no filter, 1 = sub filter)
      glReadPixels(0, i, w, 1, GL_RGB, GL_UNSIGNED_BYTE, b + line + 1); //capture line
      // apply gamma correction if necessary
      if (gammaAdjust) {
	unsigned char *ptr = b + line + 1;
	for (int j = 1; j < w * 3 + 1; j++) {
	  *ptr = gammaTable[*ptr];
	  ptr++;
	}
      }
    }

    // let the user know we're paused while saving the file
    if (temp_paused) {
      drawFrame(0.0f);
    }

    unsigned long zlength = blength + 15;	    //length of bz[], will be changed by zlib to the length of the compressed string contained therein
    unsigned char* bz = new unsigned char[zlength]; //just like b, but compressed; might get bigger, so give it room
    // compress b into bz
    compress2(bz, &zlength, b, blength, 5);

    temp = htonl(zlength);			  //(length) IDAT length after compression
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("IDAT"));		   //(tag) IDAT
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    f->write(reinterpret_cast<char*>(bz), zlength);  //(data) This line of pixels, compressed
    crc = htonl(crc32(crc, bz, zlength));
    f->write((char*) &crc, 4);		       //(crc) write crc

    // tEXt chunk containing bzflag build/version
    temp = htonl(9 + (int)strlen(getAppVersion())); //(length) tEXt is 9 + strlen(getAppVersion())
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("tEXt"));		   //(tag) tEXt
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    strcpy(reinterpret_cast<char*>(b), "Software"); //(data) Keyword
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b)));
    tempByte = 0;				    //(data) Null character separator
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    strcpy((char*) b, getAppVersion());	       //(data) Text contents (build/version)
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = htonl(crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b))));
    f->write((char*) &crc, 4);		       //(crc) write crc

    // IEND chunk
    temp = htonl((int) 0);	//(length) IEND is always 0 bytes long
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("IEND")); //(tag) IEND
    f->write((char*) &temp, 4);
    crc = htonl(crc32(crc = 0, (unsigned char*) &temp, 4));
    //(data) IEND has no data field
    f->write((char*) &crc, 4);     //(crc) write crc
    delete [] bz;
    delete [] b;
    delete f;
    char notify[128];
    snprintf(notify, 128, "%s: %dx%d", filename.c_str(), w, h);
    controlPanel->addMessage(notify);

    // unpause to prevent dt from accumulating until done screenshotting
    if (myTank && temp_paused)
	{
		// do a micro update to prevent DT overuns
		myTank->update(0.001f);
		myTank->setDeadStop();
		serverLink->sendPlayerUpdate(myTank);
    }
    // update the display regardless of pausing
    drawFrame(0.0f);
  }
  return std::string();
}
#endif

static std::string cmdTime(const std::string&,
			   const CommandManager::ArgList& args, bool*)
{
  // FIXME - time should be moved into BZDB
  if (args.size() != 1)
    return "usage: time {forward|backward|<seconds>}";
  if (args[0] == "forward") {
    clockAdjust += 5.0f * 60.0f;
  } else if (args[0] == "backward") {
    clockAdjust -= 5.0f * 60.0f;
  } else {
    float seconds;
    char* end;
    seconds = (float)strtod(args[0].c_str(), &end);
    if (end != args[0].c_str()) {
      clockAdjust += seconds;
    } else {
      return "usage: time {forward|backward|<seconds>}";
    }
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


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
