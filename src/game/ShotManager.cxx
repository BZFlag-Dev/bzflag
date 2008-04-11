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
  lastShotID = 0;
}

ShotManager::~ShotManager()
{
  clear();
}

void ShotManager::clear ( void )
{
  lastShotID = 0;
  shots.clear();
}

int ShotManager::newShot ( FiringInfo *info )
{
  lastShotID++;
  Shot shot(info);

  shots[lastShotID] = shot;
  for ( size_t c = 0; c < callbacks.size(); c++ )
  {
    if (callbacks[c])
      callbacks[c]->shotStarted(lastShotID);
  }
  return lastShotID;
}

void ShotManager::removeShot ( int id, bool notify )
{
  std::map<int,Shot>::iterator itr = shots.find(id);
  if (itr == shots.end())
    return;

  if (notify)
  {
    for ( size_t c = 0; c < callbacks.size(); c++ )
    {
      if (callbacks[c])
	callbacks[c]->shotEnded(id);
    }
  }

  shots.erase(itr);
}

ShotManager::Shot *ShotManager::getShot ( int id )
{
  std::map<int,Shot>::iterator itr = shots.find(id);
  if (itr == shots.end())
    return NULL;

  return &itr->second;
}


bool ShotManager::updateShot ( int id, float *pos, float *vec, double st, double l, double r )
{
  std::map<int,Shot>::iterator itr = shots.find(id);
  if (itr == shots.end())
    return false;

  if(pos)
    memcpy(itr->second.pos,pos,sizeof(float)*3);

  if(vec)
    memcpy(itr->second.vec,vec,sizeof(float)*3);

  itr->second.startTime = st;
  itr->second.lifetime = l;
  itr->second.range = r;

  if (itr->second.update(0))
  {
    for ( size_t c = 0; c < callbacks.size(); c++ )
    {
      if (callbacks[c])
	callbacks[c]->shotEnded(id);
    }
    shots.erase(itr);
    return false;
  }
  else
  {
    for ( size_t c = 0; c < callbacks.size(); c++ )
    {
      if (callbacks[c])
	callbacks[c]->shotUpdated(id);
    }
  }

  return true;
}

void ShotManager::update ( double dt )
{
  std::map<int,Shot>::iterator itr = shots.begin();
  std::vector<int> deadShots;

  // update all the shots, flaging the shots that want to die
  while(itr != shots.end())
  {
    if (itr->second.update(dt))
      deadShots.push_back(itr->first);
    itr++;
  }

  // notify for each dead shot then kill it
  for ( size_t s = 0; s < deadShots.size(); s++ )
  {
    for ( size_t c = 0; c < callbacks.size(); c++ )
    {
      if (callbacks[c])
	callbacks[c]->shotEnded(deadShots[s]);
    }
    shots.erase(shots.find(deadShots[s]));
  }
}

void ShotManager::addEventHandler ( ShotEventCallbacks *cb )
{
  callbacks.push_back(cb);
}

void ShotManager::removeEventHandler ( ShotEventCallbacks *cb )
{
  for (size_t i = 0; i < callbacks.size(); i++ )
  {
    if (cb == callbacks[i])
    {
      callbacks.erase(callbacks.begin()+i);
      return;
    }
  }
}

ShotManager::Shot::Shot( FiringInfo* info )
{
  if (info)
  {
    player = info->shot.player;
    startTime = info->timeSent;
    team = info->shot.team;
    flag = info->flagType;
    type = info->shotType;
    lifetime = info->lifetime;
    range = 0; // we need this in the firing info, is it based on shot or flag?

    for ( size_t i = 0; i < 3; i++)
    {
      pos[i] = info->shot.pos[i];
      vec[i] = info->shot.vel[i];
    }
  }
  else
  {
    startTime = TimeKeeper::getCurrent().getSeconds();

    team = LastTeam;
    flag = Flags::Null;
    type = NoShot;
    lifetime = 0;
    range = 0;
    for ( size_t i = 0; i < 3; i++)
      pos[i] = vec[i] = 0;

  }

  currentTime = lastUpdateTime = startTime;

}

ShotManager::Shot::Shot( const ShotManager::Shot& shot )
{
  team = shot.team;
  flag = shot.flag;
  type = shot.type;
  mode = shot.mode;

  startTime = shot.startTime;
  lastUpdateTime = shot.lastUpdateTime;
  currentTime = shot.currentTime;

  lifetime = shot.lifetime;
  range = shot.range;;

  for ( size_t i = 0; i < 3; i++)
  {
    pos[i] = shot.pos[i];
    vec[i] = shot.vec[i];
  }
}

ShotManager::Shot& ShotManager::Shot::operator = (const ShotManager::Shot& shot)
{
  team = shot.team;
  flag = shot.flag;
  type = shot.type;
  mode = shot.mode;

  startTime = shot.startTime;
  lastUpdateTime = shot.lastUpdateTime;
  currentTime = shot.currentTime;

  lifetime = shot.lifetime;
  range = shot.range;;

  for ( size_t i = 0; i < 3; i++)
  {
    pos[i] = shot.pos[i];
    vec[i] = shot.vec[i];
  }
  return *this;
}

bool ShotManager::Shot::update ( double dt )
{
  currentTime += dt;

  if ( currentTime - startTime > lifetime )
    return true;

  // figure out where the shot is now and stuff
  // don't do any updates now just let the clients end us

  return false;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
