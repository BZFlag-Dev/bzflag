/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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

/* common implementation headers */
#include "BZDBCache.h"
#include "DirectoryNames.h"
#include "EventHandler.h"
#include "FileManager.h"
#include "LuaClientScripts.h"
#include "MotionUtils.h"
#include "SceneRenderer.h"
#include "TextUtils.h"
#include "bzglob.h"
#include "version.h"

/* local implementation headers */
#include "BzPNG.h"
#include "ComposeDefaultKey.h"
#include "HUDRenderer.h"
#include "HUDDialogStack.h"
#include "HUDui.h"
#include "HubComposeKey.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "ServerCommandKey.h"
#include "SilenceDefaultKey.h"
#include "ShotStats.h"
#include "sound.h"
#include "playing.h"
// FIXME: Shouldn't need to depend on GUI elements
#include "guiplaying.h"
#include "LocalCommand.h"

typedef CommandManager::ArgList CmdArgList;


static std::string cmdAddHunt(const std::string&, const CmdArgList& args, bool*);
static std::string cmdAutoPilot(const std::string&, const CmdArgList& args, bool*);
static std::string cmdDestruct(const std::string&, const CmdArgList& args, bool*);
static std::string cmdDrop(const std::string&, const CmdArgList& args, bool*);
static std::string cmdFire(const std::string&, const CmdArgList& args, bool*);
static std::string cmdHubCompose(const std::string&, const CmdArgList& args, bool*);
static std::string cmdHunt(const std::string&, const CmdArgList& args, bool*);
static std::string cmdIconify(const std::string&, const CmdArgList& args, bool*);
static std::string cmdIdentify(const std::string&, const CmdArgList& args, bool*);
static std::string cmdJump(const std::string&, const CmdArgList& args, bool*);
static std::string cmdLuaBzOrg(const std::string&, const CmdArgList& args, bool*);
static std::string cmdLuaUser(const std::string&, const CmdArgList& args, bool*);
static std::string cmdLuaWorld(const std::string&, const CmdArgList& args, bool*);
static std::string cmdLuaRules(const std::string&, const CmdArgList& args, bool*);
static std::string cmdLocalCmd(const std::string&, const CmdArgList& args, bool*);
static std::string cmdMessagePanel(const std::string&, const CmdArgList& args, bool*);
static std::string cmdMouseBox(const std::string&, const CmdArgList& args, bool*);
static std::string cmdMouseGrab(const std::string&, const CmdArgList& args, bool*);
static std::string cmdPause(const std::string&, const CmdArgList& args, bool*);
static std::string cmdRadarZoom(const std::string&, const CmdArgList& args, bool*);
static std::string cmdRestart(const std::string&, const CmdArgList& args, bool*);
static std::string cmdRoam(const std::string&, const CmdArgList& args, bool*);
static std::string cmdScreenshot(const std::string&, const CmdArgList& args, bool*);
static std::string cmdScrollPanel(const std::string&, const CmdArgList& args, bool*);
static std::string cmdSend(const std::string&, const CmdArgList& args, bool*);
static std::string cmdSendMsg(const std::string&, const CmdArgList& args, bool*);
static std::string cmdServerCommand(const std::string&, const CmdArgList& args, bool*);
static std::string cmdSilence(const std::string&, const CmdArgList& args, bool*);
static std::string cmdShotStats(const std::string&, const CmdArgList& args, bool*);
static std::string cmdTime(const std::string&, const CmdArgList& args, bool*);
static std::string cmdToggleConsole(const std::string&, const CmdArgList& args, bool*);
static std::string cmdToggleFlags(const std::string&, const CmdArgList& args, bool*);
static std::string cmdToggleFS(const std::string&, const CmdArgList& args, bool*);
static std::string cmdToggleRadar(const std::string&, const CmdArgList& args, bool*);
static std::string cmdViewZoom(const std::string&, const CmdArgList& args, bool*);


static std::vector<CommandListItem> commandVector;


const std::vector<CommandListItem>& getCommandList() {
  if (!commandVector.empty()) {
    return commandVector;
  }
#undef  PUSHCMD
#define PUSHCMD(n, f, h) commandVector.push_back(CommandListItem(n, f, h));
  PUSHCMD("fire",          &cmdFire,          "fire:  fire a shot");
  PUSHCMD("jump",          &cmdJump,          "jump:  make player jump");
  PUSHCMD("drop",          &cmdDrop,          "drop:  drop the current flag");
  PUSHCMD("hubcompose",    &cmdHubCompose,    "hubcompose:  quick hub access");
  PUSHCMD("identify",      &cmdIdentify,      "identify:  identify/lock-on-to player in view");
  PUSHCMD("restart",       &cmdRestart,       "restart:  restart playing");
  PUSHCMD("destruct",      &cmdDestruct,      "destruct:  self destruct");
  PUSHCMD("pause",         &cmdPause,         "pause:  pause/resume");
  PUSHCMD("send",          &cmdSend,          "send {all|team|nemesis|recipient|admin}:  start composing a message");
  PUSHCMD("sendmsg",       &cmdSendMsg,       "send {all|team|nemesis|recipient|admin} <message>:  send a message");
  PUSHCMD("screenshot",    &cmdScreenshot,    "screenshot:  take a screenshot");
  PUSHCMD("shotstats",     &cmdShotStats,     "shotstats:  shot statistics");
  PUSHCMD("time",          &cmdTime,          "time {forward|backward|<seconds>}:  adjust the current time");
  PUSHCMD("roam",          &cmdRoam,          "roam {zoom|cycle} <args>:  roam around");
  PUSHCMD("silence",       &cmdSilence,       "silence:  silence/unsilence a player");
  PUSHCMD("servercommand", &cmdServerCommand, "servercommand:  quick admin");
  PUSHCMD("scrollpanel",   &cmdScrollPanel,   "scrollpanel {up|down}:  scroll message panel");
  PUSHCMD("hunt",          &cmdHunt,          "hunt:  hunt a specific player");
  PUSHCMD("addhunt",       &cmdAddHunt,       "addhunt:  add/modify hunted player(s)");
  PUSHCMD("iconify",       &cmdIconify,       "iconify: iconify & pause bzflag");
  PUSHCMD("localcmd",      &cmdLocalCmd,      "localcmd: process a local command");
  PUSHCMD("mousebox",      &cmdMouseBox,      "mousebox <size>: change the mousebox size");
  PUSHCMD("mousegrab",     &cmdMouseGrab,     "mousegrab: toggle exclusive mouse mode");
  PUSHCMD("fullscreen",    &cmdToggleFS,      "fullscreen: toggle fullscreen mode");
  PUSHCMD("autopilot",     &cmdAutoPilot,     "autopilot:  set/unset autopilot bot code");
  PUSHCMD("radarZoom",     &cmdRadarZoom,     "radarZoom {in/out}: change maxRadar range");
  PUSHCMD("viewZoom",      &cmdViewZoom,      "viewZoom {in/out/toggle}: change view angle");
  PUSHCMD("messagepanel",  &cmdMessagePanel,  "messagepanel {all|chat|server|misc|debug|prev|next}:  set message tab");
  PUSHCMD("toggleRadar",   &cmdToggleRadar,   "toggleRadar:  toggle radar visibility");
  PUSHCMD("toggleConsole", &cmdToggleConsole, "toggleConsole:  toggle console visibility");
  PUSHCMD("toggleFlags",   &cmdToggleFlags,   "toggleFlags {main|radar}:  turn off/on field radar flags");
  PUSHCMD("luauser",       &cmdLuaUser,       "luauser {'reload | disable | status }: control luauser");
  PUSHCMD("luabzorg",      &cmdLuaBzOrg,      "luabzorg {'reload | disable | status }: control luabzorg");
  PUSHCMD("luaworld",      &cmdLuaWorld,      "luaworld {'reload | disable | status }: control luaworld");
  PUSHCMD("luarules",      &cmdLuaRules,      "luarules {'reload | disable | status }: control luarules");
#undef  PUSHCMD
  return commandVector;
}


static std::string cmdToggleFS(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: fullscreen";
  }
  mainWindow->toggleFullscreen();
  mainWindow->getWindow()->callResizeCallbacks();
  return std::string();
}


static std::string cmdMouseBox(const std::string&, const CmdArgList& args, bool*) {
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


static std::string cmdMouseGrab(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: mousegrab";
  }
  const bool grabbing = !(BZDB.isTrue("mousegrab"));
  BZDB.set("mousegrab", grabbing ? "true" : "false");
  mainWindow->enableGrabMouse(grabbing);
  return std::string();
}


static std::string cmdIconify(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: iconify";
  }
  if (!BZDB.isTrue("Win32NoMin")) {
    mainWindow->iconify();
  }
  return std::string();
}


static std::string cmdLocalCmd(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: localcmd command args";
  }

  std::string cmd = args[0];
  for (size_t i = 1; i < args.size(); i++) {
    cmd += " " + args[i];
  }
  LocalCommand::execute(cmd.c_str());
  return std::string();
}


static std::string cmdJump(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: jump";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    myTank->setJump();
  }
  return std::string();
}


static std::string cmdToggleFlags(const std::string&, const
                                  CmdArgList& args, bool*) {
  if (args.size() != 1) {
    return "usage: main|radar";
  }
  if (args[0] == "main") {
    CMDMGR.run("toggle displayMainFlags");
    warnAboutMainFlags();
  }
  else if (args[0] == "radar") {
    CMDMGR.run("toggle displayRadarFlags");
    warnAboutRadarFlags();
  }
  else {
    return "usage: main|radar";
  }

  return std::string();
}


static std::string cmdToggleRadar(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: toggleRadar";
  }

  CMDMGR.run("toggle displayRadar");

  warnAboutRadar();

  return std::string();
}


static std::string cmdToggleConsole(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: toggleConsole";
  }

  CMDMGR.run("toggle displayConsole");

  warnAboutConsole();

  return std::string();
}


static std::string cmdFire(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: fire";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (fireButton &&
      (myTank != NULL) && myTank->isAlive() && !myTank->isObserver()) {
    myTank->fireShot();
  }
  return std::string();
}


static std::string cmdDrop(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: drop";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    FlagType* flag = myTank->getFlagType();
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
  return std::string();
}


static std::string cmdIdentify(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: identify";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    if (myTank->isAlive() && !myTank->isPaused()) {
      setTarget();
    }
  }
  return std::string();
}


static std::string cmdRestart(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: restart";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if ((myTank != NULL) && entered && canSpawn) {
    if (!gameOver
        && !myTank->isSpawning()
        && !myTank->isObserver()
        && !myTank->isAlive()
        && !myTank->isExploding()) {
      if (!eventHandler.ForbidSpawn()) {
        serverLink->sendAlive(myTank->getId());
        myTank->setSpawning(true);
        CmdArgList zoomArgs;
        std::string resetArg = "reset";
        zoomArgs.push_back(resetArg);
        cmdViewZoom("", zoomArgs, NULL);
      }
    }
  }
  return std::string();
}


static std::string cmdDestruct(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: destruct";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    if (destructCountdown > 0.0f) {
      destructCountdown = 0.0f;
      hud->setAlert(1, "Self Destruct canceled", 1.5f, true);
    }
    else {
      destructCountdown = 5.0f;
    }
  }
  return std::string();
}


static std::string cmdPause(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: pause";
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  ServerLink*        server = ServerLink::getServer();
  if (!pausedByUnmap && myTank && myTank->isAlive() && !myTank->isAutoPilot()) {
    if (myTank->isPaused()) {
      // already paused, so unpause
      server->sendPaused(false);
    }
    else if (pauseCountdown > 0.0f) {
      // player aborted pause
      pauseCountdown = 0.0f;
      hud->setAlert(1, "Pause cancelled", 1.5f, true);
      server->sendPaused(false);
    }
    else if (myTank->getLocation() == LocalPlayer::InBuilding) {
      // custom message when trying to pause while in a building
      // (could get stuck on un-pause if flag is taken)
      hud->setAlert(1, "Can't pause while inside a building", 1.0f, false);
    }
    else if (myTank->getLocation() == LocalPlayer::InAir || myTank->isFalling()) {
      // custom message when trying to pause when jumping/falling
      hud->setAlert(1, "Can't pause when you are in the air", 1.0f, false);
    }
    else if (myTank->getLocation() != LocalPlayer::OnGround &&
             myTank->getLocation() != LocalPlayer::OnBuilding) {
      // catch-all message when trying to pause when you should not
      hud->setAlert(1, "Unable to pause right now", 1.0f, false);
    }
    else {
      // update the pause alert message
      pauseCountdown = 5.0f;
      server->sendPaused(true);
    }
  }
  return std::string();
}


static std::string cmdAutoPilot(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: autopilot";
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();

  if (BZDB.isTrue(BZDBNAMES.DISABLEBOTS) && !myTank->isAutoPilot()) {
    hud->setAlert(0, "autopilot not allowed on this server", 1.0f, true);
    return std::string();
  }

  if ((myTank != NULL) && !myTank->isObserver()) {
    // if I am an auto pilot and I requested it to be turned on
    // in the first place, send the request to disable it.
    if (myTank->isAutoPilot() && myTank->requestedAutopilot) {
      myTank->requestAutoPilot(false);
    }
    else if (!myTank->isAutoPilot()) { // can't ask for autopilot if I'm already autopilot
      // don't enable the AutoPilot if you have within the last 5 secs
      static BzTime LastAutoPilotEnable = BzTime::getSunGenesisTime();
      if ((BzTime::getCurrent() - LastAutoPilotEnable) > 5) {
        // reset timer
        LastAutoPilotEnable = BzTime::getCurrent();

        // enable autopilot
        myTank->requestAutoPilot();
      }
      else {
        controlPanel->addMessage(
          "You may not request the Autopilot more than once every five seconds.");
        return std::string();
      }
    }
  }

  return std::string();
}


static std::string cmdRadarZoom(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 1) {
    return "usage: radarZoom {in|out}";
  }

  float range = BZDB.eval("displayRadarRange");

  if (args[0] == "out") {
    range *= 1.05f;
    if (range > 2.0f) {
      range = 2.0f;
    }
    BZDB.setFloat("displayRadarRange", range);
  }
  else if (args[0] == "in") {
    range /= 1.05f;
    if (range < 0.005f) {
      range = 0.005f;
    }
    BZDB.setFloat("displayRadarRange", range);
  }
  else {
    return "usage: radarZoom {in|out}";
  }

  return std::string();
}


static std::string cmdViewZoom(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 1) {
    return "usage: viewZoom {in|out|toggle}";
  }

  float fov = BZDB.eval("displayFOV");
  float defFov = BZDB.eval("defaultFOV");

  if (args[0] == "out") {
    fov += 1.0f;
    if (fov > defFov) {
      fov = defFov;
    }
    BZDB.setFloat("displayFOV", fov);
  }
  else if (args[0] == "in") {
    fov -= 1.0f;
    if (fov < 15.0f) {
      fov = 15.0f;
    }
    BZDB.setFloat("displayFOV", fov);
  }
  else if (args[0] == "toggle") {
    if (fov < (defFov + 15.0f) * 0.5f) { // 60+15/2
      fov = defFov;
    }
    else {
      fov = 15.0f;
    }
    BZDB.setFloat("displayFOV", fov);
    // also toggle the observer fov
    if (ROAM.getZoom() != defFov) {
      ROAM.setZoom(defFov);
    }
    else {
      ROAM.setZoom(15.0f);
    }
  }
  else if (args[0] == "reset") {
    fov = defFov;
    ROAM.setZoom(defFov);
    BZDB.setFloat("displayFOV", fov);
  }
  else {
    return "usage: viewZoom {in|out|toggle|reset}";
  }

  return std::string();
}


static std::string cmdMessagePanel(const std::string&, const CmdArgList& args, bool*) {
  if (controlPanel == NULL) {
    return "";
  }

  if (args.size() != 1) {
    return "usage: messagepanel {all|chat|server|misc|debug|prev|next}";
  }
  const std::string& tabName = args[0];

  int tab = -1;

  if (tabName == "prev") {
    const int tabCount = controlPanel->getTabCount();
    tab = controlPanel->getActiveTab();
    tab = (tab - 1 + tabCount) % tabCount;
    while (!controlPanel->isTabVisible(tab)) {
      tab = (tab - 1 + tabCount) % tabCount;
    }
  }
  else if (tabName == "next") {
    const int tabCount = controlPanel->getTabCount();
    tab = controlPanel->getActiveTab();
    tab = (tab + 1) % tabCount;
    while (!controlPanel->isTabVisible(tab)) {
      tab = (tab + 1) % tabCount;
    }
  }
  else {
    tab = controlPanel->getTabID(tabName);
  }

  if (tab < 0) {
    return "messagepanel, bad tab: '" + tabName + "'";
  }

  controlPanel->setActiveTab(tab);

  if (!BZDB.isTrue("displayConsole")) {
    BZDB.setBool("displayConsole", true);
  }

  return std::string();
}


static std::string cmdSend(const std::string&, const CmdArgList& args, bool*) {
  static ComposeDefaultKey composeKeyHandler;
  if (args.size() != 1) {
    return "usage: send {all|team|nemesis|recipient|admin}";
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();

  std::string composePrompt;

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
    }
    else {
      msgDestination = AllPlayers;
      composePrompt = "Send to all: ";
    }
  }
  else if (args[0] == "nemesis") {
    const Player* nemesis = myTank->getNemesis();
    if (!nemesis) {
      return std::string();
    }
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

  return std::string();
}


static std::string cmdSendMsg(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: sendmsg {all|team|nemesis|recipient|admin} <message>";
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return "sendmsg: you do not exist";
  }

  if (!serverLink) {
    return "sendmsg: not connected";
  }

  PlayerId msgDest = NoPlayer;

  // parse the destination
  if (args[0] == "all") {
    msgDest = AllPlayers;
  }
  else if (args[0] == "team") {
    if (World::getWorld() && World::getWorld()->allowTeams()) {
      msgDest = TeamToPlayerId(myTank->getTeam());
    }
    else {
      msgDest = AllPlayers;
    }
  }
  else if (args[0] == "nemesis") {
    const Player* nemesis = myTank->getNemesis();
    if (!nemesis) {
      return "";
    }
    msgDest = nemesis->getId();
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
      msgDest = recipient->getId();
    }
  }
  else if (args[0] == "admin") {
    msgDest = AdminPlayers;
  }
  else {
    return "usage: sendmsg {all|team|nemesis|recipient|admin} <message>";
  }

  if (msgDest == NoPlayer) {
    return "sendmsg: something is amiss";
  }

  // concatenate the args
  std::string msg = "";
  for (size_t i = 1; i < args.size(); i++) {
    if (i != 1) {
      msg += " ";
    }
    msg += args[i];
  }


  char msgBuf[MessageLen];
  memset(msgBuf, 0, MessageLen);
  strncpy(msgBuf, msg.c_str(), MessageLen);
  serverLink->sendMessage(msgDest, msgBuf);

  return std::string();
}


struct ScreenshotData {
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
    std::string file;
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

    const std::string& renderer = ssdata->renderer;
    unsigned char* pixels       = ssdata->pixels;
    const int xsize             = ssdata->xsize;
    const int ysize             = ssdata->ysize;
    const int channels          = ssdata->channels;

    // Gamma-correction is preapplied by BZFlag's gamma table
    // This ignores the PNG gAMA chunk, but so do many viewers (including Mozilla)
    if (BZDB.isSet("gamma")) {
      const float gamma = BZDB.eval("gamma");
      if (gamma != 1.0f) {
        unsigned char gammaTable[256];
        for (int i = 0; i < 256; i++) {
          const float lum    = float(i) / 256.0f;
          const float lumadj = pow(lum, 1.0f / gamma);
          gammaTable[i] = (unsigned char)(lumadj * 256);
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
    }
    else {
      snprintf(buf, sizeof(buf), "%s: failed to save", filename.c_str());
    }
    ControlPanel::addMutexMessage(buf);
  }

  delete ssdata->pixels;
  delete ssdata;

  return NULL;
}


static std::string cmdScreenshot(const std::string&, const CmdArgList& args, bool*) {
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
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, ssdata->pixels);

#if defined(HAVE_PTHREADS)
  pthread_t thread;
  pthread_create(&thread, NULL, writeScreenshot, (void*) ssdata);
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
  writeScreenshot(&ssdata);
#endif

  return std::string();
}


static std::string cmdShotStats(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: screenshot";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    HUDDialogStack::get()->push(new ShotStats);
  }
  return std::string();
}


static std::string cmdTime(const std::string&, const CmdArgList& args, bool*) {
  // FIXME - time should be moved into BZDB
  if (args.size() != 1) {
    return "usage: time {forward|backward|<seconds>}";
  }

  if (args[0] == "forward") {
    clockAdjust += 5.0f * 60.0f;
  }
  else if (args[0] == "backward") {
    clockAdjust -= 5.0f * 60.0f;
  }
  else {
    float seconds;
    char* end;
    seconds = (float)strtod(args[0].c_str(), &end);
    if (end != args[0].c_str()) {
      clockAdjust += seconds;
    }
    else {
      return "usage: time {forward|backward|<seconds>}";
    }
  }
  return std::string();
}


static std::string cmdRoam(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() == 0) {
    return "usage: roam {zoom|cycle} <args>";
  }
  if (!ROAM.isRoaming()) {
    return std::string();
  }
  if (args[0] == "zoom") {
    if (args.size() != 2) {
      return "usage: roam zoom {in|out|normal|stop}";
    }
    if (!roamButton || args[1] == "stop") {
      roamDZoom = 0.0f;
    }
    else if (args[1] == "out") {
      roamDZoom = +2.0f * BZDBCache::tankSpeed;
    }
    else if (args[1] == "in") {
      roamDZoom = -2.0f * BZDBCache::tankSpeed;
    }
    else if (args[1] == "normal") {
      ROAM.setZoom(60.0f);
    }
    else {
      return "usage: roam zoom {in|out|normal|stop}";
    }
  }
  else if (args[0] == "cycle") {
    if (args.size() != 3) {
      return "usage: roam cycle {type|subject} {forward|backward}";
    }
    if (args[1] == "type") {
      if (args[2] == "forward") {
        ROAM.setMode(Roaming::RoamingView((ROAM.getMode() + 1) % Roaming::roamViewCount));
      }
      else if (args[2] == "backward") {
        int setto = (ROAM.getMode() - 1) % Roaming::roamViewCount;
        if (setto < 0) { setto += Roaming::roamViewCount; }
        ROAM.setMode(Roaming::RoamingView(setto));
      }
      else {
        return "usage: roam cycle {type|subject} {forward|backward}";
      }
    }
    else if (args[1] == "subject") {
      if (args[2] == "forward") {
        ROAM.changeTarget(Roaming::next);
      }
      else if (args[2] == "backward") {
        ROAM.changeTarget(Roaming::previous);
      }
      else {
        return "usage: roam cycle {type|subject} {forward|backward}";
      }
    }
    else {
      return "usage: roam cycle {type|subject} {forward|backward}";
    }
  }
  else {
    return "usage: roam {zoom|cycle} <args>";
  }
  return std::string();
}


static std::string cmdSilence(const std::string&, const CmdArgList& args, bool*) {
  static SilenceDefaultKey silenceKeyHandler;
  if (args.size() != 0) {
    return "usage: silence";
  }
  messageHistoryIndex = 0;
  hud->setComposing("[Un]Silence: ");
  HUDui::setDefaultKey(&silenceKeyHandler);
  return std::string();
}


static std::string cmdServerCommand(const std::string&, const CmdArgList& args, bool*) {
  static ServerCommandKey serverCommandKeyHandler;
  if (args.size() != 0) {
    return "usage: servercommand";
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return "use only when connected";
  }
  static bool prevAdmin = myTank->isAdmin();
  if (!prevAdmin && myTank->isAdmin()) { serverCommandKeyHandler.adminInit(); }
  if (prevAdmin && !myTank->isAdmin()) { serverCommandKeyHandler.nonAdminInit(); }
  prevAdmin = myTank->isAdmin();

  messageHistoryIndex = 0;
  serverCommandKeyHandler.init();
  HUDui::setDefaultKey(&serverCommandKeyHandler);
  return std::string();
}


static std::string cmdHubCompose(const std::string&, const CmdArgList& args, bool*) {
  static HubComposeKey hubComposeKey;

  const bool keepAlive = (args.size() == 1) && (args[0] == "keepalive");

  if ((args.size() != 0) && !keepAlive) {
    return "usage: hubcompose [keepalive]";
  }

  hubComposeKey.init(keepAlive);

  return std::string();
}


static std::string cmdScrollPanel(const std::string&, const CmdArgList& args, bool*) {
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
  }
  else if (args[0] == "down") {
    controlPanel->setMessagesOffset(-linecount, 1 /* current */, false);
  }
  else if (args[0] == "up_page") {
    controlPanel->setMessagesOffset(+count, 1 /* current */, true);
  }
  else if (args[0] == "down_page") {
    controlPanel->setMessagesOffset(-count, 1 /* current */, true);
  }
  else if (args[0] == "top") {
    controlPanel->setMessagesOffset(123456789, 0 /* set */, false);
  }
  else if (args[0] == "bottom") {
    controlPanel->setMessagesOffset(0, 0 /* set */, false);
  }
  return std::string();
}


static std::string cmdHunt(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: hunt";
  }
  hud->getScoreboard()->huntKeyEvent(false);
  return std::string();
}


static std::string cmdAddHunt(const std::string&, const CmdArgList& args, bool*) {
  if (args.size() != 0) {
    return "usage: addhunt";
  }
  hud->getScoreboard()->huntKeyEvent(true);
  return std::string();
}


static std::string concatArgs(const CmdArgList& args) {
  std::string line;
  for (size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      line = args[i];
    }
    else {
      line += " " + args[i];
    }
  }
  return line;
}


static std::string cmdLuaUser(const std::string& cmd, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: luauser { reload | disable | status }";
  }
  LuaClientScripts::LuaUserCommand(cmd + " " + concatArgs(args));
  return std::string();
}


static std::string cmdLuaBzOrg(const std::string& cmd, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: luabzorg { reload | disable | status }";
  }
  LuaClientScripts::LuaBzOrgCommand(cmd + " " + concatArgs(args));
  return std::string();
}


static std::string cmdLuaWorld(const std::string& cmd, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: luaworld { reload | disable | status }";
  }
  LuaClientScripts::LuaWorldCommand(cmd + " " + concatArgs(args));
  return std::string();
}


static std::string cmdLuaRules(const std::string& cmd, const CmdArgList& args, bool*) {
  if (args.size() < 1) {
    return "usage: luarules { reload | disable | status }";
  }
  LuaClientScripts::LuaRulesCommand(cmd + " " + concatArgs(args));
  return std::string();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
