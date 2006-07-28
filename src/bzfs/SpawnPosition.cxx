/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// object that creates and contains a spawn position

/* interface header */
#include "SpawnPosition.h"

/* implementation headers */
#include "SpawnPolicyFactory.h"


/* no policy by default, constructor will set if unset */
SpawnPolicy *SpawnPosition::policy = (SpawnPolicy*)0;


SpawnPosition::SpawnPosition(int playerId, bool onGroundOnly, bool notNearEdges)
{
  /* if a spawn policy hasn't been set yet, just go with the default */
  if (!SpawnPosition::policy) {
    SetSpawnPolicy(SpawnPolicyFactory::DefaultPolicy());
  }

  SpawnPosition::policy->getPosition(pos, playerId, onGroundOnly, notNearEdges);
  SpawnPosition::policy->getAzimuth(azimuth);
}

SpawnPosition::~SpawnPosition()
{
}

void SpawnPosition::SetSpawnPolicy(SpawnPolicy *_policy)
{
  if (SpawnPosition::policy) {
    delete SpawnPosition::policy;
  }
  SpawnPosition::policy = _policy;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
