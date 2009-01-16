
#include "common.h"

// implementation header
#include "SlashCmd.h"

// system headers
#include <string>
#include <map>
using std::string;
using std::map;

// common headers
#include "bzfsAPI.h"
#include "TextUtils.h"

// local headers
#include "LuaHeader.h"


static lua_State* topL = NULL;

static map<string, class SlashCmdHandler*> slashHandlers;

static int AttachSlashCommand(lua_State* L);
static int DetachSlashCommand(lua_State* L);


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
  lua_State* L = topL;

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
  lua_State* L = topL;

  slashHandlers.erase(cmd);
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
  }
}


bool SlashCmdHandler::handle(int playerID, bz_ApiString /*command*/,
                             bz_ApiString message, bz_APIStringList* /*params*/)
{
  lua_State* L = topL;

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

  lua_pushinteger(L, playerID);
  lua_pushstring(L, TextUtils::tolower(cmd).c_str());
  lua_pushstring(L, message.c_str());

  if (lua_pcall(L, 3, 1, 0) != 0) {
    bz_debugMessagef(0, "lua call-in slashcmd error (%s): %s\n",
                     cmd.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  if (lua_isboolean(L, -1) && !lua_tobool(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  return true;    
}


/******************************************************************************/
/******************************************************************************/

bool SlashCmd::PushEntries(lua_State* L)
{
  topL = L;

  lua_pushliteral(L, "AttachSlashCommand");
  lua_pushcfunction(L, AttachSlashCommand);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DetachSlashCommand");
  lua_pushcfunction(L, DetachSlashCommand);
  lua_rawset(L, -3);

  return true;
}


bool SlashCmd::CleanUp(lua_State* /*_L*/)
{
  map<string, SlashCmdHandler*>::const_iterator it, nextIT;

  for (it = slashHandlers.begin(); it != slashHandlers.end(); /* noop */) {
    nextIT = it;
    nextIT++;
    delete it->second; // deletes itself from slashHandlers
    it = nextIT;
  }

  slashHandlers.clear();

  topL = NULL;

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int AttachSlashCommand(lua_State* L)
{
  const string cmd = TextUtils::tolower(luaL_checkstring(L, 1));
  const char* help = luaL_checkstring(L, 2);
  if (!lua_isfunction(L, 3)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, 3); // function is the third param

  if (help[0] == 0) {
    luaL_error(L, "empty slash command help");
  }
    
  if (slashHandlers.find(cmd) != slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  SlashCmdHandler* handler = new SlashCmdHandler(cmd, help);
  if (bz_registerCustomSlashCommand(cmd.c_str(), handler)) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
    delete handler;
  }

  return 1;
}

static int DetachSlashCommand(lua_State* L)
{
  const string cmd = TextUtils::tolower(luaL_checkstring(L, 1));

  map<string, SlashCmdHandler*>::iterator it = slashHandlers.find(cmd);
  if (it == slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  delete it->second;
  
  if (bz_removeCustomSlashCommand(cmd.c_str())) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}


/******************************************************************************/
/******************************************************************************/
