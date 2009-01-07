#ifndef LUA_WORLD_H
#define LUA_WORLD_H

#include "LuaHandle.h"


class LuaWorld : public LuaHandle
{
	public:
		static void LoadHandler();
		static void FreeHandler();

	protected:
		bool SetupLuaLibs();

	private:
		LuaWorld();
		~LuaWorld();
};


extern LuaWorld* luaWorld;


#endif // LUA_WORLD_H
