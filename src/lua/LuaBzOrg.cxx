
#include "common.h"

// interface header
#include "LuaBzOrg.h"

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
#include "bzfio.h"
#include "cURLManager.h"

// bzflag headers
#include "../bzflag/Downloads.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaInclude.h"
#include "LuaUtils.h"


LuaBzOrg* luaBzOrg = NULL;


static const char* sourceFile = "http://trepan.bzflag.bz/LuaBzOrg.lua"; // FIXME


//============================================================================//
//============================================================================//

static class CodeFetch* codeFetch = NULL;


class CodeFetch : private cURLManager {
	public:
		CodeFetch() {
			setURL(sourceFile);
			setGetMode();
			setDeleteOnDone();
			addHandle();
			codeFetch = this;
			LuaLog(1, "LuaBzOrg code fetch started: %s\n", sourceFile);
		}

		~CodeFetch() {
			codeFetch = NULL;
		}

		void finalization(char *data, unsigned int length, bool good) {
			if (luaBzOrg) {
				return;
			}
			if (!good) {
				LuaLog(0, "LuaBzOrg code fetch failed: %s\n", sourceFile);
				return;
			}
			luaBzOrg = new LuaBzOrg(data, length);
			if (luaBzOrg->L == NULL) {
				delete luaBzOrg;
			}
		}
};


//============================================================================//
//============================================================================//

void LuaBzOrg::LoadHandler()
{
	if (!BZDB.isTrue("luaBzOrg")) {
		return;
	}

	if (luaBzOrg || codeFetch) {
		return;
	}

	codeFetch = new CodeFetch();
}


void LuaBzOrg::FreeHandler()
{
	delete codeFetch;
	delete luaBzOrg;
}


bool LuaBzOrg::IsActive()
{
	return ((luaBzOrg != NULL) || (codeFetch != NULL));
}


//============================================================================//
//============================================================================//

LuaBzOrg::LuaBzOrg(const char* code, int length)
: LuaHandle("LuaBzOrg", LUA_BZORG_SCRIPT_ID,
            LUA_BZORG_GAME_ORDER,
            LUA_BZORG_DRAW_WORLD_ORDER,
            LUA_BZORG_DRAW_SCREEN_ORDER,
            true, false, true)
{
	luaBzOrg = this;

	if (L == NULL) {
		return;
	}

	// setup the handle pointer
	L2HH(L)->handlePtr = (LuaHandle**)&luaBzOrg;

	if (!SetupEnvironment()) {
		KillLua();
		return;
	}

	fsRead = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	         BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	         BZVFS_BASIC;
	fsReadAll = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	            BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	            BZVFS_BASIC;
	fsWrite    = BZVFS_LUA_BZORG_WRITE;
	fsWriteAll = BZVFS_LUA_BZORG_WRITE;

	// register for call-ins
	eventHandler.AddClient(this);

	const string sourceCode(code, length);
	if (!ExecSourceCode(sourceCode)) {
		KillLua();
		return;
	}
}


LuaBzOrg::~LuaBzOrg()
{
	if (L != NULL) {
		Shutdown();
		KillLua();
	}
	luaBzOrg = NULL;
}


//============================================================================//
//============================================================================//
