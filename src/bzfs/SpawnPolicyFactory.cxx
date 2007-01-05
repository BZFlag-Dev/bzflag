/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "SpawnPolicyFactory.h"

/* bzfs-specific implementation headers */
#include "DefaultSpawnPolicy.h"
#include "RandomSpawnPolicy.h"
#include "DangerousSpawnPolicy.h"


// initialize the singleton
template <>
SpawnPolicyFactory* Singleton<SpawnPolicyFactory>::_instance = (SpawnPolicyFactory*)0;

// register the available policies
static bool _init();
static bool _policiesInitialized = _init();


/* public */

SpawnPolicy *
SpawnPolicyFactory::Policy(std::string policy)
{
  std::string lcPolicy = TextUtils::tolower(policy);
  SpawnPolicy *p = (SpawnPolicy*)NULL;

  /* empty indicates request for default */
  if (lcPolicy == "") {
    if (_defaultPolicy == "") {
      DefaultSpawnPolicy dsp;
      _defaultPolicy = dsp.Name();
    }
    lcPolicy = _defaultPolicy;
  }

  /* may return NULL if policy isn't something recognized */
  p = SPAWNPOLICY.Create(lcPolicy.c_str());

  /* failsafe, just so we don't ever return NULL */
  if (!p) {
    return new DefaultSpawnPolicy();
  }

  return p;
}


void
SpawnPolicyFactory::setDefault(std::string policy)
{
  _defaultPolicy = policy;
}


/* private */

SpawnPolicyFactory::SpawnPolicyFactory()
{
}

SpawnPolicyFactory::~SpawnPolicyFactory()
{
}

/* register all the available known policies */
static bool
_init()
{
  static bool _initialized = false;
  if (!_initialized) {
    SPAWNPOLICY.Register<DefaultSpawnPolicy>("default");
    SPAWNPOLICY.Register<RandomSpawnPolicy>("random");
    SPAWNPOLICY.Register<DangerousSpawnPolicy>("dangerous");
    _initialized = true;
  }
  return _initialized;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
