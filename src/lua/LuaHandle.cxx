
#include "common.h"

// interface header
#include "LuaHandle.h"

// system headers
#include <assert.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "bzfio.h"
#include "BzVFS.h"
#include "EventHandler.h"

// local headers
#include "LuaInclude.h"

#include "LuaClientOrder.h"
#include "LuaCallInCheck.h"
#include "LuaCallInDB.h"
#include "LuaHashString.h"
#include "LuaUtils.h"
#include "LuaExtras.h"

// local lua library headers
#include "LuaBitOps.h"
#include "LuaBZDB.h"
#include "LuaBzMaterial.h"
#include "LuaCallOuts.h"
#include "LuaConstGame.h"
#include "LuaConstGL.h"
#include "LuaControl.h"
#include "LuaDouble.h"
#include "LuaDynCol.h"
#include "LuaExtras.h"
#include "LuaKeySyms.h"
//FIXME #include "LuaLink.h"
#include "LuaObstacle.h"
#include "LuaOpenGL.h"
#include "LuaPack.h"
#include "LuaPhyDrv.h"
#include "LuaScream.h"
#include "LuaSpatial.h"
#include "LuaTexMat.h"
#include "LuaURL.h"
#include "LuaVector.h"
#include "LuaVFS.h"


LuaHandle* LuaHandle::activeHandle = NULL;

bool LuaHandle::activeFullRead  = false;
bool LuaHandle::activeInputCtrl = false;

bool LuaHandle::devMode = false;


//============================================================================//
//============================================================================//

LuaHandle::LuaHandle(const string& _name, int _scriptID,
		                 int gameState, int drawWorld, int drawScreen,
                     bool _fullRead, bool _gameCtrl, bool _inputCtrl)
: EventClient(_name, _scriptID,
              gameState, drawWorld, drawScreen,
              _fullRead, _gameCtrl, _inputCtrl)
, requestReload  (false)
, requestDisable (false)
{
	L = lua_open();
	if (L == NULL) {
		return;
	}

	L2HH(L)->handle = this;

	SetupValidCallIns();

	AddBasicCalls();

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	LuaExtras::PushEntries(L);
	lua_pop(L, 1);

	if (devMode) {
		lua_register(L, "dump",    LuaExtras::dump);
		lua_register(L, "listing", LuaExtras::listing);
	}
}


LuaHandle::~LuaHandle()
{
//FIXME	LuaLinkMgr::RemoveEntries(L);

	eventHandler.RemoveClient(this);

	// free the lua state
	KillLua();

	if (this == activeHandle) {
		activeHandle = NULL;
	}

/*
	string msg = GetName();
	if (!requestMessage.empty()) {
		msg += ": " + requestMessage;
	}
	LuaLog(1, "Disabled %s", msg.c_str());
*/
}


void LuaHandle::KillLua()
{
	if (L != NULL) {
		LuaHandle* orig = activeHandle;
		SetActiveHandle();
		lua_close(L);
		SetActiveHandle(orig);
	}
	L = NULL;
}


void LuaHandle::SetupValidCallIns()
{
	validCallIns.clear();

	// setup the validCallIns set
	const LuaCallInDB::InfoMap& ciInfoMap = luaCallInDB.GetInfoMap();
	LuaCallInDB::InfoMap::const_iterator it;
	for (it = ciInfoMap.begin(); it != ciInfoMap.end(); ++it) {
		const LuaCallInDB::CallInInfo& ciInfo = it->second;
		if (!ciInfo.singleScript.empty() &&
		    (ciInfo.singleScript != GetName())) {
			continue;
		}
		if ((ciInfo.reqFullRead  && !HasFullRead()) ||
		    (ciInfo.reqInputCtrl && !HasInputCtrl())) {
			continue;
		}
		validCallIns.insert(luaCallInDB.GetCode(ciInfo.name));
	}
}


//============================================================================//
//============================================================================//

static void CheckEqualStack(const LuaHandle* lh, lua_State* L, int top,
                            const char* tableName)
{
	if (top != lua_gettop(L)) {
		string msg = __FUNCTION__;
		msg += " : " + lh->GetName() + " : ";
		msg += tableName;
		LuaLog(0, "ERROR: %s has an unequal stack\n", msg.c_str());
	}
}


bool LuaHandle::PushLib(const char* name, bool (*entriesFunc)(lua_State*))
{
	const int top = lua_gettop(L);
	lua_pushstring(L, name);
	lua_rawget(L, -2);
	if (lua_istable(L, -1)) {
		LuaHandle* origHandle = activeHandle;
		SetActiveHandle();
		const bool success = entriesFunc(L);
		SetActiveHandle(origHandle);
		lua_pop(L, 1);
		CheckEqualStack(this, L, top, name);
		return success;
	}
	lua_pop(L, 1);

	// make a new table
	lua_pushstring(L, name);
	lua_newtable(L);
	LuaHandle* origHandle = activeHandle;
	SetActiveHandle();
	const bool success = entriesFunc(L);
	SetActiveHandle(origHandle);
	if (!success) {
		lua_pop(L, 2);
		CheckEqualStack(this, L, top, name);
		return false;
	}
	lua_rawset(L, -3);
	CheckEqualStack(this, L, top, name);
	return true;
}


string LuaHandle::LoadSourceCode(const string& sourceFile,
                                 const string& sourceModes)
{
	string modes = sourceModes;
	if (devMode) {
		modes = string(BZVFS_LUA_USER) + modes;
	}
	string code;
	if (!bzVFS.readFile(sourceFile, modes, code)) {
		LuaLog(0, "FAILED to load  '%s'  with  '%s'\n",
		       sourceFile.c_str(), sourceModes.c_str());
		return "";
	}
	return code;
}


bool LuaHandle::ExecSourceCode(const string& code)
{
	int error = luaL_loadbuffer(L, code.c_str(), code.size(), GetName().c_str());

	if (error != 0) {
		LuaLog(0, "Lua LoadCode loadbuffer error = %i, %s, %s\n",
		       error, GetName().c_str(), lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	LuaHandle* orig = activeHandle;
	SetActiveHandle();
	error = lua_pcall(L, 0, 0, 0);
	SetActiveHandle(orig);

	if (error != 0) {
		LuaLog(0, "Lua LoadCode pcall error(%i), %s, %s\n",
		       error, GetName().c_str(), lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}


//============================================================================//
//============================================================================//

void LuaHandle::CheckStack()
{
	const int top = lua_gettop(L);
	if (top != 0) {
		LuaLog(0, "WARNING: %s stack check: top = %i\n", GetName().c_str(), top);
		lua_settop(L, 0);
	}
}


//============================================================================//
//============================================================================//

bool LuaHandle::CanUseCallIn(int code) const
{
	return (validCallIns.find(code) != validCallIns.end());
}


bool LuaHandle::CanUseCallIn(const string& ciName) const
{
	const int code = luaCallInDB.GetCode(luaCallInDB.GetCallInName(ciName));
	if (code == 0)  {
		return false;
	}
	return (validCallIns.find(code) != validCallIns.end());
}


bool LuaHandle::UpdateCallIn(const string& ciName, bool state)
{
	const string& eventName = luaCallInDB.GetEventName(ciName);
	if (state) {
		return eventHandler.InsertEvent(this, eventName);
	} else {
		return eventHandler.RemoveEvent(this, eventName);
	}
}


//============================================================================//
//============================================================================//

static void AddCallInError(lua_State* L, const string& funcName)
{
	// error string is on the top of the stack
	lua_checkstack(L, 4);

	lua_getglobal(L, "ERRORS");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_createtable(L, 0, 0);
		lua_pushvalue(L, -1); // make a copy
		lua_setglobal(L, "ERRORS");
	}

	const size_t len = lua_objlen(L, -1);
	if (len >= 1024) {
		lua_pop(L, 2);
		return;
	}

	lua_createtable(L, 2, 0); {
		lua_pushliteral(L, "func");
		lua_pushstdstring(L, funcName);
		lua_rawset(L, -3);
		lua_pushliteral(L, "error");
		lua_pushvalue(L, -4);
		lua_rawset(L, -3);
	}
	lua_rawseti(L, -2, len + 1);

	lua_pop(L, 2); // also pop the message
}


bool LuaHandle::RunCallIn(int ciCode, int inArgs, int outArgs)
{
	LuaHandle* orig = activeHandle;
	SetActiveHandle();
	const int error = lua_pcall(L, inArgs, outArgs, 0);
	SetActiveHandle(orig);

	if (error != 0) {
		// log the error
		const string* ciName = luaCallInDB.GetName(ciCode);
		const char* ciNameStr = ciName ? ciName->c_str() : "UNKNOWN";
		LuaLog(0, "%s::RunCallIn: error = %i, %s, %s\n",
		       GetName().c_str(), error, ciNameStr, lua_tostring(L, -1));
		// move the error string into CALLIN_ERRORS
		AddCallInError(L, ciNameStr);
		return false;
	}
	return true;
}


bool LuaHandle::RunFunction(const string& funcName, int inArgs, int outArgs)
{
	LuaHandle* orig = activeHandle;
	SetActiveHandle();
	const int error = lua_pcall(L, inArgs, outArgs, 0);
	SetActiveHandle(orig);

	if (error != 0) {
		// log the error
		LuaLog(0, "%s::RunFunction: error = %i, %s, %s\n",
		       GetName().c_str(), error, funcName.c_str(), lua_tostring(L, -1));
		// move the error string into CALLIN_ERRORS
		AddCallInError(L, funcName);
		return false;
	}
	return true;
}


//============================================================================//

bool LuaHandle::PushCallIn(int ciCode)
{
	lua_rawgeti(L, LUA_CALLINSINDEX, ciCode);
	if (lua_isfunction(L, -1)) {
		return true;
	}
	const string* name = luaCallInDB.GetName(ciCode);
	printf("Failed to get: %i %s\n", ciCode, name ? name->c_str() : "UNKNOWN");
	lua_pop(L, 1);
	return false;
}


//============================================================================//
//============================================================================//
//
//  Call-ins
//

//============================================================================//
//============================================================================//

bool LuaHandle::AddBasicCalls()
{
	lua_newtable(L); {
		HSTR_PUSH_CFUNC(L, "Reload",             ScriptReload);
		HSTR_PUSH_CFUNC(L, "Disable",            ScriptDisable);

		HSTR_PUSH_CFUNC(L, "GetName",            ScriptGetName);
		HSTR_PUSH_CFUNC(L, "GetFullRead",        ScriptGetFullRead);
		HSTR_PUSH_CFUNC(L, "GetInputCtrl",       ScriptGetInputCtrl);

		HSTR_PUSH_CFUNC(L, "GetCallInInfo",      ScriptGetCallInInfo);
		HSTR_PUSH_CFUNC(L, "CanUseCallIn",       ScriptCanUseCallIn);
		HSTR_PUSH_CFUNC(L, "SetCallIn",          ScriptSetCallIn);

		HSTR_PUSH_CFUNC(L, "GetDevMode",         ScriptGetDevMode);
		HSTR_PUSH_CFUNC(L, "GetGLOBALS",         ScriptGetGLOBALS);
		HSTR_PUSH_CFUNC(L, "GetCALLINS",         ScriptGetCALLINS);
		HSTR_PUSH_CFUNC(L, "GetREGISTRY",        ScriptGetREGISTRY);

		HSTR_PUSH_CFUNC(L, "PrintPointer",       ScriptPrintPointer);
		HSTR_PUSH_CFUNC(L, "PrintGCInfo",        ScriptPrintGCInfo);
	}
	lua_setglobal(L, "script");

	return true;
}


//============================================================================//

int LuaHandle::ScriptDisable(lua_State* L)
{
	if (activeHandle) {
		const int args = lua_gettop(L);
		if ((args >= 1) && lua_israwstring(L, 1)) {
			activeHandle->requestMessage = lua_tostring(L, 1);
		}
		activeHandle->requestDisable = true;
	}
	return 0;
}


int LuaHandle::ScriptReload(lua_State* L)
{
	if (activeHandle) {
		const int args = lua_gettop(L);
		if ((args >= 1) && lua_israwstring(L, 1)) {
			activeHandle->requestMessage = lua_tostring(L, 1);
		}
		activeHandle->requestReload = true;
	}
	return 0;
}


//============================================================================//

int LuaHandle::ScriptPrintPointer(lua_State* L)
{
	const string prefix = luaL_optstring(L, 2, "PrintPointer: ");
	LuaLog(0, "%s%p\n", prefix.c_str(), lua_topointer(L, 1));
	return 0;
}


int LuaHandle::ScriptPrintGCInfo(lua_State* L)
{
	LuaLog(0, "GCInfo: %.3f MBytes\n", (float)lua_getgccount(L) / 1024.0f);
	return 0;
}


//============================================================================//

int LuaHandle::ScriptGetName(lua_State* L)
{
	lua_pushstring(L, activeHandle->GetName().c_str());
	return 1;
}



int LuaHandle::ScriptGetFullRead(lua_State* L)
{
	lua_pushboolean(L, activeHandle->HasFullRead());
	return 1;
}


int LuaHandle::ScriptGetInputCtrl(lua_State* L)
{
	lua_pushboolean(L, activeHandle->HasInputCtrl());
	return 1;
}


int LuaHandle::ScriptGetDevMode(lua_State* L)
{
	lua_pushboolean(L, devMode);
	return 1;
}


int LuaHandle::ScriptGetGLOBALS(lua_State* L)
{
	if (devMode) {
		lua_pushvalue(L, LUA_GLOBALSINDEX);
		return 1;
	}
	return 0;
}


int LuaHandle::ScriptGetCALLINS(lua_State* L)
{
	if (devMode) {
		lua_pushvalue(L, LUA_CALLINSINDEX);
		return 1;
	}
	return 0;
}


int LuaHandle::ScriptGetREGISTRY(lua_State* L)
{
	if (devMode) {
		lua_pushvalue(L, LUA_REGISTRYINDEX);
		return 1;
	}
	return 0;
}


//============================================================================//

static void PushCallInInfo(lua_State* L, const LuaCallInDB::CallInInfo& ciInfo)
{
	lua_newtable(L);
	LuaPushNamedInt   (L, "code",         ciInfo.code);
	LuaPushNamedBool  (L, "reqFullRead",  ciInfo.reqFullRead);
	LuaPushNamedBool  (L, "reqInputCtrl", ciInfo.reqInputCtrl);
	LuaPushNamedBool  (L, "reversed",     ciInfo.reversed);
	LuaPushNamedString(L, "loopType",     ciInfo.loopType);
	if (LuaHandle::GetDevMode()) {
		lua_pushliteral(L, "func");
		lua_rawgeti(L, LUA_CALLINSINDEX, ciInfo.code);
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			lua_pushboolean(L, false);
		}
		lua_rawset(L, -3);
	}
}


int LuaHandle::ScriptGetCallInInfo(lua_State* L)
{
	vector<string> list;
	eventHandler.GetEventList(list);

	const LuaCallInDB::InfoMap& ciInfoMap = luaCallInDB.GetInfoMap();
	LuaCallInDB::InfoMap::const_iterator it;

	if (lua_israwstring(L, 1)) {
		const string ciName = lua_tostring(L, 1);
		const bool wantAll = lua_isboolean(L, 2) && lua_tobool(L, 2);
		if (wantAll || activeHandle->CanUseCallIn(ciName)) {
			it = ciInfoMap.find(ciName);
			if (it != ciInfoMap.end()) {
				PushCallInInfo(L, it->second);
			}
		} else {
			return 0;
		}
	}
	else {
		const bool wantAll = lua_isboolean(L, 1) && lua_tobool(L, 1);
		lua_createtable(L, 0, ciInfoMap.size());
		for (it = ciInfoMap.begin(); it != ciInfoMap.end(); ++it) {
			const LuaCallInDB::CallInInfo& ciInfo = it->second;
			if (wantAll || activeHandle->CanUseCallIn(ciInfo.name)) {
				lua_pushstring(L, ciInfo.name.c_str());
				PushCallInInfo(L, ciInfo);
				lua_rawset(L, -3);
			}
		}
	}

	return 1;
}


int LuaHandle::ScriptCanUseCallIn(lua_State* L)
{
	const string ciName = lua_tostring(L, 1);
	lua_pushboolean(L, activeHandle->CanUseCallIn(ciName));
	return 1;
}


int LuaHandle::ScriptSetCallIn(lua_State* L)
{
	const string ciName = luaL_checkstring(L, 1);

	const int ciCode = luaCallInDB.GetCode(ciName);
	if (ciCode == 0) {
		if (devMode) {
			LuaLog(1, "Request to update an Unknown call-in (%s)\n", ciName.c_str());
		}
		return 0;
	}

	if (!activeHandle->CanUseCallIn(ciName)) {
		return 0;
	}

	lua_settop(L, 2);
	const bool haveFunc = lua_isfunction(L, 2);

	lua_checkstack(L, 4);

	lua_pushvalue(L, 2); // make a copy
	lua_rawseti(L, LUA_CALLINSINDEX, ciCode);

	lua_pushstring(L, ciName.c_str());
	lua_pushvalue(L, 2); // make a copy
	lua_rawset(L, LUA_CALLINSINDEX);

	const string& eventName = luaCallInDB.GetEventName(ciName);
	if (eventHandler.IsManaged(eventName)) {
		lua_pushboolean(L, activeHandle->UpdateCallIn(ciName, haveFunc));
	} else {
		lua_pushboolean(L, true);
	}

	return 1;
}


//============================================================================//
//============================================================================//

#define LUA_OPEN_LIB(L, lib) \
  lua_pushcfunction((L), (lib)); \
  lua_pcall((L), 0, 0, 0);


bool LuaHandle::SetupEnvironment()
{
	// load the standard libraries
	LUA_OPEN_LIB(L, luaopen_base);
	LUA_OPEN_LIB(L, luaopen_math);
	LUA_OPEN_LIB(L, luaopen_table);
	LUA_OPEN_LIB(L, luaopen_string);
	LUA_OPEN_LIB(L, luaopen_os);
	LUA_OPEN_LIB(L, luaopen_lpeg);
	if (devMode) {
		LUA_OPEN_LIB(L, luaopen_debug);
	}
	//
	// disabled libraries  {io} and {package}
	// NOTE: if {io} is added, disable io.popen()
	//                                 ^^^^^^^^^^
	//LUA_OPEN_LIB(L, luaopen_io);
	//LUA_OPEN_LIB(L, luaopen_package);

	// disable some global functions
	// (these use stdio calls to access file data)
	lua_pushnil(L); lua_setglobal(L, "dofile");
	lua_pushnil(L); lua_setglobal(L, "loadfile");

	// only allow safe {os} functions  (excluding execute(), exit(), etc ...)
	const char* osFuncs[] = {
		"clock",
		"date",
		"time",
		"difftime"
	};
	const int osCount = sizeof(osFuncs) / sizeof(osFuncs[0]);
	lua_newtable(L); // new {os} table
	const int newOS = lua_gettop(L);
	// copy the desired entries
	lua_getglobal(L, "os"); {
		for (int i = 0; i < osCount; i++) {
			lua_getfield(L,    -1, osFuncs[i]);
			lua_setfield(L, newOS, osFuncs[i]);
		}
	}
	lua_pop(L, 1);          // old {os}
	lua_setglobal(L, "os"); // new {os}

	// push the bzflag additions
	lua_pushvalue(L, LUA_GLOBALSINDEX); {
		// into the global table
		if (!LuaExtras::PushEntries(L) ||
		    !LuaDouble::PushEntries(L)) {
			lua_pop(L, 1);
			return false;
		}
		// into sub-tables (using PushLib())
		if (!PushLib("math",   LuaBitOps::PushEntries)   ||
		    !PushLib("math",   LuaVector::PushEntries)   ||
		    !PushLib("url",    LuaURLMgr::PushEntries)   ||
		    !PushLib("vfs",    LuaVFS::PushEntries)      ||
		    !PushLib("bzdb",   LuaBZDB::PushEntries)     ||
		    !PushLib("script", LuaScream::PushEntries)   ||
//FIXME		    !PushLib("link",   LuaLinkMgr::PushEntries)  ||
		    !PushLib("gl",     LuaOpenGL::PushEntries)   ||
		    !PushLib("GL",     LuaConstGL::PushEntries)  ||
		    !PushLib("bz",     LuaPack::PushEntries)     ||
		    !PushLib("bz",     LuaCallOuts::PushEntries) ||
		    !PushLib("BZ",     LuaKeySyms::PushEntries)  ||
		    !PushLib("BZ",     LuaConstGame::PushEntries)) {
			lua_pop(L, 1);
			return false;
		}
		if (HasFullRead()) {
			if (!PushLib("bz", LuaBzMaterial::PushEntries) ||
			    !PushLib("bz", LuaDynCol::PushEntries)     ||
			    !PushLib("bz", LuaTexMat::PushEntries)     ||
			    !PushLib("bz", LuaPhyDrv::PushEntries)     ||
			    !PushLib("bz", LuaSpatial::PushEntries)    ||
			    !PushLib("bz", LuaObstacle::PushEntries)   ||
			    !PushLib("control", LuaControl::PushEntries)) {
				lua_pop(L, 1);
				return false;
			}
		}
	}
	lua_pop(L, 1); // LUA_GLOBALSINDEX
	return true;
}


//============================================================================//
//============================================================================//

