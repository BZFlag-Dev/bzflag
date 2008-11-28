
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "bzdb.h"

#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;


// FIXME -- permissions, persistence, etc...

/******************************************************************************/

static set<string> objNames;


static int GetMap(lua_State* L);
static int GetList(lua_State* L);

static int Exists(lua_State* L);
static int IsPersistent(lua_State* L);
static int GetDefault(lua_State* L); // FIXME -- add to bzfsAPI.h ?

static int GetInt(lua_State* L);
static int GetBool(lua_State* L);
static int GetFloat(lua_State* L);
static int GetString(lua_State* L);

static int SetInt(lua_State* L);
static int SetBool(lua_State* L);
static int SetFloat(lua_State* L);
static int SetString(lua_State* L);


/******************************************************************************/

bool BZDB::PushEntries(lua_State* L)
{
#define PUSH_LUA_CFUNC(x)  \
  lua_pushliteral(L, #x);  \
  lua_pushcfunction(L, x); \
  lua_rawset(L, -3)

  lua_pushliteral(L, "DB");
  lua_newtable(L);

  PUSH_LUA_CFUNC(GetMap);
  PUSH_LUA_CFUNC(GetList);

  PUSH_LUA_CFUNC(Exists);
  PUSH_LUA_CFUNC(IsPersistent);
  PUSH_LUA_CFUNC(GetDefault);

  PUSH_LUA_CFUNC(GetInt);
  PUSH_LUA_CFUNC(GetBool);
  PUSH_LUA_CFUNC(GetFloat);
  PUSH_LUA_CFUNC(GetString);

  PUSH_LUA_CFUNC(SetInt);
  PUSH_LUA_CFUNC(SetBool);
  PUSH_LUA_CFUNC(SetFloat);
  PUSH_LUA_CFUNC(SetString);

  lua_rawset(L, -3);

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
  for (int i = 0; i < list->size(); i++) {
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
  for (int i = 0; i < list->size(); i++) {
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
  lua_pushnumber(L, bz_getBZDBDouble(key));
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
  const int perms = luaL_optint(L, 3, 0);
  const bool persist = lua_isboolean(L, 4) && lua_toboolean(L, 4);
  lua_pushboolean(L, bz_setBZDBInt(key, value));
  return 1;
}


static int SetBool(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (!lua_isboolean(L, 2)) {
    luaL_error(L, "expected boolean argument for arg 2");
  }
  const bool value = lua_toboolean(L, 2);
  const int  perms = luaL_optint(L, 3, 0);
  const bool persist = lua_isboolean(L, 4) && lua_toboolean(L, 4);
  lua_pushboolean(L, bz_setBZDBBool(key, value));
  return 1;
}


static int SetFloat(lua_State* L)
{
  const char* key    = luaL_checkstring(L, 1);
  const float value = luaL_checkfloat(L, 2);
  const int   perms  = luaL_optint(L, 3, 0);
  const bool persist = lua_isboolean(L, 4) && lua_toboolean(L, 4);
  lua_pushboolean(L, bz_setBZDBDouble(key, value));
  return 1;
}


static int SetString(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  const char* value = luaL_checkstring(L, 2);
  const int perms = luaL_optint(L, 3, 0);
  const bool persist = lua_isboolean(L, 4) && lua_toboolean(L, 4);
  lua_pushboolean(L, bz_setBZDBString(key, value));
  return 1;
}


/******************************************************************************/

