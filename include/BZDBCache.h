#ifndef	BZDBCACHE_H
#define	BZDBCACHE_H

#include <string>

#include "StateDatabase.h"

class BZDBCache
{
public:
	static void init();


	static float maxLOD;
	static float tankHeight;


private:
	static void callback(const std::string &name, void *);

};

#endif
