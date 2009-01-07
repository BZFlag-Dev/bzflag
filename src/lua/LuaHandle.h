#ifndef LUA_HANDLE_H
#define LUA_HANDLE_H

#include "common.h"
#include <string>
#include <set>
#include <map>


#include "EventClient.h"
#include "../other/lua/src/lua_extra.h"


class Player;
class Flag;
class FiringInfo;
class ShotPath;

class LuaHandle;
struct lua_State;


// prepended to the beginning of lua_State's
struct LuaHandleHeader {
	LuaHandle*  handle;
	LuaHandle** handlePtr;
};

// L2HH(L)  --  lua_State* to LuaHandleHeader*
static inline LuaHandleHeader* L2HH(lua_State* L)
{
	return (LuaHandleHeader*)((char*)L - BZ_LUA_EXTRASPACE);
}

// L2H(L)  --  lua_State* to LuaHandle*
static inline LuaHandle* L2H(lua_State* L)
{
	return L2HH(L)->handle;
}

// L2HP(L)  --  lua_State* to LuaHandle**
static inline LuaHandle** L2HP(lua_State* L)
{
	return L2HH(L)->handlePtr;
}


class LuaHandle : public EventClient
{
	public:
		const std::string& GetFSRead()     const { return fsRead;     }
		const std::string& GetFSReadAll()  const { return fsReadAll;  }
		const std::string& GetFSWrite()    const { return fsWrite;    }
		const std::string& GetFSWriteAll() const { return fsWriteAll; }

	public:
		void CheckStack();
		
	public:
		bool               RequestReload()  const { return requestReload;  }
		bool               RequestDisable() const { return requestDisable; }
		const std::string& RequestMessage() const { return requestMessage; }

	public: // call-ins

		virtual bool CanUseCallIn(int code) const;
		virtual bool CanUseCallIn(const std::string& name) const;

		virtual void UpdateCallIn(const std::string& name, bool state);

		virtual bool GlobalCallInCheck(const std::string& name);
		bool WantsEvent(const std::string& eName) { // for EventHandler
			return GlobalCallInCheck(eName);
		}

		virtual void Shutdown();

		virtual void Update();

		virtual void BZDBChange(const std::string&);

    virtual void RecvCommand(const std::string& msg); // custom to LuaHandle
    virtual void RecvChatMsg(const std::string& msg, int srcID, int dstID);
    virtual void RecvLuaData(int srcPlayerID, int srcScriptID,
														 int dstPlayerID, int dstScriptID,
                             int status, const std::string& data);

		virtual void ServerJoined();
		virtual void ServerParted();

    virtual void PlayerAdded(const Player&);
    virtual void PlayerRemoved(const Player&);
    virtual void PlayerSpawned(const Player&);
    virtual void PlayerKilled(const Player&);
    virtual void PlayerJumped(const Player&);
    virtual void PlayerLanded(const Player&);
    virtual void PlayerTeleported(const Player&, int srcLink, int dstLink);
    virtual void PlayerTeamChange(const Player&);
    virtual void PlayerScoreChange(const Player&);

    virtual void ShotAdded(const FiringInfo&);
    virtual void ShotRemoved(const FiringInfo&);
    virtual void ShotTeleported(const ShotPath&, int srcLink, int dstLink);

    virtual void FlagAdded(const Flag&);
    virtual void FlagRemoved(const Flag&);
    virtual void FlagGrabbed(const Flag&,  const Player&);
    virtual void FlagDropped(const Flag&,  const Player&);
    virtual void FlagCaptured(const Flag&, const Player&);
    virtual void FlagTransferred(const Flag&, const Player& src, const Player& dst);

		virtual void ViewResize();
		virtual void GLContextInit();
		virtual void GLContextFree();

		virtual void DrawGenesis();
		virtual void DrawWorldStart();
		virtual void DrawWorld();
		virtual void DrawWorldAlpha();
		virtual void DrawWorldShadow();
		virtual void DrawScreenStart();
		virtual void DrawScreen();
		virtual void DrawRadar();

    virtual void GotGfxBlock(int /*type*/, int /*id*/);
    virtual void LostGfxBlock(int /*type*/, int /*id*/);

		virtual bool KeyPress(int /*key*/, bool /*isRepeat*/);
		virtual bool KeyRelease(int /*key*/);
		virtual bool MouseMove(int /*x*/, int /*y*/);
		virtual bool MousePress(int /*x*/, int /*y*/, int /*b*/);
		virtual bool MouseRelease(int /*x*/, int /*y*/, int /*button*/);
		virtual bool MouseWheel(float /*value*/);
		virtual bool IsAbove(int /*x*/, int /*y*/);
		virtual std::string GetTooltip(int /*x*/, int /*y*/);

		virtual void WordComplete(const std::string& /*l*/, std::set<std::string>& /*p*/);

	protected:
		LuaHandle(const std::string& name, int order, bool fullRead, bool inputCtrl);
		virtual ~LuaHandle();

		void KillLua();

		inline void SetActiveHandle() {
			activeHandle    = this;
			activeFullRead  = this->fullRead;
			activeInputCtrl = this->inputCtrl;
		}

		inline void SetActiveHandle(LuaHandle* lh) {
			activeHandle = lh;
			if (lh) {
				activeFullRead  = lh->fullRead;
				activeInputCtrl = lh->inputCtrl;
			}
		}

		virtual bool        ExecSourceCode(const std::string& sourceCode);
		virtual std::string LoadSourceCode(const std::string& sourceFile,
		                              const std::string& sourceModes);

		bool AddBasicCalls();
		bool PushLib(const char* name, bool (*entriesFunc)(lua_State*));

//		bool PushCallIn(const std::string& name);

		bool PushCallIn(int ciCode);
		bool RunCallIn(int ciCode, int inArgs, int outArgs);

	protected:
		lua_State* L;

		bool requestReload;
		bool requestDisable;
		std::string requestMessage;

		std::set<int> validCallIns;
		std::string   actionPrefix;

		std::string fsRead;
		std::string fsReadAll;
		std::string fsWrite;
		std::string fsWriteAll;

	protected: // call-outs
		static int CallAsPlayer(lua_State* L);

		static int ScriptReload(lua_State* L);
		static int ScriptDisable(lua_State* L);

		static int ScriptGetName(lua_State* L);
		static int ScriptGetFullRead(lua_State* L);
		static int ScriptGetInputCtrl(lua_State* L);

		static int ScriptGetCallInInfo(lua_State* L);
		static int ScriptCanUseCallIn(lua_State* L);
		static int ScriptSetCallIn(lua_State* L);

		static int ScriptAddTextFallback(lua_State* L);    // FIXME
		static int ScriptRemoveTextFallback(lua_State* L);

		static int ScriptGetGLOBALS(lua_State* L);
		static int ScriptGetCALLINS(lua_State* L);
		static int ScriptGetREGISTRY(lua_State* L);

		static int ScriptPrintPointer(lua_State* L);
		static int ScriptPrintGCInfo(lua_State* L);
		static int ScriptPrintDelayQueues(lua_State* L);

	public: // static
		static const LuaHandle* GetActiveHandle() { return activeHandle; }

		static const bool& GetActiveFullRead()  { return activeFullRead;  }
		static const bool& GetActiveInputCtrl() { return activeInputCtrl; }

		static void SetDevMode(bool value) { devMode = value; }
		static bool GetDevMode() { return devMode; }

		static void HandleLuaMsg(int playerID, int script, int mode,
		                         const std::string& msg);

	protected: // static
		static LuaHandle* activeHandle;
		static bool activeFullRead;
		static bool activeInputCtrl;

		static bool devMode;   // allows real file access
};


#endif // LUA_HANDLE_H
