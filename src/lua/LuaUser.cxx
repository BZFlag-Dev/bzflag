
#include "common.h"

// interface header
#include "LuaUser.h"

// system headers
#include <cctype>
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "BzVFS.h"
#include "EventHandler.h"
#include "StateDatabase.h"

// bzflag headers
#include "../bzflag/Downloads.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaInclude.h"
#include "LuaUtils.h"


LuaUser* luaUser = NULL;

static const char* sourceFile = "bzUser.lua";


//============================================================================//
//============================================================================//

void LuaUser::LoadHandler()
{
	if (luaUser) {
		return;
	}

	if (!BZDB.isTrue("luaUser")) {
		return;
	}

	const string& dir = BZDB.get("luaUserDir");
	if (dir.empty() || (dir[0] == '!')) {
		return;
	}

	new LuaUser();

	if (luaUser->L == NULL) {
		delete luaUser;
	}
}


void LuaUser::FreeHandler()
{
	delete luaUser;
}


//============================================================================//
//============================================================================//

LuaUser::LuaUser()
: LuaHandle("LuaUser", ORDER_LUA_USER, devMode, true)
{
	luaUser = this;

	if (L == NULL) {
		return;
	}

	// setup the handle pointer
	L2HH(L)->handlePtr = (LuaHandle**)&luaUser;

	if (!SetupEnvironment()) {
		KillLua();
		return;
	}

	const string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_USER);
	if (sourceCode.empty()) {
		KillLua();
		return;
	}

	fsRead = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	         BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	         BZVFS_BASIC;
	fsReadAll = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	            BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	            BZVFS_BASIC;
	fsWrite    = BZVFS_LUA_USER_WRITE;
	fsWriteAll = BZVFS_LUA_USER_WRITE;

	// register for call-ins
	eventHandler.AddClient(this);

	if (!ExecSourceCode(sourceCode)) {
		KillLua();
		return;
	}
}


LuaUser::~LuaUser()
{
	if (L != NULL) {
		Shutdown();
		KillLua();
	}
	luaUser = NULL;
}


//============================================================================//
//============================================================================//
