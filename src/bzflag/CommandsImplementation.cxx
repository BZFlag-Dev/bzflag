/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
#include "Player.h"
#include "Roster.h"
#include "playing.h"

// externs for RoamPos command
extern float roamPos[3], roamTheta, roamPhi, roamZoom;
extern float roamDPos[3], roamDTheta, roamDPhi, roamDZoom;


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


class SilenceCommand : LocalCommand {
public:
  SilenceCommand();

  virtual bool operator() (const char *commandLine);
};

class UnsilenceCommand : LocalCommand {
public:
  UnsilenceCommand();

  virtual bool operator() (const char *commandLine);
};

class DumpCommand : LocalCommand {
public:
  DumpCommand();

  virtual bool operator() (const char *commandLine);
};

class ClientQueryCommand : LocalCommand {
public:
  ClientQueryCommand();

  virtual bool operator() (const char *commandLine);
};

class HighlightCommand : LocalCommand {
public:
  HighlightCommand();

  virtual bool operator() (const char *commandLine);
};

class SetCommand : LocalCommand {
public:
  SetCommand();

  virtual bool operator() (const char *commandLine);
};

class DiffCommand : LocalCommand {
public:
  DiffCommand();

  virtual bool operator() (const char *commandLine);
};

class LocalSetCommand : LocalCommand {
public:
  LocalSetCommand();

  virtual bool operator() (const char *commandLine);
};

class QuitCommand : LocalCommand {
public:
  QuitCommand();

  virtual bool operator() (const char *commandLine);
};

class RoamPosCommand : LocalCommand {
public:
  RoamPosCommand();

  virtual bool operator() (const char *commandLine);
};

class ReTextureCommand : LocalCommand {
public:
  ReTextureCommand();

  virtual bool operator() (const char *commandLine);
};

static SilenceCommand     silenceCommand;
static UnsilenceCommand   unsilenceCommand;
static DumpCommand        dumpCommand;
static ClientQueryCommand clientQueryCommand;
static HighlightCommand   highlightCommand;
static SetCommand         setCommand;
static DiffCommand        diffCommand;
static LocalSetCommand    localSetCommand;
static QuitCommand        quitCommand;
static RoamPosCommand     roamPosCommand;
static ReTextureCommand   reTextureCommand;

SilenceCommand::SilenceCommand() : LocalCommand("SILENCE")
{
}

bool SilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 8);
  if (loudmouth) {
    silencePlayers.push_back(commandLine + 8);
    std::string silenceMessage = "Silenced ";
    silenceMessage += (commandLine + 8);
    addMessage(NULL, silenceMessage);
  }
  return true;
}

UnsilenceCommand::UnsilenceCommand() : LocalCommand("UNSILENCE")
{
}

bool UnsilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 10);
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

void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB.get(name) << std::endl;
}

DumpCommand::DumpCommand() : LocalCommand("DUMP")
{
}

bool DumpCommand::operator() (const char *)
{
  BZDB.iterate(printout, NULL);
  return true;
}

ClientQueryCommand::ClientQueryCommand() : LocalCommand("CLIENTQUERY")
{
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

HighlightCommand::HighlightCommand() : LocalCommand("/highlight")
{
}

bool HighlightCommand::operator() (const char *commandLine)
{
  const char* c = commandLine + 10;
  while ((*c != '\0') && isspace(*c)) c++; // skip leading white
  BZDB.set("highlightPattern", std::string(c));
  return true;
}

static bool foundVarDiff = false;

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


SetCommand::SetCommand() : LocalCommand("/set")
{
}

bool SetCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 4)
    return false;
  bool diff = false;
  BZDB.iterate(listSetVars, &diff);
  return true;
}

DiffCommand::DiffCommand() : LocalCommand("/diff")
{
}

bool DiffCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 5)
    return false;
  bool diff = true;
  foundVarDiff = false;
  BZDB.iterate(listSetVars, &diff);
  if (!foundVarDiff) {
    addMessage(LocalPlayer::getMyTank(), "all variables are at defaults", 2);
  }
  return true;
}

LocalSetCommand::LocalSetCommand() : LocalCommand("/localset")
{
}

bool LocalSetCommand::operator() (const char *commandLine)
{
  std::string params              = commandLine + 9;
  std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 2);
  if (tokens.size() == 2) {
    if (!(BZDB.getPermission(tokens[0]) == StateDatabase::Server)) {
      BZDB.setPersistent(tokens[0], BZDB.isPersistent(tokens[0]));
      BZDB.set(tokens[0], tokens[1]);
      std::string msg = "/localset " + tokens[0] + " " + tokens[1];
      addMessage(NULL, msg);
    } else {
      addMessage
	(NULL,
	 "This is a server-defined variable.  Use /set instead of /localset.");
    }
  } else {
    addMessage(NULL, "usage: /localset <variable> <value>");
  }
  return true;
}

QuitCommand::QuitCommand() : LocalCommand("/quit")
{
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


RoamPosCommand::RoamPosCommand() : LocalCommand("/roampos")
{
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


ReTextureCommand::ReTextureCommand() : LocalCommand("/retexture")
{
}

bool ReTextureCommand::operator() (const char *)
{
  TextureManager& tm = TextureManager::instance();
  tm.reloadTextures();
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
