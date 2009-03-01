
#include "common.h"

// interface header
#include "LuaURL.h"

// system headers
#include <new>
#include <time.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "bzfio.h"
#include "network.h"
#include "AccessList.h"
#include "CacheManager.h"

// local headers
#include "LuaInclude.h"
#include "LuaHandle.h"


const char* LuaURLMgr::metaName = "FetchURL";

AccessList* LuaURLMgr::accessList = NULL;


//============================================================================//
//============================================================================//

LuaURL::LuaURL(lua_State* _L, const std::string& _url)
: L(_L)
, active(false)
, success(false)
, wantCache(false) // FIXME -- use it?
, headOnly(false)
, fileSize(-1.0)
, fileTime(0)
, httpCode(0)
, url(_url)
, postData("")
, funcRef(LUA_NOREF)
, selfRef(LUA_NOREF)
{
	selfRef = luaL_ref(L, LUA_REGISTRYINDEX);

	int funcArg = 2;
	if (lua_israwstring(L, 2)) {
		postData = lua_tostring(L, 2);
		funcArg++;
	}
	else if (lua_istable(L, 2)) {
		const int table = funcArg;
		funcArg++;
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string key = lua_tostring(L, -2);
				if (key == "post") {
					postData = luaL_checkstring(L, -1);
				}
				else if (key == "timeout") {
					const long timeout = (long)luaL_checkint(L, -1);
					setTimeout(timeout);
				}
				else if (key == "head") {
					luaL_checktype(L, -1, LUA_TBOOLEAN);
					if (lua_tobool(L, -1)) {
					  setNoBody();
					}
				}
				else if (key == "failOnError") {
					luaL_checktype(L, -1, LUA_TBOOLEAN);
					if (lua_tobool(L, -1)) {
					  setFailOnError();
					}
				}
			}
		}
	}

	if (lua_isfunction(L, funcArg)) {
		lua_pushvalue(L, funcArg);
		funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	setURL(url);

	if (headOnly) {
		setNoBody();
	}

	if (postData.empty()) {
		setGetMode();
	}
	else {
		setHTTPPostMode();
		setPostMode(postData);
	}

	setRequestFileTime(true);

	addHandle();
	active = true;
}


LuaURL::~LuaURL()
{
	LuaLog(4, "LuaURL: deleting %s userdata\n", url.c_str()); // FIXME?
	if (active) {
		removeHandle();
	}
	ClearRefs();
}


void LuaURL::ClearRefs()
{
	if (funcRef != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
		funcRef = LUA_NOREF;
	}
	if (selfRef != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, selfRef);
		selfRef = LUA_NOREF;
	}
}


void LuaURL::finalization(char* data, unsigned int length, bool good)
{
	active = false;

	getFileTime(fileTime);
	getFileSize(fileSize);
	getHttpCode(httpCode);

	if ((funcRef == LUA_NOREF) || (selfRef == LUA_NOREF)) {
		ClearRefs();
		return;
	}

	lua_checkstack(L, 4);
	lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		ClearRefs();
		return;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, selfRef);
	if (!LuaURLMgr::TestURL(L, -1)) {
		lua_pop(L, 2);
		ClearRefs();
		return;
	}

	ClearRefs();

	lua_pushboolean(L, good);

	if (!good) {
		lua_pushliteral(L, "failed");
	} else {
		success = true;
		lua_pushlstring(L, data, length);
	}

	// call the function
	L2H(L)->RunFunction("LuaURL", 3, 0);
}


bool LuaURL::Cancel()
{
	if (!active) {
		return false;
	}
	active = false;
	removeHandle();
	ClearRefs();
	return true;
}


bool LuaURL::PushFunc(lua_State* _L) const
{
	if (L != _L) {
		return false;
	}
	lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	return true;
}


//============================================================================//
//============================================================================//
//
//  LuaURLMgr
//

bool LuaURLMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, Fetch);
	PUSH_LUA_CFUNC(L, Cancel);
	PUSH_LUA_CFUNC(L, Length);
	PUSH_LUA_CFUNC(L, Success);
	PUSH_LUA_CFUNC(L, IsActive);
	PUSH_LUA_CFUNC(L, GetURL);
	PUSH_LUA_CFUNC(L, GetPostData);
	PUSH_LUA_CFUNC(L, GetCallback);
	PUSH_LUA_CFUNC(L, GetFileSize);
	PUSH_LUA_CFUNC(L, GetFileTime);
	PUSH_LUA_CFUNC(L, GetHttpCode);

	return true;
}


void LuaURLMgr::SetAccessList(AccessList* list)
{
	accessList = list;
}


//============================================================================//
//============================================================================//

const LuaURL* LuaURLMgr::TestURL(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaURL*)lua_touserdata(L, index);
}


const LuaURL* LuaURLMgr::CheckURL(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected URL");
	}
	return (LuaURL*)lua_touserdata(L, index);
}


LuaURL* LuaURLMgr::GetURL(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected URL");
	}
	return (LuaURL*)lua_touserdata(L, index);
}


//============================================================================//
//============================================================================//

bool LuaURLMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);

	lua_pushliteral(L, "__gc"); // garbage collection
	lua_pushcfunction(L, MetaGC);
	lua_rawset(L, -3);

	lua_pushliteral(L, "__index");
	lua_newtable(L);
	{
		PUSH_LUA_CFUNC(L, Cancel);
		PUSH_LUA_CFUNC(L, Length);
		PUSH_LUA_CFUNC(L, Success);
		PUSH_LUA_CFUNC(L, IsActive);
		PUSH_LUA_CFUNC(L, GetURL);
		PUSH_LUA_CFUNC(L, GetPostData);
		PUSH_LUA_CFUNC(L, GetCallback);
		PUSH_LUA_CFUNC(L, GetFileSize);
		PUSH_LUA_CFUNC(L, GetFileTime);
		PUSH_LUA_CFUNC(L, GetHttpCode);
	}
	lua_rawset(L, -3);

	lua_pushliteral(L, "__metatable");
	lua_pushliteral(L, "no access");
	lua_rawset(L, -3);

	lua_pop(L, 1); // pop the metatable
	return true;
}


int LuaURLMgr::MetaGC(lua_State* L)
{
	LuaURL* fetch = GetURL(L, 1);
	fetch->~LuaURL();
	return 0;
}


//============================================================================//
//============================================================================//

static int ParseURL(lua_State* L, const string& url, string& hostname)
{
	string protocol, path;
	int port;
	if (!BzfNetwork::parseURL(url, protocol, hostname, port, path)) {
		lua_pushnil(L);
		lua_pushliteral(L, "bad URL, http:// or ftp:// is required");
		return 2;
	}
	if ((protocol != "http") && (protocol != "ftp")) {
		lua_pushnil(L);
		lua_pushliteral(L, "bad URL, http:// or ftp:// is required");
		return 2;
	}
	return 0;
}


//============================================================================//
//============================================================================//

int LuaURLMgr::Fetch(lua_State* L)
{
	string url = luaL_checkstring(L, 1);

	// default to http
	if (url.find("://") == string::npos) {
		url = "http://" + url;
	}

	string hostname;
	const int parseArgs = ParseURL(L, url, hostname);
	if (parseArgs) {
		return parseArgs;
	}

	if (accessList && !accessList->alwaysAuthorized()) {
		vector<string> hostnames;
		hostnames.push_back(hostname);
		if (!accessList->authorized(hostnames)) {
			lua_pushnil(L);
			lua_pushliteral(L, "site blocked by DownloadAccess.txt");
			return 2;
		}
	}

	// create the userdata
	void* data = lua_newuserdata(L, sizeof(LuaURL));
	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1); // push a copy of the userdata for
					              // the LuaURL constructor to reference

	LuaURL* fetch = new(data) LuaURL(L, url);
	if (!fetch->GetActive()) {
		fetch->~LuaURL();
		return 0;
	}

	return 1;
}


//============================================================================//
//============================================================================//

int LuaURLMgr::Cancel(lua_State* L)
{
	LuaURL* fetch = GetURL(L, 1);
	lua_pushboolean(L, fetch->Cancel());
	return 1;
}


int LuaURLMgr::Length(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	lua_pushinteger(L, fetch->GetLength());
	return 1;  
}


int LuaURLMgr::Success(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	lua_pushboolean(L, fetch->GetSuccess());
	return 1;  
}


int LuaURLMgr::IsActive(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	lua_pushboolean(L, fetch->GetActive());
	return 1;  
}


int LuaURLMgr::GetURL(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	lua_pushstdstring(L, fetch->GetURL());
	return 1;  
}


int LuaURLMgr::GetPostData(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	const string& postData = fetch->GetPostData();
	if (postData.empty()) {
		return 0;
	}
	lua_pushstdstring(L, postData);
	return 1;  
}


int LuaURLMgr::GetCallback(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	if (!fetch->PushFunc(L)) {
		return 0;
	}
	return 1;
}


int LuaURLMgr::GetFileSize(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);
	lua_pushnumber(L, (float)fetch->GetFileSize());
	return 1;
}


int LuaURLMgr::GetFileTime(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);

	const time_t t = fetch->GetFileTime();
	struct tm* tmPtr = gmtime(&t);
	if (tmPtr == NULL) {
		return 0;
	}

	char buf[256];
	int len;

	len = strftime(buf, sizeof(buf), "%Y-%m-%d/%H:%M:%S", tmPtr);
	lua_pushlstring(L, buf, len);

	snprintf(buf, sizeof(buf), "%lu", t);
	lua_pushstring(L, buf);

	return 2;
}


int LuaURLMgr::GetHttpCode(lua_State* L)
{
	const LuaURL* fetch = CheckURL(L, 1);

	const long code = fetch->GetHttpCode();
	lua_pushinteger(L, (int)code);

	return 1;
}


//============================================================================//
//============================================================================//
