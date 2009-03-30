#ifndef LUA_CALL_IN_DB_H
#define LUA_CALL_IN_DB_H
//
//////////////////////////////////////////////////////////////////////

#include "common.h"

#include <string>
#include <map>


//============================================================================//
//============================================================================//

enum LuaCallInCode {
	LUA_CI_Shutdown = 1, // 1..N numbering

	LUA_CI_Update,

	LUA_CI_BZDBChange,

	LUA_CI_CommandFallback,

	LUA_CI_RecvCommand,
	LUA_CI_RecvChatMsg,
	LUA_CI_RecvLuaData,

	LUA_CI_ServerJoined,
	LUA_CI_ServerParted,

	LUA_CI_PlayerAdded,
	LUA_CI_PlayerRemoved,
	LUA_CI_PlayerSpawned,
	LUA_CI_PlayerKilled,
	LUA_CI_PlayerJumped,
	LUA_CI_PlayerLanded,
	LUA_CI_PlayerTeleported,
	LUA_CI_PlayerTeamChange,
	LUA_CI_PlayerScoreChange,

	LUA_CI_ShotAdded,
	LUA_CI_ShotRemoved,
	LUA_CI_ShotRicochet,
	LUA_CI_ShotTeleported,

	LUA_CI_FlagAdded,
	LUA_CI_FlagRemoved,
	LUA_CI_FlagGrabbed,
	LUA_CI_FlagDropped,
	LUA_CI_FlagCaptured,
	LUA_CI_FlagTransferred,

	LUA_CI_GotGfxBlock,
	LUA_CI_LostGfxBlock,

	LUA_CI_GLResize,
	LUA_CI_GLReload, // EventHandler's GLInitContext
	LUA_CI_GLUnmapped,

	LUA_CI_DrawGenesis,
	LUA_CI_DrawWorldStart,
	LUA_CI_DrawWorld,
	LUA_CI_DrawWorldAlpha,
	LUA_CI_DrawWorldShadow,
	LUA_CI_DrawScreenStart,
	LUA_CI_DrawScreen,
	LUA_CI_DrawRadar,

	LUA_CI_KeyPress,
	LUA_CI_KeyRelease,
	LUA_CI_MousePress,
	LUA_CI_MouseMove,
	LUA_CI_MouseRelease,
	LUA_CI_MouseWheel,
	LUA_CI_IsAbove,
	LUA_CI_GetTooltip,

	LUA_CI_WordComplete,

	LUA_CI_ForbidSpawn,
	LUA_CI_ForbidJump,
	LUA_CI_ForbidShot,
	LUA_CI_ForbidShotLock,
	LUA_CI_ForbidFlagDrop
};


//============================================================================//
//============================================================================//

class LuaCallInDB {
	public:
		LuaCallInDB();
		~LuaCallInDB();

		bool Init();

		const string& GetEventName(const string& callIn) const;
		const string& GetCallInName(const string& event) const;

	public:
		struct CallInInfo {
			CallInInfo() {}
			CallInInfo(int c, const char* n,
			           bool fullRead, bool gameCtrl, bool inputCtrl,
			           bool rev, bool rec, const char* ss, const char* lt)
			: code(c), name(n)
			, reqFullRead(fullRead)
			, reqGameCtrl(gameCtrl)
			, reqInputCtrl(inputCtrl)
			, reversed(rev)
			, reentrant(rec)
			, loopType(lt)
			, singleScript(ss)
			{}
			int code;
			std::string name;
			bool reqFullRead;
			bool reqGameCtrl;
			bool reqInputCtrl;
			bool reversed;
			bool reentrant;
			std::string loopType;     // suggested control loop for lua handlers
			std::string singleScript; // can only be used by this script
		};

		typedef std::map<std::string, CallInInfo> InfoMap;

		const InfoMap& GetInfoMap() const { return infoMap; }

	public:
		void SetupMaps();

		inline const std::map<int, std::string>& GetCodeToNameMap() const
		{
			return codeToName;
		}

		inline const std::map<std::string, int>& GetNameToCodeMap() const
		{
			return nameToCode;
		}

		inline int GetCode(const std::string& ciName) const
		{
			std::map<std::string, int>::const_iterator it = nameToCode.find(ciName);
			if (it == nameToCode.end()) {
				return 0;
			}
			return it->second;
		}

		inline const std::string* GetName(int ciCode) const
		{
			std::map<int, std::string>::const_iterator it = codeToName.find(ciCode);
			if (it == codeToName.end()) {
				return NULL;
			}
			return &(it->second);
		}

	protected:
		bool SetupCallIn(int code, const std::string& name);
		void CheckCallIn(int code, const std::string& name);

	protected:
		InfoMap infoMap;
		std::map<int, std::string> codeToName;
		std::map<std::string, int> nameToCode;
};


extern LuaCallInDB luaCallInDB;


//============================================================================//
//============================================================================//

#endif // LUA_CALL_IN_DB_H
