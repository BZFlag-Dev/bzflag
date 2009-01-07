
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
//
//  Copied directly from src/other/lua/src/ldblib.c
//

static lua_State *getthread (lua_State *L, int *arg) {
  if (lua_isthread(L, 1)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {   
    *arg = 0;
    return L;
  }
}


#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

int LuaExtras::traceback(lua_State *L)  // db_errorfb() function
{
  int level;
  int firstpart = 1;  /* still before eventual `...' */
  int arg;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  if (lua_isnumber(L, arg+2)) {
    level = (int)lua_tointeger(L, arg+2);
    lua_pop(L, 1);
  }
  else
    level = (L == L1) ? 1 : 0;  /* level 0 may be this own function */
  if (lua_gettop(L) == arg)
    lua_pushliteral(L, "");
  else if (!lua_isstring(L, arg+1)) return 1;  /* message is not a string */
  else lua_pushliteral(L, "\n");
  lua_pushliteral(L, "stack traceback:");
  while (lua_getstack(L1, level++, &ar)) {
    if (level > LEVELS1 && firstpart) {
      /* no more than `LEVELS2' more levels? */
      if (!lua_getstack(L1, level+LEVELS2, &ar))
        level--;  /* keep going */
      else {
        lua_pushliteral(L, "\n\t...");  /* too many levels */
        while (lua_getstack(L1, level+LEVELS2, &ar))  /* find last levels */
          level++;
      }
      firstpart = 0;
      continue;
    }
    lua_pushliteral(L, "\n\t");
    lua_getinfo(L1, "Snl", &ar);
    lua_pushfstring(L, "%s:", ar.short_src);
    if (ar.currentline > 0)
      lua_pushfstring(L, "%d:", ar.currentline);
    if (*ar.namewhat != '\0')  /* is there a name? */
        lua_pushfstring(L, " in function " LUA_QS, ar.name);
    else {
      if (*ar.what == 'm')  /* main? */
        lua_pushfstring(L, " in main chunk");
      else if (*ar.what == 'C' || *ar.what == 't')
        lua_pushliteral(L, " ?");  /* C function or tail call */
      else
        lua_pushfstring(L, " in function <%s:%d>",
                           ar.short_src, ar.linedefined);
    }
    lua_concat(L, lua_gettop(L) - arg);
  }
  lua_concat(L, lua_gettop(L) - arg);
  return 1;
}


/******************************************************************************/
/******************************************************************************/
