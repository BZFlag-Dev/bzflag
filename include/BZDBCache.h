#ifndef	BZDBCACHE_H
#define	BZDBCACHE_H

#include <string>

#include "StateDatabase.h"

class BZDBCache
{
public:
	static void init();

	static bool  displayMainFlags;

	static float maxLOD;
	static float tankHeight;


private:
	static void clientCallback(const std::string &name, void *);
	static void serverCallback(const std::string &name, void *);
};

#endif
