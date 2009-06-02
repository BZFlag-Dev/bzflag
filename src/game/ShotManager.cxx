/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "ShotManager.h"
#include "TimeKeeper.h"

//
// ShotManager
//

/** initialize the singleton */
template <>
ShotManager* Singleton<ShotManager>::_instance = (ShotManager*)0;


ShotManager::ShotManager()
{
}

ShotManager::~ShotManager()
{
}

int ShotManager::newShot (FiringInfo * /*info*/, int /*param*/)
{
  return 0;
}

void ShotManager::update (double /*dt*/)
{
}

void ShotManager::addEventHandler (ShotEventCallbacks *cb)
{
  callbacks.push_back(cb);
}

void ShotManager::removeEventHandler (ShotEventCallbacks *cb)
{
  for (size_t i = 0; i < callbacks.size(); i++) {
    if (cb == callbacks[i]) {
      callbacks.erase(callbacks.begin()+i);
      return;
    }
  }
}

ShotManager::Shot::Shot(FiringInfo* info, int /*GUID*/, int /*p*/)
: lifetime(0.0)
, range(0.0)
, pos(0.0f, 0.0f, 0.0f)
, vec(0.0f, 0.0f, 0.0f)
{
  if (info) {
    startTime = info->timeSent;
  } else {
    startTime = TimeKeeper::getCurrent().getSeconds();
  }

  // team = LastTeam;
  //flag = Flags::Null;
  //type = NoShot;

  lastUpdateTime = startTime;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
