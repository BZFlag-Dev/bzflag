/*
** $Id$
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/


#include <stddef.h>
#include <string.h> /*BZ*/

#define ltablib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

/* for sort() */
#include "lstate.h"
#include "ltable.h"
#include "lvm.h"
#include "lgc.h"


#define aux_getn(L,n)	(luaL_checktype(L, n, LUA_TTABLE), luaL_getn(L, n))


static int foreachi (lua_State *L) {
  int i;
  int n = aux_getn(L, 1);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  for (i=1; i <= n; i++) {
    lua_pushvalue(L, 2);  /* function */
    lua_pushinteger(L, i);  /* 1st argument */
    lua_rawgeti(L, 1, i);  /* 2nd argument */
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1))
      return 1;
    lua_pop(L, 1);  /* remove nil result */
  }
  return 0;
}


static int foreach (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1)) {
    lua_pushvalue(L, 2);  /* function */
    lua_pushvalue(L, -3);  /* key */
    lua_pushvalue(L, -3);  /* value */
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1))
      return 1;
    lua_pop(L, 2);  /* remove value and result */
  }
  return 0;
}


static int maxn (lua_State *L) {
  lua_Number max = 0;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
    if (lua_type(L, -1) == LUA_TNUMBER) {
      lua_Number v = lua_tonumber(L, -1);
      if (v > max) max = v;
    }
  }
  lua_pushnumber(L, max);
  return 1;
}


static int getn (lua_State *L) {
  lua_pushinteger(L, aux_getn(L, 1));
  return 1;
}


static int setn (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
#ifndef luaL_setn
  luaL_setn(L, 1, luaL_checkint(L, 2));
#else
  luaL_error(L, LUA_QL("setn") " is obsolete");
#endif
  lua_pushvalue(L, 1);
  return 1;
}


static int tinsert (lua_State *L) {
  int e = aux_getn(L, 1) + 1;  /* first empty element */
  int pos;  /* where to insert new element */
  switch (lua_gettop(L)) {
    case 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      break;
    }
    case 3: {
      int i;
      pos = luaL_checkint(L, 2);  /* 2nd argument is the position */
      if (pos > e) e = pos;  /* `grow' array if necessary */
      for (i = e; i > pos; i--) {  /* move up elements */
        lua_rawgeti(L, 1, i-1);
        lua_rawseti(L, 1, i);  /* t[i] = t[i-1] */
      }
      break;
    }
    default: {
      return luaL_error(L, "wrong number of arguments to " LUA_QL("insert"));
    }
  }
  luaL_setn(L, 1, e);  /* new size */
  lua_rawseti(L, 1, pos);  /* t[pos] = v */
  return 0;
}


static int tremove (lua_State *L) {
  int e = aux_getn(L, 1);
  int pos = luaL_optint(L, 2, e);
  if (!(1 <= pos && pos <= e))  /* position is outside bounds? */
   return 0;  /* nothing to remove */
  luaL_setn(L, 1, e - 1);  /* t.n = n-1 */
  lua_rawgeti(L, 1, pos);  /* result = t[pos] */
  for ( ;pos<e; pos++) {
    lua_rawgeti(L, 1, pos+1);
    lua_rawseti(L, 1, pos);  /* t[pos] = t[pos+1] */
  }
  lua_pushnil(L);
  lua_rawseti(L, 1, e);  /* t[e] = nil */
  return 1;
}


static void addfield (lua_State *L, luaL_Buffer *b, int i) {
  lua_rawgeti(L, 1, i);
  if (!lua_isstring(L, -1))
    luaL_error(L, "invalid value (%s) at index %d in table for "
                  LUA_QL("concat"), luaL_typename(L, -1), i);
    luaL_addvalue(b);
}


static int tconcat (lua_State *L) {
  luaL_Buffer b;
  size_t lsep;
  int i, last;
  const char *sep = luaL_optlstring(L, 2, "", &lsep);
  luaL_checktype(L, 1, LUA_TTABLE);
  i = luaL_optint(L, 3, 1);
  last = luaL_opt(L, luaL_checkint, 4, luaL_getn(L, 1));
  luaL_buffinit(L, &b);
  for (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  if (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  return 1;
}


static int tcreate(lua_State *L) {
  const int narr  = luaL_optint(L, 1, 0);
  const int nhash = luaL_optint(L, 2, 0);
  lua_createtable(L, narr, nhash);
  return 1;
}


#ifdef LUA_READONLY_TABLES
static int treadonly (lua_State *L) {
  static const int luabits = (LUA_READONLY_OLD_LUA_BIT |
                              LUA_READONLY_NEW_LUA_BIT);
  int readonly;

  luaL_checktype(L, 1, LUA_TTABLE);

  readonly = lua_getreadonly(L, 1);

  if (lua_isnone(L, 2)) {  /* read the state */
    if (!readonly) {
      lua_pushboolean(L, 0);
      return 1;
    } else {
      char modes[3], *c = modes;
      if (readonly & LUA_READONLY_OLD_BITS) { *c = 'o'; c++; }
      if (readonly & LUA_READONLY_NEW_BITS) { *c = 'n'; c++; }
      *c = '\0';  /* terminate */
      lua_pushstring(L, modes);
      return 1;
    }
  }

  if (lua_type(L, 2) == LUA_TSTRING) {
    const char *s = lua_tostring(L, 2);
    readonly &= ~luabits;
    if (strchr(s, 'o')) { readonly |= LUA_READONLY_OLD_LUA_BIT; }
    if (strchr(s, 'n')) { readonly |= LUA_READONLY_NEW_LUA_BIT; }
  }
  else {
    if (lua_toboolean(L, 2)) {
      readonly |=  luabits;
    } else {
      readonly &= ~luabits;
    }
  }

  lua_setreadonly(L, 1, readonly);  /* return the table */
  lua_settop(L, 1);
  return 1;
}
#endif


/*
** {======================================================
+** MergeSort
+**   - An `array only' copy of the original table is made so that garbage
+**     collection does not invalidate the table values during sorting.
+**   - The main sorting routine is called via a lua_pcall() so that
+**     the scratch pad memory is freed should an error occur.
*/

typedef int (*compfunc)(lua_State *L, const TValue *a, const TValue *b);

typedef struct SortInfo {
  lua_State* L;
  int size;
  Table* tcopy;
  TValue* data;  /* tcopy->array */
  TValue* tmp;   /* `size' elements of scratch space */
  compfunc func;
} SortInfo;


static int comp_custom (lua_State *L, const TValue *a, const TValue *b) {
  int res;
  lua_pushvalue(L, 2);  /* comparison function, see sort() */
  lua_lock(L);
  setobj2s(L, L->top, a); L->top++;
  setobj2s(L, L->top, b); L->top++;
  lua_unlock(L);
  lua_call(L, 2, 1);
  res = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}


static void mergesort (SortInfo *sortInfo, TValue* data, int size) {
  int s0, s1;
  TValue *d0, *d1;

  if (size <= 1) {
    return;
  }

  s0 = size / 2;
  s1 = size - s0;
  d0 = data;
  d1 = data + s0;

  /* sort sub-sections */
  mergesort(sortInfo, d0, s0);
  mergesort(sortInfo, d1, s1);

  /* merge sub-sections -- stable order */
  {
    lua_State* L    = sortInfo->L;
    compfunc   func = sortInfo->func;
    TValue*    tmp  = sortInfo->tmp;

    while ((s0 > 0) && (s1 > 0)) {
      if (func(L, d1, d0)) {
        *tmp = *d1; d1++; s1--;
      } else {
        *tmp = *d0; d0++; s0--;
      }
      tmp++;
    }

    if (s0 > 0) {
      memcpy(tmp, d0, s0 * sizeof(TValue));
    }
    memcpy(data, sortInfo->tmp, (size - s1) * sizeof(TValue));
  }
}


static int sortproxy (lua_State* L) {
  SortInfo* si = (SortInfo*) lua_touserdata(L, 1);
  mergesort(si, si->data, si->size);
  return 0;
}


static void sortcleanup (SortInfo *sortInfo) {
  Table *tcopy = sortInfo->tcopy;
  lua_lock(L);  /* clear the copy table's array */
  luaM_freearray(sortInfo->L, tcopy->array, tcopy->sizearray, TValue);
  tcopy->array = NULL;
  tcopy->sizearray = 0;
  lua_unlock(L);
  luaM_freearray(sortInfo->L, sortInfo->tmp, sortInfo->size, TValue);
}


static int sort (lua_State *L) {
  int i;
  Table *tcopy;
  SortInfo sortInfo;

  const int n = aux_getn(L, 1);
  if (n <= 1) {
    return 0;  /* no sorting required */
  }

  if (!lua_isnoneornil(L, 2)) {  /* is there a 2nd argument? */
    luaL_checktype(L, 2, LUA_TFUNCTION);
  }
  lua_settop(L, 2);  /* make sure there are two arguments */

  lua_lock(L);  /* lock to get tcopy  (index 3) */
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, n, 0));  /* lua_createtable(L, n, 0); */
  tcopy = hvalue(L->top);
  L->top++;
  lua_unlock(L);

  for (i = 1; i <= n; i++) {
    lua_rawgeti(L, 1, i);
    lua_rawseti(L, 3, i);
  }

  sortInfo.L = L;
  sortInfo.size = n;
  sortInfo.tcopy = tcopy;
  sortInfo.data = tcopy->array;
  sortInfo.tmp = luaM_newvector(L, n, TValue);  /* tmp scratchpad */
  sortInfo.func = lua_isnil(L, 2) ? luaV_lessthan : comp_custom;

  lua_pushcfunction(L, sortproxy);
  lua_pushlightuserdata(L, &sortInfo);  /* 1 = lightuserdata */
  lua_pushvalue(L, 2);                  /* 2 = function or nil */
  if (lua_pcall(L, 2, 0, 0) != 0) {
    sortcleanup(&sortInfo);
    lua_error(L);
  }

  for (i = 1; i <= n; i++) {  /* copy the data back to the original table */
    lua_rawgeti(L, 3, i);
    lua_rawseti(L, 1, i);
  }

  sortcleanup(&sortInfo);

  return 0;
}


/* }====================================================== */

static const luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"foreach", foreach},
  {"foreachi", foreachi},
  {"getn", getn},
  {"maxn", maxn},
  {"insert", tinsert},
#ifdef LUA_READONLY_TABLES
  {"readonly", treadonly},
#endif
  {"remove", tremove},
  {"setn", setn},
  {"sort", sort},
  {NULL, NULL}
};


LUALIB_API int luaopen_table (lua_State *L) {
  luaL_register(L, LUA_TABLIBNAME, tab_funcs);
  return 1;
}

