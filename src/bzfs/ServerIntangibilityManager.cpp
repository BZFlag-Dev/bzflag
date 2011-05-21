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

/* interface header */

#include "ServerIntangibilityManager.h"
#include "ObstacleMgr.h"
#include "bzfsMessages.h"

template <>
ServerIntangibilityManager* Singleton<ServerIntangibilityManager>::_instance =
  (ServerIntangibilityManager*) NULL;


void ServerIntangibilityManager::setWorldObjectTangibility(uint32_t objectGUID,
                                                           unsigned char tangible) {
  tangibilityMap[objectGUID] = tangible;
  sendMsgTangibilityUpdate(objectGUID, tangible);
}


void ServerIntangibilityManager::sendNewPlayerWorldTangibility(int playerID) {
  if (false) {
    // FIXME -- disabled sendNewPlayerWorldTangibility until meshFaces are fixed
    TangibilityMap::iterator itr;
    for (itr = tangibilityMap.begin(); itr != tangibilityMap.end(); ++itr) {
      // send out the tangibility update message
      sendMsgTangibilityUpdate(itr->first, itr->second, playerID);
    }
  }
}


unsigned char ServerIntangibilityManager::getWorldObjectTangibility(uint32_t objectGUID) {
  TangibilityMap::iterator itr = tangibilityMap.find(objectGUID);
  if (itr != tangibilityMap.end()) {
    return itr->second;
  }

  const Obstacle* obs = OBSTACLEMGR.getObstacleFromID(objectGUID);
  if (!obs) {
    return _INVALID_TANGIBILITY;
  }

  return obs->isDriveThrough() ? _INVALID_TANGIBILITY : 0;
}


unsigned char ServerIntangibilityManager::getWorldObjectTangibility(const Obstacle* obs) {
  if (!obs) {
    return _INVALID_TANGIBILITY;
  }

  if (false) {
    // FIXME -- obstacle GUID's are broken, meshFaces are not accounted for
    TangibilityMap::iterator itr = tangibilityMap.find(obs->getGUID());
    if (itr != tangibilityMap.end()) {
      return itr->second;
    }
  }

  return obs->isDriveThrough() ? _INVALID_TANGIBILITY : 0;
}


void ServerIntangibilityManager::resetTangibility(void) {
  tangibilityMap.clear();
  sendMsgTangibilityReset();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
