
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "rawlink.h"

#include <string>
#include <set>
#include <map>
using std::string;
using std::set;
using std::map;


static lua_State* L = NULL;

static int AttachRawLink(lua_State* L);
static int DetachRawLink(lua_State* L);
static int DisconnectRawLink(lua_State* L);
static int WriteRawLink(lua_State* L);
static int GetRawLinkIP(lua_State* L);
static int GetRawLinkHost(lua_State* L);
static int GetRawLinkQueued(lua_State* L);


class Link;
typedef map<int, Link*> LinkMap;
static LinkMap links;


/******************************************************************************/
/******************************************************************************/

class Link : public bz_NonPlayerConnectionHandler {
  public:
    Link(lua_State* L, int linkID);
    ~Link();

    void pending(int id, void* data, unsigned int size);
    void disconnect(int id);

    bool IsValid() const { return (funcRef != LUA_NOREF); }

  private:
    int id;
    int funcRef;
};


/******************************************************************************/
/******************************************************************************/

Link::Link(lua_State* L, int _id)
: id(_id)
, funcRef(LUA_NOREF)
{
  if (links.find(id) != links.end()) {
    return;
  }
  if (!bz_registerNonPlayerConnectionHandler(id, this)) {
    return;
  }

  if (lua_isnil(L, 2)) {
    funcRef = LUA_REFNIL;
  }
  else if (lua_isfunction(L, 2)) {
    funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else {
    return;
  }

  links[id] = this;
}


Link::~Link()
{
  bz_removeNonPlayerConnectionHandler(id, this);
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
  }

  links.erase(id);
}


void Link::pending(int id, void* data, unsigned int size)
{
  if (L == NULL) {
    return;
  }
  if (funcRef == LUA_REFNIL) {
    return;
  }

  lua_checkstack(L, 4);

  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return;
  }

  lua_pushinteger(L, id);

  if (data == NULL) {
    lua_pushnil(L);
  } else {
    lua_pushlstring(L, (char*)data, size);
  }

  if (lua_pcall(L, 2, 0, 0) != 0) {
    bz_debugMessagef(0, "lua call-in rawlink error (%i): %s\n",
                     id, lua_tostring(L, -1));
    lua_pop(L, 1);
    return;
  }

  return;
}


void Link::disconnect(int id)
{
  pending(id, NULL, 0);
  delete this;
}


/******************************************************************************/
/******************************************************************************/

bool RawLink::PushEntries(lua_State* _L)
{
  L = _L;

  lua_pushliteral(L, "AttachRawLink");
  lua_pushcfunction(L, AttachRawLink);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DetachRawLink");
  lua_pushcfunction(L, DetachRawLink);
  lua_rawset(L, -3);

  lua_pushliteral(L, "WriteRawLink");
  lua_pushcfunction(L, WriteRawLink);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DisconnectRawLink");
  lua_pushcfunction(L, DisconnectRawLink);
  lua_rawset(L, -3);

  lua_pushliteral(L, "GetRawLinkIP");
  lua_pushcfunction(L, GetRawLinkIP);
  lua_rawset(L, -3);

  lua_pushliteral(L, "GetRawLinkHost");
  lua_pushcfunction(L, GetRawLinkHost);
  lua_rawset(L, -3);

  lua_pushliteral(L, "GetRawLinkQueued");
  lua_pushcfunction(L, GetRawLinkQueued);
  lua_rawset(L, -3);

  return true;
}


bool RawLink::CleanUp(lua_State* _L)
{
  L = NULL;

  LinkMap::iterator it, next;
  for (it = links.begin(); it != links.end(); /* no-op */) {
    next = it;
    next++;
    delete it->second;
    it = next;
  }
  links.clear();

  return true;
}


/******************************************************************************/
/******************************************************************************/

static int AttachRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);

  if (!lua_isnoneornil(L, 2) && !lua_isfunction(L, 2)) {
    return 0;
  }
  lua_settop(L, 2); // discard any extras

  // if fetch->IsActive() is true, this will push a userdata
  //  on to the top of the stack. Otherwise, return a nil.
  Link* link = new Link(L, linkID);
  if (!link->IsValid()) {
    delete link;
    return 0;
  }
  lua_pushboolean(L, true);
  return 1;
}


static int DetachRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  LinkMap::iterator it = links.find(linkID);
  if (it == links.end()) {
    return 0;
  }
  Link* link = it->second;
  delete link;
  lua_pushboolean(L, true);
  return 1;  
}


static int WriteRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (links.find(linkID) == links.end()) {
    return 0;
  }
  size_t size;
  const char* data = luaL_checklstring(L, 2, &size);
  lua_pushboolean(L, bz_sendNonPlayerData(linkID, data, size));
  return 1;
}


static int DisconnectRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (links.find(linkID) == links.end()) {
    return 0;
  }
  lua_pushboolean(L, bz_disconnectNonPlayerConnection(linkID));
  return 1;
}


static int GetRawLinkIP(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (links.find(linkID) == links.end()) {
    return 0;
  }
  lua_pushstring(L, bz_getNonPlayerConnectionIP(linkID));
  return 1;
}


static int GetRawLinkHost(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (links.find(linkID) == links.end()) {
    return 0;
  }
  lua_pushstring(L, bz_getNonPlayerConnectionHost(linkID));
  return 1;
}


static int GetRawLinkQueued(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (links.find(linkID) == links.end()) {
    return 0;
  }
  lua_pushinteger(L, bz_getNonPlayerConnectionOutboundPacketCount(linkID));
  return 1;
}


/******************************************************************************/
/******************************************************************************/
