
#include "common.h"

// interface header
#include "BZDB.h"

// system headers
#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;

// common headers
#include "bzfsAPI.h"

// local headers
#include "LuaHeader.h"


/******************************************************************************/

static set<string> objNames;


static int GetMap(lua_State* L);
static int GetList(lua_State* L);

static int Exists(lua_State* L);
static int IsPersistent(lua_State* L);
static int GetDefault(lua_State* L);

static int GetInt(lua_State* L);
static int GetBool(lua_State* L);
static int GetFloat(lua_State* L);
static int GetString(lua_State* L);

static int SetInt(lua_State* L);
static int SetBool(lua_State* L);
static int SetFloat(lua_State* L);
static int SetString(lua_State* L);


/******************************************************************************/

bool LuaBZDB::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetMap);
  PUSH_LUA_CFUNC(L, GetList);

  PUSH_LUA_CFUNC(L, Exists);
  PUSH_LUA_CFUNC(L, IsPersistent);
  PUSH_LUA_CFUNC(L, GetDefault);

  PUSH_LUA_CFUNC(L, GetInt);
  PUSH_LUA_CFUNC(L, GetBool);
  PUSH_LUA_CFUNC(L, GetFloat);
  PUSH_LUA_CFUNC(L, GetString);

  PUSH_LUA_CFUNC(L, SetInt);
  PUSH_LUA_CFUNC(L, SetBool);
  PUSH_LUA_CFUNC(L, SetFloat);
  PUSH_LUA_CFUNC(L, SetString);

  return true;
}


/******************************************************************************/

static int GetMap(lua_State* L)
{
  bz_APIStringList* list = bz_newStringList();
  lua_newtable(L);
  if (!bz_getBZDBVarList(list)) {
    bz_deleteStringList(list);    
    return 1;
  }
  for (unsigned int i = 0; i < list->size(); i++) {
    const char* key = list->get(i).c_str();
    lua_pushstring(L, key);
    lua_pushstring(L, bz_getBZDBString(key).c_str());
    lua_rawset(L, -3);
  }
  bz_deleteStringList(list);    
  return 1;
}


static int GetList(lua_State* L)
{
  bz_APIStringList* list = bz_newStringList();
  lua_newtable(L);
  if (!bz_getBZDBVarList(list)) {
    bz_deleteStringList(list);    
    return 1;
  }
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, list->get(i).c_str());
    lua_rawset(L, -3);
  }
  bz_deleteStringList(list);    
  return 1;
}


/******************************************************************************/

static int Exists(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_BZDBItemExists(key));
  return 1;
}


static int IsPersistent(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_getBZDBItemPersistent(key));
  return 1;
}


static int GetDefault(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  bz_ApiString s = bz_getBZDBDefault(key);
  lua_pushstring(L, s.c_str());
  return 1;
}


/******************************************************************************/

static int GetInt(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushinteger(L, bz_getBZDBInt(key));
  return 1;
}


static int GetBool(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_getBZDBBool(key));
  return 1;
}


static int GetFloat(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushdouble(L, bz_getBZDBDouble(key));
  return 1;
}


static int GetString(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  bz_ApiString s = bz_getBZDBString(key);
  lua_pushstring(L, s.c_str());
  return 1;
}


/******************************************************************************/

static int SetInt(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  const int value = luaL_checkint(L, 2);
  if (lua_gettop(L) == 2) {
    lua_pushboolean(L, bz_updateBZDBInt(key, value));
    return 1;
  }

  const int  perms = luaL_optint(L, 3, bz_getBZDBItemPerms(key));
  const bool persist = lua_isboolean(L, 4) ? lua_tobool(L, 4)
                                           : bz_getBZDBItemPersistent(key);

  lua_pushboolean(L, bz_setBZDBInt(key, value, perms, persist));
  return 1;
}


static int SetBool(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  const bool value = lua_tobool(L, 2);

  const int  perms = luaL_optint(L, 3, bz_getBZDBItemPerms(key));
  const bool persist = lua_isboolean(L, 4) ? lua_tobool(L, 4)
                                           : bz_getBZDBItemPersistent(key);

  lua_pushboolean(L, bz_setBZDBBool(key, value, perms, persist));
  return 1;
}


static int SetFloat(lua_State* L)
{
  const char* key    = luaL_checkstring(L, 1);
  const float value = luaL_checkfloat(L, 2);

  const int  perms = luaL_optint(L, 3, bz_getBZDBItemPerms(key));
  const bool persist = lua_isboolean(L, 4) ? lua_tobool(L, 4)
                                           : bz_getBZDBItemPersistent(key);

  lua_pushboolean(L, bz_setBZDBDouble(key, value, perms, persist));
  return 1;
}


static int SetString(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  const char* value = luaL_checkstring(L, 2);

  const int  perms = luaL_optint(L, 3, bz_getBZDBItemPerms(key));
  const bool persist = lua_isboolean(L, 4) ? lua_tobool(L, 4)
                                           : bz_getBZDBItemPersistent(key);

  lua_pushboolean(L, bz_setBZDBString(key, value, perms, persist));
  return 1;
}


/******************************************************************************/

