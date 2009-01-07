
#include "common.h"

// implementation header
#include "LuaUser.h"

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

// local headers
#include "LuaEventOrder.h"
#include "LuaInclude.h"
#include "LuaUtils.h"

#include "LuaCallInCheck.h"
#include "LuaCallInDB.h"
#include "LuaCallOuts.h"
#include "LuaUtils.h"
#include "LuaBitOps.h"
#include "LuaOpenGL.h"
#include "LuaConstGL.h"
#include "LuaConstGame.h"
#include "LuaKeySyms.h"
#include "LuaSpatial.h"
#include "LuaObstacle.h"
#include "LuaScream.h"
#include "LuaVFS.h"
#include "LuaBZDB.h"
#include "LuaPack.h"
#include "LuaExtras.h"
#include "LuaVector.h"
#include "LuaBzMaterial.h"
#include "LuaDynCol.h"
#include "LuaTexMat.h"
#include "LuaPhyDrv.h"


LuaUser* luaUser = NULL;

static const char* sourceFile = "bzUser.lua";


/******************************************************************************/
/******************************************************************************/

#include "BzVFS.h"

static void bzdbCallback(const std::string& name, void* /*data*/)
{
	bzVFS.removeFS(BZVFS_LUA_USER);
	bzVFS.addFS(BZVFS_LUA_USER, BZDB.get(name));
	LuaUser::FreeHandler();
	LuaUser::LoadHandler();
}


/******************************************************************************/
/******************************************************************************/

void LuaUser::LoadHandler()
{
	static bool needCallback = true;
	if (needCallback) {
		needCallback = false;
		BZDB.addCallback("LuaUserDir", bzdbCallback, NULL);
	}

	if (luaUser) {
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


/******************************************************************************/
/******************************************************************************/

LuaUser::LuaUser()
: LuaHandle("LuaUser", ORDER_LUA_USER, false, true)
{
	luaUser = this;

	if (L == NULL) {
		return;
	}

	// setup the handle pointer
	L2HH(L)->handlePtr = (LuaHandle**)&luaUser;

	if (!SetupLuaLibs()) {
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
	fsWrite = BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE;

	if (!ExecSourceCode(sourceCode)) {
		KillLua();
		return;
	}

	// register for call-ins
	eventHandler.AddClient(this);
}


LuaUser::~LuaUser()
{
	if (L != NULL) {
		Shutdown();
		KillLua();
	}
	luaUser = NULL;
}


/******************************************************************************/
/******************************************************************************/

#define LUA_OPEN_LIB(L, lib)   \
  lua_pushcfunction((L), lib); \
  lua_pcall((L), 0, 0, 0); 


bool LuaUser::SetupLuaLibs()
{
	// load the standard libraries
	LUA_OPEN_LIB(L, luaopen_base);
	LUA_OPEN_LIB(L, luaopen_math);
	LUA_OPEN_LIB(L, luaopen_table);
	LUA_OPEN_LIB(L, luaopen_string);
	LUA_OPEN_LIB(L, luaopen_io);
	LUA_OPEN_LIB(L, luaopen_os);
	LUA_OPEN_LIB(L, luaopen_package);
	LUA_OPEN_LIB(L, luaopen_debug);

	// remove a few dangerous calls
	lua_getglobal(L, "io");
	lua_pushstring(L, "popen"); lua_pushnil(L); lua_rawset(L, -3);
	lua_pop(L, 1); // io
	lua_getglobal(L, "os");
	lua_pushstring(L, "exit");      lua_pushnil(L); lua_rawset(L, -3);
	lua_pushstring(L, "execute");   lua_pushnil(L); lua_rawset(L, -3);
	lua_pushstring(L, "setlocale"); lua_pushnil(L); lua_rawset(L, -3);
	lua_pop(L, 1); // os

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	if (!LuaExtras::PushEntries(L)) {
		lua_pop(L, 1);
		return false;
	}
	lua_pop(L, 1);

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	if (!PushLib("math",   LuaBitOps::PushEntries)     ||
	    !PushLib("math",   LuaVector::PushEntries)     ||
	    !PushLib("VFS",    LuaVFS::PushEntries)        ||
	    !PushLib("BZDB",   LuaBZDB::PushEntries)       ||
	    !PushLib("pack",   LuaPack::PushEntries)       ||
	    !PushLib("Script", LuaScream::PushEntries)     ||
	    !PushLib("gl",     LuaOpenGL::PushEntries)     ||
	    !PushLib("GL",     LuaConstGL::PushEntries)    ||
	    !PushLib("bz",     LuaBzMaterial::PushEntries) ||
	    !PushLib("bz",     LuaDynCol::PushEntries)     ||
	    !PushLib("bz",     LuaTexMat::PushEntries)     ||
	    !PushLib("bz",     LuaPhyDrv::PushEntries)     ||
	    !PushLib("bz",     LuaCallOuts::PushEntries)   ||
	    !PushLib("bz",     LuaSpatial::PushEntries)    ||
	    !PushLib("bz",     LuaObstacle::PushEntries)   ||
	    !PushLib("BZ",     LuaKeySyms::PushEntries)    ||
	    !PushLib("BZ",     LuaConstGame::PushEntries)) {
		KillLua();
	}
	lua_pop(L, 1); // LUA_GLOBALSINDEX

	return true;
}


/******************************************************************************/
/******************************************************************************/
