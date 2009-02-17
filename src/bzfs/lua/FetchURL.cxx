
#include "common.h"

// interface header
#include "FetchURL.h"

// system headers
#include <new>
#include <string>
#include <set>
#include <map>
using std::string;
using std::set;
using std::map;

// common headers
#include "bzfsAPI.h"
#include "bzfio.h"

// local headers
#include "LuaHeader.h"
#include "LuaServer.h"


static const char* metaName = "FetchURL";

static int FetchURL_callout(lua_State* L);

static bool CreateMetatble(lua_State* L);
static int MetaGC(lua_State* L);

static int Cancel(lua_State* L);
static int Success(lua_State* L);
static int IsActive(lua_State* L);
static int GetURL(lua_State* L);
static int GetPostData(lua_State* L);


/******************************************************************************/
/******************************************************************************/

class FetchHandler : public bz_BaseURLHandler {
  public:
    FetchHandler(lua_State* L, const char* url, const char* postData);
    ~FetchHandler();

    void done(const char* URL, void* data, unsigned int size, bool complete);
    void error(const char* URL, int errorCode, const char* errorString);
    void timeout(const char* URL, int errorCode);

    bool Handle(const char* URL, void* data, unsigned int size,
                const char* failType, int errorCode, const char* errorStr);

    bool Cancel();
    bool Success()  const { return success; }
    bool IsActive() const { return (fetchID != 0); }
    const std::string& GetURL()      const { return urlText; }
    const std::string& GetPostData() const { return postData; }

  private:
    void ClearRefs(lua_State* L);

  private:
    size_t fetchID;
    string urlText;
    string postData;
    bool success;
    int funcRef; // reference to the callback function
    int selfRef; // reference to this userdata object
};


FetchHandler::FetchHandler(lua_State* L, const char* url, const char* post)
: fetchID(0)
, urlText(url)
, postData((post == NULL) ? "" : post)
, success(false)
, funcRef(LUA_NOREF)
, selfRef(LUA_NOREF)
{
  fetchID = bz_addURLJobForID(url, (bz_BaseURLHandler*)this, post);
  if (fetchID == 0) {
    return;
  }

  lua_pushvalue(L, -2); // push to the top
  funcRef = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_pushvalue(L, -1); // make a copy
  selfRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


FetchHandler::~FetchHandler()
{
  if (fetchID != 0) {
    bz_removeURLJobByID(fetchID);
    fetchID = 0;
  }

  lua_State* L = LuaServer::GetL();
  if (L != NULL) {
    if (funcRef != LUA_NOREF) {
      luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
    }
    if (selfRef != LUA_NOREF) {
      luaL_unref(L, LUA_REGISTRYINDEX, selfRef);
    }
  }
}


void FetchHandler::ClearRefs(lua_State* L)
{
  if (L == NULL) {
    return;
  }
  if (funcRef != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
    funcRef = LUA_NOREF;
  }
  if (selfRef != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, selfRef);
    selfRef = LUA_NOREF;
  }
}


void FetchHandler::done(const char* URL, void* data, unsigned int size,
                        bool /*complete*/)
{
  success = true;
  Handle(URL, data, size, NULL, 0, NULL);
}


void FetchHandler::timeout(const char* URL, int errorCode)
{
  Handle(URL, NULL, 0, "timeout", errorCode, NULL);
}


void FetchHandler::error(const char* URL, int errorCode, const char* errorString)
{
  Handle(URL, NULL, 0, "error", errorCode, errorString);
}


bool FetchHandler::Handle(const char* /*URL*/, void* data, unsigned int size,
                          const char* failType,
                          int errorCode, const char* errorString)
{
  fetchID = 0;

  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return false;
  }

  lua_checkstack(L, 8);

  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, selfRef);
  if (lua_getuserdataextra(L, -1) != metaName) {
    lua_pop(L, 2); 
    return false;
  }

  ClearRefs(L); // clear the references

  int args = 1;
  if (data != NULL) {
    args += 1;
    lua_pushlstring(L, (char*)data, size);
  }
  else {
    args += 3;
    lua_pushnil(L);
    lua_pushstring(L, failType);
    lua_pushinteger(L, errorCode);
    if (errorString != NULL) {
      args += 1;
      lua_pushstring(L, errorString);
    }
  }

  if (lua_pcall(L, args, 0, 0) != 0) {
    bz_debugMessagef(0, "lua call-in fetchurl error (%s): %s\n",
                     urlText.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  return true;
}


bool FetchHandler::Cancel()
{
  bool retval = false;

  if (fetchID != 0) {
    retval = bz_removeURLJobByID(fetchID);
    fetchID = 0;
  }

  ClearRefs(LuaServer::GetL()); // clear the references

  return retval;
}


/******************************************************************************/
/******************************************************************************/

bool FetchURL::PushEntries(lua_State* L)
{
  CreateMetatble(L);

  lua_pushliteral(L, "FetchURL");
  lua_pushcfunction(L, FetchURL_callout);
  lua_rawset(L, -3);

  return true;
}


bool FetchURL::CleanUp(lua_State* /*L*/)
{
  // let the lua __gc() calls do all the work

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int FetchURL_callout(lua_State* L)
{
  int funcIndex = 2;
  const char* urlText  = luaL_checkstring(L, 1);
  const char* postData = NULL;
  if (lua_israwstring(L, 2)) {
    funcIndex++;
    postData = lua_tostring(L, 2);
  }
  lua_settop(L, funcIndex); // discard any extra arguments

  if (!lua_isfunction(L, funcIndex)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, funcIndex); // discard any extras

  void* data = lua_newuserdata(L, sizeof(FetchHandler));
  FetchHandler* fetch = new(data) FetchHandler(L, urlText, postData);
  lua_setuserdataextra(L, -1, (void*)metaName);
  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);

  if (!fetch->IsActive()) {
    return 0;
  }

  return 1;
}

/******************************************************************************/

static inline FetchHandler* CheckHandler(lua_State* L, int index)
{
  if (lua_getuserdataextra(L, index) != metaName) {
    luaL_argerror(L, index, "expected FetchURL");
  }
  return (FetchHandler*)lua_touserdata(L, index);
}


/******************************************************************************/

static bool CreateMetatble(lua_State* L)
{
  luaL_newmetatable(L, metaName);

  lua_pushliteral(L, "__gc"); // garbage collection
  lua_pushcfunction(L, MetaGC);
  lua_rawset(L, -3);

  lua_pushliteral(L, "__index");
  lua_newtable(L);
  {
    PUSH_LUA_CFUNC(L, Cancel);
    PUSH_LUA_CFUNC(L, Success);
    PUSH_LUA_CFUNC(L, IsActive);
    PUSH_LUA_CFUNC(L, GetURL);
    PUSH_LUA_CFUNC(L, GetPostData);
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "__metatable");
  lua_pushliteral(L, "no access");
  lua_rawset(L, -3);

  lua_pop(L, 1); // pop the metatable
  return true;
}


static int MetaGC(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  fetch->~FetchHandler();
  return 0;
}


static int Cancel(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  lua_pushboolean(L, fetch->Cancel());
  return 1;
}


static int Success(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  lua_pushboolean(L, fetch->Success());
  return 1;  
}


static int IsActive(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  lua_pushboolean(L, fetch->IsActive());
  return 1;  
}


static int GetURL(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  lua_pushstring(L, fetch->GetURL().c_str());
  return 1;  
}


static int GetPostData(lua_State* L)
{
  FetchHandler* fetch = CheckHandler(L, 1);
  const string& postData = fetch->GetPostData();
  if (postData.empty()) {
    return 0;
  }
  lua_pushstring(L, postData.c_str());
  return 1;  
}


/******************************************************************************/
/******************************************************************************/
