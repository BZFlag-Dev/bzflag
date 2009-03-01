
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
			logDebugMessage(1, "LuaBzOrg code fetch started: %s\n", sourceFile);
		}

		~CodeFetch() {
			codeFetch = NULL;
		}

		void finalization(char *data, unsigned int length, bool good) {
			if (luaBzOrg) {
				return;
			}
			if (!good) {
				logDebugMessage(0, "LuaBzOrg code fetch failed: %s\n", sourceFile);
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
: LuaHandle("LuaBzOrg", ORDER_LUA_BZORG, true, true)
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

	// FIXME -- source from:  http://lua.bzflag.org/bzOrg.lua
	const string sourceCode(code, length);

	fsRead = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	         BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	         BZVFS_BASIC;
	fsReadAll = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	            BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	            BZVFS_BASIC;
	fsWrite    = BZVFS_LUA_BZORG_WRITE;
	fsWriteAll = BZVFS_LUA_BZORG_WRITE;

	if (!ExecSourceCode(sourceCode)) {
		logDebugMessage(1, "LuaBzOrg code:\n%s\n", sourceCode.c_str());
		KillLua();
		return;
	}

	// register for call-ins
	eventHandler.AddClient(this);
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
