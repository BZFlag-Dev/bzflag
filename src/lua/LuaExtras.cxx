
#include "common.h"

// implementation header
#include "LuaExtras.h"

// local headers
#include "LuaInclude.h"


/******************************************************************************/
/******************************************************************************/

bool LuaExtras::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, tobool);
  PUSH_LUA_CFUNC(L, isnil);
  PUSH_LUA_CFUNC(L, isbool);
  PUSH_LUA_CFUNC(L, isnumber);
  PUSH_LUA_CFUNC(L, isstring);
  PUSH_LUA_CFUNC(L, istable);
  PUSH_LUA_CFUNC(L, isthread);
  PUSH_LUA_CFUNC(L, isfunction);
  PUSH_LUA_CFUNC(L, isuserdata);

  return true;
}


/******************************************************************************/

int LuaExtras::tobool(lua_State* L)
{
	switch (lua_type(L, 1)) {
		case LUA_TBOOLEAN: {
			lua_pushvalue(L, 1);
			break;
		}
		case LUA_TNUMBER: {
			lua_pushboolean(L, (lua_tofloat(L, 1) != 0.0f));
			break;
		}
		case LUA_TSTRING: {
			const char* c = lua_tostring(L, 1);
			const bool value = (((c[0] == '1') && (c[1] == 0)) ||
			                    (((c[0] == 't') || (c[0] == 'T')) &&
			                     ((c[1] == 'r') || (c[1] == 'R')) &&
			                     ((c[2] == 'u') || (c[2] == 'U')) &&
			                     ((c[3] == 'e') || (c[3] == 'E')) &&
			                      (c[4] == 0)));
			lua_pushboolean(L, value);
			break;
		}
		default: {
			lua_pushboolean(L, false);
			break;
		}
	}
	return 1;
}


/******************************************************************************/

int LuaExtras::isnil(lua_State* L)
{
	lua_pushboolean(L, lua_isnoneornil(L, 1));
	return 1;
}


int LuaExtras::isbool(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TBOOLEAN);
	return 1;
}


int LuaExtras::isnumber(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TNUMBER);
	return 1;
}


int LuaExtras::isstring(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TSTRING);
	return 1;
}


int LuaExtras::istable(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TTABLE);
	return 1;
}


int LuaExtras::isthread(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TTHREAD);
	return 1;
}


int LuaExtras::isfunction(lua_State* L)
{
	lua_pushboolean(L, lua_type(L, 1) == LUA_TFUNCTION);
	return 1;
}


int LuaExtras::isuserdata(lua_State* L)
{
	const int type = lua_type(L, 1);
	lua_pushboolean(L, (type == LUA_TUSERDATA) ||
	                   (type == LUA_TLIGHTUSERDATA));
	return 1;
}


/******************************************************************************/
/******************************************************************************/

extern "C" {
#include "../other/lua/src/lstate.h"
#include "../other/lua/src/llimits.h"
#define luac_c // for luaU_print()
#include "../other/lua/src/lundump.h"
}


#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))


static int Writer(lua_State *L, const void* b, size_t size, void* B) {
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char *)b, size);
  return 0;
}


static int Dump(lua_State *L, lua_Writer writer, void *data, bool strip)
{
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o)) {
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, strip ? 1 : 0);
  } else {
    status = 1;
  }
  lua_unlock(L);
  return status;
}


int LuaExtras::dump(lua_State* L)
{
  luaL_Buffer b;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  const bool strip = lua_isboolean(L, 2) && lua_tobool(L, 2);
  lua_settop(L, 1);
  luaL_buffinit(L,&b);
  if (Dump(L, Writer, &b, strip) != 0) {
    luaL_error(L, "unable to dump given function");
  }
  luaL_pushresult(&b);
  return 1;
}


static int Listing(lua_State *L, lua_Writer /*writer*/, void* /*data*/, int level)
{
  int status = 0;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o)) {
    luaU_print(clvalue(o)->l.p, level);
  } else {
    status = 1;
  }
  lua_unlock(L);
  return status;
}


int LuaExtras::listing(lua_State* L)
{
  luaL_Buffer b;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  const int level = luaL_optint(L, 2, 0);
  lua_settop(L, 1);
  luaL_buffinit(L,&b);
  if (Listing(L, Writer, &b, level) != 0) {
    luaL_error(L, "unable to dump given function");
  }
  luaL_pushresult(&b);
  return 1;
}


/******************************************************************************/
/******************************************************************************/
