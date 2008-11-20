// lua.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

#include "mylua.h"
#include "lualfs.h"

#include "callins.h"
#include "callouts.h"
#include "constants.h"
#include "mapobject.h"
#include "bzdb.h"


/******************************************************************************/
/******************************************************************************/

static lua_State* L = NULL;

static const char* scriptFile = "bzfs.lua";


/******************************************************************************/
/******************************************************************************/

BZ_GET_PLUGIN_VERSION


/******************************************************************************/

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  bz_debugMessage(4, "lua plugin loaded");

  if (L != NULL) {
    printf("lua plugin is already loaded\n");
    return 1; // FAILURE
  }

  printf("lua plugin loaded. (cmdline = '%s')\n", commandLine);
  fflush(stdout);

  L = luaL_newstate();
  luaL_openlibs(L);
  luaopen_lfs (L);

  lua_newtable(L);
  {
    CallIns::PushEntries(L);
    CallOuts::PushEntries(L);
    Constants::PushEntries(L);
    MapObject::PushEntries(L);

    lua_pushliteral(L, "DB");
    lua_newtable(L);
    BZDB::PushEntries(L);
    lua_rawset(L, -3);
  }
  lua_setglobal(L, "BZ");

  if (strlen(commandLine) > 0) {
    scriptFile = commandLine;
  }

  if (luaL_dofile(L, scriptFile) != 0) {
    printf("lua init error: %s\n", lua_tostring(L, -1));
    fflush(stdout);
    lua_pop(L, 1);
    return 2; // FAILURE
  }
  
  return 0;
}


/******************************************************************************/

BZF_PLUGIN_CALL int bz_Unload()
{
  bz_debugMessage(4,"lua plugin unloaded");

  CallIns::Shutdown(L);
  MapObject::Shutdown(L);

  lua_close(L);
  L = NULL;

  return 0;
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
