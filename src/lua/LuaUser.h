#ifndef LUA_USER__H
#define LUA_USER__H

#include "LuaHandle.h"


class LuaUser : public LuaHandle
{
	public:
		static void LoadHandler();
		static void FreeHandler();

	protected:
		void ForbidCallIns();

	private:
		LuaUser();
		~LuaUser();
};


extern LuaUser* luaUser;


#endif // LUA_USER__H
