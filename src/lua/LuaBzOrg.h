#ifndef LUA_BZORG_H
#define LUA_BZORG_H

#include "LuaHandle.h"

#include <string>


class LuaBzOrg : public LuaHandle
{
	friend class CodeFetch;

	public:
		static void LoadHandler();
		static void FreeHandler();

		static bool IsActive();

	private:
		LuaBzOrg(const std::string& code, const std::string& url);
		~LuaBzOrg();

	private:
};


extern LuaBzOrg* luaBzOrg;


#endif // LUA_BZORG_H
