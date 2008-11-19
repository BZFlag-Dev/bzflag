
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "mapobject.h"
#include "callins.h"

#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;

static set<string> objNames;


static int CustomObject(lua_State* L);


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
  lua_pushliteral(L, "CustomObject");
  lua_pushcfunction(L, CustomObject);
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

static int CustomObject(lua_State* L)
{
  const char* objName = luaL_checkstring(L, 1);
  if (objNames.find(objName) != objNames.end()) {
    return 0;
  }
  objNames.insert(objName);
  bz_registerCustomMapObject(objName, &customMapObjectHandler);
  return 0;
}


/******************************************************************************/
/******************************************************************************/
