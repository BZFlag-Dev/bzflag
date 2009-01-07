#ifndef LUA_DISPLAY_LISTS_H
#define LUA_DISPLAY_LISTS_H


#include "common.h"

// system headers
#include <set>

// common headers
#include "bzfgl.h"


struct lua_State;


class LuaDList {
	public:
		LuaDList(GLuint listID);
		~LuaDList();

		bool Call();
		bool Free();
		bool IsValid() const;

	private:
		GLuint listID;

	private:
		void InitContext();
		void FreeContext();

	private:
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaDListMgr {
	public:
		static void Init();
		static void Free();

		static bool PushEntries(lua_State* L);

		bool InsertList(LuaDList* list);
		bool RemoveList(LuaDList* list);

		static const char* metaName;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);

		static LuaDList*& CheckLuaDList(lua_State* L, int index);
	
	private: // call-outs
		static int CreateList(lua_State* L);
		static int CallList(lua_State* L);
		static int DeleteList(lua_State* L);

	private:
		std::set<LuaDList*> lists;

	private:
		void InitContext();
		void FreeContext();

	private:
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


#endif // LUA_DISPLAY_LISTS_H
