#ifdef _WIN32
#pragma warning(4: 4786)
#endif

#include <string>
#include "BZDBCache.h"

float BZDBCache::tankHeight;

void BZDBCache::init()
{
  BZDB->addCallback( StateDatabase::BZDB_TANKHEIGHT, callback, NULL );
}

void BZDBCache::callback(const std::string &name, void *)
{
  if (name == StateDatabase::BZDB_TANKHEIGHT)
    tankHeight = BZDB->eval(StateDatabase::BZDB_TANKHEIGHT);
}

