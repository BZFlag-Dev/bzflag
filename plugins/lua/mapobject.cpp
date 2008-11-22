
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "mapobject.h"
#include "callins.h"

#include <string>
#include <set>
using std::string;
using std::set;

static set<string> objNames;


static int HandleMapObject(lua_State* L);


/******************************************************************************/
/******************************************************************************/

class CustomMapObjectHandler : public bz_CustomMapObjectHandler {
  public:
    bool handle(bz_ApiString object, bz_CustomMapObjectInfo *data) {
      return CallIns::CustomMapObject(object.c_str(), data);
    }
};

static CustomMapObjectHandler customMapObjectHandler;


/******************************************************************************/
/******************************************************************************/

bool MapObject::PushEntries(lua_State* L)
{
  lua_pushliteral(L, "HandleMapObject");
  lua_pushcfunction(L, HandleMapObject);
  lua_rawset(L, -3);
  return true;
}


bool MapObject::Shutdown(lua_State* L)
{
  set<string>::const_iterator it;
  for (it = objNames.begin(); it != objNames.end(); ++it) {
    bz_removeCustomMapObject((*it).c_str());
  }

  objNames.clear();

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int HandleMapObject(lua_State* L)
{
  // FIXME -- remove option?
  const char* objName = luaL_checkstring(L, 1);
  if (!lua_isboolean(L, 2) && !lua_isnone(L, 2)) {
    lua_pushboolean(L, false);
    return 1;
  }

  const bool enable = lua_isnone(L, 2) ||
                      (lua_isboolean(L, 2) && lua_toboolean(L, 2));

  if (enable) {
    if (objNames.find(objName) != objNames.end()) {
      lua_pushboolean(L, false); // already registered
    }
    else {
      if (bz_registerCustomMapObject(objName, &customMapObjectHandler)) {
        objNames.insert(objName);
        lua_pushboolean(L, true);
      } else {
        lua_pushboolean(L, false);
      }
    } 
  }
  else {
    if (objNames.find(objName) == objNames.end()) {
      lua_pushboolean(L, false); // not registered
    }
    else {
      objNames.erase(objName);
      if (bz_removeCustomMapObject(objName)) {
        lua_pushboolean(L, true);
      } else {
        lua_pushboolean(L, false);
      }
    }
  }

  return 1;
}


/******************************************************************************/
/******************************************************************************/
