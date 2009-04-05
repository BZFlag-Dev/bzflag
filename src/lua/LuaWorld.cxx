
#include "common.h"

// interface header
#include "LuaWorld.h"

// system headers
#include <string>
#include <set>
#include <cctype>
using std::string;
using std::set;

// common headers
#include "BzVFS.h"
#include "EventHandler.h"
#include "StateDatabase.h"

// bzflag headers
#include "../bzflag/Downloads.h"
#include "../bzflag/World.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaInclude.h"
#include "LuaUtils.h"


LuaWorld* luaWorld = NULL;

static const char* sourceFile = "bzWorld.lua";


//============================================================================//
//============================================================================//

void LuaWorld::LoadHandler()
{
	if (luaWorld) {
		return;
	}

	const World* world = World::getWorld();
	if (world == NULL) {
		return;
	}

	if (!BZDB.isTrue("luaWorld") && !world->luaWorldRequired()) {
		return;
	}

	new LuaWorld();

	if (luaWorld->L == NULL) {
		delete luaWorld;
	}
}


void LuaWorld::FreeHandler()
{
	delete luaWorld;
}


//============================================================================//
//============================================================================//

LuaWorld::LuaWorld()
: LuaHandle("LuaWorld", LUA_WORLD_SCRIPT_ID,
            LUA_WORLD_GAME_ORDER,
            LUA_WORLD_DRAW_WORLD_ORDER,
            LUA_WORLD_DRAW_SCREEN_ORDER,
            true, World::getWorld()->luaWorldRequired(), true)
{
	luaWorld = this;

	if (L == NULL) {
		return;
	}

	// setup the handle pointer
	L2HH(L)->handlePtr = (LuaHandle**)&luaWorld;

	if (!SetupEnvironment()) {
		KillLua();
		return;
	}

	const string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_WORLD);
	if (sourceCode.empty()) {
		KillLua();
		return;
	}

	fsRead = BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	         BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	         BZVFS_BASIC;
	fsReadAll = BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	            BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	            BZVFS_BASIC;
	fsWrite    = BZVFS_LUA_WORLD_WRITE;
	fsWriteAll = BZVFS_LUA_WORLD_WRITE;

	// register for call-ins
	eventHandler.AddClient(this);

	if (!ExecSourceCode(sourceCode)) {
		KillLua();
		return;
	}
}


LuaWorld::~LuaWorld()
{
	if (L != NULL) {
		Shutdown();
		KillLua();
	}
	luaWorld = NULL;
}


//============================================================================//
//============================================================================//
