#ifdef _WIN32
#pragma warning(4: 4786)
#endif

#include <string>
#include "BZDBCache.h"

float BZDBCache::maxLOD;
float BZDBCache::tankHeight;

void BZDBCache::init()
{
  BZDB->addCallback( StateDatabase::BZDB_TANKHEIGHT, callback, NULL );
}

void BZDBCache::callback(const std::string &name, void *)
{
  if (name == StateDatabase::BZDB_MAXLOD)
	  maxLOD = BZDB->eval(StateDatabase::BZDB_MAXLOD);
  else if (name == StateDatabase::BZDB_TANKHEIGHT)
    tankHeight = BZDB->eval(StateDatabase::BZDB_TANKHEIGHT);
}

