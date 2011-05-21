/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "sound.h"

// system headers
#include <iostream>
#include <vector>
#include <map>
#include <string.h>

// common headers
#include "PlatformSound.h"

#include "TextUtils.h"

template <>
SoundManager* Singleton<SoundManager>::_instance = (SoundManager*)0;


int SFX_FIRE = -1;
int SFX_EXPLOSION = -1;
int SFX_RICOCHET = -1;
int SFX_GRAB_FLAG = -1;
int SFX_DROP_FLAG = -1;
int SFX_CAPTURE = -1;
int SFX_LOSE = -1;
int SFX_ALERT = -1;
int SFX_JUMP = -1;
int SFX_LAND = -1;
int SFX_TELEPORT = -1;
int SFX_LASER = -1;
int SFX_SHOCK = -1;
int SFX_POP = -1;
int SFX_DIE = -1;
int SFX_GRAB_BAD = -1;
int SFX_SHOT_BOOM = -1;
int SFX_KILL_TEAM = -1;
int SFX_PHANTOM = -1;
int SFX_MISSILE = -1;
int SFX_LOCK = -1;
int SFX_TEAMGRAB = -1;
int SFX_HUNT = -1;
int SFX_HUNT_SELECT = -1;
int SFX_RUNOVER = -1;
int SFX_THIEF = -1;
int SFX_BURROW = -1;
int SFX_MESSAGE_PRIVATE = -1;
int SFX_MESSAGE_TEAM = -1;
int SFX_MESSAGE_ADMIN = -1;
int SFX_FLAP = -1;
int SFX_BOUNCE = -1;
int SFX_HIT = -1;


//============================================================================//

SoundManager::SoundManager() {
  currentSystem = new PlatformSound();
  registerSystem(currentSystem);
}


SoundManager::~SoundManager() {
  if (currentSystem) {
    currentSystem->shutdown();
  }
  for (size_t i = 0; i < soundSystems.size(); i++) {
    if (soundSystems[i]) {
      delete(soundSystems[i]);
    }
  }
}


SoundSystem& SoundManager::getSystem(void) {
  return *currentSystem;
}


void SoundManager::registerSystem(SoundSystem* sys) {
  soundSystems.push_back(sys);
}


std::vector<SoundSystem*> SoundManager::listSystems(void) {
  return soundSystems;
}


void SoundManager::activateSoundSystem(SoundSystem* sys) {
  if (!sys) {
    return;
  }

  SoundSystem* realSystem = NULL;

  for (size_t i = 0; i < soundSystems.size(); i++) {
    if (soundSystems[i] == sys) {
      realSystem = sys;
      break;
    }
  }

  if (!realSystem) {
    registerSystem(sys);
  }

  if (currentSystem) {
    currentSystem->shutdown();
  }

  currentSystem = sys;
  currentSystem->startup();
}


void SoundManager::activateSoundSystem(const std::string& name) {
  if (!name.size()) {
    return;
  }

  for (size_t i = 0; i < soundSystems.size(); i++) {
    if (soundSystems[i]->name() == name) {
      if (currentSystem) {
        currentSystem->shutdown();
      }
      currentSystem = soundSystems[i];
      currentSystem->startup();
      return;
    }
  }
}

//============================================================================//

// used by getStandardSoundID() and setStandardSoundIDs()
static bool foundSoundFile = false;


int SoundSystem::getStandardSoundID(const std::string& filename) {
  const int soundCode = getID(filename);
  if (soundCode >= 0) {
    foundSoundFile = true;
  }
  return soundCode;
}


bool SoundSystem::setStandardSoundIDs(void) {
  foundSoundFile = false;

  SFX_SHOT_BOOM       = getStandardSoundID("boom");
  SFX_BOUNCE          = getStandardSoundID("bounce");
  SFX_BURROW          = getStandardSoundID("burrow");
  SFX_DIE             = getStandardSoundID("explosion");
  SFX_EXPLOSION       = getStandardSoundID("explosion");
  SFX_FIRE            = getStandardSoundID("fire");
  SFX_ALERT           = getStandardSoundID("flag_alert");
  SFX_DROP_FLAG       = getStandardSoundID("flag_drop");
  SFX_GRAB_BAD        = getStandardSoundID("flag_grab");
  SFX_GRAB_FLAG       = getStandardSoundID("flag_grab");
  SFX_LOSE            = getStandardSoundID("flag_lost");
  SFX_CAPTURE         = getStandardSoundID("flag_won");
  SFX_FLAP            = getStandardSoundID("flap");
  SFX_HIT             = getStandardSoundID("hit");
  SFX_HUNT            = getStandardSoundID("hunt");
  SFX_HUNT_SELECT     = getStandardSoundID("hunt_select");
  SFX_JUMP            = getStandardSoundID("jump");
  SFX_KILL_TEAM       = getStandardSoundID("killteam");
  SFX_LAND            = getStandardSoundID("land");
  SFX_LASER           = getStandardSoundID("laser");
  SFX_LOCK            = getStandardSoundID("lock");
  SFX_MESSAGE_ADMIN   = getStandardSoundID("message_admin");
  SFX_MESSAGE_PRIVATE = getStandardSoundID("message_team");
  SFX_MESSAGE_TEAM    = getStandardSoundID("message_team");
  SFX_MISSILE         = getStandardSoundID("missile");
  SFX_PHANTOM         = getStandardSoundID("phantom");
  SFX_POP             = getStandardSoundID("pop");
  SFX_RICOCHET        = getStandardSoundID("ricochet");
  SFX_SHOCK           = getStandardSoundID("shock");
  SFX_RUNOVER         = getStandardSoundID("steamroller");
  SFX_TEAMGRAB        = getStandardSoundID("teamgrab");
  SFX_TELEPORT        = getStandardSoundID("teleport");
  SFX_THIEF           = getStandardSoundID("thief");

  return foundSoundFile;
}


//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
