// lua.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_files.h"

#include <ctype.h>
#include <string>
using std::string;

#include "mylua.h"
#include "lualfs.h"

#include "callins.h"
#include "callouts.h"
#include "constants.h"
#include "mapobject.h"
#include "slashcmd.h"
#include "bzdb.h"


BZ_GET_PLUGIN_VERSION


/******************************************************************************/
/******************************************************************************/

static lua_State* L = NULL;

static const char* scriptFile = "bzfs.lua";

static bool CreateLuaState();
static bool DestroyLuaState();
static string EnvExpand(const string& path);


/******************************************************************************/

static string directory = "./";

static bool SetupLuaDirectory(const string& fileName);

extern const string& GetLuaDirectory() { return directory; } // extern


/******************************************************************************/

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  bz_debugMessage(4, "lua plugin loaded");

  if (L != NULL) {
    bz_debugMessagef(0, "lua plugin is already loaded");
    return 1; // FAILURE
  }

  string script = scriptFile;
  if (commandLine && (commandLine[0] != 0)) {
    script = commandLine;
  }
  script = EnvExpand(script);

  if (!fileExists(script)) {
    script = string(bz_pluginBinPath()) + "/" + script;
  }
  if (!fileExists(script)) {
    bz_debugMessagef(0, "lua plugin: could not find the script file");
    return 2; // FAILURE
  }

  SetupLuaDirectory(script);

  CreateLuaState();

  if (luaL_dofile(L, script.c_str()) != 0) {
    bz_debugMessagef(0, "lua init error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 3; // FAILURE
  }
  
  return 0;
}


/******************************************************************************/

BZF_PLUGIN_CALL int bz_Unload()
{
  bz_debugMessage(4, "lua plugin unloaded");

  DestroyLuaState();

  return 0;
}


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

static bool CreateLuaState()
{
  L = luaL_newstate();
  luaL_openlibs(L);
  luaopen_lfs (L);

  lua_newtable(L);
  {
    CallIns::PushEntries(L);
    CallOuts::PushEntries(L);
    Constants::PushEntries(L);
    BZDB::PushEntries(L);
    MapObject::PushEntries(L);
    SlashCmd::PushEntries(L);
  }
  lua_setglobal(L, "BZ");

  return true;
}


/******************************************************************************/

static bool DestroyLuaState()
{
  CallIns::Shutdown(L);
  MapObject::Shutdown(L);
  SlashCmd::Shutdown(L);

  lua_close(L);
  L = NULL;

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
