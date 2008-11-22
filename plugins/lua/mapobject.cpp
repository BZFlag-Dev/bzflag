
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "mapobject.h"

#include <string>
#include <map>
using std::string;
using std::map;


static lua_State* L = NULL;

static map<string, class MapHandler*> mapHandlers;

static int AttachMapObject(lua_State* L);
static int DetachMapObject(lua_State* L);


/******************************************************************************/
/******************************************************************************/

class MapHandler : public bz_CustomMapObjectHandler {
  public:
    MapHandler(const string& objName);
    ~MapHandler();

    bool handle(bz_ApiString object, bz_CustomMapObjectInfo *data);

  private:
    string objName;
    int luaRef;
};


MapHandler::MapHandler(const string& name)
: objName(name)
, luaRef(LUA_NOREF)
{
  mapHandlers[objName] = this;
  if (L == NULL) {
    return;
  }
  if (!lua_isfunction(L, -1)) {
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


MapHandler::~MapHandler()
{
  mapHandlers.erase(objName);
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
  }
}


bool MapHandler::handle(bz_ApiString objName, bz_CustomMapObjectInfo *info)
{
  if (L == NULL) {
    return false;
  }

  lua_checkstack(L, 5);

  lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return false;
  }

  lua_pushstring(L, makelower(objName.c_str()).c_str());

  lua_newtable(L);
  bz_APIStringList& list = info->data;
  for (int i = 0; i < list.size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, list[i].c_str());
    lua_rawset(L, -3);
  }

  if (lua_pcall(L, 2, 1, 0) != 0) {
    bz_debugMessagef(0, "lua call-in mapobject error (%s): %s\n",
                     objName.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  string newData = info->newData.c_str();

  if (lua_israwstring(L, -1)) {
    newData += lua_tostring(L, -1);
  }
  else if (lua_istable(L, -1)) {
    const int table = lua_gettop(L);
    for (int i = 1; lua_checkgeti(L, table, i) != 0; lua_pop(L, 1), i++) {
      if (lua_israwstring(L, -1)) {
        newData += lua_tostring(L, -1);
        newData += "\n";
      }
    } 
  }

  info->newData = newData;

  lua_pop(L, 1);

  return true;
}


/******************************************************************************/
/******************************************************************************/

bool MapObject::PushEntries(lua_State* _L)
{
  L = _L;

  lua_pushliteral(L, "AttachMapObject");
  lua_pushcfunction(L, AttachMapObject);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DetachMapObject");
  lua_pushcfunction(L, DetachMapObject);
  lua_rawset(L, -3);

  return true;
}


bool MapObject::Shutdown(lua_State* _L)
{
  map<string, MapHandler*>::const_iterator it, nextIT;

  for (it = mapHandlers.begin(); it != mapHandlers.end(); /* noop */) {
    nextIT = it;
    nextIT++;
    delete it->second; // deletes itself from mapHandlers
    it = nextIT;
  }

  mapHandlers.clear();

  L = NULL;

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int AttachMapObject(lua_State* L)
{
  const string objName  = makelower(luaL_checkstring(L, 1));
  if (!lua_isfunction(L, 2)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, 2); // function is the third param

  if (mapHandlers.find(objName) != mapHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  MapHandler* handler = new MapHandler(objName);
  if (bz_registerCustomMapObject(objName.c_str(), handler)) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
    delete handler;
  }

  return 1;
}


static int DetachMapObject(lua_State* L)
{
  const string objName = makelower(luaL_checkstring(L, 1));

  map<string, MapHandler*>::iterator it = mapHandlers.find(objName);
  if (it == mapHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  delete it->second;
  
  if (bz_removeCustomMapObject(objName.c_str())) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}


/******************************************************************************/
/******************************************************************************/
