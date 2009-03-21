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

/* interface header */
#include "commands.h"

/* system implementation headers */
#include "zlib.h"
#ifndef _WIN32
#  include <sys/types.h>
#  include <dirent.h>
#endif
#ifdef HAVE_PTHREADS
#  include <pthread.h>
#endif
#include <string>
using std::string;

/* common implementation headers */
#include "BZDBCache.h"
#include "DirectoryNames.h"
#include "EventHandler.h"
#include "FileManager.h"
#include "LuaClientScripts.h"
#include "MotionUtils.h"
#include "TextUtils.h"
#include "bzglob.h"
#include "version.h"

/* local implementation headers */
#include "ComposeDefaultKey.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "ServerCommandKey.h"
#include "SilenceDefaultKey.h"
#include "sound.h"
#include "playing.h"


typedef CommandManager::ArgList CmdArgList;


static string cmdAddHunt       (const string&, const CmdArgList& args, bool*);
static string cmdAutoPilot     (const string&, const CmdArgList& args, bool*);
static string cmdDestruct      (const string&, const CmdArgList& args, bool*);
static string cmdDrop          (const string&, const CmdArgList& args, bool*);
static string cmdFire          (const string&, const CmdArgList& args, bool*);
static string cmdHunt          (const string&, const CmdArgList& args, bool*);
static string cmdIconify       (const string&, const CmdArgList& args, bool*);
static string cmdIdentify      (const string&, const CmdArgList& args, bool*);
static string cmdJump          (const string&, const CmdArgList& args, bool*);
static string cmdLuaBzOrg      (const string&, const CmdArgList& args, bool*);
static string cmdLuaUser       (const string&, const CmdArgList& args, bool*);
static string cmdLuaWorld      (const string&, const CmdArgList& args, bool*);
static string cmdMessagePanel  (const string&, const CmdArgList& args, bool*);
static string cmdMouseGrab     (const string&, const CmdArgList& args, bool*);
static string cmdPause         (const string&, const CmdArgList& args, bool*);
static string cmdRadarZoom     (const string&, const CmdArgList& args, bool*);
static string cmdRestart       (const string&, const CmdArgList& args, bool*);
static string cmdRoam          (const string&, const CmdArgList& args, bool*);
static string cmdScreenshot    (const string&, const CmdArgList& args, bool*);
static string cmdScrollPanel   (const string&, const CmdArgList& args, bool*);
static string cmdSend          (const string&, const CmdArgList& args, bool*);
static string cmdServerCommand (const string&, const CmdArgList& args, bool*);
static string cmdSilence       (const string&, const CmdArgList& args, bool*);
static string cmdTime          (const string&, const CmdArgList& args, bool*);
static string cmdToggleConsole (const string&, const CmdArgList& args, bool*);
static string cmdToggleFlags   (const string&, const CmdArgList& args, bool*);
static string cmdToggleFS      (const string&, const CmdArgList& args, bool*);
static string cmdToggleRadar   (const string&, const CmdArgList& args, bool*);
static string cmdViewZoom      (const string&, const CmdArgList& args, bool*);


static std::vector<CommandListItem> commandVector;


const std::vector<CommandListItem>& getCommandList()
{
  if (!commandVector.empty()) {
    return commandVector;
  }
#undef  PUSHCMD
#define PUSHCMD(n, f, h) commandVector.push_back(CommandListItem(n, f, h));
  PUSHCMD("fire",          &cmdFire,          "fire:  fire a shot");
  PUSHCMD("jump",          &cmdJump,          "jump:  make player jump");
  PUSHCMD("drop",          &cmdDrop,          "drop:  drop the current flag");
  PUSHCMD("identify",      &cmdIdentify,      "identify:  identify/lock-on-to player in view");
  PUSHCMD("restart",       &cmdRestart,       "restart:  restart playing");
  PUSHCMD("destruct",      &cmdDestruct,      "destruct:  self destruct");
  PUSHCMD("pause",         &cmdPause,         "pause:  pause/resume");
  PUSHCMD("send",          &cmdSend,          "send {all|team|nemesis|recipient|admin}:  start composing a message");
  PUSHCMD("screenshot",    &cmdScreenshot,    "screenshot:  take a screenshot");
  PUSHCMD("time",          &cmdTime,          "time {forward|backward|<seconds>}:  adjust the current time");
  PUSHCMD("roam",          &cmdRoam,          "roam {zoom|cycle} <args>:  roam around");
  PUSHCMD("silence",       &cmdSilence,       "silence:  silence/unsilence a player");
  PUSHCMD("servercommand", &cmdServerCommand, "servercommand:  quick admin");
  PUSHCMD("scrollpanel",   &cmdScrollPanel,   "scrollpanel {up|down}:  scroll message panel");
  PUSHCMD("hunt",          &cmdHunt,          "hunt:  hunt a specific player");
  PUSHCMD("addhunt",       &cmdAddHunt,       "addhunt:  add/modify hunted player(s)");
  PUSHCMD("iconify",       &cmdIconify,       "iconify: iconify & pause bzflag");
  PUSHCMD("mousegrab",     &cmdMouseGrab,     "mousegrab: toggle exclusive mouse mode");
  PUSHCMD("fullscreen",    &cmdToggleFS,      "fullscreen: toggle fullscreen mode");
  PUSHCMD("autopilot",     &cmdAutoPilot,     "autopilot:  set/unset autopilot bot code");
  PUSHCMD("radarZoom",     &cmdRadarZoom,     "radarZoom {in/out}: change maxRadar range");
  PUSHCMD("viewZoom",      &cmdViewZoom,      "viewZoom {in/out/toggle}: change view angle");
  PUSHCMD("messagepanel",  &cmdMessagePanel,  "messagepanel {all|chat|server|misc|debug}:  set message tab");
  PUSHCMD("toggleRadar",   &cmdToggleRadar,   "toggleRadar:  toggle radar visibility");
  PUSHCMD("toggleConsole", &cmdToggleConsole, "toggleConsole:  toggle console visibility");
  PUSHCMD("toggleFlags",   &cmdToggleFlags,   "toggleFlags {main|radar}:  turn off/on field radar flags");
  PUSHCMD("luauser",       &cmdLuaUser,       "luauser {'reload | disable | status }: control luauser");
  PUSHCMD("luabzorg",      &cmdLuaBzOrg,      "luabzorg {'reload | disable | status }: control luabzorg");
  PUSHCMD("luaworld",      &cmdLuaWorld,      "luaworld {'reload | disable | status }: control luaworld");
#undef  PUSHCMD
  return commandVector;
}


static string cmdToggleFS(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: fullscreen";
  mainWindow->toggleFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  return string();
}


static string cmdMouseGrab(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: mousegrab";
  const bool grabbing = !(BZDB.isTrue("mousegrab"));
  BZDB.set("mousegrab", grabbing ? "true" : "false");
  mainWindow->enableGrabMouse(grabbing);
  return string();
}


static string cmdIconify(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: iconify";
  if (!BZDB.isTrue("Win32NoMin"))
    mainWindow->iconify();
  return string();
}


static string cmdJump(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: jump";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL)
    myTank->setJump();
  return string();
}


static string cmdToggleFlags(const string&, const
                                  CmdArgList& args, bool*)
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

  return string();
}


static string cmdToggleRadar(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: toggleRadar";
  }

  CMDMGR.run("toggle displayRadar");

  warnAboutRadar();

  return string();
}


static string cmdToggleConsole(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: toggleConsole";
  }

  CMDMGR.run("toggle displayConsole");

  warnAboutConsole();

  return string();
}


static string cmdFire(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: fire";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (fireButton && myTank != NULL && myTank->isAlive()
      && myTank->getTeam() != ObserverTeam)
    myTank->fireShot();
  return string();
}


static string cmdDrop(const string&, const CmdArgList& args, bool*)
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
      if (!eventHandler.ForbidFlagDrop()) {
        serverLink->sendDropFlag(myTank->getPosition());
        // changed: on windows it may happen the MsgDropFlag
        // never comes back to us, so we drop it right away
        handleFlagDropped(myTank);
      }
    }
  }
  return string();
}


static string cmdIdentify(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: identify";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL)
    if (myTank->isAlive() && !myTank->isPaused())
      setTarget();
  return string();
}


static string cmdRestart(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: restart";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL && canSpawn) {
    if (!gameOver && !myTank->isSpawning() &&
        (myTank->getTeam() != ObserverTeam) &&
        !myTank->isAlive() && !myTank->isExploding()) {
      if (!eventHandler.ForbidSpawn()) {
        serverLink->sendAlive(myTank->getId());
        myTank->setSpawning(true);
        CmdArgList zoomArgs;
        string resetArg = "reset";
        zoomArgs.push_back(resetArg);
        cmdViewZoom("", zoomArgs,NULL);
      }
    }
  }
  return string();
}


static string cmdDestruct(const string&, const CmdArgList& args, bool*)
{
  char msgBuf[40];

  if (args.size() != 0)
    return "usage: destruct";
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    if (destructCountdown > 0.0f) {
      destructCountdown = 0.0f;
      hud->setAlert(1, "Self Destruct canceled", 1.5f, true);
    } else {
      static BZDB_float maxVelocity(StateDatabase::BZDB_MAXSELFDESTRUCTVEL);
      if (getMagnitude(myTank->getVelocity()) > maxVelocity) {
        hud->setAlert(1, "No Self Destruct while moving", 1.0f, false);
      }
      else {
        destructCountdown = 5.0f;
        sprintf(msgBuf, "Self Destructing in %d", (int)(destructCountdown + 0.99f));
        hud->setAlert(1, msgBuf, 1.0f, false);
      }
    }
  }
  return string();
}


static string cmdPause(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: pause";

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  ServerLink        *server = ServerLink::getServer();
  if (!pausedByUnmap && myTank && myTank->isAlive() && !myTank->isAutoPilot()) {
    if (myTank->isPaused()) {
      // already paused, so unpause
      myTank->setPause(false);
      controlPanel->addMessage("Resumed");

      // restore the sound
      if (savedVolume != -1) {
        SOUNDSYSTEM.setVolume(savedVolume*0.1f);
        savedVolume = -1;
      }

      // grab mouse
      if (shouldGrabMouse()) {
        mainWindow->grabMouse();
      }

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
  return string();
}


static string cmdAutoPilot(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: autopilot";

  LocalPlayer *myTank = LocalPlayer::getMyTank();

  if (BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS) && !myTank->isAutoPilot()) {
    hud->setAlert(0, "autopilot not allowed on this server", 1.0f, true);
    return string();
  }

  if (myTank != NULL && myTank->getTeam() != ObserverTeam) {
    // if I am an auto pilot and I requested it to be turned on
    // in the first place, send the request to disable it.
    if (myTank->isAutoPilot() && myTank->requestedAutopilot) {
      myTank->requestAutoPilot(false);
    }
    else if (!myTank->isAutoPilot()) { // can't ask for autopilot if I'm already autopilot
      // don't enable the AutoPilot if you have within the last 5 secs
      static TimeKeeper LastAutoPilotEnable = TimeKeeper::getSunGenesisTime();
      if ((TimeKeeper::getCurrent() - LastAutoPilotEnable) > 5) {
        // reset timer
        LastAutoPilotEnable = TimeKeeper::getCurrent();

        // enable autopilot
        myTank->requestAutoPilot();
      }
      else {
        controlPanel->addMessage(
          "You may not request the Autopilot more than once every five seconds.");
        return string();
      }
    }
  }

  return string();
}


static string cmdRadarZoom(const string&, const CmdArgList& args, bool*)
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

  return string();
}


static string cmdViewZoom(const string&, const CmdArgList& args, bool*)
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
    if (fov < (defFov+15.0f)*0.5f) { // 60+15/2
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

  return string();
}


static string cmdMessagePanel(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 1) {
    return "usage: messagepanel {all|chat|server|misc|debug}";
  }

  ControlPanel::MessageModes mode = ControlPanel::MessageAll;
       if (args[0] == "all")    { mode = ControlPanel::MessageAll;    }
  else if (args[0] == "chat")   { mode = ControlPanel::MessageChat;   }
  else if (args[0] == "server") { mode = ControlPanel::MessageServer; }
  else if (args[0] == "misc")   { mode = ControlPanel::MessageMisc;   }
  else if (args[0] == "debug")  { mode = ControlPanel::MessageDebug;  }
  else {
    return "usage: messagepanel {all|chat|server|misc|debug}";
  }

  controlPanel->setMessagesMode(mode);

  if (!BZDB.isTrue("displayConsole")) {
    BZDB.setBool("displayConsole", true);
  }

  return string();
}


static string cmdSend(const string&, const CmdArgList& args, bool*)
{
  static ComposeDefaultKey composeKeyHandler;
  if (args.size() != 1) {
    return "usage: send {all|team|nemesis|recipient|admin}";
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();

  string composePrompt;

  if (!myTank) {
    msgDestination = AllPlayers;
    composePrompt = "Local command: ";
  }
  else if (args[0] == "all") {
    msgDestination = AllPlayers;
    composePrompt = "Send to all: ";
  }
  else if (args[0] == "team") {
    if (World::getWorld() && World::getWorld()->allowTeams()) {
      msgDestination = TeamToPlayerId(myTank->getTeam());
      composePrompt = "Send to teammates: ";
    } else {
      msgDestination = AllPlayers;
      composePrompt = "Send to all: ";
    }
  }
  else if (args[0] == "nemesis") {
    const Player* nemesis = myTank->getNemesis();
    if (!nemesis)
      return string();
    msgDestination = nemesis->getId();
    composePrompt = "Send to ";
    composePrompt += nemesis->getCallSign();
    composePrompt += ": ";
  }
  else if (args[0] == "recipient") {
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
      msgDestination = recipient->getId();
      composePrompt = "Send to ";
      composePrompt += recipient->getCallSign();
      composePrompt += ": ";
    }
  }
  else if (args[0] == "admin") {
    msgDestination = AdminPlayers;
    composePrompt = "Send to Admin : ";
  }
  else {
    return "usage: send {all|team|nemesis|recipient|admin}";
  }

  messageHistoryIndex = 0;
  hud->setComposing(composePrompt);
  HUDui::setDefaultKey(&composeKeyHandler);

  return string();
}


// yeah, we reuse the mutex for different crit sections.  so shoot me.
#if defined(HAVE_PTHREADS)
  static pthread_mutex_t screenshot_mutex;
# define LOCK_SCREENSHOT_MUTEX pthread_mutex_lock(&screenshot_mutex);
# define UNLOCK_SCREENSHOT_MUTEX pthread_mutex_unlock(&screenshot_mutex);
#elif defined(_WIN32)
  static CRITICAL_SECTION screenshot_critical;
# define LOCK_SCREENSHOT_MUTEX EnterCriticalSection(&screenshot_critical);
# define UNLOCK_SCREENSHOT_MUTEX LeaveCriticalSection(&screenshot_critical);
#else
# define LOCK_SCREENSHOT_MUTEX
# define UNLOCK_SCREENSHOT_MUTEX
#endif

struct ScreenshotData
{
  unsigned char* rawPixels;
  string renderer;
};

#ifdef _WIN32
static DWORD WINAPI writeScreenshot(void* data)
#else
static void* writeScreenshot(void* data)
#endif
{
  ScreenshotData* ssdata = (ScreenshotData*)data;

  const string dirname = getScreenShotDirName();
  const string prefix  = "bzfi";
  const string ext     = ".png";

  // scan the directory and start numbering with the filename
  // that follows the existing filename with the highest snap number
  int snap = 0;

#ifdef _WIN32
  const string pattern = dirname + prefix + "*" + ext;
  WIN32_FIND_DATA findData;
  HANDLE h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    string file;
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      const int number = atoi((file.substr(file.length() - 8, 4)).c_str());
      if (snap < number) {
        snap = number;
      }
    }
  }
#else
  const string pattern = prefix + "*" + ext;
  DIR* directory = opendir(dirname.c_str());
  if (directory) {
    struct dirent* contents;
    string file;
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
  string filename = dirname + prefix + TextUtils::format("%04d", snap) + ext;

  std::ostream* f = FILEMGR.createDataOutStream(filename.c_str(), true, true);

  if (f != NULL) {
    int w = mainWindow->getWidth(), h = mainWindow->getHeight();
    const unsigned long blength = h * w * 3 + h;    //size of b[] and br[]
    unsigned char* b = new unsigned char[blength];  //screen of pixels + column for filter type required by PNG - filtered data

    // Prepare gamma table
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
    temp = htonl(w);              //(data) Image width
    f->write((char*) &temp, 4);
    crc = crc32(crc, (unsigned char*) &temp, 4);
    temp = htonl(h);              //(data) Image height
    f->write((char*) &temp, 4);
    crc = crc32(crc, (unsigned char*) &temp, 4);
    tempByte = 8;                 //(data) Image bitdepth (8 bits/sample = 24 bits/pixel)
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    tempByte = 2;                 //(data) Color type: RGB = 2
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
      memcpy(b + line + 1, ssdata->rawPixels + i * w * 3, w * 3); //copy captured line
      // apply gamma correction if necessary
      if (gammaAdjust) {
        unsigned char *ptr = b + line + 1;
        for (int j = 1; j < w * 3 + 1; j++) {
          *ptr = gammaTable[*ptr];
          ptr++;
        }
      }
    }

    unsigned long zlength = blength + 15;            //length of bz[], will be changed by zlib to the length of the compressed string contained therein
    unsigned char* bz = new unsigned char[zlength]; //just like b, but compressed; might get bigger, so give it room
    // compress b into bz
    compress2(bz, &zlength, b, blength, 5);

    temp = htonl(zlength);                          //(length) IDAT length after compression
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("IDAT"));                   //(tag) IDAT
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    f->write(reinterpret_cast<char*>(bz), zlength);  //(data) This line of pixels, compressed
    crc = htonl(crc32(crc, bz, zlength));
    f->write((char*) &crc, 4);                       //(crc) write crc

    // tEXt chunk containing bzflag build/version
    string version = "BZFlag "; version += getAppVersion();
    temp = htonl(9 + (int)version.size()); //(length) tEXt is strlen("Software") + 1 + version.size()
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("tEXt"));                   //(tag) tEXt
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    strcpy(reinterpret_cast<char*>(b), "Software"); //(data) Keyword
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b)));
    tempByte = 0;                                    //(data) Null character separator
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    strncpy((char*) b, version.c_str(), blength-1);               //(data) Text contents (build/version)
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = htonl(crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b))));
    f->write((char*) &crc, 4);                       //(crc) write crc

    // tEXt chunk containing gl renderer information
    temp = htonl(12 + (int)ssdata->renderer.size()); //(length) tEXt is len("GL Renderer") + 1 + renderer.size()
    f->write((char*) &temp, 4);
    temp = htonl(PNGTAG("tEXt"));                   //(tag) tEXt
    f->write((char*) &temp, 4);
    crc = crc32(crc = 0, (unsigned char*) &temp, 4);
    strcpy(reinterpret_cast<char*>(b), "GL Renderer"); //(data) Keyword
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b)));
    tempByte = 0;                                    //(data) Null character separator
    f->write(&tempByte, 1);
    crc = crc32(crc, (unsigned char*) &tempByte, 1);
    strncpy((char*) b, ssdata->renderer.c_str(), blength-1);  //(data) Text contents (GL information)
    f->write(reinterpret_cast<char*>(b), (unsigned)strlen(reinterpret_cast<const char*>(b)));
    crc = htonl(crc32(crc, b, (unsigned)strlen(reinterpret_cast<const char*>(b))));
    f->write((char*) &crc, 4);                       //(crc) write crc

    // IEND chunk
    temp = htonl((int) 0);        //(length) IEND is always 0 bytes long
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
    LOCK_SCREENSHOT_MUTEX // prevent simultaneous access to the control panel messages array
    controlPanel->addMessage(notify);
    UNLOCK_SCREENSHOT_MUTEX
  }

  delete ssdata->rawPixels;
  delete ssdata;

  return NULL;
}


static string cmdScreenshot(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0) {
    return "usage: screenshot";
  }

  ScreenshotData* ssdata = new ScreenshotData;
  int w = mainWindow->getWidth(), h = mainWindow->getHeight();
  ssdata->rawPixels = new unsigned char[h * w * 3];
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, ssdata->rawPixels);
  ssdata->renderer = reinterpret_cast<const char*>(glGetString(GL_VENDOR)); ssdata->renderer += ": ";
  ssdata->renderer += reinterpret_cast<const char*>(glGetString(GL_RENDERER)); ssdata->renderer += " (OpenGL ";
  ssdata->renderer += reinterpret_cast<const char*>(glGetString(GL_VERSION)); ssdata->renderer += ")";

#if defined(HAVE_PTHREADS)
  pthread_t thread;
  static bool inited = false;

  if (!inited) {
    pthread_mutex_init(&screenshot_mutex, NULL);
    inited = true;
  }

  pthread_create(&thread, NULL, writeScreenshot, (void *) ssdata);
#elif defined(_WIN32)
  static bool inited = false;

  if (!inited) {
    InitializeCriticalSection(&screenshot_critical);
    inited = true;
  }

  CreateThread(
            NULL, // Security attributes
            0, // Stack size (0 -> default)
            writeScreenshot,
            ssdata,
            0, // creation flags (0 -> run immediately)
            NULL); // thread id return value (NULL -> don't care)
#else
  // no threads?  sucks to be you, but we'll still write the screenshot
  writeScreenshot(&ssdata);
#endif

  return string();
}


static string cmdTime(const string&, const CmdArgList& args, bool*)
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
  return string();
}


static string cmdRoam(const string&, const CmdArgList& args, bool*)
{
  if (args.size() == 0)
    return "usage: roam {zoom|cycle} <args>";
  if (!ROAM.isRoaming())
    return string();
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
  return string();
}


static string cmdSilence(const string&, const CmdArgList& args, bool*)
{
  static SilenceDefaultKey silenceKeyHandler;
  if (args.size() != 0)
    return "usage: silence";
  messageHistoryIndex = 0;
  hud->setComposing("[Un]Silence: ");
  HUDui::setDefaultKey(&silenceKeyHandler);
  return string();
}


static string cmdServerCommand(const string&, const CmdArgList& args, bool*)
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
  return string();
}


static string cmdScrollPanel(const string&, const CmdArgList& args, bool*)
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
  return string();
}


static string cmdHunt(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: hunt";
  hud->getScoreboard()->huntKeyEvent (false);
  return string();
}

static string cmdAddHunt(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 0)
    return "usage: addhunt";
  hud->getScoreboard()->huntKeyEvent (true);
  return string();
}


static string concatArgs(const CmdArgList& args)
{
  string line;
  for (size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      line = args[i];
    } else {
      line += " " + args[i];
    }
  }
  return line;
}


static string cmdLuaUser(const string& cmd, const CmdArgList& args, bool*)
{
  if (args.size() < 1) {
    return "usage: luauser { reload | disable | status }";
  }
  LuaClientScripts::LuaUserCommand(cmd + " " + concatArgs(args));
  return string();
}


static string cmdLuaBzOrg(const string& cmd, const CmdArgList& args, bool*)
{
  if (args.size() < 1) {
    return "usage: luabzorg { reload | disable | status }";
  }
  LuaClientScripts::LuaBzOrgCommand(cmd + " " + concatArgs(args));
  return string();
}


static string cmdLuaWorld(const string& cmd, const CmdArgList& args, bool*)
{
  if (args.size() < 1) {
    return "usage: luaworld { reload | disable | status }";
  }
  LuaClientScripts::LuaWorldCommand(cmd + " " + concatArgs(args));
  return string();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
