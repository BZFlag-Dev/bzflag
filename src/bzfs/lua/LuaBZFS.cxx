
#include "common.h"

// implementation header
#include "LuaBZFS.h"

// system headers
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "bzfsAPI.h"
#include "TextUtils.h"
#include "bzfio.h"

// bzfs headers
#include "../bzfs.h"
#include "../CmdLineOptions.h"

//local headers
#include "LuaHeader.h"
#include "BZDB.h"
#include "CallIns.h"
#include "CallOuts.h"
#include "Constants.h"
#include "FetchURL.h"
#include "MapObject.h"
#include "RawLink.h"
#include "SlashCmd.h"


/******************************************************************************/
/******************************************************************************/

static lua_State* L = NULL;

static bool CreateLuaState(const string& script);
static string EnvExpand(const string& path);


/******************************************************************************/
/******************************************************************************/

static string directory = "./";

static bool SetupLuaDirectory(const string& fileName);

extern const string& GetLuaDirectory() { return directory; } // extern


/******************************************************************************/
/******************************************************************************/

static bool fileExists(const string& file)
{
  FILE* f = fopen(file.c_str(), "r");
  if (f == NULL) {
    return false;
  }
  fclose(f);
  return true;
}


/******************************************************************************/
/******************************************************************************/

bool LuaBZFS::init(const string& cmdLine)
{
  if (cmdLine.empty()) {
    return false;
  }

  bool dieHard = false;

  logDebugMessage(1, "loading luaBZFS");

  if (L != NULL) {
    logDebugMessage(1, "luaBZFS is already loaded");
    return false;
  }

  string scriptFile = cmdLine.c_str();
  
  if (scriptFile.size() > 8) {
    if (scriptFile.substr(0, 8) == "dieHard,") {
      dieHard = true;
      scriptFile = scriptFile.substr(8);
    }
  }

  scriptFile = EnvExpand(scriptFile);

  if (!fileExists(scriptFile)) {
    scriptFile = string(bz_pluginBinPath()) + "/" + scriptFile;
  }
  if (!fileExists(scriptFile)) {
    logDebugMessage(1, "luaBZFS: could not find the script file");
    if (dieHard) {
      exit(2);
    }
    return false;
  }

  SetupLuaDirectory(scriptFile);

  if (!CreateLuaState(scriptFile)) {
    if (dieHard) {
      exit(3);
    }
    return false;
  }

  return true;
}


/******************************************************************************/
/******************************************************************************/

bool LuaBZFS::kill()
{
  if (L == NULL) {
    return false;
  }

  CallIns::Shutdown(); // send the call-in

  RawLink::CleanUp(L);
  FetchURL::CleanUp(L);
  SlashCmd::CleanUp(L);
  MapObject::CleanUp(L);
  CallIns::CleanUp(L);

  lua_close(L);
  L = NULL;

  return true;
}


/******************************************************************************/
/******************************************************************************/

bool LuaBZFS::isActive()
{
  return (L != NULL);
}


/******************************************************************************/
/******************************************************************************/

void LuaBZFS::recvCommand(const string& cmdLine, int playerIndex)
{
  vector<string> args = TextUtils::tokenize(cmdLine, " \t", 3);

  GameKeeper::Player* p = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (p == NULL) {
    return;
  }

  if (args[0] != "/luabzfs") {
    return; // something is amiss, bail
  }

  if (args.size() < 2) {
    sendMessage(ServerPlayer, playerIndex,
                "/luabzfs < status | disable | reload >");
    return;
  }

  if (args[1] == "status") {
    if (isActive()) {
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS is enabled");
    } else {
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS is disabled");
    }
    return;
  }

  // permission check -- FIXME add a luabzfs permission, or use the "PLUGINS" permission?
  if (!p->accessInfo.hasPerm(PlayerAccessInfo::superKill)) {
    sendMessage(ServerPlayer, playerIndex,
                "You do not have permission to control LuaBZFS");
    return;
  }

  if (args[1] == "disable") {
    if (isActive()) {
      kill();
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS has been disabled");
    } else {
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS is not loaded");
    }
    return;
  }

  if (args[1] == "reload") {
    kill();
    bool success = false;
    if (args.size() > 2) {
      success = init(args[2]);
    } else {
      success = init(clOptions->luaBZFS);
    }
    if (success) {
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS reload succeeded");
    } else {
      sendMessage(ServerPlayer, playerIndex, "LuaBZFS reload failed");
    }
    return;
  }
    
  return;
}


/******************************************************************************/
/******************************************************************************/

static bool SetupLuaDirectory(const string& fileName)
{
  const string::size_type pos = fileName.find_last_of("/\\");
  if (pos == string::npos) {
    directory = "./";
  } else {
    directory = fileName.substr(0, pos + 1);
  }
  return true;
}


/******************************************************************************/
/******************************************************************************/

static bool CreateLuaState(const string& script)
{
  if (L != NULL) {
    return false;
  }

  L = luaL_newstate();
  luaL_openlibs(L);

  lua_newtable(L);
  {
    CallIns::PushEntries(L);
    CallOuts::PushEntries(L);
    Constants::PushEntries(L);
    LuaBZDB::PushEntries(L);
    MapObject::PushEntries(L);
    SlashCmd::PushEntries(L);
    FetchURL::PushEntries(L);
    RawLink::PushEntries(L);
  }
  lua_setglobal(L, "bz");

  if (luaL_dofile(L, script.c_str()) != 0) {
    logDebugMessage(1, "lua init error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  return true;
}


/******************************************************************************/
/******************************************************************************/

static string EnvExpand(const string& path)
{
  string::size_type pos = path.find('$');
  if (pos == string::npos) {
    return path;
  }

  if (path[pos + 1] == '$') { // allow $$ escapes
    return path.substr(0, pos + 1) + EnvExpand(path.substr(pos + 2));
  }

  const char* b = path.c_str(); // beginning of string
  const char* s = b + pos + 1;  // start of the key name
  const char* e = s;            //  end  of the key Name
  while ((*e != 0) && (isalnum(*e) || (*e == '_'))) {
    e++;
  }

  const string head = path.substr(0, pos);
  const string key  = path.substr(pos + 1, e - s);
  const string tail = path.substr(e - b);

  const char* env = getenv(key.c_str());
  const string value = (env == NULL) ? "" : env;

  return head + value + EnvExpand(tail);
}


/******************************************************************************/
/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
