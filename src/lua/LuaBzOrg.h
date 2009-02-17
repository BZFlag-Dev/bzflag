#ifndef LUA_BZORG_H
#define LUA_BZORG_H

#include "LuaHandle.h"


class LuaBzOrg : public LuaHandle
{
	friend class CodeFetch;

	public:
		static void LoadHandler();
		static void FreeHandler();

		static bool IsActive();

	protected:
		void ForbidCallIns();

	private:
		LuaBzOrg(const char* code, int length);
		~LuaBzOrg();
};


extern LuaBzOrg* luaBzOrg;


#endif // LUA_BZORG_H
