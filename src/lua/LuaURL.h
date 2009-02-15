#ifndef LUA_URL_H
#define LUA_URL_H

#include "common.h"

// system headers
#include <string>

// common headers
#include "cURLManager.h"


struct lua_State;
class AccessList;


class LuaURL : private cURLManager {
	public:
		LuaURL(lua_State* L, const std::string& url);
		~LuaURL();

		// virtuals from cURLManager
    void finalization(char* data, unsigned int length, bool good);
    
		bool Cancel();

		bool GetActive()  const { return active; }
		bool GetSuccess() const { return success; }
		bool WantCache()  const { return wantCache; }

		unsigned int  GetLength() const { return theLen; }

		const std::string& GetURL()      const { return url; }
		const std::string& GetPostData() const { return postData; }

		double GetFileSize() const { return fileSize; }
		time_t GetFileTime() const { return fileTime; }
		long   GetHttpCode() const { return httpCode; }

		bool PushFunc(lua_State* L) const;

	private:
		void ClearRefs();

	private:
		lua_State* L;
		bool active;
		bool success;
		bool wantCache;
		bool headOnly; // do not fetch the data

		double fileSize;
		time_t fileTime;
		long   httpCode;

		std::string url;
		std::string postData;
		int funcRef;
		int selfRef;
};


class LuaURLMgr {
	public:
		static bool PushEntries(lua_State* L);

		static void SetAccessList(AccessList*);

	public:
		static const char* metaName;

	private: // metatable methods
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);

	public:
		static const LuaURL* TestURL(lua_State* L, int index);
		static const LuaURL* CheckURL(lua_State* L, int index);

	private:
		static LuaURL* GetURL(lua_State* L, int index);

	private: // call-outs

		static int Fetch(lua_State* L);

		static int Cancel(lua_State* L);
		static int Length(lua_State* L);
		static int Success(lua_State* L);
		static int IsActive(lua_State* L);
		static int GetURL(lua_State* L);
		static int GetPostData(lua_State* L);
		static int GetCallback(lua_State* L);
		static int GetFileSize(lua_State* L);
		static int GetFileTime(lua_State* L);
		static int GetHttpCode(lua_State* L);

	private:
		static AccessList* accessList;
};


#endif // LUA_URL_H
