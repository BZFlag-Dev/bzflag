
#include "common.h"

// implementation header
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

#include "LuaEventOrder.h"
#include "LuaCallInCheck.h"
#include "LuaCallInDB.h"
#include "LuaHashString.h"
#include "LuaUtils.h"
#include "LuaExtras.h"


LuaHandle* LuaHandle::activeHandle = NULL;

bool LuaHandle::activeFullRead  = false;
bool LuaHandle::activeInputCtrl = false;

bool LuaHandle::devMode = false;


/******************************************************************************/
/******************************************************************************/

LuaHandle::LuaHandle(const string& _name, int _order,
                     bool _fullRead, bool _inputCtrl)
: EventClient(_name, _order, _fullRead, _inputCtrl)
, requestReload  (false)
, requestDisable (false)
{
	L = lua_open();
	if (L == NULL) {
		return;
	}

	L2HH(L)->handle = this;

	printf("FULLREAD  %s:  %s\n", GetName().c_str(), HasFullRead() ? "true" : "false");
	printf("INPUTCTRL %s:  %s\n", GetName().c_str(), HasInputCtrl() ? "true" : "false");

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

	AddBasicCalls();
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	LuaExtras::PushEntries(L);
	lua_pop(L, 1);

	if (devMode) { // FIXME
		lua_register(L, "dump",    LuaExtras::dump); 
		lua_register(L, "listing", LuaExtras::listing);
	}
}


LuaHandle::~LuaHandle()
{
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
	LuaLog("Disabled %s", msg.c_str());
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


/******************************************************************************/
/******************************************************************************/

static void CheckEqualStack(const LuaHandle* lh, lua_State* L, int top,
                            const char* tableName)
{
  if (top != lua_gettop(L)) {
    string msg = __FUNCTION__;
    msg += " : " + lh->GetName() + " : ";
    msg += tableName;
//FIXME    throw std::runtime_error(msg);
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
	string code;
	string modes = !devMode ? sourceModes
	                        : string(BZVFS_LUA_USER) + sourceModes;
	if (!bzVFS.readFile(sourceFile, modes, code)) {
		printf("FAILED to load  '%s'  with  '%s'\n",
		       sourceFile.c_str(), sourceModes.c_str());//FIXME
		return "";
	}
	return code;
}


bool LuaHandle::ExecSourceCode(const string& code)
{
	int error = luaL_loadbuffer(L, code.c_str(), code.size(), GetName().c_str());

	if (error != 0) {
		LuaLog("Lua LoadCode loadbuffer error = %i, %s, %s\n",
		       error, GetName().c_str(), lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	LuaHandle* orig = activeHandle;
	SetActiveHandle();
	error = lua_pcall(L, 0, 0, 0);
	SetActiveHandle(orig);

	if (error != 0) {
		LuaLog("Lua LoadCode pcall error = %i, %s, %s\n",
		       error, GetName().c_str(), lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::CheckStack()
{
	const int top = lua_gettop(L);
	if (top != 0) {
		LuaLog("WARNING: %s stack check: top = %i\n", GetName().c_str(), top);
		lua_settop(L, 0);
	}
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::UpdateCallIn(const string& ciName, bool state)
{
	if (state) {
		eventHandler.InsertEvent(this, ciName);
	} else {
		eventHandler.RemoveEvent(this, ciName);
	}
}


bool LuaHandle::GlobalCallInCheck(const string& ciName)
{
	if (L == NULL) {
		return false;
	}

	if (!CanUseCallIn(ciName)) {
		return false;
	}

	const int ciCode = luaCallInDB.GetCode(ciName);
	if (ciCode == 0) {
		return false;
	}

	lua_checkstack(L, 4);
	lua_getglobal(L, ciName.c_str());
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	lua_pushvalue(L, -1); // make a copy
	lua_rawseti(L, LUA_CALLINSINDEX, ciCode);

	lua_pushstring(L, ciName.c_str());
	lua_insert(L, -2); // move the function below the string
  lua_rawset(L, LUA_CALLINSINDEX);

	UpdateCallIn(ciName, true);

	return true;
}


bool LuaHandle::CanUseCallIn(int code) const
{
	return (validCallIns.find(code) != validCallIns.end());
}


bool LuaHandle::CanUseCallIn(const string& name) const
{
	const int code = luaCallInDB.GetCode(name);
	if (code == 0)  {
		return false;
	}	
	return (validCallIns.find(code) != validCallIns.end());
}


/******************************************************************************/
/******************************************************************************/

bool LuaHandle::RunCallIn(int ciCode, int inArgs, int outArgs)
{
	LuaHandle* orig = activeHandle;
	SetActiveHandle();
	const int error = lua_pcall(L, inArgs, outArgs, 0);
	SetActiveHandle(orig);

	if (error != 0) {
	  const string* ciName = luaCallInDB.GetName(ciCode);
	  const char* ciNameStr = ciName ? ciName->c_str() : "UNKNOWN";
		LuaLog("%s::RunCallIn: error = %i, %s, %s\n", GetName().c_str(),
		                error, ciNameStr, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}
	return true;
}


/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/
//
//  Call-ins
//

/******************************************************************************/

void LuaHandle::HandleLuaMsg(int playerID, int script, int mode,
                              const string& msg)
{
	playerID = script + mode + msg.size();
/* FIXME -- HandleLuaMsg
	if (script == ORDER_LUA_USER) {
		if (luaUser) {
			bool sendMsg = false;
			if (mode == 0) {
				sendMsg = true;
			}
			else if (mode == 's') {
				sendMsg = gu->spectating;
			}
			else if (mode == 'a') {
				const CPlayer* player = gs->players[playerID];
				if (player == NULL) {
					return;
				}
				if (gu->spectatingFullView) {
					sendMsg = true;
				}
				else if (player->spectator) {
					sendMsg = gu->spectating;
				} else {
					const int msgAllyTeam = gs->AllyTeam(player->team);
					sendMsg = gs->Ally(msgAllyTeam, gu->myAllyTeam);
				}
			}
			if (sendMsg) {
				luaUI->RecvLuaMsg(msg, playerID);
			}
		}
	}
	else if (script == ORDER_LUA_WORLD) {
		if (luaWorld) {
			luaWorld->RecvLuaMsg(msg, playerID);
		}
	}
*/
}


/******************************************************************************/
/******************************************************************************/

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

		HSTR_PUSH_CFUNC(L, "GetGLOBALS",         ScriptGetGLOBALS);
		HSTR_PUSH_CFUNC(L, "GetCALLINS",         ScriptGetCALLINS);
		HSTR_PUSH_CFUNC(L, "GetREGISTRY",        ScriptGetREGISTRY);

		HSTR_PUSH_CFUNC(L, "PrintPointer",       ScriptPrintPointer);
		HSTR_PUSH_CFUNC(L, "PrintGCInfo",        ScriptPrintGCInfo);
	}
	lua_setglobal(L, "Script");

	return true;
}


/******************************************************************************/

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


/******************************************************************************/

int LuaHandle::ScriptPrintPointer(lua_State* L)
{
	const string prefix = luaL_optstring(L, 2, "PrintPointer: ");
	LuaLog("%s%p\n", prefix.c_str(), lua_topointer(L, 1));
	return 0;
}


int LuaHandle::ScriptPrintGCInfo(lua_State* L)
{
	LuaLog("GCInfo: %.3f MBytes\n", (float)lua_getgccount(L) / 1024.0f);
	return 0;
}


/******************************************************************************/

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


/******************************************************************************/

static void PushCallInInfo(lua_State* L, const LuaCallInDB::CallInInfo& ciInfo)
{
	lua_newtable(L);
	LuaPushNamedNumber(L, "code",         ciInfo.code);
	LuaPushNamedBool  (L, "reqFullRead",  ciInfo.reqFullRead);
	LuaPushNamedBool  (L, "reqInputCtrl", ciInfo.reqInputCtrl);
	LuaPushNamedBool  (L, "reversed",     ciInfo.reversed);
	LuaPushNamedString(L, "loopType",     ciInfo.loopType);
}


int LuaHandle::ScriptGetCallInInfo(lua_State* L)
{
	vector<string> list;
	eventHandler.GetEventList(list);

	const LuaCallInDB::InfoMap& ciInfoMap = luaCallInDB.GetInfoMap();
	LuaCallInDB::InfoMap::const_iterator it;
	
	const LuaHandle* lh = GetActiveHandle();

	if (lua_israwstring(L, 1)) {
		const string ciName = lua_tostring(L, 1);
		const bool wantAll = lua_isboolean(L, 2) && lua_tobool(L, 2);
		if (wantAll || lh->CanUseCallIn(ciName)) {
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
			if (wantAll || lh->CanUseCallIn(ciInfo.name)) {
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
	lua_pushboolean(L, GetActiveHandle()->CanUseCallIn(ciName));
	return 1;
}


int LuaHandle::ScriptSetCallIn(lua_State* L)
{
	const string ciName = luaL_checkstring(L, 1);

	if (!GetActiveHandle()->CanUseCallIn(ciName)) {
		return 0;
	}
	const int ciCode = luaCallInDB.GetCode(ciName);
	if (ciCode == 0) {
    if (devMode) {
      LuaLog("Request to update an Unknown call-in (%s)\n",
             ciName.c_str());
    }
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

	activeHandle->UpdateCallIn(ciName, haveFunc);

	lua_pushboolean(L, true);
	return 1;
}


/******************************************************************************/
/******************************************************************************/

