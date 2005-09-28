/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include <math.h>

// common implementation headers
#include "BZDBCache.h"
#include "AnsiCodes.h"
#include "TextUtils.h"
#include "CommandsStandard.h"
#include "StateDatabase.h"
#include "TextureManager.h"

// local implementation headers
#include "LocalCommand.h"
#include "World.h"
#include "Player.h"
#include "Roster.h"
#include "playing.h"

// externs for RoamPos command
extern float roamPos[3], roamTheta, roamPhi, roamZoom;
extern float roamDPos[3], roamDTheta, roamDPhi, roamDZoom;


// class definitions

class CommandList : LocalCommand {
  public:
    CommandList();
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

class ClientQueryCommand : LocalCommand {
  public:
    ClientQueryCommand();
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

class SaveWorldCommand : LocalCommand {
  public:
    SaveWorldCommand();
    bool operator() (const char *commandLine);
};


// class instantiations
static CommandList	  commandList;
static SilenceCommand     silenceCommand;
static UnsilenceCommand   unsilenceCommand;
static DumpCommand	  dumpCommand;
static ClientQueryCommand clientQueryCommand;
static HighlightCommand   highlightCommand;
static SetCommand	  setCommand;
static DiffCommand	  diffCommand;
static LocalSetCommand    localSetCommand;
static QuitCommand	  quitCommand;
static RoamPosCommand     roamPosCommand;
static ReTextureCommand   reTextureCommand;
static SaveWorldCommand   saveWorldCommand;


// class constructors
CommandList::CommandList() :		LocalCommand("/cmds") {}
DiffCommand::DiffCommand() :		LocalCommand("/diff") {}
DumpCommand::DumpCommand() :		LocalCommand("/dumpvars") {}
HighlightCommand::HighlightCommand() :	LocalCommand("/highlight") {}
LocalSetCommand::LocalSetCommand() :	LocalCommand("/localset") {}
QuitCommand::QuitCommand() :		LocalCommand("/quit") {}
ReTextureCommand::ReTextureCommand() :	LocalCommand("/retexture") {}
RoamPosCommand::RoamPosCommand() :	LocalCommand("/roampos") {}
SaveWorldCommand::SaveWorldCommand() :	LocalCommand("/saveworld") {}
SetCommand::SetCommand() :		LocalCommand("/set") {}
SilenceCommand::SilenceCommand() :	LocalCommand("/silence") {}
UnsilenceCommand::UnsilenceCommand() :	LocalCommand("/unsilence") {}
ClientQueryCommand::ClientQueryCommand() : LocalCommand("CLIENTQUERY") {}


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
      maxCmdLen = cmds[i]->size();
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
  const int prefixLen = strlen(prefix);

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

  addMessage(NULL, ANSI_STR_UNDERLINE ANSI_STR_FG_BLACK
		   "Server-side Commands");

  const char* msg = "/?";
  const int msgLen = strlen(msg) + 1;
  cptr = (char*) nboPackUByte(buffer, ServerPlayer);
  nboPackString(cptr, msg, msgLen);
  serverLink->send(MsgMessage, PlayerIdPLen + msgLen, buffer);

  return true;
}


bool SilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 9);
  if (loudmouth) {
    silencePlayers.push_back(commandLine + 8);
    std::string silenceMessage = "Silenced ";
    silenceMessage += (commandLine + 8);
    addMessage(NULL, silenceMessage);
  }
  return true;
}


bool UnsilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 11);
  if (loudmouth) {
    std::vector<std::string>::iterator it = silencePlayers.begin();
    for (; it != silencePlayers.end(); it++) {
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


bool ClientQueryCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 11)
    return false;
  char messageBuffer[MessageLen];
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, "/clientquery", MessageLen);
  nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);

  return true;
}


bool HighlightCommand::operator() (const char *commandLine)
{
  const char* c = commandLine + 10;
  while ((*c != '\0') && isspace(*c)) c++; // skip leading white
  BZDB.set("highlightPattern", std::string(c));
  return true;
}


static bool foundVarDiff = false;

static float parseFloatExpr(const std::string& str, bool zeroNan)
{
  // BZDB.eval() can't take expressions directly
  BZDB.set("tmp", str);
  BZDB.setPersistent("tmp", false);
  float value = BZDB.eval("tmp");
  if (!zeroNan) {
    return value;
  } else {
    if (value == value) {
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
  const bool valNaN = !(val == val);
  const bool defNaN = !(defval == defval);

  if (valNaN != defNaN) {
    return false;
  }

  if (valNaN) {
    return (exp == defexp);
  } else {
    return (val == defval);
  }
}

static void listSetVars(const std::string& name, void* boolPtr)
{
  bool& diff = *((bool*)boolPtr);

  if (diff) {
    if (varIsEqual(name)) {
      return;
    } else {
      foundVarDiff = true;
    }
  }

  char message[MessageLen];
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    if (BZDBCache::colorful) {
      sprintf(message, "/set %s%s %s%f %s%s",
	      ColorStrings[RedColor].c_str(), name.c_str(),
	      ColorStrings[GreenColor].c_str(), BZDB.eval(name),
	      ColorStrings[BlueColor].c_str(), BZDB.get(name).c_str());
    } else {
      sprintf(message, "/set %s <%f> %s", name.c_str(),
	      BZDB.eval(name), BZDB.get(name).c_str());
    }
    addMessage(LocalPlayer::getMyTank(), message, 2);
  }
}


bool SetCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 4) {
    return false;
  }
  bool diff = false;
  BZDB.iterate(listSetVars, &diff);
  return true;
}


bool DiffCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 5) {
    return false;
  }
  bool diff = true;
  foundVarDiff = false;
  BZDB.iterate(listSetVars, &diff);
  if (!foundVarDiff) {
    addMessage(LocalPlayer::getMyTank(), "all variables are at defaults", 2);
  }
  return true;
}


bool LocalSetCommand::operator() (const char *commandLine)
{
  std::string params = commandLine + 9;
  std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 2);
  if (tokens.size() == 2) {
    if (!(BZDB.getPermission(tokens[0]) == StateDatabase::Server)) {
      BZDB.setPersistent(tokens[0], BZDB.isPersistent(tokens[0]));
      BZDB.set(tokens[0], tokens[1]);
      std::string msg = "/localset " + tokens[0] + " " + tokens[1];
      addMessage(NULL, msg);
    } else {
      addMessage (NULL, "This is a server-defined variable. "
			"Use /set instead of /localset.");
    }
  } else {
    addMessage(NULL, "usage: /localset <variable> <value>");
  }
  return true;
}


bool QuitCommand::operator() (const char *commandLine)
{
  char messageBuffer[MessageLen]; // send message
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, commandLine, MessageLen);
  nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
  CommandsStandard::quit(); // kill client
  return true;
}


bool RoamPosCommand::operator() (const char *commandLine)
{
  // change the observer position and orientation
  std::string params = commandLine + 8;
  std::vector<std::string> tokens = TextUtils::tokenize(params, " ");

  if (tokens.size() == 1) {
    if (TextUtils::tolower(tokens[0]) == "reset") {
      roamPos[0] = 0.0f;
      roamPos[1] = 0.0f;
      roamPos[2] = BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
      roamTheta = 0.0f;
      roamZoom = 60.0f;
      roamPhi = -0.0f;
      roamDPos[0] = 0.0f;
      roamDPos[1] = 0.0f;
      roamDPos[2] = 0.0f;
      roamDTheta = 0.0f;
      roamDZoom = 0.0f;
      roamDPhi = 0.0f;
    } else {
      const float degrees = parseFloatExpr(tokens[0], true);
      const float ws = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
      const float radians = degrees * ((float)M_PI/180.0f);
      roamPos[0] = cosf(radians)* 0.5f * ws * (float)M_SQRT2;
      roamPos[1] = sinf(radians)* 0.5f * ws * (float)M_SQRT2;
      roamPos[2] = +0.25f * ws;
      roamTheta = degrees + 180.0f;
      roamPhi = -30.0f;
      roamZoom = 60.0f;
    }
  }
  else if (tokens.size() >= 3) {
    roamPos[0] = parseFloatExpr(tokens[0], true);
    roamPos[1] = parseFloatExpr(tokens[1], true);
    roamPos[2] = parseFloatExpr(tokens[2], true);
    if (tokens.size() >= 4) {
      roamTheta = parseFloatExpr(tokens[3], true);
    }
    if (tokens.size() >= 5) {
      roamPhi = parseFloatExpr(tokens[4], true);
    }
    if (tokens.size() == 6) {
      roamZoom = parseFloatExpr(tokens[5], true);
    }
  }
  else {
    addMessage(NULL,
      "/roampos  [ \"reset\" | degrees | {x y z [theta [phi [ zoom ]]] } ]");

    char buffer[MessageLen];
    snprintf(buffer, MessageLen,
	     "  <%.3f, %.3f, %.3f> theta = %.3f, phi = %.3f, zoom = %.3f",
	     roamPos[0], roamPos[1], roamPos[2],
	     roamTheta, roamPhi, roamZoom);
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


static void sendSaveWorldHelp()
{
  addMessage(NULL, "/saveworld [-g] [-m] <filename>");
  addMessage(NULL, "  -g : save ungrouped");
  addMessage(NULL, "  -m : save some primitives as meshes");
  return;
}

bool SaveWorldCommand::operator() (const char *commandLine)
{
  bool meshprims = false;
  bool ungrouped = false;

  std::string cmdLine = commandLine;
  std::vector<std::string> args;
  args = TextUtils::tokenize(commandLine, " ");
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

  World* world = World::getWorld();
  if (!world) {
    return true;
  }

  char buffer[256];
  std::string fullname;
  if (World::getWorld()->writeWorld(filename, fullname)) {
    snprintf(buffer, 256, "World saved:  %s %s%s", fullname.c_str(),
	     meshprims ? " [meshprims]" : "",
	     ungrouped ? " [ungrouped]" : "");
  } else {
    snprintf(buffer, 256, "Error saving:  %s", fullname.c_str());
  }
  addMessage(NULL, buffer);

  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
