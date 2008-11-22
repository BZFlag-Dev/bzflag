
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "slashcmd.h"
#include "callins.h"

#include <string>
#include <map>
using std::string;
using std::map;


class SlashCmdHandler;

static int SlashCommandInsert(lua_State* L);
static int SlashCommandRemove(lua_State* L);

static lua_State* L = NULL;

static map<string, SlashCmdHandler*> slashHandlers;


/******************************************************************************/
/******************************************************************************/

class SlashCmdHandler : public bz_CustomSlashCommandHandler {
  public:
    SlashCmdHandler(const string& c, const string& h);
    ~SlashCmdHandler();

    bool handle(int playerID, bz_ApiString command,
                bz_ApiString message, bz_APIStringList* params);
      
    const char* help(bz_ApiString /* command */) { return helpTxt.c_str(); }

  private:
    string cmd;
    string helpTxt;
    int luaRef;
};


SlashCmdHandler::SlashCmdHandler(const string& c, const string& h)
: cmd(c)
, helpTxt(h)
, luaRef(LUA_NOREF)
{
  slashHandlers[cmd] = this;
  if (L == NULL) {
    return;
  }
  if (!lua_isfunction(L, -1)) {
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


SlashCmdHandler::~SlashCmdHandler()
{
  slashHandlers.erase(cmd);
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
  }
}


bool SlashCmdHandler::handle(int playerID, bz_ApiString command,
                             bz_ApiString message, bz_APIStringList* params)
{
  lua_checkstack(L, 5);
  lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return false;
  }
  lua_pushinteger(L, playerID);
  lua_pushstring(L, cmd.c_str());
  lua_pushstring(L, message.c_str());

  if (lua_pcall(L, 3, 1, 0) != 0) {
    bz_debugMessagef(0, "lua call-in slashcmd error (%s): %s\n",
                     cmd.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  return true;    
}


/******************************************************************************/
/******************************************************************************/

bool SlashCmd::PushEntries(lua_State* _L)
{
  L = _L;

  lua_pushliteral(L, "SlashCommandInsert");
  lua_pushcfunction(L, SlashCommandInsert);
  lua_rawset(L, -3);

  lua_pushliteral(L, "SlashCommandRemove");
  lua_pushcfunction(L, SlashCommandRemove);
  lua_rawset(L, -3);

  return true;
}


bool SlashCmd::Shutdown(lua_State* _L)
{
  map<string, SlashCmdHandler*>::const_iterator it, nextIT;
  
  // FIXME -- safe loop
  for (it = slashHandlers.begin(); it != slashHandlers.end(); /* noop */) {
    nextIT = it;
    nextIT++;
    delete it->second;
    it = nextIT;
  }

  slashHandlers.clear();

  L = NULL;

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int SlashCommandInsert(lua_State* L)
{
  const char* cmd  = luaL_checkstring(L, 1);
  const char* help = luaL_checkstring(L, 2);
  if (!lua_isfunction(L, 3)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, 3); // function is the third param

  if (strlen(help) <= 0) {
    luaL_error(L, "empty slash command help");
  }
    
  if (slashHandlers.find(cmd) != slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  SlashCmdHandler* handler = new SlashCmdHandler(cmd, help);
  if (bz_registerCustomSlashCommand(cmd, handler)) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
    delete handler;
  }

  return 1;
}

static int SlashCommandRemove(lua_State* L)
{
  const char* cmd  = luaL_checkstring(L, 1);

  map<string, SlashCmdHandler*>::iterator it = slashHandlers.find(cmd);
  if (it == slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  delete it->second;
  
  if (bz_removeCustomSlashCommand(cmd)) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}


/******************************************************************************/
/******************************************************************************/
