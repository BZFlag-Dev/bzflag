
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "fetchurl.h"

#include <string>
#include <set>
#include <map>
using std::string;
using std::set;
using std::map;


static const char* MetaTag = "FetchURL";

static lua_State* L = NULL;

static int FetchURL_func(lua_State* L);

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
    size_t fetchID;
    string urlText;
    string postData;
    bool success;
    int funcRef;
    int userdataRef;
};


FetchHandler::FetchHandler(lua_State* L, const char* url, const char* post)
: fetchID(0)
, urlText(url)
, postData("")
, success(false)
, funcRef(LUA_NOREF)
, userdataRef(LUA_NOREF)
{
  if (!lua_isfunction(L, -1)) {
    return;
  }
  funcRef = luaL_ref(L, LUA_REGISTRYINDEX);

  FetchHandler** fetchPtr =
    (FetchHandler**)lua_newuserdata(L, sizeof(FetchHandler*));
  *fetchPtr = this;
  luaL_getmetatable(L, MetaTag);
  lua_setmetatable(L, -2);
  lua_pushvalue(L, -1); // make a copy
  userdataRef = luaL_ref(L, LUA_REGISTRYINDEX);
  
  urlText = url;
  postData = (post == NULL) ? "" : post;

  fetchID = bz_addURLJobForID(url, (bz_BaseURLHandler*)this, post);
}


FetchHandler::~FetchHandler()
{
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
    luaL_unref(L, LUA_REGISTRYINDEX, userdataRef);
  }
  if (fetchID != 0) {
    bz_removeURLJobByID(fetchID);
    fetchID = 0;
  }
}


void FetchHandler::done(const char* URL, void* data, unsigned int size,
                        bool complete)
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


bool FetchHandler::Handle(const char* URL, void* data, unsigned int size,
                          const char* failType,
                          int errorCode, const char* errorString)
{
  fetchID = 0;

  if (L == NULL) {
    return false;
  }

  lua_checkstack(L, 8);

  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return false;
  }

  int args = 1;
  lua_rawgeti(L, LUA_REGISTRYINDEX, userdataRef);
  if (!lua_isuserdata(L, -1)) {
    lua_pop(L, 2); 
    delete this;
    return false;
  }

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
  if (fetchID == 0) {
    return false;
  }

  const bool success = bz_removeURLJobByID(fetchID);

  fetchID = 0;

  return success;
}


/******************************************************************************/
/******************************************************************************/

bool FetchURL::PushEntries(lua_State* _L)
{
  L = _L;

  CreateMetatble(L);

  lua_pushliteral(L, "FetchURL");
  lua_pushcfunction(L, FetchURL_func);
  lua_rawset(L, -3);

  return true;
}


bool FetchURL::CleanUp(lua_State* _L)
{
  // let the lua __gc() calls do all the work

  L = NULL;

  return true; // do nothing
}


/******************************************************************************/
/******************************************************************************/

static int FetchURL_func(lua_State* L)
{
  int funcIndex = 2;
  const char* urlText  = luaL_checkstring(L, 1);
  const char* postData = NULL;
  if (lua_israwstring(L, 2)) {
    funcIndex++;
    postData = lua_tostring(L, 2);
  }

  if (!lua_isfunction(L, funcIndex)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, funcIndex); // discard any extras

  // if fetch->IsActive() is true, this will push a userdata
  //  on to the top of the stack. Otherwise, return a nil.
  FetchHandler* fetch = new FetchHandler(L, urlText, postData);
  if (!fetch->IsActive()) {
    delete fetch;
    return 0;
  }
  return 1;
}

/******************************************************************************/

static bool CreateMetatble(lua_State* L)
{
  luaL_newmetatable(L, MetaTag);

  lua_pushstring(L, "__gc"); // garbage collection
  lua_pushcfunction(L, MetaGC);
  lua_rawset(L, -3);

  lua_pushstring(L, "__index");
  lua_newtable(L);
  {
    PUSH_LUA_CFUNC(L, Cancel);
    PUSH_LUA_CFUNC(L, Success);
    PUSH_LUA_CFUNC(L, IsActive);
    PUSH_LUA_CFUNC(L, GetURL);
    PUSH_LUA_CFUNC(L, GetPostData);
  }
  lua_rawset(L, -3);

  lua_pop(L, 1); // pop the metatable
  return true;
}


static inline FetchHandler* GetHandler(lua_State* L, int index)
{
  FetchHandler** fetchPtr =
    (FetchHandler**)luaL_checkudata(L, index, MetaTag);
  if (fetchPtr == NULL) {
    luaL_error(L, "internal FetchHandler error");
    return NULL;
  }
  return *fetchPtr;
}


static int MetaGC(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  delete fetch;
  return 0;
}


static int Cancel(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  lua_pushboolean(L, fetch->Cancel());
  return 1;
}


static int Success(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  lua_pushboolean(L, fetch->Success());
  return 1;  
}


static int IsActive(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  lua_pushboolean(L, fetch->IsActive());
  return 1;  
}


static int GetURL(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  lua_pushstring(L, fetch->GetURL().c_str());
  return 1;  
}


static int GetPostData(lua_State* L)
{
  FetchHandler* fetch = GetHandler(L, 1);
  const string& postData = fetch->GetPostData();
  if (postData.empty()) {
    return 0;
  }
  lua_pushstring(L, postData.c_str());
  return 1;  
}


/******************************************************************************/
/******************************************************************************/
