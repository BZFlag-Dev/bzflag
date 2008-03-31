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
#include "ShotList.h"
#include "SyncClock.h"

//
// ShotList
//

ShotList::ShotList()
{
  lastLocalShot = 0;
}

ShotList::~ShotList()
{
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second)
    itr++;
  }
}

ShotPath* ShotList::getShot ( int GUID )
{
  std::map<int,ShotPath*>::iterator itr = shots.find(GUID);

  if (itr == shots.end())
    return NULL;

  return itr->second;
}

std::vector<ShotPath*> ShotList::getShotList ( void )
{
  std::vector<ShotPath*> outShots;
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second)
      outShots.push_back(itr->second);
    itr++;
  }

  return outShots;
}

std::vector<int> ShotList::getExpiredShotList ( void )
{
  std::vector<int> outShots;
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second && itr->second->isExpired())
      outShots.push_back(itr->first);
    itr++;
  }
  return outShots;
}

int ShotList::addLocalShot ( FiringInfo * info )
{
  lastLocalShot--;
  ShotPath *shot = new ShotPath(*info,syncedClock.GetServerSeconds());
  shots[lastLocalShot] = shot;
  return lastLocalShot;
}

int ShotList::addShot ( int GUID, FiringInfo * info )
{
  shots[GUID] = new ShotPath(*info,syncedClock.GetServerSeconds());
  return GUID;
}

int ShotList::updateShot( int GUID, int param, FiringInfo * info )
{
  if (!removeShot(GUID))
    removeShot(param);

  addShot(GUID,info);

  return GUID;
}

bool ShotList::removeShot ( int GUID )
{
  std::map<int,ShotPath*>::iterator itr = shots.find(GUID);

  if (itr == shots.end())
    return false;

  delete(itr->second);
  shots.erase(itr);
  return true;
}

void ShotList::updateAllShots ( float dt )
{
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second)
      itr->second->update(dt);
    itr++;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
