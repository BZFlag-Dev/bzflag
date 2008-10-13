/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "BzfMedia.h"
#include "TimeKeeper.h"
#include "PlatformFactory.h"
#include "BZDBCache.h"
#include "TextUtils.h"

template <>
SoundManager* Singleton<SoundManager>::_instance = (SoundManager*)0;

int  SFX_FIRE = 0;		
int  SFX_EXPLOSION = 1;		
int  SFX_RICOCHET = 2;		
int  SFX_GRAB_FLAG = 3;		
int  SFX_DROP_FLAG = 4;		
int  SFX_CAPTURE = 5;		
int  SFX_LOSE = 6;		
int  SFX_ALERT = 7;		
int  SFX_JUMP = 8;		
int  SFX_LAND = 9;		
int  SFX_TELEPORT = 10;		
int  SFX_LASER = 11;		
int  SFX_SHOCK = 12;		
int  SFX_POP = 13;		
int  SFX_DIE = 14;		
int  SFX_GRAB_BAD = 15;		
int  SFX_SHOT_BOOM = 16;	
int  SFX_KILL_TEAM = 17;	
int  SFX_PHANTOM = 18;		
int  SFX_MISSILE = 19;		
int  SFX_LOCK = 20;		
int  SFX_TEAMGRAB = 21;		
int  SFX_HUNT = 22;		
int  SFX_HUNT_SELECT = 23;	
int  SFX_RUNOVER = 24;; 
int  SFX_THIEF = 25;		
int  SFX_BURROW = 26;		
int  SFX_MESSAGE_PRIVATE = 27;
int  SFX_MESSAGE_TEAM = 28;
int  SFX_MESSAGE_ADMIN = 29;
int  SFX_FLAP = 30;	
int  SFX_BOUNCE = 31;	
int  SFX_HIT = 32;	

static const char*	soundFiles[] = {
				"fire",
				"explosion",
				"ricochet",
				"flag_grab",
				"flag_drop",
				"flag_won",
				"flag_lost",
				"flag_alert",
				"jump",
				"land",
				"teleport",
				"laser",
				"shock",
				"pop",
				"explosion",
				"flag_grab",
				"boom",
				"killteam",
				"phantom",
				"missile",
				"lock",
				"teamgrab",
				"hunt",
				"hunt_select",
				"steamroller",
				"thief",
				"burrow",
				"message_private",
				"message_team",
				"message_admin",
				"flap",
				"bounce",
				"hit"
			};
#define	STD_SFX_COUNT	((int)(countof(soundFiles)))	// the number of "Standard" sounds

SoundManager::SoundManager()
{
  currentSystem = new PlatformSound();
  registerSystem(currentSystem);
}

SoundManager::~SoundManager()
{
  if (currentSystem)
    currentSystem->shutdown();
  for (size_t i = 0; i < soundSystems.size(); i++)
  {
    if (soundSystems[i])
	delete(soundSystems[i]);
  }
}

SoundSystem& SoundManager::getSystem ( void )
{
  return *currentSystem;
}
 
void SoundManager::registerSystem ( SoundSystem *system )
{ 
  soundSystems.push_back(system);
}  

std::vector<SoundSystem*> SoundManager::listSystems ( void )
{
  return soundSystems;
}

void SoundManager::activateSoundSystem ( SoundSystem* system )
{
  if(!system)
    return;

  SoundSystem* realSystem = NULL;

  for (size_t i = 0; i < soundSystems.size(); i++)
  {
    if (soundSystems[i] == system)
    {
      realSystem = system;
      break;
    }
  }

  if (!realSystem)
    registerSystem(system);

  if(currentSystem)
    currentSystem->shutdown();

  currentSystem = system;
  currentSystem->startup();
}

void SoundManager::activateSoundSystem ( const std::string &name )
{
  if(!name.size())
    return;

  for (size_t i = 0; i < soundSystems.size(); i++)
  {
    if (soundSystems[i]->name() == name)
    {
      if(currentSystem)
	currentSystem->shutdown();

      currentSystem = soundSystems[i];
      currentSystem->startup();
      return;
    }
  }
}

std::vector<std::string> SoundManager::getStdSounds ( void )
{
  std::vector<std::string> sounds;
  for (int i = 0; i < STD_SFX_COUNT; i++)
    sounds.push_back(std::string(soundFiles[i]));

  return sounds;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
