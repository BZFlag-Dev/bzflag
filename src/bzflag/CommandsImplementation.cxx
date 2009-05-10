/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// system implementation headers
#include <iostream>
#include <math.h>
#include <ctype.h>

#include <vector>

// common implementation headers
#include "AnsiCodes.h"
#include "BZDBCache.h"
#include "CommandsStandard.h"
#include "DirectoryNames.h"
#include "KeyManager.h"
#include "MapInfo.h"
#include "TextureManager.h"
#include "TextUtils.h"

// local implementation headers
#include "bzglob.h"
#include "playing.h"
#include "LocalCommand.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "Roster.h"
#include "ServerLink.h"

// class definitions

class CommandList : LocalCommand {
  public:
    CommandList();
    bool operator() (const char *commandLine);
};

class BindCommand : LocalCommand {
  public:
    BindCommand();
    bool operator() (const char *commandLine);
};

class SilenceCommand : LocalCommand {
  public:
    SilenceCommand();
    bool operator() (const char *commandLine);
};

class UnsilenceCommand : LocalCommand {
  public:
    UnsilenceCommand();
    bool operator() (const char *commandLine);
};

class DumpCommand : LocalCommand {
  public:
    DumpCommand();
    bool operator() (const char *commandLine);
};

class HighlightCommand : LocalCommand {
  public:
    HighlightCommand();
    bool operator() (const char *commandLine);
};

class SetCommand : LocalCommand {
  public:
    SetCommand();
    bool operator() (const char *commandLine);
};

class DiffCommand : LocalCommand {
  public:
    DiffCommand();
    bool operator() (const char *commandLine);
};

class LocalSetCommand : LocalCommand {
  public:
    LocalSetCommand();
    bool operator() (const char *commandLine);
};

class QuitCommand : LocalCommand {
  public:
    QuitCommand();
    bool operator() (const char *commandLine);
};

class RoamPosCommand : LocalCommand {
  public:
    RoamPosCommand();
    bool operator() (const char *commandLine);
};

class ReTextureCommand : LocalCommand {
  public:
    ReTextureCommand();
    bool operator() (const char *commandLine);
};

class SaveMsgsCommand : LocalCommand {
  public:
    SaveMsgsCommand();
    bool operator() (const char *commandLine);
};

class SaveWorldCommand : LocalCommand {
  public:
    SaveWorldCommand();
    bool operator() (const char *commandLine);
};

class SelfLockCommand : LocalCommand {
  public:
    SelfLockCommand();
    bool operator() (const char *commandLine);
};

class WorldInfoCommand : LocalCommand {
  public:
    WorldInfoCommand();
    bool operator() (const char *commandLine);
};

class DebugLevelCommand : LocalCommand {
  public:
    DebugLevelCommand();
    bool operator() (const char *commandLine);
};


// class instantiations
static BindCommand        bindCommand;
static CommandList        commandList;
static DebugLevelCommand  debugLevelCommand;
static DiffCommand        diffCommand;
static DumpCommand        dumpCommand;
static HighlightCommand   highlightCommand;
static LocalSetCommand    localSetCommand;
static QuitCommand        quitCommand;
static ReTextureCommand   reTextureCommand;
static RoamPosCommand     RoamPosCommand;
static SaveMsgsCommand    saveMsgsCommand;
static SaveWorldCommand   saveWorldCommand;
static SelfLockCommand    selfLockCommand;
static SetCommand         setCommand;
static SilenceCommand     silenceCommand;
static UnsilenceCommand   unsilenceCommand;
static WorldInfoCommand   worldInfoCommand;


// class constructors
BindCommand::BindCommand()             : LocalCommand("/bind")      {}
CommandList::CommandList()             : LocalCommand("/cmds")      {}
DebugLevelCommand::DebugLevelCommand() : LocalCommand("/debug")     {}
DiffCommand::DiffCommand()             : LocalCommand("/diff")      {}
DumpCommand::DumpCommand()             : LocalCommand("/dumpvars")  {}
HighlightCommand::HighlightCommand()   : LocalCommand("/highlight") {}
LocalSetCommand::LocalSetCommand()     : LocalCommand("/localset")  {}
QuitCommand::QuitCommand()             : LocalCommand("/quit")      {}
ReTextureCommand::ReTextureCommand()   : LocalCommand("/retexture") {}
RoamPosCommand::RoamPosCommand()       : LocalCommand("/roampos")   {}
SaveMsgsCommand::SaveMsgsCommand()     : LocalCommand("/savemsgs")  {}
SaveWorldCommand::SaveWorldCommand()   : LocalCommand("/saveworld") {}
SelfLockCommand::SelfLockCommand()     : LocalCommand("/selflock")  {}
SetCommand::SetCommand()               : LocalCommand("/set")       {}
SilenceCommand::SilenceCommand()       : LocalCommand("/silence")   {}
UnsilenceCommand::UnsilenceCommand()   : LocalCommand("/unsilence") {}
WorldInfoCommand::WorldInfoCommand()   : LocalCommand("/worldinfo") {}


// the meat of the matter

bool CommandList::operator() (const char * /*cmdLine*/)
{
  int i;
  const int maxLineLen = 64;

  // build a std::vector<> from the std::map<> of command names
  std::vector<const std::string*> cmds;
  MapOfCommands::iterator it;
  MapOfCommands& commandMap = *mapOfCommands;
  for (it = commandMap.begin(); it != commandMap.end(); it++) {
    const std::string& cmd = it->first;
    cmds.push_back(&cmd);
  }
  const int cmdCount = (int)cmds.size();

  // get the maximum length
  unsigned int maxCmdLen = 0;
  for (i = 0; i < cmdCount; i++) {
    if (cmds[i]->size() > maxCmdLen) {
      maxCmdLen = (unsigned int)(cmds[i]->size());
    }
  }
  maxCmdLen += 2; // add some padding

  // message generation variables
  char buffer[MessageLen];
  char* cptr = buffer;
  char format[8];
  snprintf(format, 8, "%%-%is", maxCmdLen);

  // formatting parameters
  const int cols = (maxLineLen / maxCmdLen);
  const int rows = ((cmdCount + (cols - 1)) / cols);

  addMessage(NULL, ANSI_STR_UNDERLINE ANSI_STR_FG_BLACK
		   "Client-side Commands");

  const char* prefix = ANSI_STR_FG_YELLOW ANSI_STR_PULSATING
		       "[CLIENT->] " ANSI_STR_RESET;
  const int prefixLen = (const int)strlen(prefix);

  for (int row = 0; row < rows; row++) {
    cptr = buffer;
    strncpy(buffer, prefix, MessageLen);
    cptr += prefixLen;
    for (int col = 0; col < cols; col++) {
      const int index = (col * rows) + row;
      if (index >= cmdCount) {
	break;
      }
      sprintf(cptr, format, cmds[index]->c_str());
      cptr += maxCmdLen;
    }
    addMessage(NULL, buffer);
  }

  if (serverLink != NULL) {
    addMessage(NULL, ANSI_STR_UNDERLINE ANSI_STR_FG_BLACK
               "Server-side Commands");
    const char* msg = "/?";
    strncpy(buffer, msg, MessageLen);
    serverLink->sendMessage(ServerPlayer, buffer);
  }
  return true;
}


static void printBindHelp()
{
  addMessage(NULL, "usage: /bind <event> [<press> [<action>]]");
  addMessage(NULL, "   event:   ex: \"Shift+Page Up\"");
  addMessage(NULL, "   press:   up, down, or both");
  addMessage(NULL, "   action:  ex: \"scrollpanel up 3\"");
}

bool BindCommand::operator() (const char *commandLine)
{
  const std::string params = commandLine + commandName.size();
  const std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 3, true);
  if ((tokens.size() < 1) || (tokens.size() > 3)) {
    printBindHelp();
    return true;
  }

  BzfKeyEvent ev;
  if (!KEYMGR.stringToKeyEvent(tokens[0], ev)) {
    printBindHelp();
    addMessage(NULL, "could not find the key or create a printable binding");
    return true;
  }

  if (tokens.size() >= 2) {
    bool press;
    bool both = false;
    if (tokens[1] == "up") {
      press = false;
    } else if (tokens[1] == "down") {
      press = true;
    } else if (tokens[1] == "both") {
      both = true;
    } else {
      printBindHelp();
      addMessage(NULL, "bad <press> type");
      return true;
    }

    if (both) {
      KEYMGR.unbind(ev, true);
      KEYMGR.unbind(ev, false);
      if (tokens.size() == 3) {
	KEYMGR.bind(ev, true, tokens[2]);
	KEYMGR.bind(ev, false, tokens[2]);
      }
    } else {
      KEYMGR.unbind(ev, press);
      if (tokens.size() == 3) {
	KEYMGR.bind(ev, press, tokens[2]);
      }
    }
  }
  else {
    // print the current binding
    std::string msg;
    msg = "binding:  \"";
    msg += tokens[0];
    msg += "\" down ";
    msg += KEYMGR.get(ev, true);
    addMessage(NULL, msg.c_str());
    msg = "binding:  \"";
    msg += tokens[0];
    msg += "\" up   ";
    msg += KEYMGR.get(ev, false);
    addMessage(NULL, msg.c_str());
  }

  return true;
}


bool SilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 9);
  if (loudmouth) {
    silencePlayers.push_back(commandLine + 9);
    std::string silenceMessage = "Silenced";
    silenceMessage += (commandLine + 8);
    addMessage(NULL, silenceMessage);
  }
  else {
    std::string silenceMessage = "no such callsign:";
    silenceMessage += (commandLine + 8);
    addMessage(NULL, silenceMessage);
  }
  return true;
}


bool UnsilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 11);
  if (loudmouth) {
    std::vector<std::string>::iterator it;
    for (it = silencePlayers.begin(); it != silencePlayers.end(); ++it) {
      if (*it == commandLine + 10) {
	silencePlayers.erase(it);
	std::string unsilenceMessage = "Unsilenced ";
	unsilenceMessage += (commandLine + 10);
	addMessage(NULL, unsilenceMessage);
	break;
      }
    }
  }
  return true;
}


static void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB.get(name) << std::endl;
}

bool DumpCommand::operator() (const char *)
{
  BZDB.iterate(printout, NULL);
  addMessage(NULL, "Dumped all BZDB values to stdout");
  return true;
}


bool HighlightCommand::operator() (const char *commandLine)
{
  const char* c = commandLine + 10;
  c = TextUtils::skipWhitespace(c);
  BZDB.set("highlightPattern", std::string(c));
  return true;
}


static bool foundVar = false;

class VarDispInfo {
  public:
    VarDispInfo(const std::string& _prefix)
    {
      prefix = _prefix;
      pattern = "";
      diff = client = server = false;
    }
    std::string prefix;
    std::string pattern;
    bool diff;
    bool client;
    bool server;
};

static float parseFloatExpr(const std::string& str, bool zeroNan)
{
  // BZDB.eval() can't take expressions directly
  BZDB.set("tmp", str);
  BZDB.setPersistent("tmp", false);
  float value = BZDB.eval("tmp");
  if (!zeroNan || !isnan(value)) {
    return value;
  } else {
    if (!isnan(value)) {
      return value;
    } else {
      return 0.0f;
    }
  }
}

static bool varIsEqual(const std::string& name)
{
  // avoid "poll"
  if (name[0] != '_') {
    return true;
  }

  // get the parameters
  const std::string exp = BZDB.get(name);
  const std::string defexp = BZDB.getDefault(name);
  const float val = BZDB.eval(name);
  const float defval = parseFloatExpr(defexp, false);
  const bool valNaN = (isnan(val) != 0);
  const bool defNaN = (isnan(defval) != 0);

  if (valNaN != defNaN) {
    return false;
  }

  if (valNaN) {
    return (exp == defexp);
  } else {
    return (val == defval);
  }
}

static void listSetVars(const std::string& name, void* varDispPtr)
{
  const VarDispInfo* varDisp = (VarDispInfo*)varDispPtr;
  const bool diff = varDisp->diff;

  if (!glob_match(varDisp->pattern.c_str(), name)) {
    return;
  }

  if (diff && varIsEqual(name)) {
    return;
  }

  foundVar = true;

  const bool serverVar = (BZDB.getPermission(name) == StateDatabase::Locked);

  char message[MessageLen];
  if ((serverVar && varDisp->server) || (!serverVar && varDisp->client)) {
    if (BZDBCache::colorful) {
      snprintf(message, MessageLen, "%s %s%s %s%f %s%s", varDisp->prefix.c_str(),
	      ColorStrings[RedColor], name.c_str(),
	      ColorStrings[GreenColor], BZDB.eval(name),
	      ColorStrings[BlueColor], BZDB.get(name).c_str());
    } else {
      snprintf(message, MessageLen, "%s %s <%f> %s", name.c_str(), varDisp->prefix.c_str(),
	      BZDB.eval(name), BZDB.get(name).c_str());
    }
    addMessage(LocalPlayer::getMyTank(), message, ControlPanel::MessageServer);
  }
}


bool SetCommand::operator() (const char *commandLine)
{
  const std::string params = commandLine + 4;
  const std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 2);
  if (tokens.size() > 1) {
    return false;
  }

  std::string pattern = (tokens.size() == 1) ? tokens[0] : "_*";
  if (pattern[0] != '_') {
//FIXME ?    pattern = '_' + pattern;
  }

  const std::string header = "/set " + pattern;
  addMessage(LocalPlayer::getMyTank(), header, ControlPanel::MessageServer);

  VarDispInfo varDisp(commandName);
  varDisp.server = true;
  varDisp.pattern = pattern;

  foundVar = false;
  BZDB.iterate(listSetVars, &varDisp);
  if (!foundVar) {
    addMessage(LocalPlayer::getMyTank(), "no matching variables",
               ControlPanel::MessageServer);
  }
  return true;
}


bool DiffCommand::operator() (const char *commandLine)
{
  const std::string params = commandLine + 5;
  const std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 2);

  std::string pattern = (tokens.size() == 1) ? tokens[0] : "_*";
  if (pattern[0] != '_') {
    pattern = '_' + pattern;
  }

  const std::string header = "/diff " + pattern;
  addMessage(LocalPlayer::getMyTank(), header, ControlPanel::MessageServer);

  VarDispInfo varDisp(commandName);
  varDisp.diff = true;
  varDisp.server = true;
  varDisp.pattern = pattern;

  foundVar = false;
  BZDB.iterate(listSetVars, &varDisp);
  if (!foundVar) {
    if (pattern == "_*") {
      addMessage(LocalPlayer::getMyTank(),
	"all variables are at defaults", ControlPanel::MessageServer);
    } else {
      addMessage(LocalPlayer::getMyTank(),
	"no differing variables with that pattern",
	ControlPanel::MessageServer);
    }
  }
  return true;
}


bool LocalSetCommand::operator() (const char *commandLine)
{
  const std::string params = commandLine + 9;
  const std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 0, true);
#ifdef DEBUG
  const bool debug = true;
#else
  const bool debug = false;
#endif

  if (tokens.size() == 1) {
    const std::string header = "/localset " + tokens[0];
    addMessage(LocalPlayer::getMyTank(), header, ControlPanel::MessageServer);

    if (!debug &&
	((strstr(tokens[0].c_str(), "*") != NULL) ||
	 (strstr(tokens[0].c_str(), "?") != NULL))) {
      addMessage(LocalPlayer::getMyTank(), "undefined client variable",
                 ControlPanel::MessageServer);
      return true;
    }

    VarDispInfo varDisp(commandName);
    varDisp.client = true;
    varDisp.pattern = tokens[0];

    foundVar = false;
    BZDB.iterate(listSetVars, &varDisp);
    if (!foundVar) {
      if (debug) {
	addMessage(LocalPlayer::getMyTank(),
	  "no matching client variables", ControlPanel::MessageServer);
      } else {
	addMessage(LocalPlayer::getMyTank(),
	  "undefined client variable", ControlPanel::MessageServer);
      }
    }
  }
  else if (tokens.size() == 2) {
    if (!(BZDB.getPermission(tokens[0]) == StateDatabase::Server)) {
      BZDB.setPersistent(tokens[0], BZDB.isPersistent(tokens[0]));
      BZDB.set(tokens[0], tokens[1]);
      std::string msg = "/localset " + tokens[0] + " " + tokens[1];
      addMessage(NULL, msg);
    } else {
      addMessage (NULL, "This is a server-defined variable. "
			"Use /set instead of /localset.");
    }
  }
  else {
    if (debug) {
      addMessage(NULL, "usage: /localset <pattern> [value]");
    } else {
      addMessage(NULL, "usage: /localset <variable> [value]");
    }
  }
  return true;
}


bool QuitCommand::operator() (const char *commandLine)
{
  char messageBuffer[MessageLen]; // send message
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, commandLine, MessageLen);
  serverLink->sendMessage(msgDestination, messageBuffer);
  CommandsStandard::quit(); // kill client
  return true;
}


bool RoamPosCommand::operator() (const char *commandLine)
{
  // change the observer position and orientation
  const std::string params = commandLine + 8;
  const std::vector<std::string> tokens = TextUtils::tokenize(params, " ");

  if (tokens.size() == 1) {
    Roaming::RoamingCamera cam;
    if (TextUtils::tolower(tokens[0]) == "reset") {
      ROAM.resetCamera();
    } else if (TextUtils::tolower(tokens[0]) == "send") {
      LocalPlayer* myTank = LocalPlayer::getMyTank();
      if (myTank != NULL) {
	const Roaming::RoamingCamera* camPtr = ROAM.getCamera();
	fvec3 fakeVel(camPtr->theta, camPtr->phi, camPtr->zoom);
	myTank->move(camPtr->pos, camPtr->theta);
	myTank->setVelocity(fakeVel);
	serverLink->sendPlayerUpdate(myTank);
      }
    } else {
      const float degrees = parseFloatExpr(tokens[0], true);
      const float ws = BZDBCache::worldSize;
      const float radians = degrees * ((float)M_PI/180.0f);
      cam.pos[0] = cosf(radians)* 0.5f * ws * (float)M_SQRT2;
      cam.pos[1] = sinf(radians)* 0.5f * ws * (float)M_SQRT2;
      cam.pos[2] = +0.25f * ws;
      cam.theta = degrees + 180.0f;
      cam.phi = -30.0f;
      cam.zoom = 60.0f;
      ROAM.setCamera(&cam);
    }
  }
  else if (tokens.size() >= 3) {
    Roaming::RoamingCamera cam;
    cam.pos[0] = parseFloatExpr(tokens[0], true);
    cam.pos[1] = parseFloatExpr(tokens[1], true);
    cam.pos[2] = parseFloatExpr(tokens[2], true);
    if (tokens.size() >= 4) {
      cam.theta = parseFloatExpr(tokens[3], true);
    } else {
      cam.theta = ROAM.getCamera()->theta;
    }
    if (tokens.size() >= 5) {
      cam.phi = parseFloatExpr(tokens[4], true);
    } else {
      cam.phi = ROAM.getCamera()->phi;
    }
    if (tokens.size() == 6) {
      cam.zoom = parseFloatExpr(tokens[5], true);
    } else {
      cam.zoom = ROAM.getCamera()->zoom;
    }
    ROAM.setCamera(&cam);
  }
  else {
    addMessage(NULL,
      "/roampos  "
      "[ \"reset\" | degrees | {x y z [theta [phi [ zoom ]]] } | \"send\" ]");

    const Roaming::RoamingCamera* cam = ROAM.getCamera();
    char buffer[MessageLen];
    snprintf(buffer, MessageLen,
	     "  <%.3f, %.3f, %.3f> theta = %.3f, phi = %.3f, zoom = %.3f",
	     cam->pos[0], cam->pos[1], cam->pos[2],
	     cam->theta, cam->phi, cam->zoom);
    addMessage(NULL, buffer);
  }

  return true;
}


bool ReTextureCommand::operator() (const char *)
{
  TextureManager& tm = TextureManager::instance();
  tm.reloadTextures();
  return true;
}


bool SaveMsgsCommand::operator() (const char *commandLine)
{
  if (controlPanel == NULL) {
    return true;
  }

  const std::vector<std::string> args = TextUtils::tokenize(commandLine, " ");
  const int argCount = (int)args.size();

  bool stripAnsi = false;
  if ((argCount > 1) && (args[1] == "-s")) {
    stripAnsi = true;
  }

  const std::string filename = getConfigDirName() + "msglog.txt";

  controlPanel->saveMessages(filename, stripAnsi);

  const std::string msg = "Saved messages to: " + filename;
  addMessage(NULL, msg);

  return true;
}


static void sendSaveWorldHelp()
{
  addMessage(NULL, "/saveworld [-g] [-m] [-o] [-d] <filename>");
  addMessage(NULL, "  -g : save ungrouped");
  addMessage(NULL, "  -m : save some primitives as meshes");
  addMessage(NULL, "  -o : save meshes into WaveFront OBJ files");
  return;
}

bool SaveWorldCommand::operator() (const char *commandLine)
{
  bool meshprims  = false;
  bool ungrouped  = false;
  bool wavefront  = false;

  const std::string cmdLine = commandLine;
  const std::vector<std::string> args = TextUtils::tokenize(commandLine, " ");
  const int argCount = (int)args.size();

  if (argCount <= 1) {
    sendSaveWorldHelp();
    return true;
  }

  int pos = 1;
  while (pos < (argCount - 1)) {
    const std::string& arg = args[pos];
    if (arg == "-m") {
      meshprims = true;
    } else if (arg == "-g") {
      ungrouped = true;
    } else if (arg == "-o") {
      wavefront = true;
      meshprims = true;
      ungrouped = true;
    } else {
      break;
    }
    pos++;
  }
  if (pos != (argCount - 1)) {
    sendSaveWorldHelp();
    return true;
  }
  const std::string& filename = args[pos];

  BZDB.set("saveAsMeshes", meshprims ? "1" : "0");
  BZDB.set("saveFlatFile", ungrouped ? "1" : "0");
  BZDB.set("saveAsOBJ",    wavefront ? "1" : "0");

  World* world = World::getWorld();
  if (!world) {
    return true;
  }

  char buffer[256] = { 0 };
  std::string fullname;
  if (world->writeWorld(filename, fullname)) {
    snprintf(buffer, 256, "World saved:  %s %s%s%s", fullname.c_str(),
	     meshprims ? " [meshprims]" : "",
	     ungrouped ? " [ungrouped]" : "",
	     wavefront ? " [wavefront]" : "");
  } else {
    snprintf(buffer, 256, "Error saving:  %s", fullname.c_str());
  }
  addMessage(NULL, buffer);

  return true;
}

bool SelfLockCommand::operator() (const char*)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    addMessage(NULL, "you do not exist");
    return true;
  }
  myTank->setTarget(myTank);
  addMessage(NULL, "self-locked");
  return true;
}


bool WorldInfoCommand::operator() (const char* /*commandLine*/)
{
  World* world = World::getWorld();
  if (world == NULL) {
    addMessage(NULL, "no world");
    return true;
  }

  const std::string indent = ANSI_STR_FG_GREEN "[info]" ANSI_STR_RESET;
  const MapInfo::InfoVec& infoVec = world->getMapInfo().getVec();
  if (infoVec.empty()) {
    addMessage(NULL, "no world info");
  }
  else {
    for (unsigned int i = 0; i < infoVec.size(); i++) {
      addMessage(NULL, indent + infoVec[i]);
    }
    if (false) { // FIXME - test code
      addMessage(NULL, "and now ... by map<>");
      const MapInfo::InfoMap& infoMap = world->getMapInfo().getMap();
      MapInfo::InfoMap::const_iterator it;
      for (it = infoMap.begin(); it != infoMap.end(); ++it) {
        addMessage(NULL, indent + it->first);
        const std::vector<std::string>& values = it->second;
        for (unsigned int i = 0; i < values.size(); i++) {
          addMessage(NULL, indent + "  " + values[i]);
        }
      }
    }
  }

  return true;
}


bool DebugLevelCommand::operator() (const char* cmdLine)
{
  const std::vector<std::string> args = TextUtils::tokenize(cmdLine, " ");
  if (args.size() < 2) {
    std::string msg = "debug level is ";
    msg += TextUtils::itoa(debugLevel);
    addMessage(NULL, msg);
    return true;
  }

  BZDB.setInt("debugLevel", atoi(args[1].c_str()));

  char buf[128];
  snprintf(buf, sizeof(buf), "debug level set to %i", debugLevel);
  addMessage(NULL, buf);

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
