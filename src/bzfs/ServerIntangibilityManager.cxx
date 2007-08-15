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

#include "ServerIntangibilityManager.h"
#include "ObstacleMgr.h"

template <>
ServerIntangibilityManager* Singleton<ServerIntangibilityManager>::_instance = (ServerIntangibilityManager*)0;

void ServerIntangibilityManager::setWorldObjectTangibility ( unsigned int objectGUID, bool tangible )
{
  tangibilityMap[objectGUID] = tangible;
  // send out the tangibility update message
}

void ServerIntangibilityManager::sendNewPlayerWorldTangibility ( unsigned int playeID )
{
  std::map<unsigned int, bool>::iterator itr = tangibilityMap.begin();
  while (itr != tangibilityMap.end())
  {
    // send out the tangibility update message
    itr++;
  }
}

bool ServerIntangibilityManager::isWorldObjectTangable ( unsigned int objectGUID )
{
  std::map<unsigned int, bool>::iterator itr = tangibilityMap.find(objectGUID);
  if ( itr != tangibilityMap.end())
    return itr->second;

  Obstacle *obs = OBSTACLEMGR.getObstacleFromID(objectGUID);
  if (!obs)
    return false;

  return !obs->isDriveThrough();
}

void ServerIntangibilityManager::resetTangibility ( void )
{
  tangibilityMap.clear();
  // send the reset tangability message to all clients
}




// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
