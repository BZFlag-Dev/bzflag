/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "commands.h"

/* system implementation headers */
#include <fstream>
#include <string>
#include "../zlib/zconf.h"
#include "../zlib/zlib.h"

/* common implementation headers */
#include "global.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "FileManager.h"
#include "DirectoryNames.h"
#include "version.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "sound.h"
#include "ComposeDefaultKey.h"
#include "Roster.h"
#include "SilenceDefaultKey.h"
#include "ServerCommandKey.h"

/* FIXME -- from playing.cxx */
extern LocalPlayer* myTank;
extern bool fireButton;
#include "ServerLink.h"
extern ServerLink* serverLink;
void handleFlagDropped(Player* tank);
void setTarget();
extern bool gameOver;
extern float destructCountdown;
#include "HUDRenderer.h"
extern HUDRenderer* hud;
extern bool pausedByUnmap;
#include "ControlPanel.h"
extern ControlPanel* controlPanel;
extern int savedVolume;
bool shouldGrabMouse();
#include "MainWindow.h"
extern MainWindow* mainWindow;
extern float pauseCountdown;
#include "Address.h"
extern char messageMessage[PlayerIdPLen + MessageLen];
extern float clockAdjust;
extern bool roaming;
extern enum roamingView {
  roamViewFree = 0,
  roamViewTrack,
  roamViewFollow,
  roamViewFP,
  roamViewFlag,
  roamViewCount
} roamView;
extern int roamTrackTank, roamTrackWinner, roamTrackFlag;
extern float roamPos[3], roamDPos[3];
extern float roamTheta, roamDTheta;
extern float roamPhi, roamDPhi;
extern float roamZoom, roamDZoom;
extern bool roamButton;
#include "World.h"
extern World* world;
void setRoamingLabel(bool force);
extern void warnAboutMainFlags();
extern void warnAboutRadarFlags();
extern void warnAboutConsoleAndRadar();


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
  { "time",	&cmdTime,	"time {forward|backward}:  adjust the current time" },
  { "roam",	&cmdRoam,	"roam {zoom|cycle} <args>:  roam around" },
  { "silence",	&cmdSilence,	"silence:  silence/unsilence a player" },
  { "servercommand",	&cmdServerCommand,	"servercommand:  quick admin" },
  { "scrollpanel",	&cmdScrollPanel,	"scrollpanel {up|down}:  scroll message panel" },
  { "hunt",	&cmdHunt,	"hunt:  hunt a specific player" },
  { "iconify",  &cmdIconify,	"iconify: iconify & pause bzflag" },
  { "fullscreen", &cmdToggleFS, "fullscreen: toggle fullscreen mode" },
  { "autopilot",&cmdAutoPilot,	"autopilot:  set/unset autopilot bot code" },
  { "radarZoom", &cmdRadarZoom, "radarZoom {in/out}: change maxRadar range"},
  { "viewZoom",  &cmdViewZoom,  "viewZoom {in/out/toggle}: change view angle"},
  { "messagepanel", &cmdMessagePanel,
    "messagepanel {all|chat|server|misc}:  set message tab" },
  { "toggleConsoleAndRadar", &cmdToggleConsoleAndRadar, "toggleConsoleAndRadar:  turn off/on radar and console"},
  { "toggleFlags", &cmdToggleFlags, "toggleFlags {main|radar}:  turn off/on field radar flags"}
};


std::string cmdToggleFS(const std::string&,
			const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: fullscreen";
  mainWindow->toggleFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  return std::string();
}

std::string cmdIconify(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: iconify";
  mainWindow->iconify();
  return std::string();
}

std::string cmdJump(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: jump";
  if (myTank != NULL)
    myTank->setJump();
  return std::string();
}

std::string cmdToggleFlags (const std::string&, const
CommandManager::ArgList& args)
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

std::string cmdToggleConsoleAndRadar (const std::string&, const
CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: toggleConsoleAndRadar";
  CMDMGR.run("toggle displayConsoleAndRadar");
  warnAboutConsoleAndRadar();

  return std::string();
}



std::string cmdFire(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: fire";
  if (fireButton && myTank != NULL && myTank->isAlive()
      && myTank->getTeam() != ObserverTeam)
    myTank->fireShot();
  return std::string();
}

std::string cmdDrop(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: drop";
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

std::string cmdIdentify(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: identify";
  if (myTank != NULL)
    if (myTank->isAlive() && !myTank->isPaused())
      setTarget();
  return std::string();
}

std::string cmdRestart(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: restart";
  if (myTank != NULL)
    if (!gameOver && !myTank->isSpawning() && (myTank->getTeam() != ObserverTeam) && !myTank->isAlive() && !myTank->isExploding()) {
      serverLink->sendAlive();
      myTank->setSpawning(true);
    }

  return std::string();
}

std::string cmdDestruct(const std::string&, const CommandManager::ArgList& args)
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

std::string cmdPause(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: pause";

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

std::string cmdAutoPilot(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: autopilot";

  if (BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    hud->setAlert(0, "autopilot not allowed on this server", 1.0f, true);
    return std::string();
  }

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

std::string cmdRadarZoom(const std::string&,
			 const CommandManager::ArgList& args)
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
    if (range < 0.125f)
      range = 0.125f;
    BZDB.setFloat("displayRadarRange", range);
  } else {
    return "usage: radarZoom {in|out}";
  }

  return std::string();
}

std::string cmdViewZoom(const std::string&,
			const CommandManager::ArgList& args)
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
    if (fov < 15.5f)
      fov = 60.0f;
    else
      fov = 15.0f;
    BZDB.setFloat("displayFOV", fov);
  } else {
    return "usage: viewZoom {in|out|toggle}";
  }

  return std::string();
}

std::string cmdMessagePanel(const std::string&,
			    const CommandManager::ArgList& args)
{
  if (args.size() != 1)
    return "usage: messagepanel {all|chat|server|misc}";

  if (args[0] == "all")
    controlPanel->setMessagesMode(0);
  else if (args[0] == "chat")
    controlPanel->setMessagesMode(1);
  else if (args[0] == "server")
    controlPanel->setMessagesMode(2);
  else if (args[0] == "misc")
    controlPanel->setMessagesMode(3);
  else
    return "usage: messagepanel {all|chat|server|misc}";

  return std::string();
}

std::string cmdSend(const std::string&, const CommandManager::ArgList& args)
{
  static ComposeDefaultKey composeKeyHandler;
  if (args.size() != 1)
    return "usage: send {all|team|nemesis|recipient|admin}";
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
  } else if (args[0] == "admin") {
    void* buf = messageMessage;
    buf = nboPackUByte(buf, AdminPlayers);
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
std::string cmdScreenshot(const std::string&, const CommandManager::ArgList& args)
{
  static int snap = 0;
  if (args.size() != 0)
    return "usage: screenshot";

  std::string filename = getScreenShotDirName();
  filename += TextUtils::format("bzfi%04d.png", snap++);

  std::ostream* f = FILEMGR.createDataOutStream (filename.c_str(), true, true);

  if (f != NULL) {
    int w = mainWindow->getWidth(), h = mainWindow->getHeight();
    const unsigned long blength = h * w * 3 + h;    //size of b[] and br[]
    unsigned char* b = new unsigned char[blength];  //screen of pixels + column for filter type required by PNG - filtered data

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
	for (int i = 1; i < w * 3 + 1; i++) {
	  *ptr = gammaTable[*ptr];
	  ptr++;
	}
      }
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
    temp = htonl((int) 9 + strlen(getAppVersion()));//(length) tEXt is 9 + strlen(getAppVersion())
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("tEXt"));		   //(tag) tEXt
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    strcpy(reinterpret_cast<char*>(b), "Software"); //(data) Keyword
    f->write(reinterpret_cast<char*>(b), strlen(reinterpret_cast<const char*>(b)));
    crc = crc32(crc, b, strlen(reinterpret_cast<const char*>(b)));
    tempByte = 0;				    //(data) Null character separator
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    strcpy((char*) b, getAppVersion());	     //(data) Text contents (build/version)
    f->write(reinterpret_cast<char*>(b), strlen(reinterpret_cast<const char*>(b)));
    crc = htonl(crc32(crc, b, strlen(reinterpret_cast<const char*>(b))));
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
  }
  return std::string();
}
#endif

std::string cmdTime(const std::string&, const CommandManager::ArgList& args)
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

std::string cmdRoam(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() == 0)
    return "usage: roam {zoom|cycle} <args>";
  if (!roaming)
    return std::string();
  if (args[0] == "zoom") {
    if (args.size() != 2)
      return "usage: roam zoom {in|out|normal|stop}";
    if (!roamButton || args[1] == "stop") {
      roamDZoom = 0.0f;
    } else if (args[1] == "out") {
      roamDZoom = 50.0f;
    } else if (args[1] == "in") {
      roamDZoom = -50.0f;
    } else if (args[1] == "normal") {
      roamZoom = 60.0f;
    } else {
      return "usage: roam zoom {in|out|normal|stop}";
    }
  } else if (args[0] == "cycle") {
    if (args.size() != 3)
      return "usage: roam cycle {type|subject} {forward|backward}";
    if (args[1] == "type") {
      if (args[2] == "forward") {
	roamView = roamingView((roamView + 1) % roamViewCount);
	if (roamView == roamViewFlag) {
	  const int maxFlags = world->getMaxFlags();
	  bool found = false;
	  for (int i = 0; i < maxFlags; i++) {
	    const Flag& flag = world->getFlag(i);
	    if (flag.type->flagTeam != NoTeam) {
	      roamTrackFlag = i;
	      found = true;
	      break;
	    }
	  }
	  if (!found)
	    roamView = roamViewFree;
	} else if ((roamTrackTank != -1) && (roamView == roamViewTrack || roamView == roamViewFollow || roamView == roamViewFP)) {
	  if ((player[roamTrackTank] != NULL) && (!player[roamTrackTank]->isAlive())) {
	    bool found = false;
	    for (int i = 0; i < curMaxPlayers; i++) {
	      if (player[i] && player[i]->isAlive()) {
		roamTrackTank = roamTrackWinner = i;
		found = true;
		break;
	      }
	    }
	    if (!found)
	      roamTrackTank = -1;
	  }
	}
	setRoamingLabel(true);
      } else if (args[2] == "backward") {
	// FIXME
      } else {
	return "usage: roam cycle {type|subject} {forward|backward}";
      }
    } else if (args[1] == "subject") {
      if (args[2] == "forward") {
	if (roamView == roamViewFree) {
	  // do nothing
	} else if (roamView == roamViewFlag) {
	  const int maxFlags = world->getMaxFlags();
	  for (int i = 1; i < maxFlags; i++) {
	    int j = (roamTrackFlag + i) % maxFlags;
	    const Flag& flag = world->getFlag(j);
	    if (flag.type->flagTeam != NoTeam) {
	      roamTrackFlag = j;
	      break;
	    }
	  }
	} else {
	  int i, j;
	  for (i = 0; i < curMaxPlayers; i++) {
	    j = (roamTrackTank + i + 2) % (curMaxPlayers + 1) - 1;
	    if ((j == -1) || (player[j] && player[j]->isAlive())) {
	      roamTrackTank = roamTrackWinner = j;
	      break;
	    }
	  }
	}
	setRoamingLabel(true);
      } else if (args[2] == "backward") {
	if (roamView == roamViewFree) {
	  // do nothing
	} else if (roamView == roamViewFlag) {
	  const int maxFlags = world->getMaxFlags();
	  for (int i = 1; i < maxFlags; i++) {
	    int j = (roamTrackFlag - i + maxFlags) % maxFlags;
	    const Flag& flag = world->getFlag(j);
	    if (flag.type->flagTeam != NoTeam) {
	      roamTrackFlag = j;
	      break;
	    }
	  }
	} else {
	  for (int i = 0; i < curMaxPlayers; i++) {
	    int j = (roamTrackTank - i + curMaxPlayers + 1) % (curMaxPlayers + 1) - 1;
	    if ((j == -1) || (player[j] && player[j]->isAlive())) {
	      roamTrackTank = roamTrackWinner = j;
	      break;
	    }
	  }
	}
	setRoamingLabel(true);
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

std::string cmdSilence(const std::string&, const CommandManager::ArgList& args)
{
  static SilenceDefaultKey silenceKeyHandler;
  if (args.size() != 0)
    return "usage: silence";
  messageHistoryIndex = 0;
  hud->setComposing("[Un]Silence: ");
  HUDui::setDefaultKey(&silenceKeyHandler);
  return std::string();
}

std::string cmdServerCommand(const std::string&, const CommandManager::ArgList& args)
{
  static ServerCommandKey serverCommandKeyHandler;
  if (args.size() != 0)
    return "usage: servercommand";
  static bool prevAdmin = myTank->isAdmin();
  if (!prevAdmin && myTank->isAdmin()) serverCommandKeyHandler.adminInit();
  if (prevAdmin && !myTank->isAdmin()) serverCommandKeyHandler.nonAdminInit();
  prevAdmin = myTank->isAdmin();

  messageHistoryIndex = 0;
  serverCommandKeyHandler.init();
  HUDui::setDefaultKey(&serverCommandKeyHandler);
  return std::string();
}

std::string cmdScrollPanel(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 1)
    return "usage: scrollpanel {up|down}\n";
  if (args[0] == "up") {
    controlPanel->setMessagesOffset(2,1);
  } else if (args[0] == "down") {
    controlPanel->setMessagesOffset(-2,1);
  } else {
    return "usage: scrollpanel {up|down}\n";
  }
  return std::string();
}

std::string cmdHunt(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 0)
    return "usage: hunt";
  if (hud->getHunting()) {
    hud->setHunting(false);
  } else {
    playLocalSound(SFX_HUNT);
    hud->setHunt(!hud->getHunt());
    hud->setHuntPosition(0);
    if (!BZDB.isTrue("displayScore"))
      BZDB.set("displayScore", "1");
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
