
#include "common.h"

// system headers
#include <string.h>
#include <string>
#include <map>
#include <stdexcept>
using std::string;
using std::map;

// local headers
#include "LuaCallInDB.h"
#include "LuaInclude.h"


LuaCallInDB luaCallInDB;


/******************************************************************************/
/******************************************************************************/

bool LuaCallInDB::SetupCallIn(int code, const string& name)
{
	map<int, string>::const_iterator codeIt = codeToName.find(code);
	if (codeIt != codeToName.end()) {
//FIXME		throw std::runtime_error("duplicated lua call-in code: "
//		                         + IntToString(code) + 
//		                         + "  (" + name + " vs. " + codeIt->second + ")");
		return false;
	}

	map<string, int>::const_iterator nameIt = nameToCode.find(name);
	if (nameIt != nameToCode.end()) {
//FIXME		throw std::runtime_error("duplicate lua call-in name: " + name + "  ("
//		                         + IntToString(code) + " vs. "
//		                         + IntToString(code) + ")");
		return false;
	}
	
	codeToName[code] = name;
	nameToCode[name] = code;
	return true;
}


LuaCallInDB::LuaCallInDB()
{
	// NOTE: less chance of typos doing it this way
	const int NO_REQS        = 0;
	const int REQ_FULL_READ  = (1 << 0);
	const int REQ_INPUT_CTRL = (1 << 1);

	// loopType
	const char* BASIC        = "BASIC";
	const char* FIRST_TRUE   = "FIRST_TRUE";
//	const char* FIRST_FALSE  = "FIRST_FALSE";
//	const char* FIRST_NUMBER = "FIRST_NUMBER";
	const char* FIRST_STRING = "FIRST_STRING";
//	const char* BOOLEAN_OR   = "BOOLEAN_OR";
	const char* SPECIAL      = "SPECIAL";

	// singleScript	
	const char* ANY_SCRIPT      = "";
//	const char* ONLY_LuaUser    = "LuaUser";
//	const char* ONLY_LuaWorld   = "LuaWorld";

#define ADD_CI(n, bits, retType, singleScript) \
	const char* n = #n;              \
	SetupCallIn(LUA_CI_ ## n, n);    \
	infoMap[#n] = CallInInfo(        \
	  LUA_CI_ ## n, n,               \
	  (bits) & REQ_FULL_READ,        \
	  (bits) & REQ_INPUT_CTRL,       \
	  strncmp(#n, "Draw", 4) == 0, singleScript, retType)

	////////////////////////////////////
	// CEventHandler managed call-ins //
	////////////////////////////////////

	ADD_CI(Shutdown, NO_REQS, BASIC, ANY_SCRIPT);
	
	ADD_CI(Update, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(BZDBChange, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(RecvCommand, NO_REQS, FIRST_TRUE, ANY_SCRIPT);
	ADD_CI(RecvChatMsg, NO_REQS, FIRST_TRUE, ANY_SCRIPT);
	ADD_CI(RecvLuaData, NO_REQS, FIRST_TRUE, ANY_SCRIPT);

	ADD_CI(ServerJoined, NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(ServerParted, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(PlayerAdded,       NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerRemoved,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerSpawned,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerKilled,      NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerJumped,      NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerLanded,      NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerTeleported,  NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerTeamChange,  NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(PlayerScoreChange, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(ShotAdded,      NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(ShotRemoved,    NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(ShotTeleported, NO_REQS, BASIC, ANY_SCRIPT);

 	ADD_CI(FlagAdded,       NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(FlagRemoved,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(FlagGrabbed,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(FlagDropped,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(FlagCaptured,    NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(FlagTransferred, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(ViewResize,    NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(GLContextInit, NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(GLContextFree, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(DrawGenesis,     NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawWorldStart,  NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawWorld,       NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawWorldAlpha,  NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawWorldShadow, NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawScreenStart, NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawScreen,      NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(DrawRadar,       NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(GotGfxBlock,  NO_REQS, BASIC, ANY_SCRIPT);
	ADD_CI(LostGfxBlock, NO_REQS, BASIC, ANY_SCRIPT);

	ADD_CI(KeyPress,     REQ_INPUT_CTRL, FIRST_TRUE,   ANY_SCRIPT);
	ADD_CI(KeyRelease,   REQ_INPUT_CTRL, FIRST_TRUE,   ANY_SCRIPT);
	ADD_CI(MousePress,   REQ_INPUT_CTRL, FIRST_TRUE,   ANY_SCRIPT);
	ADD_CI(MouseMove,    REQ_INPUT_CTRL, SPECIAL,      ANY_SCRIPT);
	ADD_CI(MouseRelease, REQ_INPUT_CTRL, SPECIAL,      ANY_SCRIPT);
	ADD_CI(MouseWheel,   REQ_INPUT_CTRL, FIRST_TRUE,   ANY_SCRIPT);
	ADD_CI(IsAbove,      REQ_INPUT_CTRL, FIRST_TRUE,   ANY_SCRIPT);
	ADD_CI(GetTooltip,   REQ_INPUT_CTRL, FIRST_STRING, ANY_SCRIPT);
}


LuaCallInDB::~LuaCallInDB()
{
}


/******************************************************************************/
/******************************************************************************/
