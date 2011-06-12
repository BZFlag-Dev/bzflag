/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "common.h"

// interface header
#include "LuaTable.h"

// system headers
#include <algorithm>

// common headers
#include "ParseColor.h"

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//
//
//  LuaTable
//

LuaTable::LuaTable()
  : L(NULL)
  , luaRef(LUA_NOREF) {
}


LuaTable::LuaTable(lua_State* L_, int index) : L(L_), luaRef(LUA_NOREF) {
  if (L == NULL) {
    return;
  }
  lua_checkstack(L, 1);
  lua_pushvalue(L, index);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    L = NULL;
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


LuaTable::LuaTable(const LuaTable& tbl) : L(tbl.L) {
  if (!tbl.PushTable()) {
    L = NULL;
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


LuaTable& LuaTable::operator=(const LuaTable& tbl) {
  if (L) {
    luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
  }
  if (!tbl.PushTable()) {
    L = NULL;
    return *this;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

  return *this;
}


LuaTable::~LuaTable() {
  if (L && (luaRef != LUA_NOREF)) {
    luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
  }
}


LuaTable LuaTable::SubTable(int key) const {
  if (!PushTable()) {
    return LuaTable();
  }
  lua_pushinteger(L, key);
  lua_gettable(L, -2);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 2);
    return LuaTable();
  }
  LuaTable subTable(L, -1);
  lua_pop(L, 1);
  return subTable;
}


LuaTable LuaTable::SubTable(const std::string& key) const {
  if (!PushTable()) {
    return LuaTable();
  }
  lua_pushstdstring(L, key);
  lua_gettable(L, -2);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return LuaTable();
  }
  LuaTable subTable(L, -1);
  lua_pop(L, 1);
  return subTable;
}


LuaTable LuaTable::SubTableExpr(const std::string& expr) const {
  if (expr.empty()) {
    return LuaTable(*this);
  }
  if (L == NULL) {
    return LuaTable();
  }

  std::string::size_type endPos;
  LuaTable nextTable;

  if (expr[0] == '[') { // numeric key
    endPos = expr.find(']');
    if (endPos == std::string::npos) {
      return LuaTable(); // missing brace
    }
    const char* startPtr = expr.c_str() + 1; // skip the '['
    char* endPtr;
    const int index = strtol(startPtr, &endPtr, 10);
    if (endPtr == startPtr) {
      return LuaTable(); // invalid index
    }
    endPos++; // eat the ']'
    nextTable = SubTable(index);
  }
  else { // string key
    endPos = expr.find_first_of(".[");
    if (endPos == std::string::npos) {
      return SubTable(expr);
    }
    nextTable = SubTable(expr.substr(0, endPos));
  }

  if (expr[endPos] == '.') {
    endPos++; // eat the dot
  }
  return nextTable.SubTableExpr(expr.substr(endPos));
}


//============================================================================//

bool LuaTable::PushTable() const {
  if (!L) {
    return false;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    printf("ERROR in LuaTable::PushTable()\n");
    const_cast<LuaTable*>(this)->L = NULL;
    return false;
  }

  return true;
}


bool LuaTable::PushValue(int key) const {
  if (!PushTable()) {
    return false;
  }
  lua_pushinteger(L, key);
  lua_gettable(L, -2);
  if (lua_isnoneornil(L, -1)) {
    lua_pop(L, 2);
    return false;
  }
  return true;
}


bool LuaTable::PushValue(const std::string& key) const {
  if (!PushTable()) {
    return false;
  }
  lua_pushstdstring(L, key);
  lua_gettable(L, -2);
  if (lua_isnoneornil(L, -1)) {
    lua_pop(L, 2);
    return false;
  }
  return true;
}


//============================================================================//
//============================================================================//
//
//  Key existance testing
//

bool LuaTable::KeyExists(int key) const {
  if (!PushValue(key)) {
    return false;
  }
  lua_pop(L, 2);
  return true;
}


bool LuaTable::KeyExists(const std::string& key) const {
  if (!PushValue(key)) {
    return false;
  }
  lua_pop(L, 2);
  return true;
}


//============================================================================//
//============================================================================//
//
//  Value types
//

LuaTable::DataType LuaTable::GetType(int key) const {
  if (!PushValue(key)) {
    return none_type;;
  }
  const int luaType = lua_type(L, -1);
  lua_pop(L, 2);
  return LuaTypeToDataType(luaType);
}


LuaTable::DataType LuaTable::GetType(const std::string& key) const {
  if (!PushValue(key)) {
    return none_type;;
  }
  const int luaType = lua_type(L, -1);
  lua_pop(L, 2);
  return LuaTypeToDataType(luaType);
}


int LuaTable::DataTypeToLuaType(DataType tblType) {
  switch (tblType) {
    case nil_type:           { return LUA_TNIL;           }
    case boolean_type:       { return LUA_TBOOLEAN;       }
    case lightuserdata_type: { return LUA_TLIGHTUSERDATA; }
    case number_type:        { return LUA_TNUMBER;        }
    case string_type:        { return LUA_TSTRING;        }
    case table_type:         { return LUA_TTABLE;         }
    case function_type:      { return LUA_TFUNCTION;      }
    case userdate_type:      { return LUA_TUSERDATA;      }
    case thread_type:        { return LUA_TTHREAD;        }
    default: {
      return none_type;
    }
  }
  return none_type;
}


LuaTable::DataType LuaTable::LuaTypeToDataType(int luaType) {
  switch (luaType) {
    case LUA_TNIL:           { return nil_type;           }
    case LUA_TBOOLEAN:       { return boolean_type;       }
    case LUA_TLIGHTUSERDATA: { return lightuserdata_type; }
    case LUA_TNUMBER:        { return number_type;        }
    case LUA_TSTRING:        { return string_type;        }
    case LUA_TTABLE:         { return table_type;         }
    case LUA_TFUNCTION:      { return function_type;      }
    case LUA_TUSERDATA:      { return userdate_type;      }
    case LUA_TTHREAD:        { return thread_type;        }
    default: {
      return none_type;
    }
  }
  return none_type;
}

//============================================================================//
//============================================================================//
//
//  Object lengths
//

int LuaTable::GetLength() const {
  if (!PushTable()) {
    return luaL_pushnil(L);
  }
  return lua_objlen(L, -1);
}


int LuaTable::GetLength(int key) const {
  if (!PushValue(key)) {
    return luaL_pushnil(L);
  }
  const int len = lua_objlen(L, -1);
  lua_pop(L, 2);
  return len;
}


int LuaTable::GetLength(const std::string& key) const {
  if (!PushValue(key)) {
    return luaL_pushnil(L);
  }
  const int len = lua_objlen(L, -1);
  lua_pop(L, 2);
  return len;
}


//============================================================================//
//============================================================================//
//
//  luaget_value(L, index, T& value)  -- typed get templates
//

enum luaget_state {
  luaget_success,
  luaget_missing,
  luaget_baddata
};

template <typename T>
luaget_state luaget_value(lua_State* L, int index, T& value);

template <> // int
luaget_state luaget_value(lua_State* L, int index, int& value) {
  if (!lua_israwnumber(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  value = lua_toint(L, index);
  return luaget_success;
}

template <> // bool
luaget_state luaget_value(lua_State* L, int index, bool& value) {
  if (!lua_isboolean(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  value = lua_tobool(L, index);
  return luaget_success;
}

template <> // float
luaget_state luaget_value(lua_State* L, int index, float& value) {
  if (!lua_israwnumber(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  value = lua_tofloat(L, index);
  return luaget_success;
}

template <> // stdstring -- FIXME
luaget_state luaget_value(lua_State* L, int index, std::string& value) {
  if (!lua_israwstring(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  value = lua_tostdstring(L, index);
  return luaget_success;
}

template <> // fvec2
luaget_state luaget_value(lua_State* L, int index, fvec2& value) {
  if (!lua_istable(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  const int table = (index > 0) ? index : lua_gettop(L) + 1 + index;
  fvec2 tmp;
  luaget_state state;
  for (int i = 0; i < 2; i++) {
    lua_pushint(L, i + 1); lua_gettable(L, table);
    state = luaget_value(L, -1, tmp[i]);
    lua_pop(L, 1);
    if (state != luaget_success) { return state; }
  }
  value = tmp;
  return luaget_success;
}

template <> // fvec3
luaget_state luaget_value(lua_State* L, int index, fvec3& value) {
  if (!lua_istable(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  const int table = (index > 0) ? index : lua_gettop(L) + 1 + index;
  fvec3 tmp;
  luaget_state state;
  for (int i = 0; i < 3; i++) {
    lua_pushint(L, i + 1); lua_gettable(L, table);
    state = luaget_value(L, -1, tmp[i]);
    lua_pop(L, 1);
    if (state != luaget_success) { return state; }
  }
  value = tmp;
  return luaget_success;
}

template <> // fvec4
luaget_state luaget_value(lua_State* L, int index, fvec4& value) {
  if (!lua_istable(L, index)) {
    return lua_isnil(L, index) ? luaget_missing : luaget_baddata;
  }
  const int table = (index > 0) ? index : lua_gettop(L) + 1 + index;
  fvec4 tmp;
  luaget_state state;
  for (int i = 0; i < 4; i++) {
    lua_pushint(L, i + 1); lua_gettable(L, table);
    state = luaget_value(L, -1, tmp[i]);
    lua_pop(L, 1);
    if (state != luaget_success) { return state; }
  }
  value = tmp;
  return luaget_success;
}


//============================================================================//
//============================================================================//
//
//  Key list functions
//

template <typename C>
bool LuaTable::GetKeys(C& data) const {
  if (!PushTable()) {
    return false;
  }
  const int table = lua_gettop(L);
  for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
    typename C::value_type value;
    if (luaget_value(L, -2, value) == luaget_success) {
      data.push_back(value);
    }
  }
  lua_pop(L, 1);
  std::sort(data.begin(), data.end());
  return true;
}

bool LuaTable::GetKeys(std::set<int>&             data) const { return GetKeys(data); }
bool LuaTable::GetKeys(std::set<float>&           data) const { return GetKeys(data); }
bool LuaTable::GetKeys(std::set<std::string>&     data) const { return GetKeys(data); }
bool LuaTable::GetKeys(std::vector<int>&          data) const { return GetKeys(data); }
bool LuaTable::GetKeys(std::vector<float>&        data) const { return GetKeys(data); }
bool LuaTable::GetKeys(std::vector<std::string>&  data) const { return GetKeys(data); }

//============================================================================//
//============================================================================//
//
//  Value list functions
//


template <typename C>
bool LuaTable::GetValues(C& data) const {
  printf("GetValues(std::vector<T>& data)\n"); fflush(stdout); // FIXME
  if (!PushTable()) {
    return false;
  }
  printf("GetValues(std::vector<T>& data)\n"); fflush(stdout); // FIXME
  const int table = lua_gettop(L);
  for (int i = 1; true; i++) {
    printf("GetValues(std::vector<T>& data)  i = %i\n", i); fflush(stdout); // FIXME
    lua_pushint(L, i);
    lua_gettable(L, table);
    typename C::value_type value;
    if (luaget_value(L, -1, value) != luaget_success) {
      break;
    }
    lua_pop(L, 1);
    data.add(value);
  }
  lua_pop(L, 2);
  return true;
}

bool LuaTable::GetValues(std::vector<int>& data) const {
  printf("GetValues(std::vector<T>& data)\n"); fflush(stdout); // FIXME
  if (!PushTable()) {
    return false;
  }
  printf("GetValues(std::vector<T>& data)\n"); fflush(stdout); // FIXME
  const int table = lua_gettop(L);
  for (int i = 1; true; i++) {
    printf("GetValues(std::vector<T>& data)  i = %i\n", i); fflush(stdout); // FIXME
    lua_pushint(L, i);
    lua_gettable(L, table);
    std::vector<int>::value_type value;
    if (luaget_value(L, -1, value) != luaget_success) {
      break;
    }
    lua_pop(L, 1);
    data.push_back(value);
  }
  lua_pop(L, 2);
  return true;
}

bool LuaTable::GetValues(std::set<int>&             data) const { return GetValues(data); }
bool LuaTable::GetValues(std::set<float>&           data) const { return GetValues(data); }
bool LuaTable::GetValues(std::set<std::string>&     data) const { return GetValues(data); }
//FIXMEbool LuaTable::GetValues(std::vector<int>&   data) const { return GetValues(data); }
bool LuaTable::GetValues(std::vector<float>&        data) const { return GetValues(data); }
bool LuaTable::GetValues(std::vector<std::string>&  data) const { return GetValues(data); }

//============================================================================//
//============================================================================//
//
//  Map functions
//

template <typename K, typename V>
bool LuaTable::GetMap(std::map<K, V>& data) const {
  if (!PushTable()) {
    return false;
  }
  const int table = lua_gettop(L);
  for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
    K key;
    V value;
    if ((luaget_value(L, -2, key)   == luaget_success) &&
        (luaget_value(L, -1, value) == luaget_success)) {
      data[key] = value;
    }
  }
  lua_pop(L, 1);
  return true;
}

bool LuaTable::GetMap(std::map<int,                 int>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<int,               float>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<int,         std::string>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<std::string,         int>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<std::string,       float>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<std::string, std::string>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<float,               int>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<float,             float>& data) const { return GetMap(data); }
bool LuaTable::GetMap(std::map<float,       std::string>& data) const { return GetMap(data); }

//============================================================================//
//============================================================================//
//
//  Type Gets
//

template <typename K, typename V>
bool LuaTable::GetValue(K key, V& value) const {
  if (!PushValue(key)) {
    return false;
  }
  luaget_state state = luaget_value(L, -1, value);
  lua_pop(L, 2);
  return state == luaget_success;
}

bool LuaTable::GetInt(int k, int&   v) const { return GetValue(k, v); }
bool LuaTable::GetBool(int k, bool&  v) const { return GetValue(k, v); }
bool LuaTable::GetFloat(int k, float& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec2(int k, fvec2& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec3(int k, fvec3& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec4(int k, fvec4& v) const { return GetValue(k, v); }
bool LuaTable::GetString(int k, std::string& v) const { return GetValue(k, v); }

bool LuaTable::GetInt(const std::string& k, int&   v) const { return GetValue(k, v); }
bool LuaTable::GetBool(const std::string& k, bool&  v) const { return GetValue(k, v); }
bool LuaTable::GetFloat(const std::string& k, float& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec2(const std::string& k, fvec2& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec3(const std::string& k, fvec3& v) const { return GetValue(k, v); }
bool LuaTable::GetFVec4(const std::string& k, fvec4& v) const { return GetValue(k, v); }
bool LuaTable::GetString(const std::string& k, std::string& v) const { return GetValue(k, v); }

//============================================================================//
//============================================================================//

bool LuaTable::GetColor(const std::string& key, fvec4& value) const {
  if (GetFVec4(key, value)) {
    return true;
  }
  if (GetFVec3(key, value.rgb())) {
    value.a = 1.0f;
    return true;
  }
  std::string s;
  if (!GetString(key, s)) {
    return false;
  }
  return parseColorString(s, value);
}

//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
