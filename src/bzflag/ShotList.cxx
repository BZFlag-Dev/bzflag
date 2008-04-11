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
/** initialize the singleton */
template <>
ShotList* Singleton<ShotList>::_instance = (ShotList*)0;


ShotList::ShotList()
{
  lastLocalShot = 0;
}

ShotList::~ShotList()
{
  clear();
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

std::vector<ShotPath*> ShotList::getLocalShotList ( void )
{
  std::vector<ShotPath*> outShots;
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second && itr->second->isLocal())
      outShots.push_back(itr->second);
    itr++;
  }

  return outShots;
}

std::vector<ShotPath*> ShotList::getShotsFromPlayer ( PlayerId player )
{
  std::vector<ShotPath*> outShots;
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second && itr->second->getPlayer() == player)
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
  shot->setLocal(true);
  shots[lastLocalShot] = shot;
  return lastLocalShot;
}

int ShotList::addShot ( int GUID, FiringInfo * info )
{
  shots[GUID] = new ShotPath(*info,syncedClock.GetServerSeconds());
  return GUID;
}

int ShotList::updateShot( int GUID, FiringInfo * info )
{
  bool local = false;
  if (getShot(GUID) && getShot(GUID)->isLocal())
    local = true;

  if (!removeShot(GUID))
    return 0;

  addShot(GUID,info);
  getShot(GUID)->setLocal(local);

  return GUID;
}

int ShotList::updateShotID( int oldID, int newID )
{
  if (oldID == newID)
    return oldID;

  std::map<int,ShotPath*>::iterator itr = shots.find(oldID);

  if (itr == shots.end())
    return 0;

  ShotPath *shot = itr->second;
  if (!shot)
    return 0;

  itr = shots.find(newID);
  if (itr != shots.end())
    removeShot(newID);

  shots.erase(itr);
  shots[newID] = shot;

  return newID;
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

void ShotList::flushExpiredShots( void )
{
  std::vector<int> expiredList = getExpiredShotList();
  for (size_t i = 0; i < expiredList.size(); i++)
    shots.erase(shots.find(expiredList[i]));
}

void ShotList::flushShotsFromPlayer( PlayerId player )
{
  std::vector<int> expiredList;
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second && itr->second->getPlayer() == player)
      expiredList.push_back(itr->first);
    itr++;
  }

  for (size_t i = 0; i < expiredList.size(); i++)
    shots.erase(shots.find(expiredList[i]));
}

void ShotList::clear ( void )
{
  std::map<int,ShotPath*>::iterator itr = shots.begin();
  while(itr != shots.end())
  {
    if (itr->second)
      itr++;
  }
  shots.clear();
  lastLocalShot = 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
