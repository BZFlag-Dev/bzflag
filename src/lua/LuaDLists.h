#ifndef LUA_DISPLAY_LISTS_H
#define LUA_DISPLAY_LISTS_H


#include "common.h"

// common headers
#include "bzfgl.h"


struct lua_State;


class LuaDList {
	public:
		LuaDList(GLuint listID);
		~LuaDList();

		bool Delete();

		bool Call() const;
		bool IsValid() const;

		inline int GetMaxAttribDepth()  const { return maxAttribDepth;  }
		inline int GetMinAttribDepth()  const { return minAttribDepth;  }
		inline int GetExitAttribDepth() const { return exitAttribDepth; }

	private:
		GLuint listID;

		int maxAttribDepth;  // maximum GL attribute stack depth during execution
		int minAttribDepth;  // minimum GL attribute stack depth during execution
		int exitAttribDepth; // GL attribute stack depth change on exit

	private:
		void InitContext();
		void FreeContext();

	private:
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaDListMgr {
	public:
		static bool PushEntries(lua_State* L);

		static const LuaDList* TestLuaDList(lua_State* L, int index);
		static const LuaDList* CheckLuaDList(lua_State* L, int index);
	
	public:
		static const char* metaName;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);

		static LuaDList* GetLuaDList(lua_State* L, int index);

	private: // call-outs
		static int CreateList(lua_State* L);
		static int CallList(lua_State* L);
		static int DeleteList(lua_State* L);
};


#endif // LUA_DISPLAY_LISTS_H
